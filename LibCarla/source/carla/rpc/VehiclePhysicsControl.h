// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/geom/Location.h"
#include "carla/geom/Vector2D.h"
#include "carla/rpc/WheelPhysicsControl.h"

#include <string>
#include <vector>

namespace carla {
namespace rpc {
  class VehiclePhysicsControl {
  public:

    VehiclePhysicsControl() = default;

    VehiclePhysicsControl(
        const std::vector<carla::geom::Vector2D> &in_torque_curve,
        float in_max_torque,
        float in_max_rpm,
        float in_moi,
        float in_rev_down_rate,

        float in_front_rear_split,

        bool in_use_gear_autobox,
        float in_gear_switch_time,
        float in_final_ratio,
        std::vector<float> &in_forward_gears,
        std::vector<float> &in_reverse_gears,
        float in_change_up_rpm,
        float in_change_down_rpm,
        float in_transmission_efficiency,

        float in_mass,
        float in_drag_coefficient,
        geom::Location in_center_of_mass,
        const std::vector<carla::geom::Vector2D> &in_steering_curve,
        std::vector<WheelPhysicsControl> &in_wheels,
        bool in_use_sweep_wheel_collision)
      : torque_curve(in_torque_curve),
        max_torque(in_max_torque),
        max_rpm(in_max_rpm),
        moi(in_moi),
        rev_down_rate(in_rev_down_rate),
        front_rear_split(in_front_rear_split),
        use_gear_autobox(in_use_gear_autobox),
        gear_switch_time(in_gear_switch_time),
        final_ratio(in_final_ratio),
        forward_gears(in_forward_gears),
        reverse_gears(in_reverse_gears),
        change_up_rpm(in_change_up_rpm),
        change_down_rpm(in_change_down_rpm),
        transmission_efficiency(in_transmission_efficiency),
        mass(in_mass),
        drag_coefficient(in_drag_coefficient),
        center_of_mass(in_center_of_mass),
        steering_curve(in_steering_curve),
        wheels(in_wheels),
        use_sweep_wheel_collision(in_use_sweep_wheel_collision) {}

    const std::vector<float> &GetForwardGears() const {
      return forward_gears;
    }

    void SetForwardGears(std::vector<float> &in_forward_gears) {
      forward_gears = in_forward_gears;
    }

    const std::vector<float> &GetReverseGears() const {
      return reverse_gears;
    }

    void SetReverseGears(std::vector<float> &in_reverse_gears) {
      reverse_gears = in_reverse_gears;
    }

    const std::vector<WheelPhysicsControl> &GetWheels() const {
      return wheels;
    }

    void SetWheels(std::vector<WheelPhysicsControl> &in_wheels) {
      wheels = in_wheels;
    }

    const std::vector<geom::Vector2D> &GetTorqueCurve() const {
      return torque_curve;
    }

    void SetTorqueCurve(std::vector<geom::Vector2D> &in_torque_curve) {
      torque_curve = in_torque_curve;
    }

    const std::vector<geom::Vector2D> &GetSteeringCurve() const {
      return steering_curve;
    }

    void SetSteeringCurve(std::vector<geom::Vector2D> &in_steering_curve) {
      steering_curve = in_steering_curve;
    }

    void SetUseSweepWheelCollision(bool in_sweep) {
      use_sweep_wheel_collision = in_sweep;
    }

    bool GetUseSweepWheelCollision() {
      return use_sweep_wheel_collision;
    }

    std::vector<geom::Vector2D> torque_curve = {geom::Vector2D(0.0f, 500.0f), geom::Vector2D(5000.0f, 500.0f)};
    float max_torque = 300.0f;
    float max_rpm = 5000.0f;
    float moi = 1.0f;
    float rev_down_rate = 600.0f;

    float front_rear_split = 0.5f;

    bool use_gear_autobox = true;
    float gear_switch_time = 0.5f;
    float final_ratio = 4.0f;
    std::vector<float> forward_gears = {2.85, 2.02, 1.35, 1.0, 2.85, 2.02, 1.35, 1.0};
    std::vector<float> reverse_gears = {2.86, 2.86};
    float change_up_rpm = 4500.0f;
    float change_down_rpm = 2000.0f;
    float transmission_efficiency = 0.9f;

    float mass = 1000.0f;
    float drag_coefficient = 0.3f;
    geom::Location center_of_mass;

    std::vector<geom::Vector2D> steering_curve = {geom::Vector2D(0.0f, 1.0f), geom::Vector2D(10.0f, 0.5f)};
    std::vector<WheelPhysicsControl> wheels;

    bool use_sweep_wheel_collision = false;

    bool operator!=(const VehiclePhysicsControl &rhs) const {
      return
        max_torque != rhs.max_torque ||
        max_rpm != rhs.max_rpm ||
        moi != rhs.moi ||
        rev_down_rate != rhs.rev_down_rate ||
          
        front_rear_split != rhs.front_rear_split ||

        use_gear_autobox != rhs.use_gear_autobox ||
        gear_switch_time != rhs.gear_switch_time ||
        final_ratio != rhs.final_ratio ||
        forward_gears != rhs.forward_gears ||
        reverse_gears != rhs.reverse_gears ||
        change_up_rpm != rhs.change_up_rpm ||
        change_down_rpm != rhs.change_down_rpm ||
        transmission_efficiency != rhs.transmission_efficiency ||

        mass != rhs.mass ||
        drag_coefficient != rhs.drag_coefficient ||
        steering_curve != rhs.steering_curve ||
        center_of_mass != rhs.center_of_mass ||
        wheels != rhs.wheels ||
        use_sweep_wheel_collision != rhs.use_sweep_wheel_collision;
    }

