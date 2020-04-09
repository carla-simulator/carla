// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/MotionPlannerStage.h"

namespace carla {
namespace traffic_manager {

namespace PlannerConstants {

  static const float HIGHWAY_SPEED = 50.0f / 3.6f;
  static const float RELATIVE_APPROACH_SPEED = 10.0f / 3.6f;
  static const float MIN_FOLLOW_LEAD_DISTANCE = 5.0f;
  static const float MAX_FOLLOW_LEAD_DISTANCE = 10.0f;
  static const float ARBITRARY_MAX_SPEED = 100.0f / 3.6f;
  static const float FOLLOW_DISTANCE_RATE = (MAX_FOLLOW_LEAD_DISTANCE-MIN_FOLLOW_LEAD_DISTANCE)/ARBITRARY_MAX_SPEED;
  static const float CRITICAL_BRAKING_MARGIN = 0.25f;
  static const float HYBRID_MODE_DT = 0.05f;
  static const float EPSILON_VELOCITY = 0.001f;
} // namespace PlannerConstants

  using namespace PlannerConstants;

  MotionPlannerStage::MotionPlannerStage(
      std::string stage_name,
      std::shared_ptr<LocalizationToPlannerMessenger> localization_messenger,
      std::shared_ptr<CollisionToPlannerMessenger> collision_messenger,
      std::shared_ptr<TrafficLightToPlannerMessenger> traffic_light_messenger,
      std::shared_ptr<PlannerToControlMessenger> control_messenger,
      Parameters &parameters,
      std::vector<float> urban_longitudinal_parameters,
      std::vector<float> highway_longitudinal_parameters,
      std::vector<float> urban_lateral_parameters,
      std::vector<float> highway_lateral_parameters,
      cc::DebugHelper &debug_helper)
    : PipelineStage(stage_name),
      localization_messenger(localization_messenger),
      collision_messenger(collision_messenger),
      traffic_light_messenger(traffic_light_messenger),
      control_messenger(control_messenger),
      parameters(parameters),
      urban_longitudinal_parameters(urban_longitudinal_parameters),
      highway_longitudinal_parameters(highway_longitudinal_parameters),
      urban_lateral_parameters(urban_lateral_parameters),
      highway_lateral_parameters(highway_lateral_parameters),
      debug_helper(debug_helper){

    // Initializing the output frame selector.
    frame_selector = true;

    // Initializing number of vehicles to zero in the beginning.
    number_of_vehicles = 0u;
  }

  MotionPlannerStage::~MotionPlannerStage() {}

