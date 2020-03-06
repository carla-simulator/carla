// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CollisionStage.h"

namespace carla {
namespace traffic_manager {

namespace CollisionStageConstants {

  static const float VERTICAL_OVERLAP_THRESHOLD = 2.0f;
  static const float BOUNDARY_EXTENSION_MINIMUM = 1.0f;
  static const float EXTENSION_SQUARE_POINT = 7.5f;
  static const float TIME_HORIZON = 0.5f;
  static const float HIGHWAY_SPEED = 50.0f / 3.6f;
  static const float HIGHWAY_TIME_HORIZON = 5.0f;
  static const float CRAWL_SPEED = 10.0f / 3.6f;
  static const float BOUNDARY_EDGE_LENGTH = 2.0f;
  static const float MAX_COLLISION_RADIUS = 100.0f;
  static const float MIN_COLLISION_RADIUS = 15.0f;
  static const float WALKER_TIME_EXTENSION = 1.5f;
  static const float EPSILON_VELOCITY = 0.1f;
  static const float INTER_BBOX_DISTANCE_THRESHOLD = 0.3f;
  static const float BBOX_EXTENT_MULTIPLIER = 1.4f;
} // namespace CollisionStageConstants

  using namespace CollisionStageConstants;

  CollisionStage::CollisionStage(
      std::string stage_name,
      std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger,
      std::shared_ptr<CollisionToPlannerMessenger> planner_messenger,
      Parameters &parameters,
      cc::DebugHelper &debug_helper)
    : PipelineStage(stage_name),
      localization_messenger(localization_messenger),
      planner_messenger(planner_messenger),
      parameters(parameters),
      debug_helper(debug_helper) {

    // Initializing clock for checking unregistered actors periodically.
    last_world_actors_pass_instance = chr::system_clock::now();
    // Initializing output array selector.
    frame_selector = true;
    // Initializing the number of vehicles to zero in the beginning.
    number_of_vehicles = 0u;
    // Initializing srand.
    srand(static_cast<unsigned>(time(NULL)));
  }

  CollisionStage::~CollisionStage() {}

  void CollisionStage::Action() {

    const auto current_planner_frame = frame_selector ? planner_frame_a : planner_frame_b;

    // Looping over registered actors.
    for (uint64_t i = 0u; i < number_of_vehicles && localization_frame != nullptr; ++i) {

      LocalizationToCollisionData &data = localization_frame->at(i);
      if (!data.actor->IsAlive()) {
        continue;
      }

      const Actor ego_actor = data.actor;
      const ActorId ego_actor_id = ego_actor->GetId();
      const std::unordered_map<ActorId, Actor> overlapping_actors = data.overlapping_actors;
      const cg::Location ego_location = ego_actor->GetLocation();
      const SimpleWaypointPtr& closest_point = data.closest_waypoint;
      const SimpleWaypointPtr& junction_look_ahead = data.junction_look_ahead_waypoint;

      // Retrieve actors around the path of the ego vehicle.
      bool collision_hazard = false;
      const SimpleWaypointPtr safe_point_junction = localization_frame->at(vehicle_id_to_index.at(ego_actor->GetId())).safe_point_after_junction;

      // Check every actor in the vicinity if it poses a collision hazard.
      for (auto j = overlapping_actors.begin(); (j != overlapping_actors.end()) && !collision_hazard; ++j) {

        try {

          const Actor other_actor = j->second;
          const auto other_actor_type = other_actor->GetTypeId();
          const ActorId other_actor_id = j->first;
          const cg::Location other_location = other_actor->GetLocation();

          // Collision checks increase with speed
          float collision_distance = std::pow(floor(ego_actor->GetVelocity().Length()*3.6f/10.0f),2.0f);
          collision_distance = cg::Math::Clamp(collision_distance, MIN_COLLISION_RADIUS, MAX_COLLISION_RADIUS);

          // Temporary fix to (0,0,0) bug
          if (!(other_location.x == 0 && other_location.y == 0 && other_location.z == 0)) {

            if (other_actor_id != ego_actor_id &&
                (cg::Math::DistanceSquared(ego_location, other_location)
                < std::pow(MAX_COLLISION_RADIUS, 2)) &&
                (std::abs(ego_location.z - other_location.z) < VERTICAL_OVERLAP_THRESHOLD)) {

              if (parameters.GetCollisionDetection(ego_actor, other_actor)) {

                if((safe_point_junction != nullptr && !IsLocationAfterJunctionSafe(ego_actor, other_actor, safe_point_junction, other_location)) ||
                  NegotiateCollision(ego_actor, other_actor, ego_location, other_location, closest_point, junction_look_ahead)) {

                  if ((other_actor_type[0] == 'v' && parameters.GetPercentageIgnoreVehicles(ego_actor) <= (rand() % 101)) ||
                      (other_actor_type[0] == 'w' && parameters.GetPercentageIgnoreWalkers(ego_actor) <= (rand() % 101))) {

                    collision_hazard = true;
                  }
                }
              }
            }
          }
        } catch (const std::exception &e) {
          carla::log_info("Actor might not be alive \n");
        }
      }

      CollisionToPlannerData &message = current_planner_frame->at(i);
      message.hazard = collision_hazard;
    }
  }

