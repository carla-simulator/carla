// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CollisionStage.h"

namespace carla {
namespace traffic_manager {

namespace CollisionStageConstants {

  static const float VERTICAL_OVERLAP_THRESHOLD = 2.0f;
  static const float BOUNDARY_EXTENSION_MINIMUM = 2.0f;
  static const float EXTENSION_SQUARE_POINT = 7.5f;
  static const float TIME_HORIZON = 0.5f;
  static const float HIGHWAY_SPEED = 50.0f / 3.6f;
  static const float HIGHWAY_TIME_HORIZON = 5.0f;
  static const float CRAWL_SPEED = 10.0f / 3.6f;
  static const float BOUNDARY_EDGE_LENGTH = 2.0f;
  static const float MAX_COLLISION_RADIUS = 100.0f;

} // namespace CollisionStageConstants

  using namespace CollisionStageConstants;

  CollisionStage::CollisionStage(
      std::string stage_name,
      std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger,
      std::shared_ptr<CollisionToPlannerMessenger> planner_messenger,
      cc::World &world,
      Parameters &parameters,
      cc::DebugHelper &debug_helper)
    : PipelineStage(stage_name),
      localization_messenger(localization_messenger),
      planner_messenger(planner_messenger),
      world(world),
      parameters(parameters),
      debug_helper(debug_helper){

    // Initializing clock for checking unregistered actors periodically.
    last_world_actors_pass_instance = chr::system_clock::now();
    // Initializing output array selector.
    frame_selector = true;
    // Initializing messenger states.
    localization_messenger_state = localization_messenger->GetState();
    // Initializing this messenger to preemptively write since it precedes
    // motion planner stage.
    planner_messenger_state = planner_messenger->GetState() - 1;
    // Initializing the number of vehicles to zero in the beginning.
    number_of_vehicles = 0u;
  }

  CollisionStage::~CollisionStage() {}

  void CollisionStage::Action() {
    const auto current_planner_frame = frame_selector ? planner_frame_a : planner_frame_b;

    // Handle vehicles not spawned by TrafficManager.
    const auto current_time = chr::system_clock::now();
    const chr::duration<double> diff = current_time - last_world_actors_pass_instance;

    // Periodically check for actors not spawned by TrafficManager.
    if (diff.count() > 1.0f) {

      const auto world_actors = world.GetActors()->Filter("vehicle.*");
      const auto world_walker = world.GetActors()->Filter("walker.*");
      // Scanning for vehicles.
      for (auto actor: *world_actors.get()) {
        const auto unregistered_id = actor->GetId();
        if (vehicle_id_to_index.find(unregistered_id) == vehicle_id_to_index.end() &&
            unregistered_actors.find(unregistered_id) == unregistered_actors.end()) {
          unregistered_actors.insert({unregistered_id, actor});
        }
      }
      // Scanning for pedestrians.
      for (auto walker: *world_walker.get()) {
        const auto unregistered_id = walker->GetId();
        if (unregistered_actors.find(unregistered_id) == unregistered_actors.end()) {
          unregistered_actors.insert({unregistered_id, walker});
        }
      }
      // Regularly update unregistered actors.
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

      last_world_actors_pass_instance = current_time;
    }

    // Looping over registered actors.
    for (uint64_t i = 0u; i < number_of_vehicles; ++i) {

      const LocalizationToCollisionData &data = localization_frame->at(i);
      const Actor ego_actor = data.actor;
      const ActorId ego_actor_id = ego_actor->GetId();

      // Retrieve actors around the path of the ego vehicle.
      std::unordered_set<ActorId> actor_id_list = GetPotentialVehicleObstacles(ego_actor);

      bool collision_hazard = false;

      // Generate number between 0 and 100
      const int r = rand() % 101;

      // Continue only if random number is lower than our %, default is 0.
      if (parameters.GetPercentageIgnoreActors(boost::shared_ptr<cc::Actor>(ego_actor)) <= r) {
      // Check every actor in the vicinity if it poses a collision hazard.
        for (auto j = actor_id_list.begin(); (j != actor_id_list.end()) && !collision_hazard; ++j) {
          const ActorId actor_id = *j;
          try {

            Actor actor = nullptr;
            if (vehicle_id_to_index.find(actor_id) != vehicle_id_to_index.end()) {
              actor = localization_frame->at(vehicle_id_to_index.at(actor_id)).actor;
            } else if (unregistered_actors.find(actor_id) != unregistered_actors.end()) {
              actor = unregistered_actors.at(actor_id);
            }

            const cg::Location ego_location = ego_actor->GetLocation();
            const cg::Location other_location = actor->GetLocation();

            if (actor_id != ego_actor_id &&
                (cg::Math::DistanceSquared(ego_location, other_location)
                < std::pow(MAX_COLLISION_RADIUS, 2)) &&
                (std::abs(ego_location.z - other_location.z) < VERTICAL_OVERLAP_THRESHOLD)) {

              if (parameters.GetCollisionDetection(ego_actor, actor) &&
                  NegotiateCollision(ego_actor, actor)) {

                collision_hazard = true;
              }
            }

          } catch (const std::exception &e) {
            carla::log_warning("Encountered problem while determining collision \n");
            carla::log_info("Actor might not be alive \n");
          }

        }
      }

      CollisionToPlannerData &message = current_planner_frame->at(i);
      message.hazard = collision_hazard;

    }
  }

