// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <spdlog/spdlog.h>
#include <memory>

namespace ad {
namespace rss {
namespace world {

/// @brief forward declararion for the RSS Acceleration restrictions
struct AccelerationRestriction;

}  // namespace world
}  // namespace rss
}  // namespace ad

namespace carla {
namespace rpc {
class VehicleControl;
class VehiclePhysicsControl;
}  // namespace rpc

namespace rss {

/// @brief forward declararion for ego vehicles current dynamics in respect to
/// the current route
struct EgoDynamicsOnRoute;

/// @brief class implementing the RSS restrictions within CARLA
class RssRestrictor {
public:
  /// @brief constructor
  RssRestrictor();

  /// @brief destructor
  ~RssRestrictor();

  /// @brief the actual function to restrict the given vehicle control input to
  /// mimick
  ///        RSS conform behavior by braking
  ///        Lateral braking is achieved by counter-steering, so is only a very
  ///        rough solution
  carla::rpc::VehicleControl RestrictVehicleControl(const carla::rpc::VehicleControl &vehicle_control,
                                                    const ::ad::rss::world::AccelerationRestriction &restriction,
                                                    const carla::rss::EgoDynamicsOnRoute &ego_dynamics_on_route,
                                                    const carla::rpc::VehiclePhysicsControl &vehicle_physics);

private:
  /// @brief the logger instance
  std::shared_ptr<spdlog::logger> _logger;
};

}  // namespace rss
}  // namespace carla
