// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Sensor.h"

#include "carla/Logging.h"
#include "carla/client/detail/Client.h"

namespace carla {
namespace client {

  Sensor::~Sensor() {
    if (IsAlive()) {
      log_warning(
          "sensor object went out of the scope but the sensor is still alive",
          "in the simulation:",
          GetDisplayId());
    }
  }

  void Sensor::Listen(CallbackFunctionType callback) {
    log_debug(GetDisplayId(), ": subscribing to stream");
    if (_is_listening) {
      log_warning(
          "attempting to listen to stream but sensor is already listeninig:",
          GetDisplayId());
      return;
    }
    GetClientImplementation()->SubscribeToStream(
        GetActorDescription().GetStreamToken(),
        std::move(callback));
    _is_listening = true;
  }

} // namespace client
} // namespace carla
