#pragma once

#include <unordered_map>

#include "carla/client/Vehicle.h"
#include "carla/rpc/TrafficLightState.h"

#include "MessengerAndDataTypes.h"
#include "PipelineStage.h"

namespace traffic_manager {

  /// This class provides the information about the Traffic Lights at the
  /// junctions.
  class TrafficLightStage : public PipelineStage {

  private:

    /// Variables to remember messenger states
    int localization_messenger_state;
    int planner_messenger_state;
    /// Selection key to switch between output frames
    bool frame_selector;
    /// Pointer data frame received from localization stage
    std::shared_ptr<LocalizationToTrafficLightFrame> localization_frame;
    /// Pointers to data frames to be shared with motion planner stage
    std::shared_ptr<TrafficLightToPlannerFrame> planner_frame_a;
    std::shared_ptr<TrafficLightToPlannerFrame> planner_frame_b;
    /// Pointers to messenger objects
    std::shared_ptr<LocalizationToTrafficLightMessenger> localization_messenger;
    std::shared_ptr<TrafficLightToPlannerMessenger> planner_messenger;

  public:

    TrafficLightStage(
        std::shared_ptr<LocalizationToTrafficLightMessenger> localization_messenger,
        std::shared_ptr<TrafficLightToPlannerMessenger> planner_messenger,
        uint number_of_vehicle,
        uint pool_size);
    ~TrafficLightStage();

    void DataReceiver() override;

    void Action(const uint start_index, const uint end_index) override;

    void DataSender() override;

  };

}
