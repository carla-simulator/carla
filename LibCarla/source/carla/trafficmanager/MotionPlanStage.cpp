
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

MotionPlanStage::MotionPlanStage(
  const std::vector<ActorId> &vehicle_id_list,
  const SimulationState &simulation_state,
  const Parameters &parameters,
  const BufferMap &buffer_map,
  const TrackTraffic &track_traffic,
  const std::vector<float> &urban_longitudinal_parameters,
  const std::vector<float> &highway_longitudinal_parameters,
  const std::vector<float> &urban_lateral_parameters,
  const std::vector<float> &highway_lateral_parameters,
  const LocalizationFrame &localization_frame,
  const CollisionFrame&collision_frame,
  const TLFrame &tl_frame,
  const cc::World &world,
  ControlFrame &output_array)
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
    output_array(output_array) {}

void MotionPlanStage::Update(const unsigned long index) {
  const ActorId actor_id = vehicle_id_list.at(index);
  const cg::Location ego_location = simulation_state.GetLocation(actor_id);
  const cg::Vector3D ego_velocity = simulation_state.GetVelocity(actor_id);
  const float ego_speed = ego_velocity.Length();
  const cg::Vector3D ego_heading = simulation_state.GetHeading(actor_id);
  const bool ego_physics_enabled = simulation_state.IsPhysicsEnabled(actor_id);
  const Buffer &waypoint_buffer = buffer_map.at(actor_id);
  const LocalizationData &localization = localization_frame.at(index);
  const CollisionHazardData &collision_hazard = collision_frame.at(index);
  const bool &tl_hazard = tl_frame.at(index);

  const float target_point_distance = std::max(ego_speed * TARGET_WAYPOINT_TIME_HORIZON,
                                               TARGET_WAYPOINT_HORIZON_LENGTH);
  const SimpleWaypointPtr &target_waypoint = GetTargetWaypoint(waypoint_buffer, target_point_distance).first;
  const cg::Location target_location = target_waypoint->GetLocation();
  float dot_product = DeviationDotProduct(ego_location, ego_heading, target_location);
  float cross_product = DeviationCrossProduct(ego_location, ego_heading, target_location);
  dot_product = 1.0f - dot_product;
  if (cross_product < 0.0f) {
    dot_product *= -1.0f;
  }
  const float current_deviation = dot_product;
  current_timestamp = world.GetSnapshot().GetTimestamp();

  // If previous state for vehicle not found, initialize state entry.
  if (pid_state_map.find(actor_id) == pid_state_map.end()) {
    const auto initial_state = StateEntry{current_timestamp, 0.0f, 0.0f, 0.0f, 0.0f};
    pid_state_map.insert({actor_id, initial_state});
  }

  // Retrieving the previous state.
  traffic_manager::StateEntry previous_state;
  previous_state = pid_state_map.at(actor_id);

  // Select PID parameters.
  std::vector<float> longitudinal_parameters;
  std::vector<float> lateral_parameters;
  if (ego_speed > HIGHWAY_SPEED) {
    longitudinal_parameters = highway_longitudinal_parameters;
    lateral_parameters = highway_lateral_parameters;
  } else {
    longitudinal_parameters = urban_longitudinal_parameters;
    lateral_parameters = urban_lateral_parameters;
  }

  // Target velocity for vehicle.
  const float ego_speed_limit = simulation_state.GetSpeedLimit(actor_id);
  float max_target_velocity = parameters.GetVehicleTargetVelocity(actor_id, ego_speed_limit) / 3.6f;

  // Collision handling and target velocity correction.
  std::pair<bool, float> collision_response = CollisionHandling(collision_hazard, tl_hazard, ego_velocity,
                                                                ego_heading, max_target_velocity);
  bool collision_emergency_stop = collision_response.first;
  float dynamic_target_velocity = collision_response.second;

  // Don't enter junction if there isn't enough free space after the junction.
  bool safe_after_junction = SafeAfterJunction(localization, tl_hazard, collision_emergency_stop);

  // In case of collision or traffic light hazard.
  bool emergency_stop = tl_hazard || collision_emergency_stop || !safe_after_junction;

  ActuationSignal actuation_signal{0.0f, 0.0f, 0.0f};
  cg::Transform teleportation_transform;

  // If physics is enabled for the vehicle, use PID controller.
  StateEntry current_state;
  if (ego_physics_enabled) {

    // State update for vehicle.
    current_state = PID::StateUpdate(previous_state, ego_speed, dynamic_target_velocity,
                                     current_deviation, current_timestamp);

    // Controller actuation.
    actuation_signal = PID::RunStep(current_state, previous_state,
                                    longitudinal_parameters, lateral_parameters);

    if (emergency_stop) {

      current_state.deviation_integral = 0.0f;
      current_state.velocity_integral = 0.0f;
      actuation_signal.throttle = 0.0f;
      actuation_signal.brake = 1.0f;
    }
  }
  // For physics-less vehicles, determine position and orientation for teleportation.
  else {
    // Flushing controller state for vehicle.
    current_state = {current_timestamp,
                     0.0f, 0.0f,
                     0.0f, 0.0f};

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
      const SimpleWaypointPtr teleport_target_waypoint = GetTargetWaypoint(waypoint_buffer, target_displacement).first;

      // Construct target transform to accurately achieve desired velocity.
      float missing_displacement = 0.0f;
      const float base_displacement = teleport_target_waypoint->Distance(ego_location);
      if (base_displacement < target_displacement) {
        missing_displacement = target_displacement - base_displacement;
      }
      cg::Transform target_base_transform = teleport_target_waypoint->GetTransform();
      cg::Location target_base_location = target_base_transform.location;
      cg::Vector3D target_heading = target_base_transform.GetForwardVector();
      cg::Location teleportation_location = target_base_location + cg::Location(target_heading * missing_displacement);
      teleportation_transform = cg::Transform(teleportation_location, target_base_transform.rotation);
    }
    // In case of an emergency stop, stay in the same location.
    // Also, teleport only once every dt in asynchronous mode.
    else {
      teleportation_transform = cg::Transform(ego_location, simulation_state.GetRotation(actor_id));
    }
  }

  // Updating PID state.
  StateEntry &state = pid_state_map.at(actor_id);
  state = current_state;

  // Constructing the actuation signal.
  if (ego_physics_enabled) {
    carla::rpc::VehicleControl vehicle_control;
    vehicle_control.throttle = actuation_signal.throttle;
    vehicle_control.brake = actuation_signal.brake;
    vehicle_control.steer = actuation_signal.steer;

    output_array.at(index) = carla::rpc::Command::ApplyVehicleControl(actor_id, vehicle_control);
  } else {
    output_array.at(index) = carla::rpc::Command::ApplyTransform(actor_id, teleportation_transform);
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

    ActorIdSet initial_set = track_traffic.GetPassingVehicles(junction_end_point->GetId());
    float safe_interval_length_squared = junction_end_point->DistanceSquared(safe_point);
    cg::Location mid_point = (junction_end_point->GetLocation() + safe_point->GetLocation())/2.0f;

    // Scan through the safe interval and find if any vehicles are present in it
    // by finding their occupied waypoints.
    for (SimpleWaypointPtr current_waypoint = junction_end_point;
         current_waypoint->DistanceSquared(junction_end_point) < safe_interval_length_squared && safe_after_junction;
         current_waypoint = current_waypoint->GetNextWaypoint().front()) {

      ActorIdSet current_set = track_traffic.GetPassingVehicles(current_waypoint->GetId());
      ActorIdSet difference;
      std::set_difference(current_set.begin(), current_set.end(),
                          initial_set.begin(), initial_set.end(),
                          std::inserter(difference, difference.begin()));
      if (difference.size() > 0) {
        for (const ActorId &blocking_id: difference) {
          cg::Location blocking_actor_location = simulation_state.GetLocation(blocking_id);
          if (cg::Math::DistanceSquared(blocking_actor_location, mid_point) < SQUARE(MAX_JUNCTION_BLOCK_DISTANCE)
              && simulation_state.GetVelocity(blocking_id).SquaredLength() < SQUARE(AFTER_JUNCTION_MIN_SPEED)) {
            safe_after_junction = false;
          }
        }
      }
    }
  }

  return safe_after_junction;
}