  void CollisionStage::DataReceiver() {
    const auto packet = localization_messenger->ReceiveData(localization_messenger_state);
    localization_frame = packet.data;
    localization_messenger_state = packet.id;

    if (localization_frame != nullptr) {
      // Connecting actor ids to their position indices on data arrays.
      // This map also provides us the additional benefit of being able to
      // quickly identify
      // if a vehicle id is registered with the traffic manager or not.
      uint64_t index = 0u;
      for (auto &element: *localization_frame.get()) {
        vehicle_id_to_index.insert({element.actor->GetId(), index++});
      }

      // Allocating new containers for the changed number of registered
      // vehicles.
      if (number_of_vehicles != (*localization_frame.get()).size()) {

        number_of_vehicles = static_cast<uint>((*localization_frame.get()).size());
        // Allocating output arrays to be shared with motion planner stage.
        planner_frame_a = std::make_shared<CollisionToPlannerFrame>(number_of_vehicles);
        planner_frame_b = std::make_shared<CollisionToPlannerFrame>(number_of_vehicles);
      }
    }
  }

  void CollisionStage::DataSender() {

    const DataPacket<std::shared_ptr<CollisionToPlannerFrame>> packet{
      planner_messenger_state,
      frame_selector ? planner_frame_a : planner_frame_b
    };
    frame_selector = !frame_selector;
    planner_messenger_state = planner_messenger->SendData(packet);
  }

  bool CollisionStage::NegotiateCollision(const Actor &reference_vehicle, const Actor &other_vehicle) const {

    bool hazard = false;

    auto& data_packet = localization_frame->at(vehicle_id_to_index.at(reference_vehicle->GetId()));
    Buffer& waypoint_buffer = data_packet.buffer;

    auto& other_packet = localization_frame->at(vehicle_id_to_index.at(other_vehicle->GetId()));
    Buffer& other_buffer = other_packet.buffer;

    const cg::Location reference_location = reference_vehicle->GetLocation();
    const cg::Location other_location = other_vehicle->GetLocation();

    const cg::Vector3D reference_heading = reference_vehicle->GetTransform().GetForwardVector();
    cg::Vector3D reference_to_other = other_location - reference_location;
    reference_to_other = reference_to_other.MakeUnitVector();

    const auto reference_vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(reference_vehicle);
    const auto other_vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(other_vehicle);

    if (waypoint_buffer.front()->CheckJunction() &&
        other_buffer.front()->CheckJunction()) {

      const Polygon reference_geodesic_polygon = GetPolygon(GetGeodesicBoundary(reference_vehicle));
      const Polygon other_geodesic_polygon = GetPolygon(GetGeodesicBoundary(other_vehicle));
      const Polygon reference_polygon = GetPolygon(GetBoundary(reference_vehicle));
      const Polygon other_polygon = GetPolygon(GetBoundary(other_vehicle));

      const double reference_vehicle_to_other_geodesic = bg::distance(reference_polygon, other_geodesic_polygon);
      const double other_vehicle_to_reference_geodesic = bg::distance(other_polygon, reference_geodesic_polygon);

      const auto inter_geodesic_distance = bg::distance(reference_geodesic_polygon, other_geodesic_polygon);
      const auto inter_bbox_distance = bg::distance(reference_polygon, other_polygon);

      const cg::Vector3D other_heading = other_vehicle->GetTransform().GetForwardVector();
      cg::Vector3D other_to_reference = reference_vehicle->GetLocation() - other_vehicle->GetLocation();
      other_to_reference = other_to_reference.MakeUnitVector();

      // Whichever vehicle's path is farthest away from the other vehicle gets
      // priority to move.
      if (inter_geodesic_distance < 0.1 &&
          ((
            inter_bbox_distance > 0.1 &&
            reference_vehicle_to_other_geodesic > other_vehicle_to_reference_geodesic
          ) || (
            inter_bbox_distance < 0.1 &&
            cg::Math::Dot(reference_heading, reference_to_other) > cg::Math::Dot(other_heading, other_to_reference)
          )) ) {

        hazard = true;
      }

    } else if (!waypoint_buffer.front()->CheckJunction()) {

      const float reference_vehicle_length = reference_vehicle_ptr->GetBoundingBox().extent.x;
      const float other_vehicle_length = other_vehicle_ptr->GetBoundingBox().extent.x;
      const float vehicle_length_sum = reference_vehicle_length + other_vehicle_length;

      const float bbox_extension_length = GetBoundingBoxExtention(reference_vehicle);

      if ((cg::Math::Dot(reference_heading, reference_to_other) > 0.0f) &&
          (cg::Math::DistanceSquared(reference_location, other_location) <
           std::pow(bbox_extension_length+vehicle_length_sum, 2))) {

        hazard = true;
      }
    }

    return hazard;
  }

