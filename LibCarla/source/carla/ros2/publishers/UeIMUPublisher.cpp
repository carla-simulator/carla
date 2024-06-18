// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#define _USE_MATH_DEFINES
#include <cmath>

#include "UeIMUPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"
#include "carla/ros2/types/Acceleration.h"
#include "carla/ros2/types/AngularVelocity.h"

namespace carla {
namespace ros2 {

UeIMUPublisher::UeIMUPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                               std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBaseSensor(sensor_actor_definition, transform_publisher),
    _impl(std::make_shared<UeIMUPublisherImpl>()) {}

bool UeIMUPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name(), get_topic_qos());
}

bool UeIMUPublisher::Publish() {
  return _impl->Publish();
}
bool UeIMUPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void UeIMUPublisher::UpdateSensorData(
    std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
    carla::SharedBufferView buffer_view) {
  auto imu_data = data(buffer_view);
  _impl->SetMessageHeader(GetTime(sensor_header), frame_id());
  _impl->Message().angular_velocity(carla::ros2::types::AngularVelocity(imu_data.gyroscope).angular_velocity());
  _impl->Message().linear_acceleration(carla::ros2::types::Acceleration(imu_data.accelerometer).accel().linear());

  /*
    TODO: original ROS bridge had taken the transform to provide a correct 3D orientation
    The question is how this should be implemented by the IMU accoringly
    Regardless, the transform of the IMU sensor can still be used within ROS in case the quaternion resulting in the 1D
    compass value is not sufficient
  */

  // optimized rotation calculation
  /*const float rp = 0.0f;                           // pitch*/
  const float ry = float(M_PI_2) - imu_data.compass;  // -yaw
  /*const float rr = 0.0f;                           // roll*/

  const float cr = 1.f;
  const float sr = 0.f;
  const float cp = 1.f;
  ;
  const float sp = 0.f;
  const float cy = cosf(ry * 0.5f);
  const float sy = sinf(ry * 0.5f);

  _impl->Message().orientation().w(cr * cp * cy + sr * sp * sy);
  _impl->Message().orientation().x(sr * cp * cy - cr * sp * sy);
  _impl->Message().orientation().y(cr * sp * cy + sr * cp * sy);
  _impl->Message().orientation().z(cr * cp * sy - sr * sp * cy);
}
}  // namespace ros2
}  // namespace carla
