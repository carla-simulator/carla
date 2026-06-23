// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>

#include "carla/ros2/ROS2CallbackData.h"
#include "carla/ros2/subscribers/BaseSubscriber.h"

namespace carla {
namespace ros2 {

// Forward declarations keep the FastDDS-heavy SubscriberImpl<> definition out of the
// main carla-server compile unit. The full instantiation lives in
// CarlaEgoVehicleControlSubscriber.cpp, which is built by the carla-ros2-native
// ExternalProject (where FastDDS headers are on the include path).
template <typename Traits> class SubscriberImpl;
struct CarlaEgoVehicleControlTraits;

class CarlaEgoVehicleControlSubscriber : public BaseSubscriber {
public:
  CarlaEgoVehicleControlSubscriber(void *vehicle, std::string base_topic_name, std::string frame_id);
  ~CarlaEgoVehicleControlSubscriber() override;

  void ProcessMessages(ActorCallback callback) override;

protected:
  ROS2CallbackData GetMessage() override;

private:
  std::shared_ptr<SubscriberImpl<CarlaEgoVehicleControlTraits>> _impl;
};

}  // namespace ros2
}  // namespace carla
