// Definition of class members

#include "MotionPlannerStage.h"

namespace traffic_manager {

  const float HIGHWAY_SPEED = 50 / 3.6;
  const float INTERSECTION_APPROACH_SPEED = 15 / 3.6;

  MotionPlannerStage::MotionPlannerStage(
    float urban_target_velocity,
    float highway_target_velocity,
    std::vector<float> longitudinal_parameters = {0.1f, 0.15f, 0.01f},
    std::vector<float> highway_longitudinal_parameters = {5.0, 0.1, 0.01},
    std::vector<float> lateral_parameters = {10.0f, 0.0f, 0.1f},
    std::shared_ptr<LocalizationToPlannerMessenger> localization_messenger,
    std::shared_ptr<PlannerToControlMessenger> control_messenger,
    int pool_size,
    int number_of_vehicles
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
    control_frame_a = PlannerToControlFrame(number_of_vehicles);
  }

  MotionPlannerStage::~MotionPlannerStage() {}

  void MotionPlannerStage::Action(int thread_id) {

    int array_size = number_of_vehicles;
    int load_per_thread = std::floor(array_size/pool_size);

    while (run_stage.load()) {
      std::unique_lock<std::mutex> lock(wait_for_action_mutex);
      wake_action_notifier.wait(lock, [=] {return run_stage.load();});
      lock.unlock();

      int array_start_index = thread_id*load_per_thread;
      int array_end_index = thread_id == pool_size-1 ? array_size-1 : (thread_id+1)*load_per_thread-1;

      for (int i = array_start_index; i < array_end_index; i++) {

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
        auto& message = control_frame_a.at(i);
        message.actor = actor;
        message.throttle = actuation_signal.throttle;
        message.brake = actuation_signal.brake;
        message.steer = actuation_signal.steer;
      }

      action_counter++;

      if (run_threads.load()) {
        run_threads.store(false);
      }

      if (thread_id == pool_size-1) {
        while (action_counter.load() < pool_size);
        action_counter.store(0);
        run_sender.store(true);
        wake_sender_notifier.notify_one();
      }
    }
  }

  void MotionPlannerStage::DataReceiver() {
    while (run_stage.load()) {
      std::unique_lock<std::mutex> lock(wait_receiver_mutex);
      if (!run_receiver.load()) {
        wake_receiver_notifier.wait(lock, [=] {return run_receiver.load();});
      }

      auto packet = localization_messenger->RecieveData(localization_messenger_state);
      localization_frame = packet.data;
      localization_messenger_state = packet.id;

      run_threads.store(true);
      wake_action_notifier.notify_all();
      run_receiver.store(false);
    }
  }

  void MotionPlannerStage::DataSender() {
    while (run_stage.load()) {
      std::unique_lock<std::mutex> lock(wait_sender_mutex);
      if (!run_sender.load()) {
        wake_sender_notifier.wait(lock, [=] {return run_sender.load();});
      }
      // std::cout << "Running sender" << std::endl;

      DataPacket<PlannerToControlFrame*> data_packet = {
        control_messenger_state,
        &control_frame_a
      };
      control_messenger_state = control_messenger->SendData(data_packet);

      run_receiver.store(true);
      wake_receiver_notifier.notify_one();
      run_sender.store(false);
    }
  }
}
