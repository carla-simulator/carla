#include "TrafficLightStage.h"

namespace traffic_manager {

  TrafficLightStage::TrafficLightStage(
      std::shared_ptr<LocalizationToTrafficLightMessenger> localization_messenger,
      std::shared_ptr<TrafficLightToPlannerMessenger> planner_messenger,
      uint number_of_vehicle,
      uint pool_size,
      cc::DebugHelper &debug_helper)
    : localization_messenger(localization_messenger),
      planner_messenger(planner_messenger),
      PipelineStage(pool_size, number_of_vehicle),
      debug_helper(debug_helper){

    // Initializing output frame selector.
    frame_selector = true;

    // Allocating output frames.
    planner_frame_a = std::make_shared<TrafficLightToPlannerFrame>(number_of_vehicle);
    planner_frame_b = std::make_shared<TrafficLightToPlannerFrame>(number_of_vehicle);

    // Initializing messenger state.
    localization_messenger_state = localization_messenger->GetState();

    // Initializing this messenger state to preemptively write
    // since this stage precedes motion planner stage.
    planner_messenger_state = planner_messenger->GetState() - 1;

  }

  TrafficLightStage::~TrafficLightStage() {}

  void TrafficLightStage::Action(const uint start_index, const uint end_index) {

    // Selecting the output frame based on the selection key.
    auto current_planner_frame = frame_selector ? planner_frame_a : planner_frame_b;

    // Looping over array's partitions for the current thread.
    for (uint i = start_index; i <= end_index; ++i) {

      bool traffic_light_hazard = false;
      LocalizationToTrafficLightData &data = localization_frame->at(i);
      Actor ego_actor = data.actor;
      ActorId ego_actor_id = ego_actor->GetId();
      SimpleWaypointPtr closest_waypoint = data.closest_waypoint;
      SimpleWaypointPtr look_ahead_point = data.junction_look_ahead_waypoint;

      auto vehicle = boost::static_pointer_cast<cc::Vehicle>(ego_actor);
      TLS traffic_light_state = vehicle->GetTrafficLightState();

      // We determine to stop if the current position of the vehicle is not a junction,
      // a point on the path beyond a threshold (velocity-dependent) distance
      // is inside the junction and there is a red or yellow light.
      if (vehicle->IsAtTrafficLight() &&
          !closest_waypoint->CheckJunction() &&
          look_ahead_point->CheckJunction() &&
          traffic_light_state != TLS::Green) {

        traffic_light_hazard = true;
      }
      TrafficLightToPlannerData &message = current_planner_frame->at(i);
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
