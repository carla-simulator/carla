// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
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
  const SimulationState &simulation_state,
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
  RandomGeneratorMap &random_devices,
  const LocalMapPtr &local_map)
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
    random_devices(random_devices),
    local_map(local_map) {

      // Adding structure to avoid retrieving traffic lights when checking for landmarks.
      std::vector<SharedPtr<cc::Landmark>> traffic_lights = world.GetMap()->GetAllLandmarksOfType("1000001");
      for (auto &tl : traffic_lights) {
        std::string landmark_id = tl->GetId();
        SharedPtr<cc::Actor> actor = world.GetTrafficLight(*tl);
        tl_map.insert({landmark_id, actor});
      }
    }

void MotionPlanStage::Update(const unsigned long index) {
  const ActorId actor_id = vehicle_id_list.at(index);
  const cg::Location vehicle_location = simulation_state.GetLocation(actor_id);
  const cg::Vector3D vehicle_velocity = simulation_state.GetVelocity(actor_id);
  const cg::Rotation vehicle_rotation = simulation_state.GetRotation(actor_id);
  const float vehicle_speed = vehicle_velocity.Length();
  const cg::Vector3D vehicle_heading = simulation_state.GetHeading(actor_id);
  const bool vehicle_physics_enabled = simulation_state.IsPhysicsEnabled(actor_id);
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
      float random_sample = (static_cast<float>(random_devices.at(actor_id).next())*dilate_factor) + lower_bound;
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
  }

  else {

    // Target velocity for vehicle.
    const float vehicle_speed_limit = simulation_state.GetSpeedLimit(actor_id);
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

      const float target_point_distance = std::max(vehicle_speed * TARGET_WAYPOINT_TIME_HORIZON,
                                                  MIN_TARGET_WAYPOINT_DISTANCE);
      const SimpleWaypointPtr &target_waypoint = GetTargetWaypoint(waypoint_buffer, target_point_distance).first;
      const cg::Location target_location = target_waypoint->GetLocation();
      float dot_product = DeviationDotProduct(vehicle_location, vehicle_heading, target_location);
      float cross_product = DeviationCrossProduct(vehicle_location, vehicle_heading, target_location);
      dot_product = acos(dot_product) / PI;
      if (cross_product < 0.0f) {
        dot_product *= -1.0f;
      }
      const float angular_deviation = dot_product;
      const float velocity_deviation = (dynamic_target_velocity - vehicle_speed) / dynamic_target_velocity;
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
        auto landmark_id = landmark->GetId();
        if (tl_map.find(landmark_id) != tl_map.end()) {
          auto actor = tl_map.at(landmark_id);
          if (actor != nullptr) {

            cc::TrafficLight* tl = static_cast<cc::TrafficLight*>(actor.get());
            auto state = tl->GetState();

            if (state == carla::rpc::TrafficLightState::Green) {
              minimum_velocity = TL_GREEN_TARGET_VELOCITY;
            } else if (state == carla::rpc::TrafficLightState::Yellow || state == carla::rpc::TrafficLightState::Red){
              minimum_velocity = TL_RED_TARGET_VELOCITY;
            } else if (state == carla::rpc::TrafficLightState::Unknown){
              minimum_velocity = TL_UNKNOWN_TARGET_VELOCITY;
            } else {
              // Traffic light is off
              continue;
            }
          } else {
            // It is a traffic light, but it's not present in our structure
            minimum_velocity = TL_UNKNOWN_TARGET_VELOCITY;
          }
        } else {
          // It is a traffic light, but it's not present in our structure
          minimum_velocity = TL_UNKNOWN_TARGET_VELOCITY;
        }
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
    const SimpleWaypointPtr middle_waypoint = waypoint_buffer.at(static_cast<uint16_t>(waypoint_buffer.size() / 2));

    float radius = GetThreePointCircleRadius(first_waypoint->GetLocation(),
                                             middle_waypoint->GetLocation(),
                                             last_waypoint->GetLocation());

    // Return the max velocity at the turn
    return std::sqrt(radius * FRICTION * GRAVITY);
  }
}

float MotionPlanStage::GetThreePointCircleRadius(cg::Location first_location,
                                                 cg::Location middle_location,
                                                 cg::Location last_location) {

    float x1 = first_location.x;
    float y1 = first_location.y;
    float x2 = middle_location.x;
    float y2 = middle_location.y;
    float x3 = last_location.x;
    float y3 = last_location.y;

    float x12 = x1 - x2;
    float x13 = x1 - x3;
    float y12 = y1 - y2;
    float y13 = y1 - y3;
    float y31 = y3 - y1;
    float y21 = y2 - y1;
    float x31 = x3 - x1;
    float x21 = x2 - x1;

    float sx13 = x1 * x1 - x3 * x3;
    float sy13 = y1 * y1 - y3 * y3;
    float sx21 = x2 * x2 - x1 * x1;
    float sy21 = y2 * y2 - y1 * y1;

    float f_denom = 2 * (y31 * x12 - y21 * x13);
    if (f_denom == 0) {
      return std::numeric_limits<float>::max();
    }
    float f = (sx13 * x12 + sy13 * x12 + sx21 * x13 + sy21 * x13) / f_denom;

    float g_denom = 2 * (x31 * y12 - x21 * y13);
    if (g_denom == 0) {
      return std::numeric_limits<float>::max();
    }
    float g = (sx13 * y12 + sy13 * y12 + sx21 * y13 + sy21 * y13) / g_denom;

    float c = - (x1 * x1 + y1 * y1) - 2 * g * x1 - 2 * f * y1;
    float h = -g;
    float k = -f;

  return std::sqrt(h * h + k * k - c);
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
