#include "LocalizationStage.h"

namespace traffic_manager {

  const float WAYPOINT_TIME_HORIZON = 3.0;
  const float MINIMUM_HORIZON_LENGTH = 25.0;
  const float TARGET_WAYPOINT_TIME_HORIZON = 0.5;
  const float TARGET_WAYPOINT_HORIZON_LENGTH = 2.0;

  LocalizationStage::LocalizationStage (
    int pool_size,
    std::shared_ptr<MessengerType> motion_control_messenger,
    std::vector<carla::SharedPtr<carla::client::Actor>>& actor_list,
    InMemoryMap& local_map
  ) :
  PipelineStage(pool_size),
  motion_control_messenger(motion_control_messenger),
  actor_list(actor_list),
  local_map(local_map)
  {

    /// Initializing buffer list
    for (int i=0; i < actor_list.size(); i++) {
      buffer_list.push_back(std::deque<std::shared_ptr<SimpleWaypoint>>());
    }
  }

  LocalizationStage::~LocalizationStage() {}

  void LocalizationStage::Action(int thread_id) {

    int array_size = actor_list.size();
    int load_per_thread = std::floor(array_size/pool_size);

    while (run_stage.load()) {
      std::shared_lock<std::shared_timed_mutex> lock(wait_for_action_mutex);
      wake_action_notifier.wait(lock, [=] {return run_stage.load();});

      int array_start_index = thread_id*load_per_thread;
      int array_end_index = thread_id == pool_size-1 ? array_size-1 : (thread_id+1)*load_per_thread-1;

      for (int i = array_start_index; i < array_end_index; i++) {

          auto vehicle = actor_list.at(i);
          auto actor_id = vehicle->GetId();

          auto vehicle_location = vehicle->GetLocation();
          auto vehicle_velocity = vehicle->GetVelocity().Length();

          float horizon_size = std::max(
              WAYPOINT_TIME_HORIZON * vehicle_velocity,
              MINIMUM_HORIZON_LENGTH);


          auto waypoint_buffer = buffer_list.at(i);
          if (!waypoint_buffer.empty()) {
            /// Purge past waypoints
            auto dot_product = DeviationDotProduct(
              vehicle,
              waypoint_buffer.front()->getLocation());
            while (dot_product <= 0) {
              waypoint_buffer.pop_front();
              if (!waypoint_buffer.empty()) {
                dot_product = DeviationDotProduct(
                  vehicle,
                  waypoint_buffer.front()->getLocation());
              } else {
                break;
              }
            }
          }

          /// Initialize buffer if empty
          if (waypoint_buffer.empty()) {
            auto closest_waypoint = local_map.getWaypoint(vehicle_location);
            waypoint_buffer.push_back(closest_waypoint);
          }

          /// Populate buffer
          while (
            waypoint_buffer.back()->distance(
            waypoint_buffer.front()->getLocation()) <= horizon_size
          ) {

            auto way_front = waypoint_buffer.back();
            auto next_waypoints = way_front->getNextWaypoint();
            auto selection_index = next_waypoints.size() > 1 ? rand() % next_waypoints.size() : 0;
            way_front = next_waypoints[selection_index];
            waypoint_buffer.push_back(way_front);
          }

          /// Generate output
          auto horizon_index = static_cast<int>(
            std::max(
              std::ceil(vehicle_velocity * TARGET_WAYPOINT_TIME_HORIZON),
              TARGET_WAYPOINT_HORIZON_LENGTH
            )
          );
          auto target_waypoint = waypoint_buffer.at(horizon_index);
          auto dot_product = DeviationDotProduct(vehicle, target_waypoint->getLocation());
          float cross_product = DeviationCrossProduct(vehicle, target_waypoint->getLocation());
          dot_product = 1 - dot_product;
          if (cross_product < 0) {
            dot_product *= -1;
          }

          auto message = motion_control_frame->at(i);
          message.actor = vehicle;
          message.deviation = dot_product;
      }
      action_counter++;

      if (action_counter.load() == pool_size) {
        run_sender.store(true);
        wake_sender_notifier.notify_one();
      }

      if (run_threads.load()) {
        run_threads.store(false);
      }
    }
  }

  void LocalizationStage::DataReceiver() {
    while (run_stage.load()) {
      std::unique_lock<std::mutex> lock(wait_receiver_mutex);
      if (!run_receiver.load()) {
        wake_receiver_notifier.wait(lock, [=] {return run_receiver.load();});
      }

      motion_control_frame = std::make_shared<MessageFrame>(actor_list.size());

      run_threads.store(true);
      wake_action_notifier.notify_all();
      run_receiver.store(false);
    }
  }

  void LocalizationStage::DataSender() {
    while (run_stage.load()) {
      std::unique_lock<std::mutex> lock(wait_sender_mutex);
      wake_sender_notifier.wait(lock, [=] {return run_sender.load();});

      DataPacket<std::shared_ptr<MessageFrame>> data_packet = {
        motion_control_messenger_state,
        motion_control_frame
      };
      motion_control_messenger_state = motion_control_messenger->SendData(data_packet);

      run_receiver.store(true);
      wake_receiver_notifier.notify_one();
      run_sender.store(false);
    }
  }

  float LocalizationStage::DeviationDotProduct(
      carla::SharedPtr<carla::client::Actor> actor,
      const carla::geom::Location &target_location) const {
    auto heading_vector = actor->GetTransform().GetForwardVector();
    auto next_vector = target_location - actor->GetLocation();
    next_vector = next_vector.MakeUnitVector();
    auto dot_product = next_vector.x * heading_vector.x +
        next_vector.y * heading_vector.y + next_vector.z * heading_vector.z;
    return dot_product;
  }

  float LocalizationStage::DeviationCrossProduct(
      carla::SharedPtr<carla::client::Actor> actor,
      const carla::geom::Location &target_location) const {
    auto heading_vector = actor->GetTransform().GetForwardVector();
    auto next_vector = target_location - actor->GetLocation();
    next_vector = next_vector.MakeUnitVector();
    float cross_z = heading_vector.x * next_vector.y - heading_vector.y * next_vector.x;
    return cross_z;
  }
}
