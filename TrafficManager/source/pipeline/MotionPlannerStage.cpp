#include "MotionPlannerStage.h"

namespace traffic_manager {

namespace PlannerConstants {
  static const float HIGHWAY_SPEED = 50 / 3.6f;
  static const float INTERSECTION_APPROACH_SPEED = 15 / 3.6f;
}
  using namespace PlannerConstants;

  MotionPlannerStage::MotionPlannerStage(
      std::shared_ptr<LocalizationToPlannerMessenger> localization_messenger,
      std::shared_ptr<CollisionToPlannerMessenger> collision_messenger,
      std::shared_ptr<TrafficLightToPlannerMessenger> traffic_light_messenger,
      std::shared_ptr<PlannerToControlMessenger> control_messenger,
      uint number_of_vehicles,
      uint pool_size = 1u,
      float urban_target_velocity = 25 / 3.6f,
      float highway_target_velocity = 50 / 3.6f,
      std::vector<float> longitudinal_parameters = {
    0.1f, 0.15f, 0.01f
  },
      std::vector<float> highway_longitudinal_parameters = {5.0f, 0.0f, 0.1f},
      std::vector<float> lateral_parameters = {10.0f, 0.0f, 0.1f})
    : urban_target_velocity(urban_target_velocity),
      highway_target_velocity(highway_target_velocity),
      longitudinal_parameters(longitudinal_parameters),
      highway_longitudinal_parameters(highway_longitudinal_parameters),
      lateral_parameters(lateral_parameters),
      localization_messenger(localization_messenger),
      control_messenger(control_messenger),
      collision_messenger(collision_messenger),
      traffic_light_messenger(traffic_light_messenger),
      PipelineStage(pool_size, number_of_vehicles) {

    // Allocate and initialize vector to keep track of contoller states for all
    // vehicles
    pid_state_vector = std::make_shared<std::vector<StateEntry>>(number_of_vehicles);
    for (auto &entry: *pid_state_vector.get()) {
      entry.time_instance = chr::system_clock::now();
    }

    // Initialize output frame selector
    frame_selector = true;

    // Allocate output frames
    control_frame_a = std::make_shared<PlannerToControlFrame>(number_of_vehicles);
    control_frame_b = std::make_shared<PlannerToControlFrame>(number_of_vehicles);

    // Initialize messenger states
    localization_messenger_state = localization_messenger->GetState();
    collision_messenger_state = collision_messenger->GetState();
    traffic_light_messenger_state = traffic_light_messenger->GetState();
    // Initialize this messenger to preemptively write since it precedes
    // batch control stage
    control_messenger_state = control_messenger->GetState() - 1;
  }

  MotionPlannerStage::~MotionPlannerStage() {}

  void MotionPlannerStage::Action(const uint start_index, const uint end_index) {

    // Selecting output frame
    auto current_control_frame = frame_selector ? control_frame_a : control_frame_b;

    // Looping over arrays' partitions for current thread
    for (uint i = start_index; i <= end_index; ++i) {

      auto &localization_data = localization_frame->at(i);
      auto actor = localization_data.actor;
      float current_deviation = localization_data.deviation;
      auto actor_id = actor->GetId();

      auto vehicle = boost::static_pointer_cast<carla::client::Vehicle>(actor);
      float current_velocity = vehicle->GetVelocity().Length();
      auto current_time = chr::system_clock::now();

      // Retreiving previous state
      traffic_manager::StateEntry previous_state;
      previous_state = pid_state_vector->at(i);

      auto dynamic_target_velocity = urban_target_velocity;

      // Increase speed if on highway
      auto speed_limit = vehicle->GetSpeedLimit() / 3.6f;
      if (speed_limit > HIGHWAY_SPEED) {
        dynamic_target_velocity = highway_target_velocity;
        longitudinal_parameters = highway_longitudinal_parameters;
      }

      // Decrease speed approaching intersection
      if (localization_data.approaching_true_junction) {
        dynamic_target_velocity = INTERSECTION_APPROACH_SPEED;
      }

      // State update for vehicle
      auto current_state = controller.StateUpdate(
          previous_state,
          current_velocity,
          dynamic_target_velocity,
          current_deviation,
          current_time);

      // Controller actuation
      auto actuation_signal = controller.RunStep(
          current_state,
          previous_state,
          longitudinal_parameters,
          lateral_parameters);

      // In case of collision or traffic light or approaching a junction
      if ((collision_messenger_state != 0u && collision_frame->at(i).hazard) ||
          (traffic_light_messenger_state != 0u &&
          traffic_light_frame->at(i).traffic_light_hazard > 0.0f) ||
          (localization_data.approaching_true_junction &&
          current_velocity > INTERSECTION_APPROACH_SPEED)) {

        current_state.deviation_integral = 0.0f;
        current_state.velocity_integral = 0.0f;
        actuation_signal.throttle = 0.0f;
        actuation_signal.brake = 1.0f;
      }

      // Updating state
      auto &state = pid_state_vector->at(i);
      state = current_state;

      // Constructing actuation signal
      auto &message = current_control_frame->at(i);
      message.actor_id = actor_id;
      message.throttle = actuation_signal.throttle;
      message.brake = actuation_signal.brake;
      message.steer = actuation_signal.steer;

    }
  }

  void MotionPlannerStage::DataReceiver() {

    auto localization_packet = localization_messenger->ReceiveData(localization_messenger_state);
    localization_frame = localization_packet.data;
    localization_messenger_state = localization_packet.id;

    // Block on receive call only if new data is available on the messenger
    auto collision_messenger_current_state = collision_messenger->GetState();
    if (collision_messenger_current_state != collision_messenger_state) {
      auto collision_packet = collision_messenger->ReceiveData(collision_messenger_state);
      collision_frame = collision_packet.data;
      collision_messenger_state = collision_packet.id;
    }

    // Block on receive call only if new data is available on the messenger
    auto traffic_light_messenger_current_state = traffic_light_messenger->GetState();
    if (traffic_light_messenger_current_state != traffic_light_messenger_state) {
      auto traffic_light_packet = traffic_light_messenger->ReceiveData(traffic_light_messenger_state);
      traffic_light_frame = traffic_light_packet.data;
      traffic_light_messenger_state = traffic_light_packet.id;
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
