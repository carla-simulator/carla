#include "TrafficLightStage.h"

namespace traffic_manager {

    TrafficLightStage::TrafficLightStage(
        std::shared_ptr<LocalizationToTrafficLightMessenger> localization_messenger,
        std::shared_ptr<TrafficLightToPlannerMessenger> planner_messenger,
        int number_of_vehicle,
        int pool_size
    ):
      localization_messenger(localization_messenger),
      planner_messenger(planner_messenger),
      PipelineStage(pool_size, number_of_vehicle) {
        frame_selector = true;

        planner_frame_a = std::make_shared<TrafficLightToPlannerFrame>(number_of_vehicle);
        planner_frame_b = std::make_shared<TrafficLightToPlannerFrame>(number_of_vehicle);

        planner_frame_map.insert(std::pair<bool, std::shared_ptr<TrafficLightToPlannerFrame>>(true, planner_frame_a));
        planner_frame_map.insert(std::pair<bool, std::shared_ptr<TrafficLightToPlannerFrame>>(false, planner_frame_b));

        localization_messenger_state = localization_messenger->GetState();
        planner_messenger_state = planner_messenger->GetState() -1;

      }

    TrafficLightStage::~TrafficLightStage(){}

    void TrafficLightStage::Action(int start_index, int end_index) {

        for (int i = start_index; i <= end_index; i++) {

            float traffic_light_hazard = -1;
            auto& data = localization_frame->at(i);
            auto ego_actor = data.actor;
            auto ego_actor_id = ego_actor->GetId();
            auto closest_waypoint = data.closest_waypoint;
            auto next_fifth_waypoint = data.junction_look_ahead_waypoint;

            auto vehicle = boost::static_pointer_cast<carla::client::Vehicle>(ego_actor);
            auto traffic_light_state = vehicle->GetTrafficLightState();

            if (!(closest_waypoint->checkJunction()) &&
                (traffic_light_state == carla::rpc::TrafficLightState::Red ||
                traffic_light_state == carla::rpc::TrafficLightState::Yellow) &&
                next_fifth_waypoint->checkJunction()) {
                    traffic_light_hazard = 1;
            }
            auto& message = planner_frame_map.at(frame_selector)->at(i);
            message.traffic_light_hazard = traffic_light_hazard > 0 ? 1: -1;
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
        planner_frame_map.at(frame_selector)
        };
        frame_selector = !frame_selector;
        planner_messenger_state = planner_messenger->SendData(packet);
    }
}