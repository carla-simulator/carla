// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Sensor.h"
#include <bitset>

namespace carla {
namespace client {

  class ServerSideSensor final : public Sensor {
  public:

    using Sensor::Sensor;

    ~ServerSideSensor();

    /// Register a @a callback to be executed each time a new measurement is
    /// received.
    ///
    /// @warning Calling this function on a sensor that is already listening
    /// steals the data stream from the previously set callback. Note that
    /// several instances of Sensor (even in different processes) may point to
    /// the same sensor in the simulator.
    void Listen(CallbackFunctionType callback) override;

    /// Stop listening for new measurements.
    void Stop() override;

    /// Return whether this Sensor instance is currently listening to the
    /// associated sensor in the simulator.
    bool IsListening() const override {
      return listening_mask.test(0);
    }

    /// Listen fr
    void ListenToGBuffer(uint32_t GBufferId, CallbackFunctionType callback);

    /// Stop listening for a specific gbuffer stream.
    void StopGBuffer(uint32_t GBufferId);

    inline bool IsListeningGBuffer(uint32_t id) const {
      return listening_mask.test(id + 1);
    }

    /// Enable this sensor for ROS2 publishing
    void EnableForROS();

    /// Disable this sensor for ROS2 publishing
    void DisableForROS();

    /// Return if the sensor is publishing for ROS2
    bool IsEnabledForROS();

    /// @copydoc Actor::Destroy()
    ///
    /// Additionally stop listening.
    bool Destroy() override;

  private:

    std::bitset<16> listening_mask;
  };

} // namespace client
} // namespace carla
