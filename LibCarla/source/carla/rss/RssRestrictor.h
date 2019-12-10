// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

namespace ad_rss {
namespace world {
  struct AccelerationRestriction;
  struct Velocity;
}  // namespace world
}  // namespace ad_rss

namespace carla {
namespace rpc {
  class VehicleControl;
  class VehiclePhysicsControl;
}  // namespace rpc

namespace rss {

  class RssRestrictor {
  public:

    RssRestrictor();

    ~RssRestrictor();

    carla::rpc::VehicleControl restrictVehicleControl(const carla::rpc::VehicleControl &vehicleControl,
    const ad_rss::world::AccelerationRestriction &restriction,
    const ad_rss::world::Velocity &egoVelocity,
    const carla::rpc::VehiclePhysicsControl &vehiclePhysics);

  private:

  };

}  // namespace rss
}  // namespace carla