    bool operator==(const VehiclePhysicsControl &rhs) const {
      return !(*this != rhs);
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    VehiclePhysicsControl(const FVehiclePhysicsControl &Control) {
      // Engine Setup
      torque_curve = std::vector<carla::geom::Vector2D>();
      TArray<FRichCurveKey> TorqueCurveKeys = Control.TorqueCurve.GetCopyOfKeys();
      for (int32 KeyIdx = 0; KeyIdx < TorqueCurveKeys.Num(); KeyIdx++) {
        geom::Vector2D point(TorqueCurveKeys[KeyIdx].Time, TorqueCurveKeys[KeyIdx].Value);
        torque_curve.push_back(point);
      }
      max_torque = Control.MaxTorque;
      max_rpm = Control.MaxRPM;
      moi = Control.MOI;
      rev_down_rate = Control.RevDownRate;

      // Differential Setup
      front_rear_split = Control.FrontRearSplit;

      // Transmission Setup
      use_gear_autobox = Control.bUseGearAutoBox;
      gear_switch_time = Control.GearSwitchTime;
      final_ratio = Control.FinalRatio;
      change_up_rpm = Control.ChangeUpRPM;
      change_down_rpm = Control.ChangeDownRPM;
      transmission_efficiency = Control.TransmissionEfficiency;
      forward_gears = std::vector<float>();
      forward_gears.reserve(Control.ForwardGears.Num());
      for (const auto &Gear : Control.ForwardGears) {
        forward_gears.push_back(Gear);
      }
      reverse_gears = std::vector<float>();
      reverse_gears.reserve(Control.ReverseGears.Num());
      for (const auto &Gear : Control.ReverseGears) {
        reverse_gears.push_back(Gear);
      }

      // Vehicle Setup
      mass = Control.Mass;
      drag_coefficient = Control.DragCoefficient;

      steering_curve = std::vector<carla::geom::Vector2D>();
      TArray<FRichCurveKey> SteeringCurveKeys = Control.SteeringCurve.GetCopyOfKeys();
      for (int32 KeyIdx = 0; KeyIdx < SteeringCurveKeys.Num(); KeyIdx++) {
        geom::Vector2D point(SteeringCurveKeys[KeyIdx].Time, SteeringCurveKeys[KeyIdx].Value);
        steering_curve.push_back(point);
      }

      center_of_mass = Control.CenterOfMass;

      // Wheels Setup
      wheels = std::vector<WheelPhysicsControl>();
      for (const auto &Wheel : Control.Wheels) {
        wheels.push_back(WheelPhysicsControl(Wheel));
      }

      use_sweep_wheel_collision = Control.UseSweepWheelCollision;
    }

    operator FVehiclePhysicsControl() const {
      FVehiclePhysicsControl Control;

      // Engine Setup
      FRichCurve TorqueCurve;
      for (const auto &point : torque_curve) {
        TorqueCurve.AddKey(point.x, point.y);
      }
      Control.TorqueCurve = TorqueCurve;
      Control.MaxTorque = max_torque;
      Control.MaxRPM = max_rpm;
      Control.MOI = moi;
      Control.RevDownRate = rev_down_rate;

      // Differential Setup
      Control.FrontRearSplit = front_rear_split;
      
      // Transmission Setup
      Control.bUseGearAutoBox = use_gear_autobox;
      Control.GearSwitchTime = gear_switch_time;
      Control.FinalRatio = final_ratio;
      Control.ChangeUpRPM = change_up_rpm;
      Control.ChangeDownRPM = change_down_rpm;
      Control.TransmissionEfficiency = transmission_efficiency;
      TArray<float> ForwardGears;
      ForwardGears.Reserve(forward_gears.size());
      for (const auto &gear : forward_gears) {
        ForwardGears.Add(gear);
      }
      Control.ForwardGears = ForwardGears;
      TArray<float> ReverseGears;
      ReverseGears.Reserve(reverse_gears.size());
      for (const auto &gear : reverse_gears) {
        ReverseGears.Add(gear);
      }
      Control.ReverseGears = ReverseGears;


      // Vehicle Setup
      Control.Mass = mass;
      Control.DragCoefficient = drag_coefficient;

      // Transmission Setup
      FRichCurve SteeringCurve;
      for (const auto &point : steering_curve) {
        SteeringCurve.AddKey(point.x, point.y);
      }
      Control.SteeringCurve = SteeringCurve;

      Control.CenterOfMass = center_of_mass;

      // Wheels Setup
      TArray<FWheelPhysicsControl> Wheels;
      for (const auto &wheel : wheels) {
        Wheels.Add(FWheelPhysicsControl(wheel));
      }
      Control.Wheels = Wheels;

      Control.UseSweepWheelCollision = use_sweep_wheel_collision;

      return Control;
    }

#endif

    MSGPACK_DEFINE_ARRAY(torque_curve,
        max_torque,
        max_rpm,
        moi,
        rev_down_rate,
        front_rear_split,
        use_gear_autobox,
        gear_switch_time,
        final_ratio,
        forward_gears,
        reverse_gears,
        change_up_rpm,
        change_down_rpm,
        transmission_efficiency,
        mass,
        drag_coefficient,
        center_of_mass,
        steering_curve,
        wheels,
        use_sweep_wheel_collision);
  };

} // namespace rpc
} // namespace carla
