// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaEgoVehicleStatusPublisher.h"

#include "carla/ros2/publishers/UeToRosConversions.h"

namespace carla {
namespace ros2 {

bool CarlaEgoVehicleStatusPublisher::Write(
    int32_t seconds,
    uint32_t nanoseconds,
    std::string frame_id,
    const geom::Transform &transform,
    const geom::Vector3D &linear_velocity,
    float delta_seconds,
    const rpc::VehicleControl &control) {
  auto message = _impl->GetMessage();

  message->header.stamp.sec = seconds;
  message->header.stamp.nanosec = nanoseconds;
  message->header.frame_id = frame_id;

  message->velocity = linear_velocity.Length();

  // Acceleration from the per-publisher velocity history; the angular part
  // stays zero, mirroring the ros-bridge CarlaEgoVehicleStatus semantics.
  const geom::Vector3D acceleration = ComputeAcceleration(
      linear_velocity, _previous_velocity, _has_previous_velocity, delta_seconds);
  message->acceleration.linear = ue_vector_to_ros_vector(acceleration);
  _previous_velocity = linear_velocity;
  _has_previous_velocity = true;

  message->orientation = ue_rotation_to_ros_quaternion(transform.rotation);

  message->control.throttle = control.throttle;
  message->control.steer = control.steer;
  message->control.brake = control.brake;
  message->control.hand_brake = control.hand_brake;
  message->control.reverse = control.reverse;
  message->control.gear = control.gear;
  message->control.manual_gear_shift = control.manual_gear_shift;

  return true;
}

}  // namespace ros2
}  // namespace carla