  void CollisionStage::DataReceiver() {

    localization_frame = localization_messenger->Peek();

    if (localization_frame != nullptr) {

      // Connecting actor ids to their position indices on data arrays.
      // This map also provides us the additional benefit of being
      // able to quickly identify if a vehicle id is registered
      // with the traffic manager or not.
      vehicle_id_to_index.clear();
      uint64_t index = 0u;
      for (auto &element: *localization_frame.get()) {
        vehicle_id_to_index.insert({element.actor->GetId(), index++});
      }

      // Allocating new containers for the changed number
      // of registered vehicles.
      if (number_of_vehicles != (*localization_frame.get()).size()) {

        number_of_vehicles = static_cast<uint64_t>((*localization_frame.get()).size());

        // Allocating output arrays to be shared with motion planner stage.
        planner_frame_a = std::make_shared<CollisionToPlannerFrame>(number_of_vehicles);
        planner_frame_b = std::make_shared<CollisionToPlannerFrame>(number_of_vehicles);
      }
    }

    // Cleaning geodesic boundaries from the last iteration.
    geodesic_boundaries.clear();
  }

  void CollisionStage::DataSender() {

    localization_messenger->Pop();

    planner_messenger->Push(frame_selector ? planner_frame_a : planner_frame_b);
    frame_selector = !frame_selector;
  }