std::pair<bool, float> MotionPlanStage::CollisionHandling(const CollisionHazardData &collision_hazard,
                                                          const bool tl_hazard,
                                                          const cg::Vector3D ego_velocity,
                                                          const cg::Vector3D ego_heading,
                                                          const float max_target_velocity) {
  bool collision_emergency_stop = false;
  float dynamic_target_velocity = max_target_velocity;

  if (collision_hazard.hazard && !tl_hazard) {
    const ActorId other_actor_id = collision_hazard.hazard_actor_id;
    const cg::Vector3D other_velocity = simulation_state.GetVelocity(other_actor_id);
    const float ego_relative_speed = (ego_velocity - other_velocity).Length();
    const float available_distance_margin = collision_hazard.available_distance_margin;

    const float other_speed_along_heading = cg::Math::Dot(other_velocity, ego_heading);

    // Consider collision avoidance decisions only if there is positive relative velocity
    // of the ego vehicle (meaning, ego vehicle is closing the gap to the lead vehicle).
    if (ego_relative_speed > EPSILON_RELATIVE_SPEED) {
      // If other vehicle is approaching lead vehicle and lead vehicle is further
      // than follow_lead_distance 0 kmph -> 5m, 100 kmph -> 10m.
      float follow_lead_distance = ego_relative_speed * FOLLOW_DISTANCE_RATE + MIN_FOLLOW_LEAD_DISTANCE;
      if (available_distance_margin > follow_lead_distance) {
        // Then reduce the gap between the vehicles till FOLLOW_LEAD_DISTANCE
        // by maintaining a relative speed of RELATIVE_APPROACH_SPEED
        dynamic_target_velocity = other_speed_along_heading + RELATIVE_APPROACH_SPEED;
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

  dynamic_target_velocity = std::min(max_target_velocity, dynamic_target_velocity);

  return {collision_emergency_stop, dynamic_target_velocity};
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
