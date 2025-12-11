// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/SubscriberBase.h"
#include "carla/rpc/RpcServerInterface.h"
#include "carla_msgs/msg/CarlaWeatherParametersPubSubTypes.h"

namespace carla {
namespace ros2 {

using WeatherControlSubscriberImpl =
    DdsSubscriberImpl<carla_msgs::msg::CarlaWeatherParameters, carla_msgs::msg::CarlaWeatherParametersPubSubType>;

class WeatherControlSubscriber : public SubscriberBase<carla_msgs::msg::CarlaWeatherParameters> {
public:
  explicit WeatherControlSubscriber(ROS2NameRecord& parent,
                                   carla::rpc::RpcServerInterface &carla_server);
  virtual ~WeatherControlSubscriber() = default;

  /**
   * Implements SubscriberBase::ProcessMessages()
   */
  void ProcessMessages() override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  std::shared_ptr<WeatherControlSubscriberImpl> _impl;
  carla::rpc::RpcServerInterface &_carla_server;
};
}  // namespace ros2
}  // namespace carla
