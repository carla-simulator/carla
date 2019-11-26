#include "CollisionStage.h"

namespace traffic_manager {

namespace CollisionStageConstants {
  static const float VERTICAL_OVERLAP_THRESHOLD = 2.0f;
  static const float ZERO_AREA = 0.0001f;
  static const float BOUNDARY_EXTENSION_MINIMUM = 2.0f;
  static const float EXTENSION_SQUARE_POINT = 7.5f;
  static const float TIME_HORIZON = 0.5f;
  static const float HIGHWAY_SPEED = 50.0f / 3.6f;
  static const float HIGHWAY_TIME_HORIZON = 5.0f;
  static const float CRAWL_SPEED = 10.0f / 3.6f;
  static const float BOUNDARY_EDGE_LENGTH = 2.0f;
  static const float MINIMUM_GRID_EXTENSION = 10.0f;
  static const float MAX_COLLISION_RADIUS = 100.0f;
}
  using namespace  CollisionStageConstants;

  CollisionStage::CollisionStage(
      std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger,
      std::shared_ptr<CollisionToPlannerMessenger> planner_messenger,
      cc::World &world,
      Parameters &parameters,
      cc::DebugHelper &debug_helper)
    : localization_messenger(localization_messenger),
      planner_messenger(planner_messenger),
      world(world),
      parameters(parameters),
      debug_helper(debug_helper) {

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
    auto current_planner_frame = frame_selector ? planner_frame_a : planner_frame_b;

    // Handle vehicles not spawned by TrafficManager.
    auto current_time = chr::system_clock::now();
    chr::duration<double> diff = current_time - last_world_actors_pass_instance;

    // Periodically check for actors not spawned by TrafficManager.
    if (diff.count() > 1.0f) {

      auto world_actors = world.GetActors()->Filter("vehicle.*");
      auto world_walker = world.GetActors()->Filter("walker.*");
      // Scanning for vehicles.
      for (auto actor: *world_actors.get()) {
        auto unregistered_id = actor->GetId();
        if (vehicle_id_to_index.find(unregistered_id) == vehicle_id_to_index.end() &&
            unregistered_actors.find(unregistered_id) == unregistered_actors.end()) {
          unregistered_actors.insert({unregistered_id, actor});
        }
      }
      // Scanning for pedestrians.
      for (auto walker: *world_walker.get()) {
        auto unregistered_id = walker->GetId();
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
    for (uint i = 0u; i < number_of_vehicles; ++i) {

      LocalizationToCollisionData &data = localization_frame->at(i);
      Actor ego_actor = data.actor;
      ActorId ego_actor_id = ego_actor->GetId();

      // DrawBoundary(GetGeodesicBoundary(ego_actor));

      // Retrieve actors around the path of the ego vehicle.
      std::unordered_set<ActorId> actor_id_list = GetPotentialVehicleObstacles(ego_actor);

      bool collision_hazard = false;
      // Check every actor in the vicinity if it poses a collision hazard.
      for (auto j = actor_id_list.begin(); (j != actor_id_list.end()) && !collision_hazard; ++j) {
        ActorId actor_id = *j;
        try {

          Actor actor = nullptr;
          if (vehicle_id_to_index.find(actor_id) != vehicle_id_to_index.end()) {
            actor = localization_frame->at(vehicle_id_to_index.at(actor_id)).actor;
          } else if (unregistered_actors.find(actor_id) != unregistered_actors.end()) {
            actor = unregistered_actors.at(actor_id);
          }

          cg::Location ego_location = ego_actor->GetLocation();
          cg::Location other_location = actor->GetLocation();
          if (actor_id != ego_actor_id &&
              (cg::Math::DistanceSquared(ego_location, other_location)
              < std::pow(MAX_COLLISION_RADIUS, 2)) &&
              (std::abs(ego_location.z - other_location.z) < VERTICAL_OVERLAP_THRESHOLD)) {

            debug_helper.DrawLine(
              ego_actor->GetLocation() + cg::Location(0, 0, 2),
              actor->GetLocation() + cg::Location(0, 0, 2), 0.2,
              {255u, 0u, 0u}, 0.1
            );

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

      CollisionToPlannerData &message = current_planner_frame->at(i);
      message.hazard = collision_hazard;

    }
  }

  void CollisionStage::DataReceiver() {
    auto packet = localization_messenger->ReceiveData(localization_messenger_state);
    localization_frame = packet.data;
    localization_messenger_state = packet.id;

    if (localization_frame != nullptr) {
      // Connecting actor ids to their position indices on data arrays.
      // This map also provides us the additional benefit of being able to
      // quickly identify
      // if a vehicle id is registered with the traffic manager or not.
      uint index = 0u;
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

    DataPacket<std::shared_ptr<CollisionToPlannerFrame>> packet{
      planner_messenger_state,
      frame_selector ? planner_frame_a : planner_frame_b
    };
    frame_selector = !frame_selector;
    planner_messenger_state = planner_messenger->SendData(packet);
  }

  bool CollisionStage::NegotiateCollision(const Actor &reference_vehicle, const Actor &other_vehicle) const {

    bool hazard = false;
    bool geodesic_overlap = false;


    LocationList reference_geodesic_bbox = GetGeodesicBoundary(reference_vehicle);
    LocationList other_geodesic_bbox = GetGeodesicBoundary(other_vehicle);

    geodesic_overlap = CheckOverlap(reference_geodesic_bbox, other_geodesic_bbox);

    Polygon reference_geodesic_polygon = GetPolygon(reference_geodesic_bbox);
    Polygon other_geodesic_polygon = GetPolygon(other_geodesic_bbox);
    Polygon reference_polygon = GetPolygon(GetBoundary(reference_vehicle));
    Polygon other_polygon = GetPolygon(GetBoundary(other_vehicle));

    double reference_vehicle_to_other_geodesic = bg::distance(reference_polygon, other_geodesic_polygon);
    double other_vehicle_to_reference_geodesic = bg::distance(other_polygon, reference_geodesic_polygon);

    // Whichever vehicle's path is farthest away from the other vehicle gets
    // priority to move.
    if (geodesic_overlap &&
        reference_vehicle_to_other_geodesic > other_vehicle_to_reference_geodesic) {
      hazard = true;
    }

    return hazard;
  }

  bool CollisionStage::CheckOverlap(
      const LocationList &boundary_a,
      const LocationList &boundary_b) const {

    bool overlap = false;
    if (boundary_a.size() > 0 && boundary_b.size() > 0) {

      Polygon reference_polygon = GetPolygon(boundary_a);
      Polygon other_polygon = GetPolygon(boundary_b);

      std::deque<Polygon> output;
      bg::intersection(reference_polygon, other_polygon, output);

      for (uint j = 0u; j < output.size() && !overlap; ++j) {
        Polygon &p = output.at(j);
        if (bg::area(p) > ZERO_AREA) {
          overlap = true;
        }
      }
    }

    return overlap;
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

    LocationList bbox = GetBoundary(actor);

    if (vehicle_id_to_index.find(actor->GetId()) != vehicle_id_to_index.end()) {

      cg::Location vehicle_location = actor->GetLocation();

      float bbox_extension = GetBoundingBoxExtention(actor);

      float specific_distance_margin = parameters.GetDistanceToLeadingVehicle(actor);
      if (specific_distance_margin > 0) {
        bbox_extension = std::max(specific_distance_margin, bbox_extension);
      }

      auto &waypoint_buffer =  localization_frame->at(vehicle_id_to_index.at(actor->GetId())).buffer;

      LocationList left_boundary;
      LocationList right_boundary;
      auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
      float width = vehicle->GetBoundingBox().extent.y;
      float length = vehicle->GetBoundingBox().extent.x;

      SimpleWaypointPtr boundary_start = waypoint_buffer.front();
      uint boundary_start_index = 0u;
      while (boundary_start->DistanceSquared(vehicle_location) < std::pow(length, 2) &&
             boundary_start_index < waypoint_buffer.size() -1) {
        boundary_start = waypoint_buffer.at(boundary_start_index);
        ++boundary_start_index;
      }
      SimpleWaypointPtr boundary_end = nullptr;
      SimpleWaypointPtr current_point = waypoint_buffer.at(boundary_start_index);

      auto vehicle_reference = boost::static_pointer_cast<cc::Vehicle>(actor);
      // At non-signalized junctions, we extend the boundary across the junction
      // and in all other situations, boundary length is velocity-dependent.
      bool reached_distance = false;
      for (uint j = boundary_start_index; !reached_distance && (j < waypoint_buffer.size()); ++j) {

        if (boundary_start->DistanceSquared(current_point) > std::pow(bbox_extension, 2)) {
          reached_distance = true;
        }

        if (boundary_end == nullptr ||
            boundary_end->DistanceSquared(current_point) > std::pow(BOUNDARY_EDGE_LENGTH, 2) ||
            reached_distance) {

          cg::Vector3D heading_vector = current_point->GetForwardVector();
          cg::Location location = current_point->GetLocation();
          cg::Vector3D perpendicular_vector = cg::Vector3D(-heading_vector.y, heading_vector.x, 0);
          perpendicular_vector = perpendicular_vector.MakeUnitVector();
          // Direction determined for the left-handed system.
          cg::Vector3D scaled_perpendicular = perpendicular_vector * width;
          left_boundary.push_back(location + cg::Location(scaled_perpendicular));
          right_boundary.push_back(location + cg::Location(-1 * scaled_perpendicular));

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

    float velocity = actor->GetVelocity().Length();
    float bbox_extension = BOUNDARY_EXTENSION_MINIMUM;
    if (velocity > HIGHWAY_SPEED) {
      bbox_extension = HIGHWAY_TIME_HORIZON * velocity;
    } else if (velocity < CRAWL_SPEED) {
      bbox_extension = BOUNDARY_EXTENSION_MINIMUM;
    } else {
      bbox_extension = std::sqrt(EXTENSION_SQUARE_POINT * velocity) +
                        velocity * TIME_HORIZON +
                        BOUNDARY_EXTENSION_MINIMUM;
    }

    return bbox_extension;
  }

  std::unordered_set<ActorId> CollisionStage::GetPotentialVehicleObstacles(const Actor &ego_vehicle) {

    vicinity_grid.UpdateGrid(ego_vehicle);
    float grid_extension = std::max(GetBoundingBoxExtention(ego_vehicle), MINIMUM_GRID_EXTENSION);
    std::unordered_set<ActorId> actor_id_list;

    Buffer &waypoint_buffer =  localization_frame->at(
      vehicle_id_to_index.at(ego_vehicle->GetId())).buffer;

    actor_id_list = vicinity_grid.GetActors(
      ego_vehicle, waypoint_buffer, grid_extension);

    return actor_id_list;
  }

  LocationList CollisionStage::GetBoundary(const Actor &actor) const {
    auto actor_type = actor->GetTypeId();

    cg::BoundingBox bbox;
    cg::Location location;
    cg::Vector3D heading_vector;

    if (actor_type[0] == 'v') {

      auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
      bbox = vehicle->GetBoundingBox();
      location = vehicle->GetLocation();
      heading_vector = vehicle->GetTransform().GetForwardVector();
    } else if (actor_type[0] == 'w') {

      auto walker = boost::static_pointer_cast<cc::Walker>(actor);
      bbox = walker->GetBoundingBox();
      location = walker->GetLocation();
      heading_vector = walker->GetTransform().GetForwardVector();
    }

    cg::Vector3D extent = bbox.extent;
    heading_vector.z = 0;
    cg::Vector3D perpendicular_vector = cg::Vector3D(-heading_vector.y, heading_vector.x, 0);

    // Four corners of the vehicle in top view clockwise order (left-handed
    // system).
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
    for (uint i = 0u; i < boundary.size(); ++i) {
      debug_helper.DrawLine(
          boundary[i] + cg::Location(0, 0, 1),
          boundary[(i + 1) % boundary.size()] + cg::Location(0, 0, 1),
          0.1f, {255u, 0u, 0u}, 0.1f);
    }
  }
}
