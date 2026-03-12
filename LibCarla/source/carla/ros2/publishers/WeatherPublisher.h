// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/PublisherBase.h"
#include "carla/rpc/RpcServerInterface.h"
#include "carla_msgs/msg/CarlaWeatherParametersPubSubTypes.h"

namespace carla {
namespace ros2 {

using CarlaWeatherParametersPublisherImpl =
    DdsPublisherImpl<carla_msgs::msg::CarlaWeatherParameters, carla_msgs::msg::CarlaWeatherParametersPubSubType>;

class WeatherPublisher : public PublisherBase {
public:
  WeatherPublisher(carla::rpc::RpcServerInterface &carla_server);
  virtual ~WeatherPublisher() = default;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;
  /**
   * Implement PublisherInterface::SubscribersConnected interface
   */
  bool SubscribersConnected() const override;

  /**
   * Query the not streamed data from the server before processing the sensor data.
   */
  void UpdateSensorDataPreAction() override;

private:
  std::shared_ptr<CarlaWeatherParametersPublisherImpl> _impl;
  carla::rpc::RpcServerInterface &_carla_server;
};
}  // namespace ros2
}  // namespace carla
