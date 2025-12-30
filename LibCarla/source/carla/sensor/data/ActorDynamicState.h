// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Transform.h"
#include "carla/geom/Quaternion.h"
#include "carla/geom/Vector3D.h"
#include "carla/rpc/ActorId.h"
#include "carla/rpc/ActorState.h"
#include "carla/rpc/VehicleFailureState.h"
#include "carla/rpc/TrafficLightState.h"
#include "carla/rpc/VehicleAckermannControl.h"
#include "carla/rpc/VehicleControl.h"
#include "carla/rpc/VehicleControlType.h"
#include "carla/rpc/WalkerControl.h"

#include <cstdint>

namespace carla {
namespace sensor {
namespace data {

namespace detail {

#pragma pack(push, 1)
  class PackedVehicleControl {
  public:

    PackedVehicleControl() = default;

    PackedVehicleControl(const rpc::VehicleControl &control)
      : throttle(control.throttle),
        steer(control.steer),
        brake(control.brake),
        hand_brake(control.hand_brake),
        reverse(control.reverse),
        manual_gear_shift(control.manual_gear_shift),
        gear(control.gear),
        timestamp(control.timestamp) {}

    operator rpc::VehicleControl() const {
      return {throttle, steer, brake, hand_brake, reverse, manual_gear_shift, gear, timestamp};
    }

  private:

    float throttle = 0.f;
    float steer = 0.f;
    float brake = 0.f;
    bool hand_brake = true;
    bool reverse = false;
    bool manual_gear_shift = false;
    int32_t gear = 0;
    float timestamp = 0.f;
  };

 class PackedVehicleAckermannControl {
  public:
  
    PackedVehicleAckermannControl() = default;

    PackedVehicleAckermannControl(const rpc::VehicleAckermannControl &control)
      : steer(control.steer),
        steer_speed(control.steer_speed),
        speed(control.speed),
        acceleration(control.acceleration),
        jerk(control.jerk),
        timestamp(control.timestamp) {}

    operator rpc::VehicleAckermannControl() const {
      return {steer, steer_speed, speed, acceleration, jerk, timestamp};
    }
  private:
    float steer = 0.f;
    float steer_speed = 0.f;
    float speed = 0.f;
    float acceleration = 0.f;
    float jerk = 0.f;
    float timestamp = 0.f;
 };

  struct VehicleData {
    VehicleData()=default;

    rpc::VehicleControlType control_type{ rpc::VehicleControlType::VehicleControl};
    union ControlTypeDependentData {
      PackedVehicleControl vehicle_control;
      PackedVehicleAckermannControl ackermann_control;
    } control_data {PackedVehicleControl()};
    
    rpc::VehicleControl GetVehicleControl() const {
      if ( rpc::VehicleControlType::VehicleControl == control_type ) {
        return control_data.vehicle_control;
      }
      return rpc::VehicleControl();
    }

    rpc::VehicleAckermannControl GetAckermannControl() const {
      if ( rpc::VehicleControlType::AckermannControl == control_type ) {
        return control_data.ackermann_control;
      }
      return rpc::VehicleAckermannControl();
    }

    float speed_limit;
    rpc::TrafficLightState traffic_light_state;
    bool has_traffic_light;
    rpc::ActorId traffic_light_id;
    rpc::VehicleFailureState failure_state;
  };

  class PackedWalkerControl {
  public:

    PackedWalkerControl() = default;

    PackedWalkerControl(const rpc::WalkerControl &control)
      : direction{control.direction.x, control.direction.y, control.direction.z},
        speed(control.speed),
        jump(control.jump),
        timestamp(control.timestamp) {}

    operator rpc::WalkerControl() const {
      return {geom::Vector3D{direction[0u], direction[1u], direction[2u]}, speed, jump, timestamp};
    }

  private:

    float direction[3u];
    float speed;
    bool jump;
    float timestamp;
  };

  struct TrafficLightData {
    TrafficLightData() = default;

    char sign_id[32u];
    float green_time;
    float yellow_time;
    float red_time;
    float elapsed_time;
    uint32_t pole_index;
    bool time_is_frozen;
    rpc::TrafficLightState state;
  };

  struct TrafficSignData {
    TrafficSignData() = default;

    char sign_id[32u];
  };
} // namespace detail

  /// Dynamic state of an actor at a certain frame.
  struct ActorDynamicState {

    ActorId id {0};

    rpc::ActorState actor_state;

    geom::Transform transform;

    geom::Quaternion quaternion;

    geom::Vector3D velocity;

    geom::Vector3D angular_velocity;

    geom::Vector3D acceleration;

    union TypeDependentState {
      detail::TrafficLightData traffic_light_data;
      detail::TrafficSignData traffic_sign_data;
      detail::VehicleData vehicle_data{};
      detail::PackedWalkerControl walker_control;
    } state;
  };

#pragma pack(pop)

static_assert(
    sizeof(ActorDynamicState) == 135u,
    "Invalid ActorDynamicState size! "
    "If you modified this class please update the size here, else you may "
    "comment this assert, but your platform may have compatibility issues "
    "connecting to other platforms.");

} // namespace data
} // namespace sensor
} // namespace carla