  void MotionPlannerStage::Action() {

    // Selecting an output frame.
    const auto current_control_frame = frame_selector ? control_frame_a : control_frame_b;

    // Looping over all vehicles.
    for (uint64_t i = 0u;
         i < number_of_vehicles &&
         localization_frame != nullptr &&
         collision_frame != nullptr &&
         traffic_light_frame != nullptr;
         ++i) {

      const LocalizationToPlannerData &localization_data = localization_frame->at(i);
      const CollisionToPlannerData& collision_data = collision_frame->at(i);

      if (!localization_data.actor->IsAlive()) {
        continue;
      }

      const Actor actor = localization_data.actor;
      const float current_deviation = localization_data.deviation;
      const float current_distance = localization_data.distance;
      const cg::Vector3D current_velocity_vector = localization_data.velocity;
      const float current_velocity = current_velocity_vector.Length();

      const ActorId actor_id = actor->GetId();
      const cg::Vector3D ego_heading = actor->GetTransform().GetForwardVector();
      const auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);

      const auto current_time = chr::system_clock::now();

      // If previous state for vehicle not found, initialize state entry.
      if (pid_state_map.find(actor_id) == pid_state_map.end()) {
        const auto initial_state = StateEntry{0.0f, 0.0f, 0.0f, chr::system_clock::now(), 0.0f, 0.0f, 0.0f};
        pid_state_map.insert({actor_id, initial_state});
      }

      // Retrieving the previous state.
      traffic_manager::StateEntry previous_state;
      previous_state = pid_state_map.at(actor_id);

      // Change PID parameters if on highway.
      if (current_velocity > HIGHWAY_SPEED) {
        longitudinal_parameters = highway_longitudinal_parameters;
        lateral_parameters = highway_lateral_parameters;
      } else {
        longitudinal_parameters = urban_longitudinal_parameters;
        lateral_parameters = urban_lateral_parameters;
      }

      // Target velocity for vehicle.
      float max_target_velocity = parameters.GetVehicleTargetVelocity(actor) / 3.6f;
      float dynamic_target_velocity = max_target_velocity;
      //////////////////////// Collision related data handling ///////////////////////////
      bool collision_emergency_stop = false;
      if (collision_data.hazard) {
        cg::Vector3D other_vehicle_velocity = collision_data.other_vehicle_velocity;
        float ego_relative_velocity =  (current_velocity_vector - other_vehicle_velocity).Length();
        float other_velocity_along_heading = cg::Math::Dot(other_vehicle_velocity, ego_heading);

        // Consider collision avoidance decisions only if there is positive relative velocity
        // of the ego vehicle (meaning, ego vehicle is closing the gap to the lead vehicle).
        if (ego_relative_velocity > EPSILON_VELOCITY) {
          // If other vehicle is approaching lead vehicle and lead vehicle is further
          // than follow_lead_distance 0 kmph -> 5m, 100 kmph -> 10m.
          float follow_lead_distance = ego_relative_velocity * FOLLOW_DISTANCE_RATE + MIN_FOLLOW_LEAD_DISTANCE;
          if (collision_data.distance_to_other_vehicle > follow_lead_distance) {
            // Then reduce the gap between the vehicles till FOLLOW_LEAD_DISTANCE
            // by maintaining a relative speed of RELATIVE_APPROACH_SPEED
            dynamic_target_velocity = other_velocity_along_heading + RELATIVE_APPROACH_SPEED;
          }
          // If vehicle is approaching a lead vehicle and the lead vehicle is further
          // than CRITICAL_BRAKING_MARGIN but closer than FOLLOW_LEAD_DISTANCE.
          else if (collision_data.distance_to_other_vehicle > CRITICAL_BRAKING_MARGIN) {
            // Then follow the lead vehicle by acquiring it's speed along current heading.
            dynamic_target_velocity = std::max(other_velocity_along_heading, RELATIVE_APPROACH_SPEED);
          } else {
            // If lead vehicle closer than CRITICAL_BRAKING_MARGIN, initiate emergency stop.
            collision_emergency_stop = true;
          }
        }
        if (collision_data.distance_to_other_vehicle < CRITICAL_BRAKING_MARGIN) {
          collision_emergency_stop = true;
        }
      }
      ///////////////////////////////////////////////////////////////////////////////////

      // Clip dynamic target velocity to maximum allowed speed for the vehicle.
      dynamic_target_velocity = std::min(max_target_velocity, dynamic_target_velocity);

      bool emergency_stop = false;
      // In case of collision or traffic light hazard.
      if (traffic_light_frame->at(i).traffic_light_hazard
          || collision_emergency_stop) {
      emergency_stop = true;
      }

      // Message items to be sent to batch control stage.
      ActuationSignal actuation_signal {0.0f, 0.0f, 0.0f};
      bool physics_enabled = true;
      cg::Transform teleportation_transform;

      // If physics is enabled for the vehicle, use PID controller.
      StateEntry current_state;
      if (localization_data.physics_enabled) {

        // State update for vehicle.
        current_state = controller.StateUpdate(previous_state, current_velocity,
                                               dynamic_target_velocity, current_deviation,
                                               current_distance, current_time);

        // Controller actuation.
        actuation_signal = controller.RunStep(current_state, previous_state,
                                              longitudinal_parameters, lateral_parameters);

        if (emergency_stop) {

          current_state.deviation_integral = 0.0f;
          current_state.velocity_integral = 0.0f;
          actuation_signal.throttle = 0.0f;
          actuation_signal.brake = 1.0f;
        }

      }
      // For physics-less vehicles, determine position and orientation for teleportation.
      else if (hybrid_physics_mode) {

        physics_enabled = false;

        // Flushing controller state for vehicle.
        current_state = {0.0f, 0.0f, 0.0f,
                         chr::system_clock::now(),
                         0.0f, 0.0f, 0.0f};

        // Add entry to teleportation duration clock table if not present.
        if (teleportation_instance.find(actor_id) == teleportation_instance.end()) {
          teleportation_instance.insert({actor_id, chr::system_clock::now()});
        }

        // Measuring time elapsed since last teleportation for the vehicle.
        chr::duration<float> elapsed_time = current_time - teleportation_instance.at(actor_id);

        // Find a location ahead of the vehicle for teleportation to achieve intended velocity.
        if (!emergency_stop && (parameters.GetSynchronousMode() || elapsed_time.count() > HYBRID_MODE_DT)) {

          // Target displacement magnitude to achieve target velocity.
          const float target_displacement = dynamic_target_velocity * HYBRID_MODE_DT;
          const float target_displacement_square = std::pow(target_displacement, 2.0f);

          SimpleWaypointPtr target_interval_begin = nullptr;
          SimpleWaypointPtr target_interval_end = nullptr;
          bool teleportation_interval_found = false;
          cg::Location vehicle_location = actor->GetLocation();

          // Find the interval containing position to achieve target displacement.
          for (uint32_t j = 0u;
                j+1 < localization_data.position_window.size() && !teleportation_interval_found;
                ++j) {

            target_interval_begin = localization_data.position_window.at(j);
            target_interval_end = localization_data.position_window.at(j+1);

            cg::Vector3D relative_position = target_interval_begin->GetLocation() - vehicle_location;
            if (cg::Math::Dot(relative_position, ego_heading) > 0.0f
                && ((target_interval_begin->DistanceSquared(vehicle_location) > target_displacement_square)
                    || (target_interval_begin->DistanceSquared(vehicle_location) < target_displacement_square
                        && target_interval_end->DistanceSquared(vehicle_location) > target_displacement_square))) {
              teleportation_interval_found = true;
            }
          }

          if (target_interval_begin != nullptr && target_interval_end != nullptr) {

            // Construct target transform to accurately achieve desired velocity.
            float missing_displacement = 0.0f;
            const float base_displacement = target_interval_begin->Distance(vehicle_location);
            if (base_displacement < target_displacement) {
              missing_displacement = target_displacement - base_displacement;
            }
            cg::Transform target_base_transform = target_interval_begin->GetTransform();
            cg::Location target_base_location = target_base_transform.location;
            cg::Vector3D target_heading = target_base_transform.GetForwardVector();
            cg::Location teleportation_location = target_base_location
                                                  + cg::Location(target_heading * missing_displacement);
            teleportation_transform = cg::Transform(teleportation_location, target_base_transform.rotation);

          } else {

            teleportation_transform = actor->GetTransform();

          }

        }
        // In case of an emergency stop, stay in the same location.
        // Also, teleport only once every dt in asynchronous mode.
        else {

          teleportation_transform = actor->GetTransform();

        }

      }

      // Updating PID state.
      StateEntry &state = pid_state_map.at(actor_id);
      state = current_state;

      // Constructing the actuation signal.
      PlannerToControlData &message = current_control_frame->at(i);
      message.actor = actor;
      message.throttle = actuation_signal.throttle;
      message.brake = actuation_signal.brake;
      message.steer = actuation_signal.steer;
      message.physics_enabled = physics_enabled;
      message.transform = teleportation_transform;
    }
  }

