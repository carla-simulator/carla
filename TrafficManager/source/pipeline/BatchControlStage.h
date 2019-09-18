#pragma once

#include <chrono>
#include <memory>

#include "carla/client/Client.h"
#include "carla/rpc/Command.h"
#include "carla/rpc/VehicleControl.h"
#include "carla/Logging.h"

#include "MessengerAndDataTypes.h"
#include "PipelineStage.h"

namespace traffic_manager {

  /// This class is receives actuation signals (throttle, brake, steer)
  /// from MotionPlannerStage class and communicates these signals to
  /// the simulator in batches to control vehicles' movement.
  class BatchControlStage : public PipelineStage {

  private:

    int messenger_state;
    std::shared_ptr<PlannerToControlFrame> data_frame;
    std::shared_ptr<PlannerToControlMessenger> messenger;

    int frame_count = 0;
    carla::client::Client &carla_client;
    std::shared_ptr<std::vector<carla::rpc::Command>> commands;
    std::chrono::time_point<
        std::chrono::_V2::system_clock,
        std::chrono::nanoseconds
        > last_update_instance;

  public:

    BatchControlStage(
        std::shared_ptr<PlannerToControlMessenger> messenger,
        carla::client::Client &carla_client,
        int number_of_vehicles,
        int pool_size);
    ~BatchControlStage();

    void DataReceiver() override;

    void Action(const int start_index, const int end_index) override;

    void DataSender() override;

    using PipelineStage::Start;
    using PipelineStage::Stop;

  };

}
