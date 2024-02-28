// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/ros2/publishers/UePublisherBaseSensor.h"
#include "carla/sensor/s11n/GnssSerializer.h"
#include "sensor_msgs/msg/NavSatFixPubSubTypes.h"

namespace carla {
namespace ros2 {

using UeGNSSPublisherImpl = DdsPublisherImpl<sensor_msgs::msg::NavSatFix, sensor_msgs::msg::NavSatFixPubSubType>;

class UeGNSSPublisher : public UePublisherBaseSensor {
public:
  UeGNSSPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                  std::shared_ptr<TransformPublisher> transform_publisher);
  virtual ~UeGNSSPublisher() = default;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;
  /**
   * Implement PublisherInterface::SubsribersConnected interface
   */
  bool SubsribersConnected() const override;

  /**
   * Implements UePublisherBaseSensor::UpdateSensorData() interface
   */
  void UpdateSensorData(std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
                        const carla::SharedBufferView buffer_view) override;

private:
  using GnssDataConst = carla::geom::GeoLocation const;

  std::shared_ptr<GnssDataConst> data_view_ptr(carla::SharedBufferView buffer_view) {
    return std::shared_ptr<GnssDataConst>(buffer_view, reinterpret_cast<GnssDataConst*>(buffer_view->data()));
  }

  std::shared_ptr<UeGNSSPublisherImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
