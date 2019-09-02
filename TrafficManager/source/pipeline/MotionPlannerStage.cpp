#include "MotionPlannerStage.h"

namespace traffic_manager {

  const float HIGHWAY_SPEED = 50 / 3.6;
  const float INTERSECTION_APPROACH_SPEED = 15 / 3.6;

  MotionPlannerStage::MotionPlannerStage(
    std::shared_ptr<LocalizationToPlannerMessenger> localization_messenger,
    std::shared_ptr<PlannerToControlMessenger> control_messenger,
    int number_of_vehicles,
    int pool_size = 1,
    float urban_target_velocity = 25/3.6,
    float highway_target_velocity = 50/3.6,
    std::vector<float> longitudinal_parameters = {0.1f, 0.15f, 0.01f},
    std::vector<float> highway_longitudinal_parameters = {10.0f, 0.01f, 0.1f},
    std::vector<float> lateral_parameters = {10.0f, 0.0f, 0.1f}
  ) :
  urban_target_velocity(urban_target_velocity),
  highway_target_velocity(highway_target_velocity),
  longitudinal_parameters(longitudinal_parameters),
  highway_longitudinal_parameters(highway_longitudinal_parameters),
  lateral_parameters(lateral_parameters),
  localization_messenger(localization_messenger),
  control_messenger(control_messenger),
  PipelineStage(pool_size, number_of_vehicles)
  {
    control_frame_a = std::make_shared<PlannerToControlFrame>(number_of_vehicles);
    control_frame_b = std::make_shared<PlannerToControlFrame>(number_of_vehicles);

    frame_selector = true;
    frame_map.insert(std::pair<bool, std::shared_ptr<PlannerToControlFrame>>(true, control_frame_a));
    frame_map.insert(std::pair<bool, std::shared_ptr<PlannerToControlFrame>>(false, control_frame_b));

    localization_messenger_state = localization_messenger->GetState();
    control_messenger_state = control_messenger->GetState() - 1;
  }

  MotionPlannerStage::~MotionPlannerStage() {}

  void MotionPlannerStage::Action(int start_index, int end_index) {
    // std::cout 
    // << "Running planner's action"
    // << " with messenger's state "
    // << localization_messenger->GetState()
    // << " previous state "
    // << localization_messenger_state
    // << std::endl;

    for (int i = start_index; i < end_index; i++) {

      auto& localization_data = localization_frame->at(i);
      auto actor = localization_data.actor;
      float current_deviation = localization_data.deviation;
      // bool approaching_true_junction = localization_data.approaching_true_junction;
      int actor_id = actor->GetId();

      auto vehicle = boost::static_pointer_cast<carla::client::Vehicle>(actor);
      float current_velocity = vehicle->GetVelocity().Length();
      auto current_time = std::chrono::system_clock::now();

      /// Retreiving previous state
      traffic_manager::StateEntry previous_state;
      if (pid_state_map.find(actor_id) != pid_state_map.end()) {
        previous_state = pid_state_map[actor_id];
      } else {
        previous_state.time_instance = current_time;
      }

      auto dynamic_target_velocity = urban_target_velocity;

      /// Increase speed if on highway
      auto speed_limit = vehicle->GetSpeedLimit() / 3.6;
      if (speed_limit > HIGHWAY_SPEED) {
        dynamic_target_velocity = highway_target_velocity;
        longitudinal_parameters = highway_longitudinal_parameters;
      }

      /// State update for vehicle
      auto current_state = controller.stateUpdate(
          previous_state,
          current_velocity,
          dynamic_target_velocity,
          current_deviation,
          current_time);

      /// Controller actuation
      auto actuation_signal = controller.runStep(
          current_state,
          previous_state,
          longitudinal_parameters,
          lateral_parameters);

      /// In case of collision or traffic light or approaching a junction
      // if (
      //   message.getAttribute("collision") > 0
      //   or message.getAttribute("traffic_light") > 0
      //   or (approaching_junction and current_velocity > INTERSECTION_APPROACH_SPEED)
      //   ) {
      //   current_state.deviation_integral = 0;
      //   current_state.velocity_integral = 0;
      //   actuation_signal.throttle = 0;
      //   actuation_signal.brake = 1.0;
      // }

      /// Updating state
      pid_state_map[actor_id] = current_state;

      /// Constructing actuation signal
      auto& message = frame_map.at(frame_selector)->at(i);
      message.actor_id = actor_id;
      message.throttle = actuation_signal.throttle;
      message.brake = actuation_signal.brake;
      message.steer = actuation_signal.steer;

    }

    // std::cout 
    // << "Finished planner's action"
    // << " with messenger's state "
    // << localization_messenger->GetState()
    // << " previous state "
    // << localization_messenger_state
    // << std::endl;
}

  void MotionPlannerStage::DataReceiver() {
    // std::cout 
    // << "Running planner's receiver"
    // << " with messenger's state "
    // << localization_messenger->GetState()
    // << " previous state "
    // << localization_messenger_state
    // << std::endl;

    auto packet = localization_messenger->RecieveData(localization_messenger_state);
    localization_frame = packet.data;
    localization_messenger_state = packet.id;

    // std::cout
    // << "Finished planner's receiver"
    // << " with messenger's state "
    // << localization_messenger->GetState()
    // << " previous state "
    // << localization_messenger_state
    // << std::endl;
  }

  void MotionPlannerStage::DataSender() {
    // std::cout 
    // << "Running planner's sender"
    // << " with messenger's state "
    // << control_messenger->GetState()
    // << " previous state "
    // << control_messenger_state
    // << std::endl;

    DataPacket<std::shared_ptr<PlannerToControlFrame>> data_packet = {
      control_messenger_state,
      frame_map.at(frame_selector)
    };
    frame_selector = !frame_selector;
    control_messenger_state = control_messenger->SendData(data_packet);
    
    // std::cout 
    // << "Finished planner's sender"
    // << " with messenger's state "
    // << control_messenger->GetState()
    // << " previous state "
    // << control_messenger_state
    // << std::endl;
  }
}
