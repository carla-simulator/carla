// Copyright (c) 2019 Intel Labs.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Sensor.h"
#include "carla/geom/GeoLocation.h"

namespace carla {
namespace client {

  class GnssSensor final : public Sensor {
  public:

    using Sensor::Sensor;

    ~GnssSensor();

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
      return _is_listening;
    }

  private:

    SharedPtr<sensor::SensorData> TickGnssSensor(const Timestamp &timestamp);

    geom::GeoLocation _geo_reference;

    bool _is_listening = false;
  };

} // namespace client
} // namespace carla