  void MotionPlannerStage::DataReceiver() {

    localization_frame = localization_messenger->Peek();
    collision_frame = collision_messenger->Peek();
    traffic_light_frame = traffic_light_messenger->Peek();

    // Allocating new containers for the changed number of registered vehicles.
    if (localization_frame != nullptr &&
        number_of_vehicles != (*localization_frame.get()).size()) {

      number_of_vehicles = static_cast<uint64_t>((*localization_frame.get()).size());
      // Allocate output frames.
      control_frame_a = std::make_shared<PlannerToControlFrame>(number_of_vehicles);
      control_frame_b = std::make_shared<PlannerToControlFrame>(number_of_vehicles);
    }

    hybrid_physics_mode = parameters.GetHybridPhysicsMode();
  }

  void MotionPlannerStage::DataSender() {

    localization_messenger->Pop();
    collision_messenger->Pop();
    traffic_light_messenger->Pop();

    control_messenger->Push(frame_selector ? control_frame_a : control_frame_b);
    frame_selector = !frame_selector;
  }

  void MotionPlannerStage::DrawPIDValues(const boost::shared_ptr<cc::Vehicle> vehicle,
                                         const float throttle, const float brake) {
    auto vehicle_location = vehicle->GetLocation();
    debug_helper.DrawString(vehicle_location + cg::Location(0.0f,0.0f,2.0f),
                            std::to_string(throttle), false, {0u, 255u, 0u}, 0.005f);
    debug_helper.DrawString(vehicle_location + cg::Location(0.0f,0.0f,4.0f),
                            std::to_string(brake), false, {255u, 0u, 0u}, 0.005f);
  }

} // namespace traffic_manager
} // namespace carla
