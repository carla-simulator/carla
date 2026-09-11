// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/AutowareVehicleStatusPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/AutowareSteeringCompensation.h"
#include "carla/ros2/middleware/QosProfile.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/types/msg/ControlModeReport.h"
#include "carla/ros2/types/msg/GearReport.h"
#include "carla/ros2/types/msg/HazardLightsReport.h"
#include "carla/ros2/types/msg/SteeringReport.h"
#include "carla/ros2/types/msg/TurnIndicatorsReport.h"
#include "carla/ros2/types/msg/VelocityReport.h"

namespace carla {
namespace ros2 {

namespace {
constexpr const char *kFrameId = "base_link";
}  // namespace

struct AutowareVelocityReportTraits { using msg_type = msg::VelocityReport; };
struct AutowareSteeringReportTraits { using msg_type = msg::SteeringReport; };
struct AutowareControlModeReportTraits { using msg_type = msg::ControlModeReport; };
struct AutowareGearReportTraits { using msg_type = msg::GearReport; };
struct AutowareTurnIndicatorsReportTraits { using msg_type = msg::TurnIndicatorsReport; };
struct AutowareHazardLightsReportTraits { using msg_type = msg::HazardLightsReport; };

class AutowareVehicleStatusPublisher::Impl {
public:
  PublisherImpl<AutowareVelocityReportTraits> velocity;
  PublisherImpl<AutowareSteeringReportTraits> steering;
  PublisherImpl<AutowareControlModeReportTraits> control_mode;
  PublisherImpl<AutowareGearReportTraits> gear;
  PublisherImpl<AutowareTurnIndicatorsReportTraits> turn_indicators;
  PublisherImpl<AutowareHazardLightsReportTraits> hazard_lights;
};

AutowareVehicleStatusPublisher::AutowareVehicleStatusPublisher()
  : BasePublisher("rt/vehicle/status", kFrameId),
    _impl(std::make_shared<Impl>()) {
  // QoS per tier4's AutowarePublisher: RELIABLE / VOLATILE / KEEP_LAST 1.
  const QosProfile qos = QosProfile::ReliableVolatile();
  bool ok = true;
  ok &= _impl->velocity.Init("rt/vehicle/status/velocity_status", qos);
  ok &= _impl->steering.Init("rt/vehicle/status/steering_status", qos);
  ok &= _impl->control_mode.Init("rt/vehicle/status/control_mode", qos);
  ok &= _impl->gear.Init("rt/vehicle/status/gear_status", qos);
  ok &= _impl->turn_indicators.Init("rt/vehicle/status/turn_indicators_status", qos);
  ok &= _impl->hazard_lights.Init("rt/vehicle/status/hazard_lights_status", qos);
  if (!ok) {
    log_error("AutowareVehicleStatusPublisher: failed to initialise one or more report writers");
  }
}

AutowareVehicleStatusPublisher::~AutowareVehicleStatusPublisher() = default;

void AutowareVehicleStatusPublisher::SetVelocity(
    float longitudinal_velocity, float lateral_velocity, float heading_rate) {
  auto *report = _impl->velocity.GetMessage();
  // Leave header.stamp empty; Publish() populates the stamp.
  report->header.frame_id = kFrameId;
  report->longitudinal_velocity = longitudinal_velocity;
  report->lateral_velocity = lateral_velocity;
  report->heading_rate = heading_rate;
}

void AutowareVehicleStatusPublisher::SetSteering(float steering_tire_angle) {
  auto *report = _impl->steering.GetMessage();
  // Report the *actual* tire angle: apply the inverse compensation LUT so the
  // published state matches what the command-side GetSteeringInput mapping
  // asked the vehicle to do (tier4 AutowarePublisher::SetSteering).
  report->steering_tire_angle =
      autoware_steering_compensation::GetSteeringOutput(steering_tire_angle);
}

void AutowareVehicleStatusPublisher::SetControlMode(const ControlMode mode) {
  auto *report = _impl->control_mode.GetMessage();
  switch (mode) {
#define CASE(DATUM)                                   \
  case ControlMode::DATUM:                            \
    report->mode = msg::ControlModeReport::DATUM;     \
    break;                                            \
    static_assert(true, "")

    CASE(NO_COMMAND);
    CASE(AUTONOMOUS);
    CASE(AUTONOMOUS_STEER_ONLY);
    CASE(AUTONOMOUS_VELOCITY_ONLY);
    CASE(MANUAL);
    CASE(DISENGAGED);
    CASE(NOT_READY);

#undef CASE
  }
}

void AutowareVehicleStatusPublisher::SetGear(const Gear gear) {
  auto *report = _impl->gear.GetMessage();
  switch (gear) {
#define CASE(DATUM)                              \
  case Gear::DATUM:                              \
    report->report = msg::GearReport::DATUM;     \
    break;                                       \
    static_assert(true, "")

    CASE(NONE);
    CASE(NEUTRAL);
    CASE(DRIVE);
    CASE(DRIVE_2);
    CASE(DRIVE_3);
    CASE(DRIVE_4);
    CASE(DRIVE_5);
    CASE(DRIVE_6);
    CASE(DRIVE_7);
    CASE(DRIVE_8);
    CASE(DRIVE_9);
    CASE(DRIVE_10);
    CASE(DRIVE_11);
    CASE(DRIVE_12);
    CASE(DRIVE_13);
    CASE(DRIVE_14);
    CASE(DRIVE_15);
    CASE(DRIVE_16);
    CASE(DRIVE_17);
    CASE(DRIVE_18);
    CASE(REVERSE);
    CASE(REVERSE_2);
    CASE(PARK);
    CASE(LOW);
    CASE(LOW_2);

#undef CASE
  }
}

void AutowareVehicleStatusPublisher::SetTurnIndicators(const TurnIndicatorsStatus status) {
  auto *report = _impl->turn_indicators.GetMessage();
  switch (status) {
    case TurnIndicatorsStatus::OFF:
      report->report = msg::TurnIndicatorsReport::DISABLE;
      break;
    case TurnIndicatorsStatus::LEFT:
      report->report = msg::TurnIndicatorsReport::ENABLE_LEFT;
      break;
    case TurnIndicatorsStatus::RIGHT:
      report->report = msg::TurnIndicatorsReport::ENABLE_RIGHT;
      break;
  }
}

void AutowareVehicleStatusPublisher::SetHazardLights(const bool hazard_lights_enabled) {
  auto *report = _impl->hazard_lights.GetMessage();
  report->report = hazard_lights_enabled ? msg::HazardLightsReport::ENABLE
                                         : msg::HazardLightsReport::DISABLE;
}

bool AutowareVehicleStatusPublisher::Publish(
    std::int32_t seconds, std::uint32_t nanoseconds) {
  _seconds = seconds;
  _nanoseconds = nanoseconds;
  return Publish();
}

bool AutowareVehicleStatusPublisher::Publish() {
  // All reports are published at once with the same stamp based on the
  // assumption that they share one frequency.
  // https://github.com/tier4/AWSIM/blob/v1.3.1/docs/Components/ROS2/ROS2TopicAndServiceList/index.md
  _impl->velocity.GetMessage()->header.stamp.sec = _seconds;
  _impl->velocity.GetMessage()->header.stamp.nanosec = _nanoseconds;
  _impl->steering.GetMessage()->stamp.sec = _seconds;
  _impl->steering.GetMessage()->stamp.nanosec = _nanoseconds;
  _impl->control_mode.GetMessage()->stamp.sec = _seconds;
  _impl->control_mode.GetMessage()->stamp.nanosec = _nanoseconds;
  _impl->gear.GetMessage()->stamp.sec = _seconds;
  _impl->gear.GetMessage()->stamp.nanosec = _nanoseconds;
  _impl->turn_indicators.GetMessage()->stamp.sec = _seconds;
  _impl->turn_indicators.GetMessage()->stamp.nanosec = _nanoseconds;
  _impl->hazard_lights.GetMessage()->stamp.sec = _seconds;
  _impl->hazard_lights.GetMessage()->stamp.nanosec = _nanoseconds;

  bool ok = true;
  ok &= _impl->velocity.Publish();
  ok &= _impl->steering.Publish();
  ok &= _impl->control_mode.Publish();
  ok &= _impl->gear.Publish();
  ok &= _impl->turn_indicators.Publish();
  ok &= _impl->hazard_lights.Publish();
  return ok;
}

}  // namespace ros2
}  // namespace carla
