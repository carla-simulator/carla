// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <limits>

#include "carla/client/TrafficSign.h"
#include "carla/client/TrafficLight.h"
#include "carla/rpc/TrafficLightState.h"

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/PIDController.h"
#include "carla/trafficmanager/TrafficManagerGeometry.h"

#include "carla/trafficmanager/MotionPlanStage.h"

namespace carla {
namespace traffic_manager {

using namespace constants::MotionPlan;
using namespace constants::WaypointSelection;
using namespace constants::SpeedThreshold;

using constants::HybridMode::HYBRID_MODE_DT;
using constants::HybridMode::HYBRID_MODE_DT_FL;
using constants::Collision::EPSILON;

MotionPlanStage::MotionPlanStage(
  const std::vector<ActorId> &vehicle_id_list,
  SimulationState &simulation_state,
  const Parameters &parameters,
  const BufferMap &buffer_map,
  TrackTraffic &track_traffic,
  const std::vector<float> &urban_longitudinal_parameters,
  const std::vector<float> &highway_longitudinal_parameters,
  const std::vector<float> &urban_lateral_parameters,
  const std::vector<float> &highway_lateral_parameters,
  const LocalizationFrame &localization_frame,
  const CollisionFrame&collision_frame,
  const TLFrame &tl_frame,
  const cc::World &world,
  ControlFrame &output_array,
  RandomGenerator &random_device,
  const LocalMapPtr &local_map,
  std::unordered_map<ActorId, std::pair<float, bool>> &large_vehicles)
    : vehicle_id_list(vehicle_id_list),
    simulation_state(simulation_state),
    parameters(parameters),
    buffer_map(buffer_map),
    track_traffic(track_traffic),
    urban_longitudinal_parameters(urban_longitudinal_parameters),
    highway_longitudinal_parameters(highway_longitudinal_parameters),
    urban_lateral_parameters(urban_lateral_parameters),
    highway_lateral_parameters(highway_lateral_parameters),
    localization_frame(localization_frame),
    collision_frame(collision_frame),
    tl_frame(tl_frame),
    world(world),
    output_array(output_array),
    random_device(random_device),
    local_map(local_map),
    large_vehicles(large_vehicles) {}

void MotionPlanStage::Update(const unsigned long index) {
  const ActorId actor_id = vehicle_id_list.at(index);
  const cg::Location vehicle_location = simulation_state.GetLocation(actor_id);
  const cg::Vector3D vehicle_velocity = simulation_state.GetVelocity(actor_id);
  const cg::Rotation vehicle_rotation = simulation_state.GetRotation(actor_id);
  const float vehicle_speed = vehicle_velocity.Length();
  const cg::Vector3D vehicle_heading = simulation_state.GetHeading(actor_id);
  const bool vehicle_physics_enabled = simulation_state.IsPhysicsEnabled(actor_id);
  const float vehicle_speed_limit = simulation_state.GetSpeedLimit(actor_id);
  const Buffer &waypoint_buffer = buffer_map.at(actor_id);
  const LocalizationData &localization = localization_frame.at(index);
  const CollisionHazardData &collision_hazard = collision_frame.at(index);
  const bool &tl_hazard = tl_frame.at(index);
  current_timestamp = world.GetSnapshot().GetTimestamp();
  StateEntry current_state;

  // Instanciating teleportation transform as current vehicle transform.
  cg::Transform teleportation_transform = cg::Transform(vehicle_location, vehicle_rotation);

  // Get information about the hero location from the actor_id state.
  cg::Location hero_location = track_traffic.GetHeroLocation();
  bool is_hero_alive = hero_location != cg::Location(0, 0, 0);

  if (simulation_state.IsDormant(actor_id) && parameters.GetRespawnDormantVehicles() && is_hero_alive) {
    // Flushing controller state for vehicle.
    current_state = {current_timestamp,
                    0.0f, 0.0f,
                    0.0f};

    // Add entry to teleportation duration clock table if not present.
    if (teleportation_instance.find(actor_id) == teleportation_instance.end()) {
      teleportation_instance.insert({actor_id, current_timestamp});
    }

    // Get lower and upper bound for teleporting vehicle.
    float lower_bound = parameters.GetLowerBoundaryRespawnDormantVehicles();
    float upper_bound = parameters.GetUpperBoundaryRespawnDormantVehicles();
    float dilate_factor = (upper_bound-lower_bound)/100.0f;

    // Measuring time elapsed since last teleportation for the vehicle.
    double elapsed_time = current_timestamp.elapsed_seconds - teleportation_instance.at(actor_id).elapsed_seconds;

    if (parameters.GetSynchronousMode() || elapsed_time > HYBRID_MODE_DT) {
      float random_sample = (static_cast<float>(random_device.next())*dilate_factor) + lower_bound;
      NodeList teleport_waypoint_list = local_map->GetWaypointsInDelta(hero_location, ATTEMPTS_TO_TELEPORT, random_sample);
      if (!teleport_waypoint_list.empty()) {
        for (auto &teleport_waypoint : teleport_waypoint_list) {
          GeoGridId geogrid_id = teleport_waypoint->GetGeodesicGridId();
          if (track_traffic.IsGeoGridFree(geogrid_id)) {
            teleportation_transform = teleport_waypoint->GetTransform();
            teleportation_transform.location.z += 0.5f;
            track_traffic.AddTakenGrid(geogrid_id, actor_id);
            break;
          }
        }
      }
    }
    output_array.at(index) = carla::rpc::Command::ApplyTransform(actor_id, teleportation_transform);

    // Update the simulation state with the new transform of the vehicle after teleporting it.
    KinematicState kinematic_state{teleportation_transform.location,
                                   teleportation_transform.rotation,
                                   vehicle_velocity, vehicle_speed_limit,
                                   vehicle_physics_enabled, simulation_state.IsDormant(actor_id),
                                   teleportation_transform.location};
    simulation_state.UpdateKinematicState(actor_id, kinematic_state);
  }

  else {

    // Target velocity for vehicle.
    float max_target_velocity = parameters.GetVehicleTargetVelocity(actor_id, vehicle_speed_limit) / 3.6f;

    // Algorithm to reduce speed near landmarks
    float max_landmark_target_velocity = GetLandmarkTargetVelocity(*(waypoint_buffer.at(0)), vehicle_location, actor_id, max_target_velocity);

    // Algorithm to reduce speed near turns
    float max_turn_target_velocity = GetTurnTargetVelocity(waypoint_buffer, max_target_velocity);
    max_target_velocity = std::min(std::min(max_target_velocity, max_landmark_target_velocity), max_turn_target_velocity);

    // Collision handling and target velocity correction.
    std::pair<bool, float> collision_response = CollisionHandling(collision_hazard, tl_hazard, vehicle_velocity,
                                                                  vehicle_heading, max_target_velocity);
    bool collision_emergency_stop = collision_response.first;
    float dynamic_target_velocity = collision_response.second;

    // Don't enter junction if there isn't enough free space after the junction.
    bool safe_after_junction = SafeAfterJunction(localization, tl_hazard, collision_emergency_stop);

    // In case of collision or traffic light hazard.
    bool emergency_stop = tl_hazard || collision_emergency_stop || !safe_after_junction;

    if (vehicle_physics_enabled && !simulation_state.IsDormant(actor_id)) {
      ActuationSignal actuation_signal{0.0f, 0.0f, 0.0f};

      // Resolve the target waypoint by interpolating along the buffer at
      // target_point_distance ahead of the vehicle.
      const float target_point_distance{std::max(
          vehicle_speed * TARGET_WAYPOINT_TIME_HORIZON,
          MIN_TARGET_WAYPOINT_DISTANCE)};
      const auto [interp_target_location, target_index] = GetTargetData(
          waypoint_buffer,
          target_point_distance,
          vehicle_location);
      cg::Location target_location{interp_target_location};
      const SimpleWaypointPtr target_waypoint = waypoint_buffer.at(target_index);

      float base_offset{CalculateBaseOffset(
          actor_id,
          waypoint_buffer,
          target_waypoint->CheckJunction(),
          target_index)};
      const auto right_vector = target_waypoint->GetTransform().GetRightVector();

      // Suppress the wide-turn swing when the side it would move into is
      // occupied by a nearby vehicle (parked, stopped or adjacent traffic).
      if (std::abs(base_offset) > EPSILON) {
        const cg::Vector3D offset_direction{
            (base_offset > 0.0f) ? right_vector : (-1.0f * right_vector)};
        if (IsWideTurnSideOccupied(actor_id, offset_direction, std::abs(base_offset))) {
          base_offset = 0.0f;
        }
      }

      const float offset{parameters.GetLaneOffset(actor_id) + base_offset};
      const auto offset_location = cg::Location(cg::Vector3D(
          offset * right_vector.x,
          offset * right_vector.y,
          0.0f));
      target_location = target_location + offset_location;

      // Compute the angular deviation directly as the angle between the
      // vehicle heading and the target direction. atan2(0, 0) is well-defined
      // and returns 0 when the vehicle and target locations coincide on the
      // very first tick before any displacement.
      const cg::Vector3D target_vector{target_location - vehicle_location};
      const float target_yaw{std::atan2(target_vector.y, target_vector.x) * 180.0f / PI};
      float angular_deviation{target_yaw - vehicle_rotation.yaw};
      if (angular_deviation > 180.0f) {
        angular_deviation -= 360.0f;
      } else if (angular_deviation < -180.0f) {
        angular_deviation += 360.0f;
      }
      angular_deviation /= 180.0f;  // Normalised to [-1, 1].
      const float velocity_deviation{(dynamic_target_velocity - vehicle_speed) / dynamic_target_velocity};

      // If previous state for vehicle not found, initialize state entry.
      if (pid_state_map.find(actor_id) == pid_state_map.end()) {
        const auto initial_state = StateEntry{current_timestamp, 0.0f, 0.0f, 0.0f};
        pid_state_map.insert({actor_id, initial_state});
      }

      // Retrieving the previous state.
      traffic_manager::StateEntry previous_state;
      previous_state = pid_state_map.at(actor_id);

      // Select PID parameters.
      std::vector<float> longitudinal_parameters;
      std::vector<float> lateral_parameters;
      if (vehicle_speed > HIGHWAY_SPEED) {
        longitudinal_parameters = highway_longitudinal_parameters;
        lateral_parameters = highway_lateral_parameters;
      } else {
        longitudinal_parameters = urban_longitudinal_parameters;
        lateral_parameters = urban_lateral_parameters;
      }

      // If physics is enabled for the vehicle, use PID controller.
      // State update for vehicle.
      current_state = {current_timestamp, angular_deviation, velocity_deviation, 0.0f};

      // Controller actuation.
      actuation_signal = PID::RunStep(current_state, previous_state,
                                      longitudinal_parameters, lateral_parameters);

      if (emergency_stop) {
        actuation_signal.throttle = 0.0f;
        actuation_signal.brake = 1.0f;
      }

      // Constructing the actuation signal.
      carla::rpc::VehicleControl vehicle_control;
      vehicle_control.throttle = actuation_signal.throttle;
      vehicle_control.brake = actuation_signal.brake;
      vehicle_control.steer = actuation_signal.steer;

      output_array.at(index) = carla::rpc::Command::ApplyVehicleControl(actor_id, vehicle_control);

      // Updating PID state.
      current_state.steer = actuation_signal.steer;
      StateEntry &state = pid_state_map.at(actor_id);
      state = current_state;
    }
    // For physics-less vehicles, determine position and orientation for teleportation.
    else {
      // Flushing controller state for vehicle.
      current_state = {current_timestamp,
                      0.0f, 0.0f,
                      0.0f};

      // Add entry to teleportation duration clock table if not present.
      if (teleportation_instance.find(actor_id) == teleportation_instance.end()) {
        teleportation_instance.insert({actor_id, current_timestamp});
      }

      // Measuring time elapsed since last teleportation for the vehicle.
      double elapsed_time = current_timestamp.elapsed_seconds - teleportation_instance.at(actor_id).elapsed_seconds;

      // Find a location ahead of the vehicle for teleportation to achieve intended velocity.
      if (!emergency_stop && (parameters.GetSynchronousMode() || elapsed_time > HYBRID_MODE_DT)) {

        // Target displacement magnitude to achieve target velocity.
        const float target_displacement = dynamic_target_velocity * HYBRID_MODE_DT_FL;
        SimpleWaypointPtr teleport_target = waypoint_buffer.front();
        cg::Transform target_base_transform = teleport_target->GetTransform();
        cg::Location target_base_location = target_base_transform.location;
        cg::Vector3D target_heading = target_base_transform.GetForwardVector();
        cg::Vector3D correct_heading = (target_base_location - vehicle_location).MakeSafeUnitVector(EPSILON);

        if (vehicle_location.Distance(target_base_location) < target_displacement) {
          cg::Location teleportation_location = vehicle_location + cg::Location(target_heading.MakeSafeUnitVector(EPSILON) * target_displacement);
          teleportation_transform = cg::Transform(teleportation_location, target_base_transform.rotation);
        }
        else {
          cg::Location teleportation_location = vehicle_location + cg::Location(correct_heading * target_displacement);
          teleportation_transform = cg::Transform(teleportation_location, target_base_transform.rotation);
        }
      // In case of an emergency stop, stay in the same location.
      // Also, teleport only once every dt in asynchronous mode.
      } else {
        teleportation_transform = cg::Transform(vehicle_location, simulation_state.GetRotation(actor_id));
      }
      // Constructing the actuation signal.
      output_array.at(index) = carla::rpc::Command::ApplyTransform(actor_id, teleportation_transform);
      simulation_state.UpdateKinematicHybridEndLocation(actor_id, teleportation_transform.location);
    }
  }
}

bool MotionPlanStage::SafeAfterJunction(const LocalizationData &localization,
                                        const bool tl_hazard,
                                        const bool collision_emergency_stop) {

  SimpleWaypointPtr junction_end_point = localization.junction_end_point;
  SimpleWaypointPtr safe_point = localization.safe_point;

  bool safe_after_junction = true;
  if (!tl_hazard && !collision_emergency_stop
      && localization.is_at_junction_entrance
      && junction_end_point != nullptr && safe_point != nullptr
      && junction_end_point->DistanceSquared(safe_point) > SQUARE(MIN_SAFE_INTERVAL_LENGTH)) {

    ActorIdSet passing_safe_point = track_traffic.GetPassingVehicles(safe_point->GetId());
    ActorIdSet passing_junction_end_point = track_traffic.GetPassingVehicles(junction_end_point->GetId());
    cg::Location mid_point = (junction_end_point->GetLocation() + safe_point->GetLocation())/2.0f;

    // Only check for vehicles that have the safe point in their passing waypoint, but not
    // the junction end point.
    ActorIdSet difference;
    std::set_difference(passing_safe_point.begin(), passing_safe_point.end(),
                        passing_junction_end_point.begin(), passing_junction_end_point.end(),
                        std::inserter(difference, difference.begin()));
    if (difference.size() > 0) {
      for (const ActorId &blocking_id: difference) {
        cg::Location blocking_actor_location = simulation_state.GetLocation(blocking_id);
        if (cg::Math::DistanceSquared(blocking_actor_location, mid_point) < SQUARE(MAX_JUNCTION_BLOCK_DISTANCE)
            && simulation_state.GetVelocity(blocking_id).SquaredLength() < SQUARE(AFTER_JUNCTION_MIN_SPEED)) {
          safe_after_junction = false;
          break;
        }
      }
    }
  }

  return safe_after_junction;
}

float MotionPlanStage::CalculateBaseOffset(
    const ActorId actor_id,
    const Buffer &waypoint_buffer,
    const bool is_target_junction,
    const uint64_t target_index) {

  // This offset is meant to make large vehicles do wider turns at intersections.
  if (large_vehicles.find(actor_id) == large_vehicles.end() || !is_target_junction) {
    return 0.0f;
  }

  // The wide-turn manoeuvre can be disabled per-vehicle or globally.
  if (!parameters.GetLargeVehicleWideTurn(actor_id)) {
    return 0.0f;
  }

  const auto &large_vehicle = large_vehicles.at(actor_id);

  // Going straight at the intersection: no offset to apply.
  if (large_vehicle.first == 0.0f) {
    return 0.0f;
  }

  float junction_missing_length{0.0f};
  for (unsigned long i = target_index; i < waypoint_buffer.size(); ++i) {
    const SimpleWaypointPtr current_waypoint = waypoint_buffer.at(i);

    if (i > target_index) {
      const SimpleWaypointPtr prev_waypoint = waypoint_buffer.at(i - 1u);
      const float new_distance = current_waypoint->Distance(prev_waypoint->GetLocation());
      junction_missing_length += new_distance;
    }

    if (!current_waypoint->CheckJunction()) {
      break;
    }
  }

  const float junction_length{large_vehicle.first};
  const bool turn_flag{large_vehicle.second};

  // Scale the offset magnitude by the vehicle's actual length so a short bus
  // and a long truck no longer share the same fixed displacement.
  // GetDimensions returns half-extents, so the full length is twice the x extent.
  const float vehicle_length{2.0f * simulation_state.GetDimensions(actor_id).x};
  const float max_offset{LargeVehicleOffsetMagnitude(
      vehicle_length,
      LARGE_VEHICLES_JUNCTION_REF_LENGTH,
      LARGE_VEHICLES_JUNCTION_OFFSET_GAIN,
      LARGE_VEHICLES_JUNCTION_OFFSET)};

  // Fraction of the junction still ahead of the target waypoint (1 at the
  // entry, 0 at the exit). The profile opens the vehicle up at the entry and
  // attenuates the inboard cut-in near the exit.
  const float t{junction_missing_length / junction_length};
  float offset{LargeVehicleJunctionOffsetProfile(
      t,
      max_offset,
      LARGE_VEHICLES_JUNCTION_POINT,
      LARGE_VEHICLES_JUNCTION_INBOARD_SCALE)};

  // Sign depends on the turn direction (right vs. left).
  offset = turn_flag ? offset : -offset;
  return offset;
}

bool MotionPlanStage::IsWideTurnSideOccupied(
    const ActorId actor_id,
    const cg::Vector3D offset_direction,
    const float offset_magnitude) {

  const ActorIdSet overlapping_vehicles{track_traffic.GetOverlappingVehicles(actor_id)};
  if (overlapping_vehicles.empty()) {
    return false;
  }

  std::vector<std::pair<cg::Location, float>> neighbours;
  neighbours.reserve(overlapping_vehicles.size());
  for (const ActorId other_id : overlapping_vehicles) {
    if (other_id == actor_id) {
      continue;
    }
    const cg::Vector3D other_dimensions{simulation_state.GetDimensions(other_id)};
    const float radius{std::max(other_dimensions.x, other_dimensions.y)};
    neighbours.emplace_back(simulation_state.GetLocation(other_id), radius);
  }

  const cg::Location ego_location{simulation_state.GetLocation(actor_id)};
  const cg::Vector3D ego_forward{simulation_state.GetHeading(actor_id)};
  const float longitudinal_window{
      simulation_state.GetDimensions(actor_id).x + LARGE_VEHICLES_JUNCTION_SIDE_MARGIN};

  return IsOffsetSideOccupied(
      ego_location,
      ego_forward,
      offset_direction,
      offset_magnitude,
      LARGE_VEHICLES_JUNCTION_CLEARANCE,
      longitudinal_window,
      neighbours);
}

std::pair<bool, float> MotionPlanStage::CollisionHandling(const CollisionHazardData &collision_hazard,
                                                          const bool tl_hazard,
                                                          const cg::Vector3D vehicle_velocity,
                                                          const cg::Vector3D vehicle_heading,
                                                          const float max_target_velocity) {
  bool collision_emergency_stop = false;
  float dynamic_target_velocity = max_target_velocity;
  const float vehicle_speed = vehicle_velocity.Length();

  if (collision_hazard.hazard && !tl_hazard) {
    const ActorId other_actor_id = collision_hazard.hazard_actor_id;
    const cg::Vector3D other_velocity = simulation_state.GetVelocity(other_actor_id);
    const float vehicle_relative_speed = (vehicle_velocity - other_velocity).Length();
    const float available_distance_margin = collision_hazard.available_distance_margin;

    const float other_speed_along_heading = cg::Math::Dot(other_velocity, vehicle_heading);

    // Consider collision avoidance decisions only if there is positive relative velocity
    // of the ego vehicle (meaning, ego vehicle is closing the gap to the lead vehicle).
    if (vehicle_relative_speed > EPSILON_RELATIVE_SPEED) {
      // If other vehicle is approaching lead vehicle and lead vehicle is further
      // than follow_lead_distance 0 kmph -> 5m, 100 kmph -> 10m.
      float follow_lead_distance = FOLLOW_LEAD_FACTOR * vehicle_speed + MIN_FOLLOW_LEAD_DISTANCE;
      if (available_distance_margin > follow_lead_distance) {
        // Then reduce the gap between the vehicles till FOLLOW_LEAD_DISTANCE
        // by maintaining a relative speed of other_speed_along_heading
        dynamic_target_velocity = other_speed_along_heading;
      }
      // If vehicle is approaching a lead vehicle and the lead vehicle is further
      // than CRITICAL_BRAKING_MARGIN but closer than FOLLOW_LEAD_DISTANCE.
      else if (available_distance_margin > CRITICAL_BRAKING_MARGIN) {
        // Then follow the lead vehicle by acquiring it's speed along current heading.
        dynamic_target_velocity = std::max(other_speed_along_heading, RELATIVE_APPROACH_SPEED);
      } else {
        // If lead vehicle closer than CRITICAL_BRAKING_MARGIN, initiate emergency stop.
        collision_emergency_stop = true;
      }
    }
    if (available_distance_margin < CRITICAL_BRAKING_MARGIN) {
      collision_emergency_stop = true;
    }
  }

  float max_gradual_velocity = PERC_MAX_SLOWDOWN * vehicle_speed;
  if (dynamic_target_velocity < vehicle_speed - max_gradual_velocity) {
    // Don't slow more than PERC_MAX_SLOWDOWN per frame.
    dynamic_target_velocity = vehicle_speed - max_gradual_velocity;
  }
  dynamic_target_velocity = std::min(max_target_velocity, dynamic_target_velocity);

  return {collision_emergency_stop, dynamic_target_velocity};
}

float MotionPlanStage::GetLandmarkTargetVelocity(const SimpleWaypoint& waypoint,
                                                 const cg::Location vehicle_location,
                                                 const ActorId actor_id,
                                                 float max_target_velocity) {

    auto const max_distance = LANDMARK_DETECTION_TIME * max_target_velocity;

    float landmark_target_velocity = std::numeric_limits<float>::max();

    auto all_landmarks = waypoint.GetWaypoint()->GetAllLandmarksInDistance(max_distance, false);

    for (auto &landmark: all_landmarks) {

      auto landmark_location = landmark->GetWaypoint()->GetTransform().location;
      auto landmark_type = landmark->GetType();
      auto distance = landmark_location.Distance(vehicle_location);

      if (distance > max_distance) {
        continue;
      }

      float minimum_velocity = max_target_velocity;
      if (landmark_type == "1000001") {  // Traffic light
        minimum_velocity = TL_TARGET_VELOCITY;
      } else if (landmark_type == "206") {  // Stop
        minimum_velocity = STOP_TARGET_VELOCITY;
      } else if (landmark_type == "205") {  // Yield
        minimum_velocity = YIELD_TARGET_VELOCITY;
      } else if (landmark_type == "274") {  // Speed limit
        float value = static_cast<float>(landmark->GetValue()) / 3.6f;
        value = parameters.GetVehicleTargetVelocity(actor_id, value);
        minimum_velocity = (value < max_target_velocity) ? value : max_target_velocity;
      } else {
        continue;
      }

      float v = std::max(((max_target_velocity - minimum_velocity) / max_distance) * distance + minimum_velocity, minimum_velocity);
      landmark_target_velocity = std::min(landmark_target_velocity, v);
    }

    return landmark_target_velocity;
}

float MotionPlanStage::GetTurnTargetVelocity(const Buffer &waypoint_buffer,
                                             float max_target_velocity) {

  if (waypoint_buffer.size() < 3) {
    return max_target_velocity;
  }
  else {
    const SimpleWaypointPtr first_waypoint = waypoint_buffer.front();
    const SimpleWaypointPtr last_waypoint = waypoint_buffer.back();
    const SimpleWaypointPtr middle_waypoint = waypoint_buffer.at(waypoint_buffer.size() / 2);

    float radius = GetThreePointCircleRadius(first_waypoint->GetLocation(),
                                             middle_waypoint->GetLocation(),
                                             last_waypoint->GetLocation());

    // Return the max velocity at the turn
    return std::sqrt(radius * FRICTION * GRAVITY);
  }
}

void MotionPlanStage::RemoveActor(const ActorId actor_id) {
  pid_state_map.erase(actor_id);
  teleportation_instance.erase(actor_id);
}

void MotionPlanStage::Reset() {
  pid_state_map.clear();
  teleportation_instance.clear();
}

} // namespace traffic_manager
} // namespace carla
