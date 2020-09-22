
#include "carla/geom/Math.h"

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/LocalizationUtils.h"

#include "carla/trafficmanager/CollisionStage.h"

namespace carla {
namespace traffic_manager {

using Point2D = bg::model::point<double, 2, bg::cs::cartesian>;
using TLS = carla::rpc::TrafficLightState;

using namespace constants::Collision;
using constants::WaypointSelection::JUNCTION_LOOK_AHEAD;

CollisionStage::CollisionStage(
  const std::vector<ActorId> &vehicle_id_list,
  const SimulationState &simulation_state,
  const BufferMap &buffer_map,
  const TrackTraffic &track_traffic,
  const Parameters &parameters,
  CollisionFrame &output_array,
  cc::DebugHelper &debug_helper,
  RandomGeneratorMap &random_devices)
  : vehicle_id_list(vehicle_id_list),
    simulation_state(simulation_state),
    buffer_map(buffer_map),
    track_traffic(track_traffic),
    parameters(parameters),
    output_array(output_array),
    debug_helper(debug_helper),
    random_devices(random_devices) {}

void CollisionStage::Update(const unsigned long index) {
  ActorId obstacle_id = 0u;
  bool collision_hazard = false;
  float available_distance_margin = std::numeric_limits<float>::infinity();

  const ActorId ego_actor_id = vehicle_id_list.at(index);
  if (simulation_state.ContainsActor(ego_actor_id)) {
    const cg::Location ego_location = simulation_state.GetLocation(ego_actor_id);
    const Buffer &ego_buffer = buffer_map.at(ego_actor_id);
    const unsigned long look_ahead_index = GetTargetWaypoint(ego_buffer, JUNCTION_LOOK_AHEAD).second;

    ActorIdSet overlapping_actors = track_traffic.GetOverlappingVehicles(ego_actor_id);
    std::vector<ActorId> collision_candidate_ids;

    // Run through vehicles with overlapping paths and filter them;
    float collision_radius_square = SQUARE(MAX_COLLISION_RADIUS);
    for (ActorId overlapping_actor_id : overlapping_actors) {
      // If actor is within maximum collision avoidance and vertical overlap range.
      const cg::Location &overlapping_actor_location = simulation_state.GetLocation(overlapping_actor_id);
      if (overlapping_actor_id != ego_actor_id
          && cg::Math::DistanceSquared(overlapping_actor_location, ego_location) < collision_radius_square
          && std::abs(ego_location.z - overlapping_actor_location.z) < VERTICAL_OVERLAP_THRESHOLD) {
        collision_candidate_ids.push_back(overlapping_actor_id);
      }
    }

    // Sorting collision candidates in accending order of distance to current vehicle.
    std::sort(collision_candidate_ids.begin(), collision_candidate_ids.end(),
              [this, &ego_location](const ActorId &a_id_1, const ActorId &a_id_2) {
                const cg::Location &e_loc = ego_location;
                const cg::Location &loc_1 = simulation_state.GetLocation(a_id_1);
                const cg::Location &loc_2 = simulation_state.GetLocation(a_id_2);
                return (cg::Math::DistanceSquared(e_loc, loc_1) < cg::Math::DistanceSquared(e_loc, loc_2));
              });

    // Check every actor in the vicinity if it poses a collision hazard.
    for (auto iter = collision_candidate_ids.begin();
         iter != collision_candidate_ids.end() && !collision_hazard;
         ++iter) {
      const ActorId other_actor_id = *iter;
      const ActorType other_actor_type = simulation_state.GetType(other_actor_id);

      if (parameters.GetCollisionDetection(ego_actor_id, other_actor_id)
          && buffer_map.find(ego_actor_id) != buffer_map.end()
          && simulation_state.ContainsActor(other_actor_id)) {
        std::pair<bool, float> negotiation_result = NegotiateCollision(ego_actor_id,
                                                                       other_actor_id,
                                                                       look_ahead_index);
        if (negotiation_result.first) {
          if ((other_actor_type == ActorType::Vehicle
               && parameters.GetPercentageIgnoreVehicles(ego_actor_id) <= random_devices.at(ego_actor_id).next())
              || (other_actor_type == ActorType::Pedestrian
                  && parameters.GetPercentageIgnoreWalkers(ego_actor_id) <= random_devices.at(ego_actor_id).next())) {
            collision_hazard = true;
            obstacle_id = other_actor_id;
            available_distance_margin = negotiation_result.second;
          }
        }
      }
    }
  }

  CollisionHazardData &output_element = output_array.at(index);
  output_element.hazard_actor_id = obstacle_id;
  output_element.hazard = collision_hazard;
  output_element.available_distance_margin = available_distance_margin;
}

void CollisionStage::RemoveActor(const ActorId actor_id) {
  collision_locks.erase(actor_id);
}

void CollisionStage::Reset() {
  collision_locks.clear();
}

float CollisionStage::GetBoundingBoxExtention(const ActorId actor_id) {

  const float velocity = cg::Math::Dot(simulation_state.GetVelocity(actor_id), simulation_state.GetHeading(actor_id));
  float bbox_extension;
  // Using a linear function to calculate boundary length.
  bbox_extension = BOUNDARY_EXTENSION_RATE * velocity + BOUNDARY_EXTENSION_MINIMUM;
  // If a valid collision lock present, change boundary length to maintain lock.
  if (collision_locks.find(actor_id) != collision_locks.end()) {
    const CollisionLock &lock = collision_locks.at(actor_id);
    float lock_boundary_length = static_cast<float>(lock.distance_to_lead_vehicle + LOCKING_DISTANCE_PADDING);
    // Only extend boundary track vehicle if the leading vehicle
    // if it is not further than velocity dependent extension by MAX_LOCKING_EXTENSION.
    if ((lock_boundary_length - lock.initial_lock_distance) < MAX_LOCKING_EXTENSION) {
      bbox_extension = lock_boundary_length;
    }
  }

  return bbox_extension;
}

LocationVector CollisionStage::GetBoundary(const ActorId actor_id) {
  const ActorType actor_type = simulation_state.GetType(actor_id);
  const cg::Vector3D heading_vector = simulation_state.GetHeading(actor_id);

  float forward_extension = 0.0f;
  if (actor_type == ActorType::Pedestrian) {
    // Extend the pedestrians bbox to "predict" where they'll be and avoid collisions.
    forward_extension = simulation_state.GetVelocity(actor_id).Length() * WALKER_TIME_EXTENSION;
  }

  cg::Vector3D dimensions = simulation_state.GetDimensions(actor_id);

  float bbox_x = dimensions.x;
  float bbox_y = dimensions.y;

  const cg::Vector3D x_boundary_vector = heading_vector * (bbox_x + forward_extension);
  const auto perpendicular_vector = cg::Vector3D(-heading_vector.y, heading_vector.x, 0.0f).MakeSafeUnitVector(EPSILON);
  const cg::Vector3D y_boundary_vector = perpendicular_vector * (bbox_y + forward_extension);

  // Four corners of the vehicle in top view clockwise order (left-handed system).
  const cg::Location location = simulation_state.GetLocation(actor_id);
  LocationVector bbox_boundary = {
      location + cg::Location(x_boundary_vector - y_boundary_vector),
      location + cg::Location(-1.0f * x_boundary_vector - y_boundary_vector),
      location + cg::Location(-1.0f * x_boundary_vector + y_boundary_vector),
      location + cg::Location(x_boundary_vector + y_boundary_vector),
  };

  return bbox_boundary;
}

LocationVector CollisionStage::GetGeodesicBoundary(const ActorId actor_id) {
  LocationVector geodesic_boundary;

  if (geodesic_boundary_map.find(actor_id) != geodesic_boundary_map.end()) {
    geodesic_boundary = geodesic_boundary_map.at(actor_id);
  } else {
    const LocationVector bbox = GetBoundary(actor_id);

    if (buffer_map.find(actor_id) != buffer_map.end()) {
      float bbox_extension = GetBoundingBoxExtention(actor_id);
      const float specific_lead_distance = parameters.GetDistanceToLeadingVehicle(actor_id);
      bbox_extension = std::max(specific_lead_distance, bbox_extension);
      const float bbox_extension_square = SQUARE(bbox_extension);

      LocationVector left_boundary;
      LocationVector right_boundary;
      cg::Vector3D dimensions = simulation_state.GetDimensions(actor_id);
      const float width = dimensions.y;
      const float length = dimensions.x;

      const Buffer &waypoint_buffer = buffer_map.at(actor_id);
      const TargetWPInfo target_wp_info = GetTargetWaypoint(waypoint_buffer, length);
      const SimpleWaypointPtr boundary_start = target_wp_info.first;
      const uint64_t boundary_start_index = target_wp_info.second;

      // At non-signalized junctions, we extend the boundary across the junction
      // and in all other situations, boundary length is velocity-dependent.
      SimpleWaypointPtr boundary_end = nullptr;
      SimpleWaypointPtr current_point = waypoint_buffer.at(boundary_start_index);
      bool reached_distance = false;
      for (uint64_t j = boundary_start_index; !reached_distance && (j < waypoint_buffer.size()); ++j) {
        if (boundary_start->DistanceSquared(current_point) > bbox_extension_square || j == waypoint_buffer.size() - 1) {
          reached_distance = true;
        }
        if (boundary_end == nullptr
            || cg::Math::Dot(boundary_end->GetForwardVector(), current_point->GetForwardVector()) < COS_10_DEGREES
            || reached_distance) {

          const cg::Vector3D heading_vector = current_point->GetForwardVector();
          const cg::Location location = current_point->GetLocation();
          cg::Vector3D perpendicular_vector = cg::Vector3D(-heading_vector.y, heading_vector.x, 0.0f);
          perpendicular_vector = perpendicular_vector.MakeSafeUnitVector(EPSILON);
          // Direction determined for the left-handed system.
          const cg::Vector3D scaled_perpendicular = perpendicular_vector * width;
          left_boundary.push_back(location + cg::Location(scaled_perpendicular));
          right_boundary.push_back(location + cg::Location(-1.0f * scaled_perpendicular));

          boundary_end = current_point;
        }

        current_point = waypoint_buffer.at(j);
      }

      // Reversing right boundary to construct clockwise (left-hand system)
      // boundary. This is so because both left and right boundary vectors have
      // the closest point to the vehicle at their starting index for the right
      // boundary,
      // we want to begin at the farthest point to have a clockwise trace.
      std::reverse(right_boundary.begin(), right_boundary.end());
      geodesic_boundary.insert(geodesic_boundary.end(), right_boundary.begin(), right_boundary.end());
      geodesic_boundary.insert(geodesic_boundary.end(), bbox.begin(), bbox.end());
      geodesic_boundary.insert(geodesic_boundary.end(), left_boundary.begin(), left_boundary.end());
    } else {

      geodesic_boundary = bbox;
    }

    geodesic_boundary_map.insert({actor_id, geodesic_boundary});
  }

  return geodesic_boundary;
}

Polygon CollisionStage::GetPolygon(const LocationVector &boundary) {

  traffic_manager::Polygon boundary_polygon;
  for (const cg::Location &location : boundary) {
    bg::append(boundary_polygon.outer(), Point2D(location.x, location.y));
  }
  bg::append(boundary_polygon.outer(), Point2D(boundary.front().x, boundary.front().y));

  return boundary_polygon;
}

GeometryComparison CollisionStage::GetGeometryBetweenActors(const ActorId reference_vehicle_id,
                                                            const ActorId other_actor_id) {


  std::pair<ActorId, ActorId> key_parts;
  if (reference_vehicle_id < other_actor_id) {
    key_parts = {reference_vehicle_id, other_actor_id};
  } else {
    key_parts = {other_actor_id, reference_vehicle_id};
  }

  uint64_t actor_id_key = 0u;
  actor_id_key |= key_parts.first;
  actor_id_key <<= 32;
  actor_id_key |= key_parts.second;

  GeometryComparison comparision_result{-1.0, -1.0, -1.0, -1.0};

  if (geometry_cache.find(actor_id_key) != geometry_cache.end()) {

    comparision_result = geometry_cache.at(actor_id_key);
    double mref_veh_other = comparision_result.reference_vehicle_to_other_geodesic;
    comparision_result.reference_vehicle_to_other_geodesic = comparision_result.other_vehicle_to_reference_geodesic;
    comparision_result.other_vehicle_to_reference_geodesic = mref_veh_other;
  } else {

    const Polygon reference_polygon = GetPolygon(GetBoundary(reference_vehicle_id));
    const Polygon other_polygon = GetPolygon(GetBoundary(other_actor_id));

    const Polygon reference_geodesic_polygon = GetPolygon(GetGeodesicBoundary(reference_vehicle_id));

    const Polygon other_geodesic_polygon = GetPolygon(GetGeodesicBoundary(other_actor_id));

    const double reference_vehicle_to_other_geodesic = bg::distance(reference_polygon, other_geodesic_polygon);
    const double other_vehicle_to_reference_geodesic = bg::distance(other_polygon, reference_geodesic_polygon);
    const auto inter_geodesic_distance = bg::distance(reference_geodesic_polygon, other_geodesic_polygon);
    const auto inter_bbox_distance = bg::distance(reference_polygon, other_polygon);

    comparision_result = {reference_vehicle_to_other_geodesic,
              other_vehicle_to_reference_geodesic,
              inter_geodesic_distance,
              inter_bbox_distance};

    geometry_cache.insert({actor_id_key, comparision_result});
  }

  return comparision_result;
}

std::pair<bool, float> CollisionStage::NegotiateCollision(const ActorId reference_vehicle_id,
                                                          const ActorId other_actor_id,
                                                          const uint64_t reference_junction_look_ahead_index) {
  // Output variables for the method.
  bool hazard = false;
  float available_distance_margin = std::numeric_limits<float>::infinity();

  const cg::Location reference_location = simulation_state.GetLocation(reference_vehicle_id);
  const cg::Location other_location = simulation_state.GetLocation(other_actor_id);

  // Ego and other vehicle heading.
  const cg::Vector3D reference_heading = simulation_state.GetHeading(reference_vehicle_id);
  // Vector from ego position to position of the other vehicle.
  cg::Vector3D reference_to_other = other_location - reference_location;
  reference_to_other = reference_to_other.MakeSafeUnitVector(EPSILON);

  // Other vehicle heading.
  const cg::Vector3D other_heading = simulation_state.GetHeading(other_actor_id);
  // Vector from other vehicle position to ego position.
  cg::Vector3D other_to_reference = reference_location - other_location;
  other_to_reference = other_to_reference.MakeSafeUnitVector(EPSILON);

  float reference_vehicle_length = simulation_state.GetDimensions(reference_vehicle_id).x * SQUARE_ROOT_OF_TWO;
  float other_vehicle_length = simulation_state.GetDimensions(other_actor_id).x * SQUARE_ROOT_OF_TWO;

  float inter_vehicle_distance = cg::Math::DistanceSquared(reference_location, other_location);
  float ego_bounding_box_extension = GetBoundingBoxExtention(reference_vehicle_id);
  float other_bounding_box_extension = GetBoundingBoxExtention(other_actor_id);
  // Calculate minimum distance between vehicle to consider collision negotiation.
  float inter_vehicle_length = reference_vehicle_length + other_vehicle_length;
  float ego_detection_range = SQUARE(ego_bounding_box_extension + inter_vehicle_length);
  float cross_detection_range = SQUARE(ego_bounding_box_extension + inter_vehicle_length + other_bounding_box_extension);

  // Conditions to consider collision negotiation.
  bool other_vehicle_in_ego_range = inter_vehicle_distance < ego_detection_range;
  bool other_vehicles_in_cross_detection_range = inter_vehicle_distance < cross_detection_range;
  float reference_heading_to_other_dot = cg::Math::Dot(reference_heading, reference_to_other);
  bool other_vehicle_in_front = reference_heading_to_other_dot > 0;
  const Buffer &reference_vehicle_buffer = buffer_map.at(reference_vehicle_id);
  SimpleWaypointPtr closest_point = reference_vehicle_buffer.front();
  bool ego_inside_junction = closest_point->CheckJunction();
  TrafficLightState reference_tl_state = simulation_state.GetTLS(reference_vehicle_id);
  bool ego_at_traffic_light = reference_tl_state.at_traffic_light;
  bool ego_stopped_by_light = reference_tl_state.tl_state != TLS::Green && reference_tl_state.tl_state != TLS::Off;
  SimpleWaypointPtr look_ahead_point = reference_vehicle_buffer.at(reference_junction_look_ahead_index);
  bool ego_at_junction_entrance = !closest_point->CheckJunction() && look_ahead_point->CheckJunction();

  // Conditions to consider collision negotiation.
  if (!(ego_at_junction_entrance && ego_at_traffic_light && ego_stopped_by_light)
      && ((ego_inside_junction && other_vehicles_in_cross_detection_range)
          || (!ego_inside_junction && other_vehicle_in_front && other_vehicle_in_ego_range))) {
    GeometryComparison geometry_comparison = GetGeometryBetweenActors(reference_vehicle_id, other_actor_id);

    // Conditions for collision negotiation.
    bool geodesic_path_bbox_touching = geometry_comparison.inter_geodesic_distance < 0.1;
    bool vehicle_bbox_touching = geometry_comparison.inter_bbox_distance < 0.1;
    bool ego_path_clear = geometry_comparison.other_vehicle_to_reference_geodesic > 0.1;
    bool other_path_clear = geometry_comparison.reference_vehicle_to_other_geodesic > 0.1;
    bool ego_path_priority = geometry_comparison.reference_vehicle_to_other_geodesic < geometry_comparison.other_vehicle_to_reference_geodesic;
    bool other_path_priority = geometry_comparison.reference_vehicle_to_other_geodesic > geometry_comparison.other_vehicle_to_reference_geodesic;
    bool ego_angular_priority = reference_heading_to_other_dot< cg::Math::Dot(other_heading, other_to_reference);

    // Whichever vehicle's path is farthest away from the other vehicle gets priority to move.
    bool lower_priority = !ego_path_priority && (other_path_priority || !ego_angular_priority);
    bool blocked_by_other_or_lower_priority = !ego_path_clear || (other_path_clear && lower_priority);
    bool yield_pre_crash = !vehicle_bbox_touching && blocked_by_other_or_lower_priority;
    bool yield_post_crash = vehicle_bbox_touching && !ego_angular_priority;

    if (geodesic_path_bbox_touching && (yield_pre_crash || yield_post_crash)) {

      hazard = true;

      const float reference_lead_distance = parameters.GetDistanceToLeadingVehicle(reference_vehicle_id);
      const float specific_distance_margin = std::max(reference_lead_distance, BOUNDARY_EXTENSION_MINIMUM);
      available_distance_margin = static_cast<float>(std::max(geometry_comparison.reference_vehicle_to_other_geodesic
                                                              - static_cast<double>(specific_distance_margin), 0.0));

      ///////////////////////////////////// Collision locking mechanism /////////////////////////////////
      // The idea is, when encountering a possible collision,
      // we should ensure that the bounding box extension doesn't decrease too fast and loose collision tracking.
      // This enables us to smoothly approach the lead vehicle.

      // When possible collision found, check if an entry for collision lock present.
      if (collision_locks.find(reference_vehicle_id) != collision_locks.end()) {
        CollisionLock &lock = collision_locks.at(reference_vehicle_id);
        // Check if the same vehicle is under lock.
        if (other_actor_id == lock.lead_vehicle_id) {
          // If the body of the lead vehicle is touching the reference vehicle bounding box.
          if (geometry_comparison.other_vehicle_to_reference_geodesic < 0.1) {
            // Distance between the bodies of the vehicles.
            lock.distance_to_lead_vehicle = geometry_comparison.inter_bbox_distance;
          } else {
            // Distance from reference vehicle body to other vehicle path polygon.
            lock.distance_to_lead_vehicle = geometry_comparison.reference_vehicle_to_other_geodesic;
          }
        } else {
          // If possible collision with a new vehicle, re-initialize with new lock entry.
          lock = {geometry_comparison.inter_bbox_distance, geometry_comparison.inter_bbox_distance, other_actor_id};
        }
      } else {
        // Insert and initialize lock entry if not present.
        collision_locks.insert({reference_vehicle_id,
                                {geometry_comparison.inter_bbox_distance,
                                 geometry_comparison.inter_bbox_distance,
                                 other_actor_id}});
      }
    }
  }

  // If no collision hazard detected, then flush collision lock held by the vehicle.
  if (!hazard && collision_locks.find(reference_vehicle_id) != collision_locks.end()) {
    collision_locks.erase(reference_vehicle_id);
  }

  return {hazard, available_distance_margin};
}

void CollisionStage::ClearCycleCache() {
  geodesic_boundary_map.clear();
  geometry_cache.clear();
}

void CollisionStage::DrawBoundary(const LocationVector &boundary) {
  cg::Location one_meter_up(0.0f, 0.0f, 1.0f);
  for (uint64_t i = 0u; i < boundary.size(); ++i) {
    debug_helper.DrawLine(
        boundary[i] + one_meter_up,
        boundary[i+1 == boundary.size()? 0: i+1] + one_meter_up,
        0.1f, {255u, 255u, 0u}, 0.05f);
  }
}

} // namespace traffic_manager
} // namespace carla
