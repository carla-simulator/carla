// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <boost/variant2/variant.hpp>
#pragma warning(pop)
#else
#include <boost/variant2/variant.hpp>
#endif

#include <functional>
#include <vector>
#include <string>

namespace carla {
namespace ros2 {

  // ===========================================================================
  // -- Control interface ------------------------------------------------------
  // ===========================================================================

  struct VehicleControl
  {
    float   throttle;
    float   steer;
    float   brake;
    bool    hand_brake;
    bool    reverse;
    int32_t gear;
    bool    manual_gear_shift;
  };

  struct AckermannControl
  {
    float steer;
    float steer_speed;
    float speed;
    float acceleration;
    float jerk;
  };

  // ===========================================================================
  // -- Pseudo publishers structures ----------------------------------------
  // ===========================================================================
  struct VehicleOdometryReport {
    float x;
    float y;
    float z;
    float roll;
    float pitch;
    float yaw;
    float linear_velocity_x;
    float linear_velocity_y;
    float linear_velocity_z;
    float angular_velocity_x;
    float angular_velocity_y;
    float angular_velocity_z;
  };

  struct VehicleChassisReport {
    bool  engine_started;
    float engine_rpm;
    float speed_mps;
    float throttle_percentage;
    float brake_percentage;
    float steering_percentage;
  };

  struct PolygonPoint {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
  };

  struct ObstaclePrediction {
    int id;
    float pos_x, pos_y, pos_z;
    float yaw;
    float speed_x, speed_y, speed_z;
    float length, width, height;
    float corner_points[12];
    double tracking_time;
    double timestamp;
    float acc_x, acc_y, acc_z;
    float anchor_x, anchor_y, anchor_z;
    std::string type;
  };

  struct ObstacleReport {
    std::vector<ObstaclePrediction> obstacles;
  };

  enum class TrafficLightState: uint8_t {
    Red = 0,
    Yellow = 1,
    Green = 2,
    Off = 3
  };
  struct TrafficLight {
    std::string id;
    TrafficLightState light_state;
    int confidence;
    double tracking_time;
    bool blink;
    bool contains_lights;
  };
  
  struct TrafficLightReport {
    std::vector<TrafficLight> traffic_lights;
  };
  
  using ROS2CallbackData = boost::variant2::variant<
    VehicleControl,
    AckermannControl,
    // Pseudo publishers structures
    VehicleOdometryReport,
    VehicleChassisReport,
    ObstacleReport,
    TrafficLightReport
  >;

  using ActorCallback = std::function<void(void *actor, ROS2CallbackData &data)>;

} // namespace ros2
} // namespace carla
