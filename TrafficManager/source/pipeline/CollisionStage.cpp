#include "CollisionStage.h"

namespace bg = boost::geometry;
namespace cg = carla::geom;

namespace CollisionStageConstants
{
  static const float SEARCH_RADIUS = 20.0;
  static const float VERTICAL_OVERLAP_THRESHOLD = 2.0;
  static const float ZERO_AREA = 0.0001;
  static const float BOUNDARY_EXTENSION_MINIMUM = 2.0;
  static const float EXTENSION_SQUARE_POINT = 7.0;
  static const float TIME_HORIZON = 0.5;
  static const float HIGHWAY_SPEED = 50 / 3.6;
  static const float HIGHWAY_TIME_HORIZON = 5.0;
}

using namespace  CollisionStageConstants;
using Actor = carla::SharedPtr<carla::client::Actor>;

namespace traffic_manager {

  CollisionStage::CollisionStage(

    std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger,
    std::shared_ptr<CollisionToPlannerMessenger> planner_messenger,
    int number_of_vehicle,
    int pool_size,
    carla::client::World &world,
    carla::client::DebugHelper &debug_helper):
    localization_messenger(localization_messenger),
    planner_messenger(planner_messenger),
    world(world),
    debug_helper(debug_helper),
    PipelineStage(pool_size, number_of_vehicle) {
      last_world_actors_pass_instance = std::chrono::system_clock::now();

      frame_selector = true;

      planner_frame_a = std::make_shared<CollisionToPlannerFrame>(number_of_vehicle);
      planner_frame_b = std::make_shared<CollisionToPlannerFrame>(number_of_vehicle);

      localization_messenger_state = localization_messenger->GetState();
      planner_messenger_state = planner_messenger->GetState() - 1;

    }

  CollisionStage::~CollisionStage() {}

  void CollisionStage::Action(const int start_index, const int end_index) {

    auto current_planner_frame = frame_selector? planner_frame_a: planner_frame_b;

    /// Handle vehicles not spawned by TrafficManager
    /// Choosing an arbitrary thread
    if (start_index == 0) {
      auto current_time = std::chrono::system_clock::now();
      std::chrono::duration<double> diff = current_time - last_world_actors_pass_instance;

      /// Periodically check for actors not spawned by TrafficManager
      if (diff.count() > 0.5f) {
        auto world_actors = world.GetActors()->Filter("vehicle.*");
        for (auto actor: *world_actors.get()) {
          auto unregistered_id = actor->GetId();
          if (
            id_to_index.find(unregistered_id) == id_to_index.end() &&
            unregistered_actors.find(unregistered_id) == unregistered_actors.end()) {
            unregistered_actors.insert({unregistered_id, actor});
          }
        }
        last_world_actors_pass_instance = current_time;
      }

      /// Regularly update unregistered actors
      std::vector<uint> actor_ids_to_erase;
      for (auto actor_info: unregistered_actors) {
        if (actor_info.second->IsAlive()) {
          vicinity_grid.UpdateGrid(actor_info.second);
        } else {
          vicinity_grid.EraseActor(actor_info.first);
          actor_ids_to_erase.push_back(actor_info.first);
        }
      }
      for (auto actor_id: actor_ids_to_erase) {
        unregistered_actors.erase(actor_id);
      }
    }

    for (int i = start_index; i <= end_index; ++i) {

      auto &data = localization_frame->at(i);
      auto ego_actor = data.actor;
      auto ego_actor_id = ego_actor->GetId();
      auto actor_id_list = vicinity_grid.GetActors(ego_actor);
      bool collision_hazard = false;

      for (auto i = actor_id_list.begin(); (i != actor_id_list.end()) && !collision_hazard; ++i) {
        auto actor_id = *i;
        try {
          
          if (actor_id != ego_actor_id) {

            Actor actor = nullptr;
            if (id_to_index.find(actor_id) != id_to_index.end()) {
              actor = localization_frame->at(id_to_index.at(actor_id)).actor;
            } else if (unregistered_actors.find(actor_id) != unregistered_actors.end()) {
              actor = unregistered_actors.at(actor_id);
            }

            auto ego_actor_location = ego_actor->GetLocation();
            float actor_distance = actor->GetLocation().Distance(ego_actor_location);
            if (actor_distance <= SEARCH_RADIUS) {
              if (NegotiateCollision(ego_actor, actor)) {
                collision_hazard = true;
              }
            }
          }
        } catch (const std::exception &e) {
          carla::log_warning("Encountered problem while determining collision \n");
          carla::log_info("Actor might not be alive \n");
        }

      }

      auto &message = current_planner_frame->at(i);
      message.hazard = collision_hazard;

    }
  }

  void CollisionStage::DataReceiver() {
    auto packet = localization_messenger->ReceiveData(localization_messenger_state);
    localization_frame = packet.data;
    localization_messenger_state = packet.id;

    int index = 0;
    for (auto &element: *localization_frame.get()) {
      id_to_index.insert(std::pair<uint, int>(element.actor->GetId(), index));
      index++;
    }
  }

  void CollisionStage::DataSender() {
    DataPacket<std::shared_ptr<CollisionToPlannerFrame>> packet{
      planner_messenger_state,
      frame_selector? planner_frame_a: planner_frame_b
    };
    frame_selector = !frame_selector;
    planner_messenger_state = planner_messenger->SendData(packet);
  }

  bool CollisionStage::NegotiateCollision(
      Actor ego_vehicle,
      Actor other_vehicle) const {

    auto reference_heading_vector = ego_vehicle->GetTransform().GetForwardVector();
    auto relative_other_vector = other_vehicle->GetLocation() - ego_vehicle->GetLocation();
    relative_other_vector = relative_other_vector.MakeUnitVector();
    auto reference_relative_dot = cg::Math::Dot(reference_heading_vector, relative_other_vector);

    auto other_heading_vector = other_vehicle->GetTransform().GetForwardVector();
    auto relative_reference_vector = ego_vehicle->GetLocation() - other_vehicle->GetLocation();
    relative_reference_vector = relative_reference_vector.MakeUnitVector();
    auto other_relative_dot = cg::Math::Dot(other_heading_vector, relative_reference_vector);

    return (reference_relative_dot > other_relative_dot &&
           CheckGeodesicCollision(ego_vehicle, other_vehicle));
  }

  bool CollisionStage::CheckGeodesicCollision(
      Actor reference_vehicle,
      Actor other_vehicle) const {
    bool overlap = false;
    auto reference_height = reference_vehicle->GetLocation().z;
    auto other_height = other_vehicle->GetLocation().z;
    if (abs(reference_height - other_height) < VERTICAL_OVERLAP_THRESHOLD) {

      auto reference_geodesic_boundary = GetGeodesicBoundary(reference_vehicle);
      auto other_geodesic_boundary = GetGeodesicBoundary(other_vehicle);
      
      if (reference_geodesic_boundary.size() > 0 && other_geodesic_boundary.size() > 0) {
      
        auto reference_polygon = GetPolygon(reference_geodesic_boundary);
        auto other_polygon = GetPolygon(other_geodesic_boundary);

        std::deque<polygon> output;
        bg::intersection(reference_polygon, other_polygon, output);

        for(polygon const& p: output) {
          if (bg::area(p) > ZERO_AREA) {
            overlap = true;
            break;
          }
        }
      }
    }

    return overlap;
  }

  traffic_manager::polygon
  CollisionStage::GetPolygon(const std::vector<cg::Location> &boundary) const {

    std::string _string;
    for (auto location: boundary) {
      _string += std::to_string(location.x) + " " + std::to_string(location.y) + ",";
    }
    _string += std::to_string(boundary[0].x) + " " + std::to_string(boundary[0].y);

    traffic_manager::polygon boundary_polygon;
    bg::read_wkt("POLYGON((" + _string + "))", boundary_polygon);

    return boundary_polygon;
  }

