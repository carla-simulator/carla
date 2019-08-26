// Member Definition for Class TrafficLightStateCallable

#include "TrafficLightStateCallable.h"

namespace traffic_manager {

  const int JUNCTION_LOOK_AHEAD_INDEX = 5;

  TrafficLightStateCallable::TrafficLightStateCallable(
      SyncQueue<PipelineMessage> *input_queue,
      SyncQueue<PipelineMessage> *output_queue,
      SharedData *shared_data)
    : PipelineCallable(input_queue, output_queue, shared_data) {}

  TrafficLightStateCallable::~TrafficLightStateCallable() {}

  PipelineMessage TrafficLightStateCallable::action(PipelineMessage &message) {
    PipelineMessage out_message;

    float traffic_hazard = -1;

    float throttle = message.getAttribute("throttle");
    float brake = message.getAttribute("brake");
    float steer = message.getAttribute("steer");

    auto actor_id = message.getActorID();
    auto vehicle = boost::static_pointer_cast<carla::client::Vehicle>(message.getActor());
    auto traffic_light_state = vehicle->GetTrafficLightState();

    if (shared_data->buffer_map.contains(actor_id)) {
      auto closest_waypoint = shared_data->buffer_map.get(actor_id)->front();
      auto next_waypoint = shared_data->buffer_map.get(actor_id)->get(JUNCTION_LOOK_AHEAD_INDEX);

      if (
        !(closest_waypoint->checkJunction())
        and
        (
          traffic_light_state == carla::rpc::TrafficLightState::Red
          or
          traffic_light_state == carla::rpc::TrafficLightState::Yellow
        )
        and
        next_waypoint->checkJunction()
      ) {
        traffic_hazard = 1;
      }
    }

    out_message.setActor(message.getActor());
    out_message.setAttribute("traffic_light", traffic_hazard);
    out_message.setAttribute("collision", message.getAttribute("collision"));
    out_message.setAttribute("velocity", message.getAttribute("velocity"));
    out_message.setAttribute("deviation", message.getAttribute("deviation"));

    return out_message;
  }
}
