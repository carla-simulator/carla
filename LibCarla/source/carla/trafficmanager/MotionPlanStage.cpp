
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
    local_map(local_map) {}

void MotionPlanStage::Update(const unsigned long index) {
  const ActorId actor_id = vehicle_id_list.at(index);
  const cg::Location vehicle_location = simulation_state.GetLocation(actor_id);
  const cg::Vector3D vehicle_velocity = simulation_state.GetVelocity(actor_id);
  const float vehicle_speed = vehicle_velocity.Length();
  const cg::Vector3D vehicle_heading = simulation_state.GetHeading(actor_id);
  const bool vehicle_physics_enabled = simulation_state.IsPhysicsEnabled(actor_id);
  const Buffer &waypoint_buffer = buffer_map.at(actor_id);
  const LocalizationData &localization = localization_frame.at(index);
  const CollisionHazardData &collision_hazard = collision_frame.at(index);
  const bool &tl_hazard = tl_frame.at(index);
  current_timestamp = world.GetSnapshot().GetTimestamp();
  StateEntry current_state;

  // Instanciating teleportation transform.
  cg::Transform teleportation_transform;

  // Get information about the hero location from the actor_id state.
  cg::Location hero_location = track_traffic.GetHeroLocation();
  bool is_hero_alive = hero_location != cg::Location(0, 0, 0);

  if (simulation_state.IsDormant(actor_id) && parameters.GetRespawnDormantVehicles() && is_hero_alive) {
    // Flushing controller state for vehicle.
    current_state = {current_timestamp,
                    0.0f, 0.0f,
                    0.0f, 0.0f};

    // Add entry to teleportation duration clock table if not present.
    if (teleportation_instance.find(actor_id) == teleportation_instance.end()) {
      teleportation_instance.insert({actor_id, current_timestamp});
    }
    // std::cout << "HERO LOC x: " << hero_location.x << " y: " << hero_location.y << " z: " << hero_location.z << std::endl;

    // Get lower and upper bound for teleporting vehicle.
    float lower_bound = parameters.GetLowerBoundaryRespawnDormantVehicles();
    float upper_bound = parameters.GetUpperBoundaryRespawnDormantVehicles();
    float dilate_factor = (upper_bound-lower_bound)/100.0f;

    // Measuring time elapsed since last teleportation for the vehicle.
    double elapsed_time = current_timestamp.elapsed_seconds - teleportation_instance.at(actor_id).elapsed_seconds;

    if (parameters.GetSynchronousMode() || elapsed_time > HYBRID_MODE_DT) {
      double r_sample = (random_devices.at(actor_id).next()*dilate_factor) + lower_bound;
      // int x_sign = random_devices.at(actor_id).next() < 50.0 ? -1 : 1;
      // int y_sign = random_devices.at(actor_id).next() < 50.0 ? -1 : 1;
      NodeList teleport_waypoint_list = local_map->GetWaypointsInDelta(hero_location, 5, r_sample);
      teleportation_transform = teleport_waypoint_list.at(0)->GetTransform();
      for (auto &teleport_waypoint : teleport_waypoint_list) {
        if (track_traffic.IsWaypointFree(teleport_waypoint->GetId())) {
          teleportation_transform = teleport_waypoint->GetTransform();
          track_traffic.AddTakenWaypoint(teleport_waypoint->GetId());
          // std::cout << "We found it!" << std::endl;
          break;
        } else {
          // std::cout << "Grid is occupied. THE STRUCT IN MPStage DOESNT GET CLEANED." << std::endl;
        }
      }
      // cg::Location teleport_location = hero_location + cg::Location(r_sample*x_sign, r_sample*y_sign, 0.0);
      // SimpleWaypointPtr teleport_waypoint = local_map->GetWaypoint(teleport_location);
      // // double inner = 50.0f;
      // // double outer = random_devices.at(actor_id).next()*2.0 + inner;
      // // SimpleWaypointPtr teleport_waypoint = local_map->GetWaypointInDistance(hero_location, inner, outer);
      // while (teleport_waypoint->CheckJunction()) {
      //   r_sample = (random_devices.at(actor_id).next()*dilate_factor) + lower_bound;
      //   x_sign = random_devices.at(actor_id).next() < 50.0 ? -1 : 1;
      //   y_sign = random_devices.at(actor_id).next() < 50.0 ? -1 : 1;
      //   teleport_location = hero_location + cg::Location(r_sample*x_sign, r_sample*y_sign, 0.0);
      //   teleport_waypoint = local_map->GetWaypoint(teleport_location);
      // }
      // SimpleWaypointPtr nearby_waypoint = nullptr;
      // NodeList possible_waypoints;
      // possible_waypoints.push_back(teleport_waypoint);
      // int elements = 1;
      // while (true) {
      //  nearby_waypoint = teleport_waypoint->GetLeftWaypoint();
      //  while (nearby_waypoint != nullptr) {
      //    possible_waypoints.push_back(nearby_waypoint);
      //     elements+=1;
      //    nearby_waypoint = nearby_waypoint->GetLeftWaypoint();
      //  }
      //  nearby_waypoint = teleport_waypoint->GetRightWaypoint();
      //  while (nearby_waypoint != nullptr) {
      //    possible_waypoints.push_back(nearby_waypoint);
      //    elements+=1;
      //    nearby_waypoint = nearby_waypoint->GetRightWaypoint();
      //  }
      //  break;
      // }
      // int selection_index = static_cast<int>(random_devices.at(actor_id).next()) % elements;
      // teleportation_transform = possible_waypoints.at(selection_index)->GetTransform();
    } else {
      // Teleport only once every dt in asynchronous mode.
      teleportation_transform = cg::Transform(vehicle_location, simulation_state.GetRotation(actor_id));
    }
    output_array.at(index) = carla::rpc::Command::ApplyTransform(actor_id, teleportation_transform);
  }

  else {

    // Target velocity for vehicle.
    const float vehicle_speed_limit = simulation_state.GetSpeedLimit(actor_id);
    float max_target_velocity = parameters.GetVehicleTargetVelocity(actor_id, vehicle_speed_limit) / 3.6f;

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
                                                  TARGET_WAYPOINT_HORIZON_LENGTH);
      const SimpleWaypointPtr &target_waypoint = GetTargetWaypoint(waypoint_buffer, target_point_distance).first;
      const cg::Location target_location = target_waypoint->GetLocation();
      float dot_product = DeviationDotProduct(vehicle_location, vehicle_heading, target_location);
      float cross_product = DeviationCrossProduct(vehicle_location, vehicle_heading, target_location);
      dot_product = 1.0f - dot_product;
      if (cross_product < 0.0f) {
        dot_product *= -1.0f;
      }
      const float current_deviation = dot_product;
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
      if (vehicle_speed > HIGHWAY_SPEED) {
        longitudinal_parameters = highway_longitudinal_parameters;
        lateral_parameters = highway_lateral_parameters;
      } else {
        longitudinal_parameters = urban_longitudinal_parameters;
        lateral_parameters = urban_lateral_parameters;
      }

      // If physics is enabled for the vehicle, use PID controller.
      // State update for vehicle.
      current_state = PID::StateUpdate(previous_state, vehicle_speed, dynamic_target_velocity,
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

      // Constructing the actuation signal.

      carla::rpc::VehicleControl vehicle_control;
      vehicle_control.throttle = actuation_signal.throttle;
      vehicle_control.brake = actuation_signal.brake;
      vehicle_control.steer = actuation_signal.steer;

      output_array.at(index) = carla::rpc::Command::ApplyVehicleControl(actor_id, vehicle_control);

      // Updating PID state.
      StateEntry &state = pid_state_map.at(actor_id);
      state = current_state;

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
                                                          const cg::Vector3D vehicle_velocity,
                                                          const cg::Vector3D vehicle_heading,
                                                          const float max_target_velocity) {
  bool collision_emergency_stop = false;
  float dynamic_target_velocity = max_target_velocity;

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
      float follow_lead_distance = vehicle_relative_speed * FOLLOW_DISTANCE_RATE + MIN_FOLLOW_LEAD_DISTANCE;
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
