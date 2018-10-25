// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/ClientSideSensor.h"

namespace carla {
namespace client {

  class LaneDetector final : public ClientSideSensor {
  public:

    using ClientSideSensor::ClientSideSensor;

    /// Register a @a callback to be executed each time a new measurement is
    /// received.
    ///
    /// @warning Calling this function on a sensor that is already listening
    /// registers another callback on the same measurement.
    /// @todo This is different from other sensors.
    void Listen(CallbackFunctionType callback) override;

    /// Stop listening for new measurements.
    /// @throw Not implemented error.
    void Stop() override;

    /// Return whether this Sensor instance is currently listening to the
    /// associated sensor in the simulator.
    bool IsListening() const override {
      return _is_listening;
    }

  private:

    bool _is_listening = false;
  };

} // namespace client
} // namespace carla
