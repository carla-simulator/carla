// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>

#include "carla/client/detail/EpisodeProxy.h"
#include "carla/client/detail/Simulator.h"
#include "carla/Logging.h"
#include "carla/rpc/ActorId.h"
#include "carla/rpc/Command.h"
#include "carla/rpc/VehicleControl.h"

#include "carla/trafficmanager/MessengerAndDataTypes.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/PipelineStage.h"

namespace carla {
namespace traffic_manager {

using namespace std::literals::chrono_literals;

/// This class receives actuation signals (throttle, brake, steer)
/// from Motion Planner Stage class and communicates these signals to
/// the simulator in batches to control vehicles' movement.
class BatchControlStage : public PipelineStage {

private:

  /// Pointer to frame received from Motion Planner.
  std::shared_ptr<PlannerToControlFrame> data_frame;
  /// Pointer to a messenger from Motion Planner.
  std::shared_ptr<PlannerToControlMessenger> messenger;
  /// Reference to CARLA client connection object.
  carla::client::detail::EpisodeProxy episode_proxy_bcs;
  /// Array to hold command batch.
  std::shared_ptr<std::vector<carla::rpc::Command>> commands;
  /// Number of vehicles registered with the traffic manager.
  uint64_t number_of_vehicles;
  /// Parameter object for changing synchronous behaviour.
  Parameters &parameters;
  /// Step runner flag.
  std::atomic<bool> run_step;
  /// Mutex for progressing synchronous execution.
  std::mutex step_execution_mutex;
  /// Condition variables for progressing synchronous execution.
  std::condition_variable step_execution_notifier;

public:

  BatchControlStage(std::string stage_name,
                    std::shared_ptr<PlannerToControlMessenger> messenger,
                    carla::client::detail::EpisodeProxy &episode_proxy,
                    Parameters &parameters);

  ~BatchControlStage();

  void DataReceiver() override;

  void Action() override;

  void DataSender() override;

  bool RunStep();
};

} // namespace traffic_manager
} // namespace carla
