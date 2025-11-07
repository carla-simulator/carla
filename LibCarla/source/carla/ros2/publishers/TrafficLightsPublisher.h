// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/PublisherBaseSensor.h"
#include "carla/rpc/ActorId.h"
#include "carla_msgs/msg/CarlaTrafficLightInfoListPubSubTypes.h"
#include "carla_msgs/msg/CarlaTrafficLightStatusListPubSubTypes.h"

namespace carla {
namespace ros2 {

using TrafficLightsInfoPublisherImpl =
    DdsPublisherImpl<carla_msgs::msg::CarlaTrafficLightInfoList, carla_msgs::msg::CarlaTrafficLightInfoListPubSubType>;
using TrafficLightsStatusPublisherImpl = DdsPublisherImpl<carla_msgs::msg::CarlaTrafficLightStatusList,
                                                          carla_msgs::msg::CarlaTrafficLightStatusListPubSubType>;

class TrafficLightsPublisher : public PublisherBaseSensor {
public:
  TrafficLightsPublisher();
  virtual ~TrafficLightsPublisher() = default;

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

  void UpdateTrafficLightStatus(carla_msgs::msg::CarlaTrafficLightStatus const &traffic_light_status);
  void UpdateTrafficLightInfo(carla_msgs::msg::CarlaTrafficLightInfo const &traffic_light_info);
  void RemoveTrafficLight(carla::streaming::detail::actor_id_type id);

private:
  std::shared_ptr<TrafficLightsInfoPublisherImpl> _traffic_light_info;
  std::shared_ptr<TrafficLightsStatusPublisherImpl> _traffic_light_status;
};
}  // namespace ros2
}  // namespace carla
