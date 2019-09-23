#include "CollisionStage.h"

namespace traffic_manager {

namespace CollisionStageConstants {
  static const float SEARCH_RADIUS = 20.0f;
  static const float VERTICAL_OVERLAP_THRESHOLD = 2.0f;
  static const float ZERO_AREA = 0.0001f;
  static const float BOUNDARY_EXTENSION_MINIMUM = 2.0f;
  static const float EXTENSION_SQUARE_POINT = 7.0f;
  static const float TIME_HORIZON = 0.5f;
  static const float HIGHWAY_SPEED = 50 / 3.6f;
  static const float HIGHWAY_TIME_HORIZON = 5.0f;
}
  using namespace  CollisionStageConstants;

  CollisionStage::CollisionStage(

      std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger,
      std::shared_ptr<CollisionToPlannerMessenger> planner_messenger,
      uint number_of_vehicle,
      uint pool_size,
      cc::World &world,
      cc::DebugHelper &debug_helper)
    : localization_messenger(localization_messenger),
      planner_messenger(planner_messenger),
      world(world),
      debug_helper(debug_helper),
      PipelineStage(pool_size, number_of_vehicle) {

    // Initializing clock for checking unregistered actors periodically
    last_world_actors_pass_instance = chr::system_clock::now();
    // Initializing output array selector
    frame_selector = true;
    // Allocating output arrays to be shared with motion planner stage
    planner_frame_a = std::make_shared<CollisionToPlannerFrame>(number_of_vehicle);
    planner_frame_b = std::make_shared<CollisionToPlannerFrame>(number_of_vehicle);
    // Initializing messenger states
    localization_messenger_state = localization_messenger->GetState();
    // Initialize this messenger to preemptively write since it precedes
    // motion planner stage
    planner_messenger_state = planner_messenger->GetState() - 1;

  }

  CollisionStage::~CollisionStage() {}