  bool CollisionStage::NegotiateCollision(const Actor &reference_vehicle, const Actor &other_vehicle,
                                          const cg::Location &reference_location, const cg::Location &other_location,
                                          const SimpleWaypointPtr& closest_point,
                                          const SimpleWaypointPtr& junction_look_ahead) {

    bool hazard = false;

    const cg::Vector3D reference_heading = reference_vehicle->GetTransform().GetForwardVector();
    cg::Vector3D reference_to_other = other_location - reference_location;
    reference_to_other = reference_to_other.MakeUnitVector();

    const cg::Vector3D other_heading = other_vehicle->GetTransform().GetForwardVector();
    cg::Vector3D other_to_reference = reference_location - other_location;
    other_to_reference = other_to_reference.MakeUnitVector();

    const auto &waypoint_buffer =  localization_frame->at(
      vehicle_id_to_index.at(reference_vehicle->GetId())).buffer;
    const SimpleWaypointPtr& reference_front_wp = waypoint_buffer.front();

    const auto reference_vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(reference_vehicle);
    const auto other_vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(other_vehicle);

    float reference_vehicle_length = reference_vehicle_ptr->GetBoundingBox().extent.x * BBOX_EXTENT_MULTIPLIER;
    float other_vehicle_length = other_vehicle_ptr->GetBoundingBox().extent.x * BBOX_EXTENT_MULTIPLIER;
    float inter_vehicle_length = reference_vehicle_length + other_vehicle_length;

    float inter_vehicle_distance = cg::Math::DistanceSquared(reference_location, other_location);
    float minimum_inter_vehicle_distance = std::pow(GetBoundingBoxExtention(reference_vehicle) + inter_vehicle_length, 2.0f);

    if (!(!reference_front_wp->CheckJunction() &&
        cg::Math::Dot(reference_heading, reference_to_other) < 0 &&
        inter_vehicle_distance > minimum_inter_vehicle_distance) &&

        !(!closest_point->CheckJunction() && junction_look_ahead->CheckJunction() &&
        reference_vehicle_ptr->GetVelocity().SquaredLength() < 0.1 &&
        reference_vehicle_ptr->GetTrafficLightState() != carla::rpc::TrafficLightState::Green &&
        inter_vehicle_distance > minimum_inter_vehicle_distance) &&

        !(!reference_front_wp->CheckJunction() &&
        cg::Math::Dot(reference_heading, reference_to_other) > 0 &&
        inter_vehicle_distance > std::max(minimum_inter_vehicle_distance,
                                          std::pow(parameters.GetDistanceToLeadingVehicle(reference_vehicle)
                                                   + inter_vehicle_length, 2.0f)))) {

      const Polygon reference_geodesic_polygon = GetPolygon(GetGeodesicBoundary(reference_vehicle, reference_location));
      const Polygon other_geodesic_polygon = GetPolygon(GetGeodesicBoundary(other_vehicle, other_location));
      const Polygon reference_polygon = GetPolygon(GetBoundary(reference_vehicle, reference_location));
      const Polygon other_polygon = GetPolygon(GetBoundary(other_vehicle, other_location));

      const double reference_vehicle_to_other_geodesic = bg::distance(reference_polygon, other_geodesic_polygon);
      const double other_vehicle_to_reference_geodesic = bg::distance(other_polygon, reference_geodesic_polygon);

      const auto inter_geodesic_distance = bg::distance(reference_geodesic_polygon, other_geodesic_polygon);
      const auto inter_bbox_distance = bg::distance(reference_polygon, other_polygon);

      // Whichever vehicle's path is farthest away from the other vehicle gets
      // priority to move.
      if (inter_geodesic_distance < 0.1 &&
          ((inter_bbox_distance > 0.1 &&
            reference_vehicle_to_other_geodesic > other_vehicle_to_reference_geodesic
            ) || (
            inter_bbox_distance < 0.1 &&
            (cg::Math::Dot(reference_heading, reference_to_other) >
            cg::Math::Dot(other_heading, other_to_reference))
          ))
      ) {

        hazard = true;
      }
    }

    return hazard;
  }

  traffic_manager::Polygon CollisionStage::GetPolygon(const LocationList &boundary) {

    std::string boundary_polygon_wkt;
    for (const cg::Location &location: boundary) {
      boundary_polygon_wkt += std::to_string(location.x) + " " + std::to_string(location.y) + ",";
    }

    boundary_polygon_wkt += std::to_string(boundary[0].x) + " " + std::to_string(boundary[0].y);

    traffic_manager::Polygon boundary_polygon;
    bg::read_wkt("POLYGON((" + boundary_polygon_wkt + "))", boundary_polygon);

    return boundary_polygon;
  }

  LocationList CollisionStage::GetGeodesicBoundary(const Actor &actor, const cg::Location &vehicle_location) {

    if (geodesic_boundaries.find(actor->GetId()) != geodesic_boundaries.end()) {
      return geodesic_boundaries.at(actor->GetId());
    }

    const LocationList bbox = GetBoundary(actor, vehicle_location);

    if (vehicle_id_to_index.find(actor->GetId()) != vehicle_id_to_index.end()) {

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
      const float length = vehicle->GetBoundingBox().extent.x*2;

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

      geodesic_boundaries.insert({actor->GetId(), geodesic_boundary});
      return geodesic_boundary;
    } else {

      geodesic_boundaries.insert({actor->GetId(), bbox});
      return bbox;
    }

  }