  traffic_manager::Polygon CollisionStage::GetPolygon(const LocationList &boundary) const {

    std::string boundary_polygon_wkt;
    for (const cg::Location &location: boundary) {
      boundary_polygon_wkt += std::to_string(location.x) + " " + std::to_string(location.y) + ",";
    }
    boundary_polygon_wkt += std::to_string(boundary[0].x) + " " + std::to_string(boundary[0].y);

    traffic_manager::Polygon boundary_polygon;
    bg::read_wkt("POLYGON((" + boundary_polygon_wkt + "))", boundary_polygon);

    return boundary_polygon;
  }

  LocationList CollisionStage::GetGeodesicBoundary(const Actor &actor) const {

    const LocationList bbox = GetBoundary(actor);

    if (vehicle_id_to_index.find(actor->GetId()) != vehicle_id_to_index.end()) {

      const cg::Location vehicle_location = actor->GetLocation();

      float bbox_extension = GetBoundingBoxExtention(actor);

      const float specific_distance_margin = parameters.GetDistanceToLeadingVehicle(actor);
      if (specific_distance_margin > 0.0f) {
        bbox_extension = std::max(specific_distance_margin, bbox_extension);
      }

      const auto &waypoint_buffer =  localization_frame->at(vehicle_id_to_index.at(actor->GetId())).buffer;

      LocationList left_boundary;
      LocationList right_boundary;
      const auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
      const float width = vehicle->GetBoundingBox().extent.y;
      const float length = vehicle->GetBoundingBox().extent.x;

      SimpleWaypointPtr boundary_start = waypoint_buffer.front();
      uint64_t boundary_start_index = 0u;
      while (boundary_start->DistanceSquared(vehicle_location) < std::pow(length, 2) &&
             boundary_start_index < waypoint_buffer.size() -1) {
        boundary_start = waypoint_buffer.at(boundary_start_index);
        ++boundary_start_index;
      }
      SimpleWaypointPtr boundary_end = nullptr;
      SimpleWaypointPtr current_point = waypoint_buffer.at(boundary_start_index);

      const auto vehicle_reference = boost::static_pointer_cast<cc::Vehicle>(actor);
      // At non-signalized junctions, we extend the boundary across the junction
      // and in all other situations, boundary length is velocity-dependent.
      bool reached_distance = false;
      for (uint64_t j = boundary_start_index; !reached_distance && (j < waypoint_buffer.size()); ++j) {

        if (boundary_start->DistanceSquared(current_point) > std::pow(bbox_extension, 2)) {
          reached_distance = true;
        }

        if (boundary_end == nullptr ||
            boundary_end->DistanceSquared(current_point) > std::pow(BOUNDARY_EDGE_LENGTH, 2) ||
            reached_distance) {

          const cg::Vector3D heading_vector = current_point->GetForwardVector();
          const cg::Location location = current_point->GetLocation();
          cg::Vector3D perpendicular_vector = cg::Vector3D(-heading_vector.y, heading_vector.x, 0.0f);
          perpendicular_vector = perpendicular_vector.MakeUnitVector();
          // Direction determined for the left-handed system.
          const cg::Vector3D scaled_perpendicular = perpendicular_vector * width;
          left_boundary.push_back(location + cg::Location(scaled_perpendicular));
          right_boundary.push_back(location + cg::Location(-1.0f * scaled_perpendicular));

          boundary_end = current_point;
        }

        current_point = waypoint_buffer.at(j);
      }

      // Connecting the geodesic path boundary with the vehicle bounding box.
      LocationList geodesic_boundary;
      // Reversing right boundary to construct clockwise (left-hand system)
      // boundary. This is so because both left and right boundary vectors have
      // the closest point to the vehicle at their starting index for the right
      // boundary,
      // we want to begin at the farthest point to have a clockwise trace.
      std::reverse(right_boundary.begin(), right_boundary.end());
      geodesic_boundary.insert(geodesic_boundary.end(), right_boundary.begin(), right_boundary.end());
      geodesic_boundary.insert(geodesic_boundary.end(), bbox.begin(), bbox.end());
      geodesic_boundary.insert(geodesic_boundary.end(), left_boundary.begin(), left_boundary.end());

      return geodesic_boundary;
    } else {

      return bbox;
    }
  }