  void CollisionStage::Action(const uint start_index, const uint end_index) {

    auto current_planner_frame = frame_selector ? planner_frame_a : planner_frame_b;

    // Handle vehicles not spawned by TrafficManager
    // Choosing an arbitrary thread
    if (start_index == 0u) {
      auto current_time = chr::system_clock::now();
      chr::duration<double> diff = current_time - last_world_actors_pass_instance;

      // Periodically check for actors not spawned by TrafficManager
      if (diff.count() > 0.5f) {
        auto world_actors = world.GetActors()->Filter("vehicle.*");
        for (auto actor: *world_actors.get()) {
          auto unregistered_id = actor->GetId();
          if (id_to_index.find(unregistered_id) == id_to_index.end() &&
              unregistered_actors.find(unregistered_id) == unregistered_actors.end()) {
            unregistered_actors.insert({unregistered_id, actor});
          }
        }
        last_world_actors_pass_instance = current_time;
      }

      // Regularly update unregistered actors
      std::vector<ActorId> actor_ids_to_erase;
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

    // Looping over arrays' partitions for current thread
    for (auto i = start_index; i <= end_index; ++i) {

      auto &data = localization_frame->at(i);
      Actor ego_actor = data.actor;
      auto ego_actor_id = ego_actor->GetId();

      // Retreive actors around ego actor
      auto actor_id_list = vicinity_grid.GetActors(ego_actor);
      bool collision_hazard = false;

      // Check every actor in vicinity if it poses a collision hazard
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
            float squared_distance = cg::Math::DistanceSquared(ego_actor_location, actor->GetLocation());
            if (squared_distance <= SEARCH_RADIUS * SEARCH_RADIUS) {
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

    // Connecting actor ids to their position indices on data arrays.
    // This map also provides us the additional benefit of being able to
    // quickly identify if a vehicle id is registered with traffic manager or
    // not.
    auto index = 0u;
    for (auto &element: *localization_frame.get()) {
      id_to_index.insert({element.actor->GetId(), index++});
    }
  }

  void CollisionStage::DataSender() {
    DataPacket<std::shared_ptr<CollisionToPlannerFrame>> packet{
      planner_messenger_state,
      frame_selector ? planner_frame_a : planner_frame_b
    };
    frame_selector = !frame_selector;
    planner_messenger_state = planner_messenger->SendData(packet);
  }

  bool CollisionStage::NegotiateCollision(const Actor &ego_vehicle, const Actor &other_vehicle) const {

    // For each vehicle, calculating the dot product between heading vector
    // and relative position vector to the other vehicle.

    auto other_vehicle_location = other_vehicle->GetLocation();
    auto ego_vehicle_location = ego_vehicle->GetLocation();

    auto reference_heading_vector = ego_vehicle->GetTransform().GetForwardVector();
    cg::Vector3D relative_other_vector = other_vehicle_location - ego_vehicle_location;
    relative_other_vector = relative_other_vector.MakeUnitVector();
    float reference_relative_dot = cg::Math::Dot(reference_heading_vector, relative_other_vector);

    auto other_heading_vector = other_vehicle->GetTransform().GetForwardVector();
    cg::Vector3D relative_reference_vector = ego_vehicle_location - other_vehicle_location;
    relative_reference_vector = relative_reference_vector.MakeUnitVector();
    float other_relative_dot = cg::Math::Dot(other_heading_vector, relative_reference_vector);

    // Give preference to vehicle who's path has higher angular separation
    // with relative position vector to the other vehicle.
    return (reference_relative_dot > other_relative_dot &&
           CheckGeodesicCollision(ego_vehicle, other_vehicle));
  }

  bool CollisionStage::CheckGeodesicCollision(
      const Actor &reference_vehicle,
      const Actor &other_vehicle) const {

    bool overlap = false;
    float reference_height = reference_vehicle->GetLocation().z;
    float other_height = other_vehicle->GetLocation().z;
    if (abs(reference_height - other_height) < VERTICAL_OVERLAP_THRESHOLD) {

      LocationList reference_geodesic_boundary = GetGeodesicBoundary(reference_vehicle);
      LocationList other_geodesic_boundary = GetGeodesicBoundary(other_vehicle);

      if (reference_geodesic_boundary.size() > 0 && other_geodesic_boundary.size() > 0) {

        Polygon reference_polygon = GetPolygon(reference_geodesic_boundary);
        Polygon other_polygon = GetPolygon(other_geodesic_boundary);

        std::deque<Polygon> output;
        bg::intersection(reference_polygon, other_polygon, output);

        for (auto i = 0u; i < output.size() && !overlap; ++i) {
          auto &p = output.at(i);
          if (bg::area(p) > ZERO_AREA) {
            overlap = true;
          }
        }
      }
    }

    return overlap;
  }

  traffic_manager::Polygon CollisionStage::GetPolygon(const LocationList &boundary) const {

    std::string boundary_polygon_wkt;
    for (auto location: boundary) {
      boundary_polygon_wkt += std::to_string(location.x) + " " + std::to_string(location.y) + ",";
    }
    boundary_polygon_wkt += std::to_string(boundary[0].x) + " " + std::to_string(boundary[0].y);

    traffic_manager::Polygon boundary_polygon;
    bg::read_wkt("POLYGON((" + boundary_polygon_wkt + "))", boundary_polygon);

    return boundary_polygon;
  }

  LocationList CollisionStage::GetGeodesicBoundary(const Actor &actor) const {

    LocationList bbox = GetBoundary(actor);

    if (id_to_index.find(actor->GetId()) != id_to_index.end()) {

      float velocity = actor->GetVelocity().Length();
      uint bbox_extension = static_cast<uint>(
        std::max(std::sqrt(EXTENSION_SQUARE_POINT * velocity), BOUNDARY_EXTENSION_MINIMUM) +
        std::max(velocity * TIME_HORIZON, BOUNDARY_EXTENSION_MINIMUM) +
        BOUNDARY_EXTENSION_MINIMUM
        );

      bbox_extension = (velocity > HIGHWAY_SPEED) ? (HIGHWAY_TIME_HORIZON * velocity) : bbox_extension;
      auto &waypoint_buffer =  localization_frame->at(id_to_index.at(actor->GetId())).buffer;

      LocationList left_boundary;
      LocationList right_boundary;
      auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
      float width = vehicle->GetBoundingBox().extent.y;

      for (auto i = 0u; (i < bbox_extension) && (i < waypoint_buffer->size()); ++i) {

        std::shared_ptr<SimpleWaypoint> swp = waypoint_buffer->at(i);
        cg::Vector3D heading_vector = swp->GetForwardVector();
        cg::Location location = swp->GetLocation();
        auto perpendicular_vector = cg::Vector3D(-heading_vector.y, heading_vector.x, 0);
        perpendicular_vector = perpendicular_vector.MakeUnitVector();
        // Direction determined for left handed system
        cg::Vector3D scaled_perpendicular = perpendicular_vector * width;
        left_boundary.push_back(location + cg::Location(scaled_perpendicular));
        right_boundary.push_back(location + cg::Location(-1 * scaled_perpendicular));
      }

      // Connecting geodesic path boundary with vehicle bounding box
      LocationList geodesic_boundary;
      // Reversing right boundary to construct clocwise (left hand system)
      // boundary
      // This is so because both left and right boundary vectors have the
      // closest
      // point to the vehicle at their starting index
      // For right boundary we want to begin at the farthest point to have a
      // clocwise trace
      std::reverse(right_boundary.begin(), right_boundary.end());
      geodesic_boundary.insert(geodesic_boundary.end(), right_boundary.begin(), right_boundary.end());
      geodesic_boundary.insert(geodesic_boundary.end(), bbox.begin(), bbox.end());
      geodesic_boundary.insert(geodesic_boundary.end(), left_boundary.begin(), left_boundary.end());

      return geodesic_boundary;
    } else {

      return bbox;
    }
  }

  LocationList CollisionStage::GetBoundary(const Actor &actor) const {

    auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
    cg::BoundingBox bbox = vehicle->GetBoundingBox();
    cg::Vector3D extent = bbox.extent;
    auto location = vehicle->GetLocation();
    auto heading_vector = vehicle->GetTransform().GetForwardVector();
    heading_vector.z = 0;
    auto perpendicular_vector = cg::Vector3D(-heading_vector.y, heading_vector.x, 0);

    // Four corners of the vehicle in top view clockwise order (left handed
    // system)
    cg::Vector3D x_boundary_vector = heading_vector * extent.x;
    cg::Vector3D y_boundary_vector = perpendicular_vector * extent.y;
    return {
             location + cg::Location(x_boundary_vector - y_boundary_vector),
             location + cg::Location(-1 * x_boundary_vector - y_boundary_vector),
             location + cg::Location(-1 * x_boundary_vector + y_boundary_vector),
             location + cg::Location(x_boundary_vector + y_boundary_vector),
    };
  }

  void CollisionStage::DrawBoundary(const LocationList &boundary) const {
    for (auto i = 0u; i < boundary.size(); ++i) {
      debug_helper.DrawLine(
          boundary[i] + cg::Location(0, 0, 1),
          boundary[(i + 1) % boundary.size()] + cg::Location(0, 0, 1),
          0.1f, {255u, 0u, 0u}, 0.1f);
    }
  }
}
