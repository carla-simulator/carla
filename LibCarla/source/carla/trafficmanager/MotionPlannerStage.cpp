#include "MotionPlannerStage.h"

namespace traffic_manager {

namespace PlannerConstants {
  static const float MAX_THROTTLE = 0.75f;
  static const float MAX_BRAKE = 1.0f;
  static const float HIGHWAY_SPEED = 50 / 3.6f;
  static const std::vector<float> URBAN_LONGITUDINAL_DEFAULTS = {0.1f, 0.15f, 0.01f};
  static const std::vector<float> HIGHWAY_LONGITUDINAL_DEFAULTS = {5.0f, 0.1f, 0.01f};
  static const std::vector<float> LATERAL_DEFAULTS = {10.0f, 0.0f, 0.1f};
}
  using namespace PlannerConstants;

  MotionPlannerStage::MotionPlannerStage(
      std::shared_ptr<LocalizationToPlannerMessenger> localization_messenger,
      std::shared_ptr<CollisionToPlannerMessenger> collision_messenger,
      std::shared_ptr<TrafficLightToPlannerMessenger> traffic_light_messenger,
      std::shared_ptr<PlannerToControlMessenger> control_messenger,
      Parameters &parameters,
      std::vector<float> longitudinal_parameters = URBAN_LONGITUDINAL_DEFAULTS,
      std::vector<float> highway_longitudinal_parameters = HIGHWAY_LONGITUDINAL_DEFAULTS,
      std::vector<float> lateral_parameters = LATERAL_DEFAULTS)
    : localization_messenger(localization_messenger),
      collision_messenger(collision_messenger),
      traffic_light_messenger(traffic_light_messenger),
      control_messenger(control_messenger),
      parameters(parameters),
      longitudinal_parameters(longitudinal_parameters),
      highway_longitudinal_parameters(highway_longitudinal_parameters),
      lateral_parameters(lateral_parameters) {

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
    auto current_control_frame = frame_selector ? control_frame_a : control_frame_b;

    // Looping over all vehicles.
    for (uint i = 0u; i < number_of_vehicles; ++i) {

      LocalizationToPlannerData &localization_data = localization_frame->at(i);
      Actor actor = localization_data.actor;
      float current_deviation = localization_data.deviation;
      ActorId actor_id = actor->GetId();

      auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
      float current_velocity = vehicle->GetVelocity().Length();
      auto current_time = chr::system_clock::now();

      if (pid_state_map.find(actor_id) == pid_state_map.end()) {
        auto initial_state = StateEntry{0.0f, 0.0f, chr::system_clock::now(), 0.0f, 0.0f};
        pid_state_map.insert({actor_id, initial_state});
      }

      // Retrieving the previous state.
      traffic_manager::StateEntry previous_state;
      previous_state = pid_state_map.at(actor_id);

      // Increase speed if on highway.
      float speed_limit = vehicle->GetSpeedLimit() / 3.6f;

      float dynamic_target_velocity = parameters.GetVehicleTargetVelocity(actor) / 3.6f;

      if (speed_limit > HIGHWAY_SPEED) {
        longitudinal_parameters = highway_longitudinal_parameters;
      }

      // State update for vehicle.
      StateEntry current_state = controller.StateUpdate(
          previous_state,
          current_velocity,
          dynamic_target_velocity,
          current_deviation,
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
        actuation_signal.brake = MAX_BRAKE;
      }

      // Updating PID state.
      StateEntry &state = pid_state_map.at(actor_id);
      state = current_state;

      // Constructing the actuation signal.
      PlannerToControlData &message = current_control_frame->at(i);
      message.actor_id = actor_id;
      message.throttle = std::min(actuation_signal.throttle, MAX_THROTTLE);
      message.brake = std::min(actuation_signal.brake, MAX_BRAKE);
      message.steer = actuation_signal.steer;

    }
  }

  void MotionPlannerStage::DataReceiver() {

    auto localization_packet = localization_messenger->ReceiveData(localization_messenger_state);
    localization_frame = localization_packet.data;
    localization_messenger_state = localization_packet.id;

    // Block on receive call only if new data is available on the messenger.
    int collision_messenger_current_state = collision_messenger->GetState();
    if (collision_messenger_current_state != collision_messenger_state) {
      auto collision_packet = collision_messenger->ReceiveData(collision_messenger_state);
      collision_frame = collision_packet.data;
      collision_messenger_state = collision_packet.id;
    }

    // Block on receive call only if new data is available on the messenger.
    int traffic_light_messenger_current_state = traffic_light_messenger->GetState();
    if (traffic_light_messenger_current_state != traffic_light_messenger_state) {
      auto traffic_light_packet = traffic_light_messenger->ReceiveData(traffic_light_messenger_state);
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
}
