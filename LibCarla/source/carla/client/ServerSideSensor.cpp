// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ServerSideSensor.h"

#include "carla/Logging.h"
#include "carla/client/detail/Simulator.h"

#include <exception>

namespace carla {
namespace client {

  ServerSideSensor::~ServerSideSensor() {
    log_warning("calling sensor destructor ", GetDisplayId());
    if (IsAlive() && IsListening()) {
      log_warning(
          "sensor object went out of the scope but the sensor is still alive",
          "in the simulation:",
          GetDisplayId());
    }
    if (IsListening() && GetEpisode().IsValid()) {
      try {
        Stop();
      } catch (const std::exception &e) {
        log_error("exception trying to stop sensor:", GetDisplayId(), ':', e.what());
      }
    }
  }

  void ServerSideSensor::Listen(CallbackFunctionType callback) {
    log_warning("calling sensor Listen() ", GetDisplayId());
    log_debug(GetDisplayId(), ": subscribing to stream");
    GetEpisode().Lock()->SubscribeToSensor(*this, std::move(callback));
    _is_listening = true;
  }

  void ServerSideSensor::Stop() {
    log_warning("calling sensor Stop() ", GetDisplayId());
    if (!_is_listening) {
      log_warning(
          "attempting to unsubscribe from stream but sensor wasn't listening:",
          GetDisplayId());
      return;
    }
    GetEpisode().Lock()->UnSubscribeFromSensor(*this);
    _is_listening = false;
  }

  bool ServerSideSensor::Destroy() {
    log_warning("calling sensor Destroy() ", GetDisplayId());
    if (IsListening()) {
      Stop();
    }
    return Actor::Destroy();
  }

} // namespace client
} // namespace carla
