// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/LaneDetector.h"

#include "carla/Logging.h"
#include "carla/client/detail/Simulator.h"

#include <exception>

namespace carla {
namespace client {

  static SharedPtr<sensor::SensorData> TickLaneDetector(const Timestamp &) {
    return nullptr;
  }

  void LaneDetector::Listen(CallbackFunctionType callback) {
    log_debug(GetDisplayId(), ": subscribing to tick event");
    GetEpisode().Lock()->RegisterOnTickEvent([cb=std::move(callback)](const auto &ts) {
      cb(TickLaneDetector(ts));
    });
    _is_listening = true;
  }

  void LaneDetector::Stop() {
    throw std::runtime_error("LaneDetector::Stop(): not implemented.");
  }

} // namespace client
} // namespace carla