  std::vector<cg::Location> CollisionStage::GetGeodesicBoundary(Actor actor) const {
  
    auto bbox = GetBoundary(actor);

    if (id_to_index.find(actor->GetId()) != id_to_index.end()) {

      auto velocity = actor->GetVelocity().Length();
      int bbox_extension = static_cast<int>(
        std::max(std::sqrt(EXTENSION_SQUARE_POINT * velocity), BOUNDARY_EXTENSION_MINIMUM) +
        std::max(velocity * TIME_HORIZON, BOUNDARY_EXTENSION_MINIMUM) +
        BOUNDARY_EXTENSION_MINIMUM
        );

      bbox_extension = (velocity > HIGHWAY_SPEED) ? (HIGHWAY_TIME_HORIZON * velocity) : bbox_extension;
      auto &waypoint_buffer =  localization_frame->at(id_to_index.at(actor->GetId())).buffer;

      std::vector<cg::Location> left_boundary;
      std::vector<cg::Location> right_boundary;
      auto vehicle = boost::static_pointer_cast<carla::client::Vehicle>(actor);
      float width = vehicle->GetBoundingBox().extent.y;

      for (int i = 0; (i < bbox_extension) && (i < waypoint_buffer->size()); ++i) {

        auto swp = waypoint_buffer->at(i);
        auto vector = swp->GetVector();
        auto location = swp->GetLocation();
        auto perpendicular_vector = cg::Vector3D(-vector.y, vector.x, 0);
        perpendicular_vector = perpendicular_vector.MakeUnitVector();
        left_boundary.push_back(location + cg::Location(perpendicular_vector * width));
        right_boundary.push_back(location - cg::Location(perpendicular_vector * width));
      }

      /// Connecting geodesic path boundary with vehicle bounding box to create
      /// one polygon
      std::vector<cg::Location> geodesic_boundary;   

      /// Assigning left boundary to geodesic boundary 
      std::reverse(left_boundary.begin(), left_boundary.end());
      geodesic_boundary.insert(geodesic_boundary.end(), left_boundary.begin(), left_boundary.end());
      geodesic_boundary.insert(geodesic_boundary.end(), bbox.begin(), bbox.end());
      geodesic_boundary.insert(geodesic_boundary.end(), right_boundary.begin(), right_boundary.end());
      // Reversing final result to stay consistent with boost polygon convention
      // for positive area
      std::reverse(geodesic_boundary.begin(), geodesic_boundary.end());

      return geodesic_boundary;
    } else {
        std::reverse(bbox.begin(), bbox.end());
        return bbox;  
      }
  }

  std::vector<cg::Location>
  CollisionStage::GetBoundary(Actor actor) const {

    auto vehicle = boost::static_pointer_cast<carla::client::Vehicle>(actor);
    auto bbox = vehicle->GetBoundingBox();
    auto extent = bbox.extent;
    auto location = vehicle->GetLocation();
    auto heading_vector = vehicle->GetTransform().GetForwardVector();
    heading_vector.z = 0;
    auto perpendicular_vector = cg::Vector3D(-heading_vector.y, heading_vector.x, 0);

    // Four corners of the vehicle in boost positve area order
    return {  
             location + cg::Location(heading_vector * extent.x + perpendicular_vector * extent.y),
             location + cg::Location(heading_vector * -extent.x + perpendicular_vector * extent.y),
             location + cg::Location(heading_vector * -extent.x - perpendicular_vector * extent.y),
             location + cg::Location(heading_vector * extent.x - perpendicular_vector * extent.y)
    };
  }

  void CollisionStage::DrawBoundary(const std::vector<cg::Location> &boundary) const {
    for (int i = 0; i < boundary.size(); ++i) {
      debug_helper.DrawLine(
          boundary[i] + cg::Location(0, 0, 1),
          boundary[(i + 1) % boundary.size()] + cg::Location(0, 0, 1),
          0.1f, {255U, 0U, 0U}, 0.1f);
    }
  }
}
