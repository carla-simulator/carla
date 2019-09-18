#pragma once

#include <unordered_map>

#include "carla/client/Vehicle.h"
#include "carla/rpc/TrafficLightState.h"

#include "MessengerAndDataTypes.h"
#include "PipelineStage.h"

namespace traffic_manager {
  /// This class provides the information about the Traffic Lights at the junctions.
  class TrafficLightStage : public PipelineStage {
  private:

    int localization_messenger_state;
    int planner_messenger_state;
    bool frame_selector;
    std::shared_ptr<LocalizationToTrafficLightFrame> localization_frame;
    std::shared_ptr<TrafficLightToPlannerFrame> planner_frame_a;
    std::shared_ptr<TrafficLightToPlannerFrame> planner_frame_b;
    std::unordered_map<bool, std::shared_ptr<TrafficLightToPlannerFrame>> planner_frame_map;
    std::shared_ptr<LocalizationToTrafficLightMessenger> localization_messenger;
    std::shared_ptr<TrafficLightToPlannerMessenger> planner_messenger;

  public:

    TrafficLightStage(
        std::shared_ptr<LocalizationToTrafficLightMessenger> localization_messenger,
        std::shared_ptr<TrafficLightToPlannerMessenger> planner_messenger,
        int number_of_vehicle,
        int pool_size);
    ~TrafficLightStage();

    void DataReceiver() override;

    void Action(const int start_index, const int end_index) override;

    void DataSender() override;

    using PipelineStage::Start;
    using PipelineStage::Stop;
  };
}
