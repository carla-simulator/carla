// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/BasePublisher.h"

#include <cstdint>
#include <memory>

namespace carla {
namespace ros2 {

// Port of tier4's AutowarePublisher: the six Autoware vehicle report
// publishers, re-expressed through PublisherImpl / IPublisherMiddleware
// instead of tier4's per-publisher Fast-DDS scaffolding
// (AutowarePublisherBase.hpp — superseded).
//
// Topics (fixed absolute names, DDS "rt/" prefix, frame_id "base_link",
// QoS RELIABLE / VOLATILE / KEEP_LAST depth 1 per tier4):
//   /vehicle/status/velocity_status        autoware_vehicle_msgs/VelocityReport
//   /vehicle/status/steering_status        autoware_vehicle_msgs/SteeringReport
//   /vehicle/status/control_mode           autoware_vehicle_msgs/ControlModeReport
//   /vehicle/status/gear_status            autoware_vehicle_msgs/GearReport
//   /vehicle/status/turn_indicators_status autoware_vehicle_msgs/TurnIndicatorsReport
//   /vehicle/status/hazard_lights_status   autoware_vehicle_msgs/HazardLightsReport

// https://github.com/autowarefoundation/autoware_msgs/blob/main/autoware_vehicle_msgs/msg/ControlModeReport.msg
enum class ControlMode {
  NO_COMMAND,
  AUTONOMOUS,
  AUTONOMOUS_STEER_ONLY,
  AUTONOMOUS_VELOCITY_ONLY,
  MANUAL,
  DISENGAGED,
  NOT_READY
};

// https://github.com/autowarefoundation/autoware_msgs/blob/main/autoware_vehicle_msgs/msg/GearReport.msg
enum class Gear {
  NONE,
  NEUTRAL,
  DRIVE,
  DRIVE_2,
  DRIVE_3,
  DRIVE_4,
  DRIVE_5,
  DRIVE_6,
  DRIVE_7,
  DRIVE_8,
  DRIVE_9,
  DRIVE_10,
  DRIVE_11,
  DRIVE_12,
  DRIVE_13,
  DRIVE_14,
  DRIVE_15,
  DRIVE_16,
  DRIVE_17,
  DRIVE_18,
  REVERSE,
  REVERSE_2,
  PARK,
  LOW,
  LOW_2
};

enum class TurnIndicatorsStatus {
  OFF,
  LEFT,
  RIGHT
};

class AutowareVehicleStatusPublisher : public BasePublisher {
public:
  AutowareVehicleStatusPublisher();
  ~AutowareVehicleStatusPublisher() override;

  AutowareVehicleStatusPublisher(const AutowareVehicleStatusPublisher &) = delete;
  AutowareVehicleStatusPublisher &operator=(const AutowareVehicleStatusPublisher &) = delete;

  void SetVelocity(float longitudinal_velocity, float lateral_velocity, float heading_rate);
  /// @param steering_tire_angle CARLA-side steering value; the inverse
  /// compensation LUT (GetSteeringOutput) is applied before publishing so
  /// Autoware sees the actual tire angle.
  void SetSteering(float steering_tire_angle);
  void SetControlMode(ControlMode mode);
  void SetGear(Gear gear);
  void SetTurnIndicators(TurnIndicatorsStatus status);
  void SetHazardLights(bool hazard_lights_enabled);

  /// Stamp all six pending reports with (seconds, nanoseconds) and publish
  /// them at once — all reports share the same frequency, matching AWSIM.
  bool Publish(std::int32_t seconds, std::uint32_t nanoseconds);

  /// BasePublisher interface; publishes with the last stamp given to the
  /// two-argument overload (0,0 before the first call).
  bool Publish() override;

private:
  class Impl;
  std::shared_ptr<Impl> _impl;
  std::int32_t _seconds{0};
  std::uint32_t _nanoseconds{0};
};

}  // namespace ros2
}  // namespace carla