  float CollisionStage::GetBoundingBoxExtention(const Actor &actor) const {

    const float velocity = actor->GetVelocity().Length();
    float bbox_extension = BOUNDARY_EXTENSION_MINIMUM;
    if (velocity > HIGHWAY_SPEED) {
      bbox_extension = HIGHWAY_TIME_HORIZON * velocity;
    } else if (velocity < CRAWL_SPEED) {
      bbox_extension = BOUNDARY_EXTENSION_MINIMUM;
    } else {
      bbox_extension = std::sqrt(
                          EXTENSION_SQUARE_POINT * velocity) +
                          velocity * TIME_HORIZON +
                          BOUNDARY_EXTENSION_MINIMUM;
    }

    return bbox_extension;
  }

  std::unordered_set<ActorId> CollisionStage::GetPotentialVehicleObstacles(const Actor &ego_vehicle) {

    vicinity_grid.UpdateGrid(ego_vehicle);

    const auto& data_packet = localization_frame->at(vehicle_id_to_index.at(ego_vehicle->GetId()));
    const Buffer &waypoint_buffer =  data_packet.buffer;
    const float velocity = ego_vehicle->GetVelocity().Length();
    std::unordered_set<ActorId> actor_id_list = data_packet.overlapping_actors;

    if (waypoint_buffer.front()->CheckJunction() && velocity < HIGHWAY_SPEED) {
      actor_id_list = vicinity_grid.GetActors(ego_vehicle);
    } else {
      actor_id_list = data_packet.overlapping_actors;
    }

    return actor_id_list;
  }

  LocationList CollisionStage::GetBoundary(const Actor &actor) const {
    const auto actor_type = actor->GetTypeId();

    cg::BoundingBox bbox;
    cg::Location location;
    cg::Vector3D heading_vector;

    if (actor_type[0] == 'v') {

      const auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
      bbox = vehicle->GetBoundingBox();
      location = vehicle->GetLocation();
      heading_vector = vehicle->GetTransform().GetForwardVector();
    } else if (actor_type[0] == 'w') {

      const auto walker = boost::static_pointer_cast<cc::Walker>(actor);
      bbox = walker->GetBoundingBox();
      location = walker->GetLocation();
      heading_vector = walker->GetTransform().GetForwardVector();
    }

    const cg::Vector3D extent = bbox.extent;
    heading_vector.z = 0.0f;
    const cg::Vector3D perpendicular_vector = cg::Vector3D(-heading_vector.y, heading_vector.x, 0.0f);

    // Four corners of the vehicle in top view clockwise order (left-handed
    // system).
    const cg::Vector3D x_boundary_vector = heading_vector * extent.x;
    const cg::Vector3D y_boundary_vector = perpendicular_vector * extent.y;
    return {
        location + cg::Location(x_boundary_vector - y_boundary_vector),
        location + cg::Location(-1.0f * x_boundary_vector - y_boundary_vector),
        location + cg::Location(-1.0f * x_boundary_vector + y_boundary_vector),
        location + cg::Location(x_boundary_vector + y_boundary_vector),
    };
  }

  void CollisionStage::DrawBoundary(const LocationList &boundary) const {
    for (uint64_t i = 0u; i < boundary.size(); ++i) {
      debug_helper.DrawLine(
          boundary[i] + cg::Location(0.0f, 0.0f, 1.0f),
          boundary[(i + 1) % boundary.size()] + cg::Location(0.0f, 0.0f, 1.0f),
          0.1f, {255u, 0u, 0u}, 0.1f);
    }
  }

} // namespace traffic_manager
} // namespace carla
