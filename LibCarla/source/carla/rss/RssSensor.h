// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <mutex>
#include "carla/client/Sensor.h"

namespace ad_rss {
namespace world {
  class RssDynamics;
}  // namespace world
}  // namespace ad_rss

namespace carla {
namespace rss {
  class RssCheck;
}  // namespace rss
namespace client {

  class RssSensor final : public Sensor {
  public:

    using Sensor::Sensor;

    ~RssSensor();

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
    bool IsListening() const override { return _is_listening;
    }

    bool visualize_results = false;

    const ::ad_rss::world::RssDynamics &GetEgoVehicleDynamics() const;
    void SetEgoVehicleDynamics(const ::ad_rss::world::RssDynamics &egoDynamics);
    const ::ad_rss::world::RssDynamics &GetOtherVehicleDynamics() const;
    void SetOtherVehicleDynamics(const ::ad_rss::world::RssDynamics &otherVehicleDynamics);

  private:

    SharedPtr<sensor::SensorData> TickRssSensor(const Timestamp &timestamp);

    std::mutex _processing_lock;

    bool _is_listening = false;

    SharedPtr<Map> _map;

    std::shared_ptr<carla::rss::RssCheck> mRssCheck;
  };

}  // namespace client
}  // namespace carla
