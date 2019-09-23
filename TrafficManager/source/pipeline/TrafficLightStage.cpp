#include "TrafficLightStage.h"

namespace traffic_manager {

  TrafficLightStage::TrafficLightStage(
      std::shared_ptr<LocalizationToTrafficLightMessenger> localization_messenger,
      std::shared_ptr<TrafficLightToPlannerMessenger> planner_messenger,
      uint number_of_vehicle,
      uint pool_size)
    : localization_messenger(localization_messenger),
      planner_messenger(planner_messenger),
      PipelineStage(pool_size, number_of_vehicle) {

    // Initializing output frame selector
    frame_selector = true;

    // Allocating output frames
    planner_frame_a = std::make_shared<TrafficLightToPlannerFrame>(number_of_vehicle);
    planner_frame_b = std::make_shared<TrafficLightToPlannerFrame>(number_of_vehicle);

    // Initializing messenger state
    localization_messenger_state = localization_messenger->GetState();

    // Initializing this messenger state to preemptively write
    // since this stage precedes motion planner stage
    planner_messenger_state = planner_messenger->GetState() - 1;

  }

  TrafficLightStage::~TrafficLightStage() {}

  void TrafficLightStage::Action(const uint start_index, const uint end_index) {

    // Selecting output frame based on selection key
    auto current_planner_frame = frame_selector ? planner_frame_a : planner_frame_b;

    // Looping over array's partitions for current thread
    for (int i = start_index; i <= end_index; ++i) {

      float traffic_light_hazard = -1.0f;
      auto &data = localization_frame->at(i);
      auto ego_actor = data.actor;
      auto ego_actor_id = ego_actor->GetId();
      auto closest_waypoint = data.closest_waypoint;
      auto look_ahead_point = data.junction_look_ahead_waypoint;

      auto vehicle = boost::static_pointer_cast<carla::client::Vehicle>(ego_actor);
      auto traffic_light_state = vehicle->GetTrafficLightState();

      // We determine to stop if current position of vehicle is not a junction,
      // a point on the path beyond a threshold (velocity dependent) distance
      // is inside the junction and there is a red or yellow light
      if (!(closest_waypoint->CheckJunction()) &&
          (traffic_light_state == carla::rpc::TrafficLightState::Red ||
          traffic_light_state == carla::rpc::TrafficLightState::Yellow) &&
          look_ahead_point->CheckJunction()) {
        traffic_light_hazard = 1.0f;
      }
      auto &message = current_planner_frame->at(i);
      message.traffic_light_hazard = traffic_light_hazard;
    }

  }

  void TrafficLightStage::DataReceiver() {
    auto packet = localization_messenger->ReceiveData(localization_messenger_state);
    localization_frame = packet.data;
    localization_messenger_state = packet.id;
  }

  void TrafficLightStage::DataSender() {
    DataPacket<std::shared_ptr<TrafficLightToPlannerFrame>> packet{
      planner_messenger_state,
      frame_selector ? planner_frame_a : planner_frame_b
    };
    frame_selector = !frame_selector;
    planner_messenger_state = planner_messenger->SendData(packet);
  }
}
