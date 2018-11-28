// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/ClientSideSensor.h"
#include "carla/geom/Location.h"

#include <array>

namespace carla {
namespace client {

  class Map;
  class Vehicle;

  class LaneDetector final : public ClientSideSensor {
  public:

    using ClientSideSensor::ClientSideSensor;

    ~LaneDetector();

    /// Register a @a callback to be executed each time a new measurement is
    /// received.
    ///
    /// @warning This function should not be called twice.
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

    SharedPtr<sensor::SensorData> TickLaneDetector(const Timestamp &timestamp);

    bool _is_listening = false;

    SharedPtr<Map> _map;

    SharedPtr<Vehicle> _vehicle;

    std::array<geom::Location, 4u> _bounds;
  };

} // namespace client
} // namespace carla
