// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/types/Timestamp.h"
#include "carla/rpc/WalkerControl.h"
#include "carla/sensor/data/ActorDynamicState.h"
#include "carla_msgs/msg/CarlaWalkerControl.h"

namespace carla {
namespace ros2 {
namespace types {

/**
 * WalkerControl: convert carla_msgs::msg::CarlaWalkerControl into FWalkerControl without the need of
 * knowing FWalkerControl class within LibCarla
 */
class WalkerControl {
public:
  explicit WalkerControl(const carla_msgs::msg::CarlaWalkerControl& walker_control) : _walker_control(walker_control) {}
  ~WalkerControl() = default;
  WalkerControl(const WalkerControl&) = default;
  WalkerControl& operator=(const WalkerControl&) = default;
  WalkerControl(WalkerControl&&) = default;
  WalkerControl& operator=(WalkerControl&&) = default;

  carla_msgs::msg::CarlaWalkerControl const& carla_walker_control() const {
    return _walker_control;
  }

  WalkerControl(const carla::rpc::WalkerControl& walker_control) {
    _walker_control.header().stamp(Timestamp(walker_control.timestamp).time());
    _walker_control.direction().x(walker_control.direction.x);
    _walker_control.direction().y(walker_control.direction.y);
    _walker_control.direction().z(walker_control.direction.z);
    _walker_control.speed(walker_control.speed);
    _walker_control.jump(walker_control.jump);
  }

#ifdef LIBCARLA_INCLUDED_FROM_UE4
  FWalkerControl GetWalkerControl() const {
    FWalkerControl walker_control;
    walker_control.Timestamp = Timestamp(_walker_control.header().stamp()).Stamp();
    walker_control.Speed = _walker_control.speed();
    walker_control.Direction.X = _walker_control.direction().x();
    walker_control.Direction.Y = -_walker_control.direction().y();
    walker_control.Direction.Z = _walker_control.direction().z();
    walker_control.Jump = _walker_control.jump();
    return walker_control;
  }

#endif  // LIBCARLA_INCLUDED_FROM_UE4
private:
  carla_msgs::msg::CarlaWalkerControl _walker_control;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla