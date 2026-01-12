// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/PublisherBaseSensor.h"
#include "carla/ros2/types/Object.h"
#include "derived_object_msgs/msg/ObjectArrayPubSubTypes.h"

namespace carla {
namespace ros2 {

using ObjectsPublisherImpl =
    DdsPublisherImpl<derived_object_msgs::msg::ObjectArray, derived_object_msgs::msg::ObjectArrayPubSubType>;

class ObjectsPublisher : public PublisherBaseSensor {
public:
  enum class ObjectMode {
    DYNAMIC_PUBLISH_ALWAYS,
    DYNAMIC_PUBLISH_ON_CHANGE,
    STATIC_PUBLISH_ONCE
  };

  ObjectsPublisher(ObjectMode const update_mode, std::string role_name = "objects");
  virtual ~ObjectsPublisher() = default;

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

  void UpdateHeader(const builtin_interfaces::msg::Time &stamp);

  void UpdateObject(std::shared_ptr<const carla::ros2::types::Object> &object);

  void AddObject(std::shared_ptr<const carla::ros2::types::Object> &object)
  {
    AddObject(*object);
  }

  void AddObject(carla::ros2::types::Object const &object);
  
  void RemoveObject(uint64_t const object_id);

private:
  std::shared_ptr<ObjectsPublisherImpl> _impl;
  ObjectMode const _update_mode;
};
}  // namespace ros2
}  // namespace carla
