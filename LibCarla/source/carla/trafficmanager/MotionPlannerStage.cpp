// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MotionPlannerStage.h"

namespace carla {
namespace traffic_manager {

namespace PlannerConstants {

  static const float HIGHWAY_SPEED = 50.0f / 3.6f;

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

    // Initializing messenger states.
    localization_messenger_state = localization_messenger->GetState();
    collision_messenger_state = collision_messenger->GetState();
    traffic_light_messenger_state = traffic_light_messenger->GetState();
    // Initializing this messenger to preemptively write since it precedes
    // batch control stage.
    control_messenger_state = control_messenger->GetState() - 1;

    // Initializing number of vehicles to zero in the beginning.
    number_of_vehicles = 0u;
  }

  MotionPlannerStage::~MotionPlannerStage() {}

  void MotionPlannerStage::Action() {

    // Selecting an output frame.
    const auto current_control_frame = frame_selector ? control_frame_a : control_frame_b;

    // Looping over all vehicles.
    for (uint64_t i = 0u; i < number_of_vehicles; ++i) {

      const LocalizationToPlannerData &localization_data = localization_frame->at(i);
      const Actor actor = localization_data.actor;
      const float current_deviation = localization_data.deviation;
      const float current_distance = localization_data.distance;

      const ActorId actor_id = actor->GetId();

      const auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
      const float current_velocity = vehicle->GetVelocity().Length();

      const auto current_time = chr::system_clock::now();

      if (pid_state_map.find(actor_id) == pid_state_map.end()) {
        const auto initial_state = StateEntry{0.0f, 0.0f, 0.0f, chr::system_clock::now(), 0.0f, 0.0f, 0.0f};
        pid_state_map.insert({actor_id, initial_state});
      }

      // Retrieving the previous state.
      traffic_manager::StateEntry previous_state;
      previous_state = pid_state_map.at(actor_id);

      // Increase speed if on highway.
      const float speed_limit = vehicle->GetSpeedLimit() / 3.6f;

      const float dynamic_target_velocity = parameters.GetVehicleTargetVelocity(actor) / 3.6f;

      if (speed_limit > HIGHWAY_SPEED) {
        longitudinal_parameters = highway_longitudinal_parameters;
        lateral_parameters = highway_lateral_parameters;
      } else {
        longitudinal_parameters = urban_longitudinal_parameters;
        lateral_parameters = urban_lateral_parameters;
      }

      // State update for vehicle.
      StateEntry current_state = controller.StateUpdate(
          previous_state,
          current_velocity,
          dynamic_target_velocity,
          current_deviation,
          current_distance,
          current_time);

      // Controller actuation.
      ActuationSignal actuation_signal = controller.RunStep(
          current_state,
          previous_state,
          longitudinal_parameters,
          lateral_parameters);

      // In case of collision or traffic light
      if ((collision_frame != nullptr && traffic_light_frame != nullptr) &&
          ((collision_messenger_state != 0 && collision_frame->at(i).hazard) ||
          (traffic_light_messenger_state != 0 &&
          traffic_light_frame->at(i).traffic_light_hazard))) {

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
      message.actor_id = actor_id;
      message.throttle = actuation_signal.throttle;
      message.brake = actuation_signal.brake;
      message.steer = actuation_signal.steer;
    }
  }

  void MotionPlannerStage::DataReceiver() {

    const auto localization_packet = localization_messenger->ReceiveData(localization_messenger_state);
    localization_frame = localization_packet.data;
    localization_messenger_state = localization_packet.id;

    // Block on receive call only if new data is available on the messenger.
    const int collision_messenger_current_state = collision_messenger->GetState();
    if (collision_messenger_current_state != collision_messenger_state) {
      const auto collision_packet = collision_messenger->ReceiveData(collision_messenger_state);
      collision_frame = collision_packet.data;
      collision_messenger_state = collision_packet.id;
    }

    // Block on receive call only if new data is available on the messenger.
    const int traffic_light_messenger_current_state = traffic_light_messenger->GetState();
    if (traffic_light_messenger_current_state != traffic_light_messenger_state) {
      const auto traffic_light_packet = traffic_light_messenger->ReceiveData(traffic_light_messenger_state);
      traffic_light_frame = traffic_light_packet.data;
      traffic_light_messenger_state = traffic_light_packet.id;
    }

    // Allocating new containers for the changed number of registered vehicles.
    if (localization_frame != nullptr &&
        number_of_vehicles != (*localization_frame.get()).size()) {

      number_of_vehicles = static_cast<uint>((*localization_frame.get()).size());
      // Allocate output frames.
      control_frame_a = std::make_shared<PlannerToControlFrame>(number_of_vehicles);
      control_frame_b = std::make_shared<PlannerToControlFrame>(number_of_vehicles);
    }
  }

  void MotionPlannerStage::DataSender() {

    DataPacket<std::shared_ptr<PlannerToControlFrame>> data_packet = {
        control_messenger_state,
        frame_selector ? control_frame_a : control_frame_b
      };
    frame_selector = !frame_selector;
    control_messenger_state = control_messenger->SendData(data_packet);
  }

  void MotionPlannerStage::DrawPIDValues(const boost::shared_ptr<cc::Vehicle> vehicle, const float throttle, const float brake) {
    debug_helper.DrawString(vehicle->GetLocation() + cg::Location(0.0f,0.0f,2.0f), std::to_string(throttle), false, {0u, 255u, 0u}, 0.005f);
    debug_helper.DrawString(vehicle->GetLocation() + cg::Location(0.0f,0.0f,4.0f), std::to_string(brake), false, {255u, 0u, 0u}, 0.005f);
  }

} // namespace traffic_manager
} // namespace carla
