#pragma once

#include <memory>
#include <chrono>

#include "carla/client/Client.h"
#include "carla/rpc/Command.h"
#include "carla/rpc/VehicleControl.h"

#include "PipelineStage.h"
#include "MessengerAndDataTypes.h"

namespace traffic_manager {
  class BatchControlStage : public PipelineStage {

    /// This class is tasked with managing communicating actuation signals to
    /// the simulator in batches.

  private:

    int messenger_state;
    std::shared_ptr<PlannerToControlFrame> data_frame;
    std::shared_ptr<PlannerToControlMessenger> messenger;

    int frame_count;
    carla::client::Client& carla_client;
    std::shared_ptr<std::vector<carla::rpc::Command>> commands;
    std::chrono::time_point<
      std::chrono::_V2::system_clock,
      std::chrono::nanoseconds
    > last_update_instance;

  public:

    BatchControlStage (
        std::shared_ptr<PlannerToControlMessenger> messenger,
        carla::client::Client& carla_client,
        int number_of_vehicles,
        int pool_size
      );
    ~BatchControlStage();

    void DataReceiver() override;
    void Action(int start_index, int end_index) override;
    void DataSender() override;

    using PipelineStage::Start;
    using PipelineStage::Stop;

  };

}
