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
  static const float STATIONARY_LEAD_APPROACH_SPEED_1 = 10.0f / 3.6f;
  static const float STATIONARY_LEAD_APPROACH_SPEED_2 = 5.0f / 3.6f;
  static const float CRITICAL_BRAKING_MARGIN_1 = 5.0f;
  static const float CRITICAL_BRAKING_MARGIN_2 = 0.1f;

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

      const ActorId actor_id = actor->GetId();

      const auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
      const cg::Vector3D current_velocity_vector = localization_data.velocity;
      const float current_velocity = current_velocity_vector.Length();

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
      float dynamic_target_velocity = parameters.GetVehicleTargetVelocity(actor) / 3.6f;

      //////////////////////// Collision related data handling ///////////////////////////
      if (collision_data.hazard)
      {
        cg::Vector3D other_vehicle_velocity = collision_data.other_vehicle_velocity;
        float ego_relative_velocity =  (current_velocity_vector - other_vehicle_velocity).Length();

        cg::Vector3D ego_heading = actor->GetTransform().GetForwardVector();
        float other_velocity_along_heading = cg::Math::Dot(other_vehicle_velocity, ego_heading);

        if (ego_relative_velocity > 0.0f
            && collision_data.distance_to_other_vehicle > CRITICAL_BRAKING_MARGIN_1)
        {
          dynamic_target_velocity = std::max(other_velocity_along_heading, STATIONARY_LEAD_APPROACH_SPEED_1);
        } else if (ego_relative_velocity > 0.0f
            && collision_data.distance_to_other_vehicle > CRITICAL_BRAKING_MARGIN_2)
        {
          dynamic_target_velocity = std::max(other_velocity_along_heading, STATIONARY_LEAD_APPROACH_SPEED_2);
        }
      }
      ///////////////////////////////////////////////////////////////////////////////////

      // State update for vehicle.
      StateEntry current_state = controller.StateUpdate(previous_state, current_velocity,
                                                        dynamic_target_velocity, current_deviation,
                                                        current_distance, current_time);

      // Controller actuation.
      ActuationSignal actuation_signal = controller.RunStep(current_state, previous_state,
                                                            longitudinal_parameters, lateral_parameters);

      // In case of traffic light hazard.
      if (traffic_light_frame->at(i).traffic_light_hazard
          || (collision_data.hazard
              && collision_data.distance_to_other_vehicle < CRITICAL_BRAKING_MARGIN_2)) {

        current_state.deviation_integral = 0.0f;
        current_state.velocity_integral = 0.0f;
        actuation_signal.throttle = 0.0f;
        actuation_signal.brake = 1.0f;
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