  float CollisionStage::GetBoundingBoxExtention(const Actor &actor) {

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

  LocationList CollisionStage::GetBoundary(const Actor &actor, const cg::Location &location) {

    const auto actor_type = actor->GetTypeId();
    cg::Vector3D heading_vector = actor->GetTransform().GetForwardVector();
    heading_vector.z = 0.0f;
    heading_vector = heading_vector.MakeUnitVector();

    cg::BoundingBox bbox;
    float forward_extension = 0.0f;
    if (actor_type[0] == 'v') {
      const auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
      bbox = vehicle->GetBoundingBox();
    } else if (actor_type[0] == 'w') {
      const auto walker = boost::static_pointer_cast<cc::Walker>(actor);
      bbox = walker->GetBoundingBox();
      // Extend the pedestrians bbox to "predict" where they'll be and avoid collisions.
      forward_extension = walker->GetVelocity().Length() * WALKER_TIME_EXTENSION;
    }

    const cg::Vector3D extent = bbox.extent;
    const cg::Vector3D perpendicular_vector = cg::Vector3D(-heading_vector.y, heading_vector.x, 0.0f);

    const cg::Vector3D x_boundary_vector = heading_vector * (extent.x + forward_extension);
    const cg::Vector3D y_boundary_vector = perpendicular_vector * (extent.y + forward_extension);

    // Four corners of the vehicle in top view clockwise order (left-handed
    // system).
    LocationList bbox_boundary = {
      location + cg::Location(x_boundary_vector - y_boundary_vector),
      location + cg::Location(-1.0f * x_boundary_vector - y_boundary_vector),
      location + cg::Location(-1.0f * x_boundary_vector + y_boundary_vector),
      location + cg::Location(x_boundary_vector + y_boundary_vector),
    };

    return bbox_boundary;
  }

  bool CollisionStage::IsLocationAfterJunctionSafe(const Actor &ego_actor, const Actor &other_actor, const SimpleWaypointPtr safe_point , const cg::Location &other_location){

    bool safe_junction = true;

    if (other_actor->GetVelocity().Length() < EPSILON_VELOCITY){

      cg::Location safe_location = safe_point->GetLocation();
      cg::Vector3D heading_vector = safe_point->GetForwardVector();
      heading_vector.z = 0.0f;
      heading_vector = heading_vector.MakeUnitVector();

      cg::BoundingBox bbox;
      const auto vehicle = boost::static_pointer_cast<cc::Vehicle>(ego_actor);
      bbox = vehicle->GetBoundingBox();
      const cg::Vector3D extent = bbox.extent;

      const cg::Vector3D perpendicular_vector = cg::Vector3D(-heading_vector.y, heading_vector.x, 0.0f);

      const cg::Vector3D x_boundary_vector = heading_vector * extent.x;
      const cg::Vector3D y_boundary_vector = perpendicular_vector * extent.y;

      LocationList ego_actor_boundary = {
        safe_location + cg::Location(x_boundary_vector - y_boundary_vector),
        safe_location + cg::Location(-1.0f * x_boundary_vector - y_boundary_vector),
        safe_location + cg::Location(-1.0f * x_boundary_vector + y_boundary_vector),
        safe_location + cg::Location(x_boundary_vector + y_boundary_vector),
      };

      const Polygon reference_polygon = GetPolygon(ego_actor_boundary);
      const Polygon other_polygon = GetPolygon(GetBoundary(other_actor, other_location));

      const auto inter_bbox_distance = bg::distance(reference_polygon, other_polygon);
      if (inter_bbox_distance < INTER_BBOX_DISTANCE_THRESHOLD){
        safe_junction = false;
      }
    }

    return safe_junction;
  }

  void CollisionStage::DrawBoundary(const LocationList &boundary) {
    for (uint64_t i = 0u; i < boundary.size(); ++i) {
      debug_helper.DrawLine(
          boundary[i] + cg::Location(0.0f, 0.0f, 1.0f),
          boundary[(i + 1) % boundary.size()] + cg::Location(0.0f, 0.0f, 1.0f),
          0.1f, {255u, 255u, 0u}, 0.05f);
    }
  }

} // namespace traffic_manager
} // namespace carla
