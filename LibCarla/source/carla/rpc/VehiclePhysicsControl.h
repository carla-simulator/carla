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
    struct VehiclePhysicsControl
    {
      // Engine Setup:
      std::vector<geom::Vector2D> torque_curve = {
        geom::Vector2D(0.0f, 500.0f),
        geom::Vector2D(5000.0f, 500.0f)
      };

      float max_torque = 300.0f;
      float max_rpm = 5000.0f;
      float idle_rpm = 1.0f;
      float brake_effect = 1.0f;
      float rev_up_moi = 1.0f;
      float rev_down_rate = 600.0f;

      // ToDo: Convert to an enum, see EVehicleDifferential.
      uint8_t differential_type = 0;
      float front_rear_split = 0.5f;

      bool use_automatic_gears = true;
      float gear_change_time = 0.5f;
      float final_ratio = 4.0f;
      std::vector<float> forward_gear_ratios = { 2.85, 2.02, 1.35, 1.0, 2.85, 2.02, 1.35, 1.0 };
      std::vector<float> reverse_gear_ratios = { 2.86, 2.86 };
      float change_up_rpm = 4500.0f;
      float change_down_rpm = 2000.0f;
      float transmission_efficiency = 0.9f;

      float mass = 1000.0f;
      float drag_coefficient = 0.3f;
      geom::Location center_of_mass = geom::Location(0, 0, 0);
      float chassis_width = 180.f;
      float chassis_height = 140.f;
      float downforce_coefficient = 0.3f;
      float drag_area = 0.0f;
      geom::Vector3D inertia_tensor_scale = geom::Vector3D(1, 1, 1);
      float sleep_threshold = 10.0f;
      float sleep_slope_limit = 0.866f;

      std::vector<geom::Vector2D> steering_curve = {
        geom::Vector2D(0.0f, 1.0f),
        geom::Vector2D(10.0f, 0.5f)
      };
      std::vector<WheelPhysicsControl> wheels;

      bool use_sweep_wheel_collision = false;

      inline bool operator==(const VehiclePhysicsControl& rhs) const {
        const bool cmp[] = {
          torque_curve == rhs.torque_curve,
          max_torque == rhs.max_torque,
          max_rpm == rhs.max_rpm,
          idle_rpm == rhs.idle_rpm,
          brake_effect == rhs.brake_effect,
          rev_up_moi == rhs.rev_up_moi,
          rev_down_rate == rhs.rev_down_rate,
          differential_type == rhs.differential_type,
          front_rear_split == rhs.front_rear_split,
          use_automatic_gears == rhs.use_automatic_gears,
          gear_change_time == rhs.gear_change_time,
          final_ratio == rhs.final_ratio,
          forward_gear_ratios == rhs.forward_gear_ratios,
          reverse_gear_ratios == rhs.reverse_gear_ratios,
          change_up_rpm == rhs.change_up_rpm,
          change_down_rpm == rhs.change_down_rpm,
          transmission_efficiency == rhs.transmission_efficiency,
          mass == rhs.mass,
          drag_coefficient == rhs.drag_coefficient,
          center_of_mass == rhs.center_of_mass,
          chassis_width == rhs.chassis_width,
          chassis_height == rhs.chassis_height,
          downforce_coefficient == rhs.downforce_coefficient,
          drag_area == rhs.drag_area,
          inertia_tensor_scale == rhs.inertia_tensor_scale,
          sleep_threshold == rhs.sleep_threshold,
          sleep_slope_limit == rhs.sleep_slope_limit,
          steering_curve == rhs.steering_curve,
          wheels == rhs.wheels,
          use_sweep_wheel_collision == rhs.use_sweep_wheel_collision,
        };

        return std::all_of(
          std::begin(cmp),
          std::end(cmp),
          std::identity());
      }

      inline bool operator!=(const VehiclePhysicsControl& rhs) const {
        return !(*this == rhs);
      }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

      static VehiclePhysicsControl FromFVehiclePhysicsControl(
        const FVehiclePhysicsControl& Control) {
        VehiclePhysicsControl Out = { };
        Out.torque_curve.reserve(Control.TorqueCurve.GetNumKeys());
        for (auto& Key : Control.TorqueCurve.GetConstRefOfKeys())
          Out.torque_curve.push_back(geom::Vector2D(Key.Time, Key.Value));
        Out.max_torque = Control.MaxTorque;
        Out.max_rpm = Control.MaxRPM;
        Out.idle_rpm = Control.IdleRPM;
        Out.brake_effect = Control.BrakeEffect;
        Out.rev_up_moi = Control.RevUpMOI;
        Out.rev_down_rate = Control.RevDownRate;
        Out.differential_type = Control.DifferentialType;
        Out.front_rear_split = Control.FrontRearSplit;
        Out.use_automatic_gears = Control.bUseAutomaticGears;
        Out.gear_change_time = Control.GearChangeTime;
        Out.final_ratio = Control.FinalRatio;
        Out.forward_gear_ratios.resize(Control.ForwardGearRatios.Num());
        for (size_t i = 0; i != Out.forward_gear_ratios.size(); ++i)
          Out.forward_gear_ratios[i] = Control.ForwardGearRatios[i];
        Out.reverse_gear_ratios.resize(Control.ReverseGearRatios.Num());
        for (size_t i = 0; i != Out.reverse_gear_ratios.size(); ++i)
          Out.reverse_gear_ratios[i] = Control.ReverseGearRatios[i];
        Out.change_up_rpm = Control.ChangeUpRPM;
        Out.change_down_rpm = Control.ChangeDownRPM;
        Out.transmission_efficiency = Control.TransmissionEfficiency;
        Out.mass = Control.Mass;
        Out.drag_coefficient = Control.DragCoefficient;
        Out.center_of_mass = Control.CenterOfMass;
        Out.chassis_width = Control.ChassisWidth;
        Out.chassis_height = Control.ChassisHeight;
        Out.downforce_coefficient = Control.DownforceCoefficient;
        Out.drag_area = Control.DragArea;
        Out.inertia_tensor_scale = geom::Vector3D(
          Control.InertiaTensorScale.X,
          Control.InertiaTensorScale.Y,
          Control.InertiaTensorScale.Z);
        Out.sleep_threshold = Control.SleepThreshold;
        Out.sleep_slope_limit = Control.SleepSlopeLimit;
        Out.steering_curve.reserve(Control.SteeringCurve.GetNumKeys());
        for (auto& Key : Control.SteeringCurve.GetConstRefOfKeys())
          Out.steering_curve.push_back(geom::Vector2D(Key.Time, Key.Value));
        Out.wheels.resize(Control.Wheels.Num());
        for (size_t i = 0; i != Out.wheels.size(); ++i)
          Out.wheels[i] = WheelPhysicsControl::FromFWheelPhysicsControl(Control.Wheels[i]);
        Out.use_sweep_wheel_collision = Control.UseSweepWheelCollision;
        return Out;
      }

      operator FVehiclePhysicsControl() const {
        FVehiclePhysicsControl Control = { };
        for (auto [x, y] : torque_curve)
          Control.TorqueCurve.AddKey(x, y);
        Control.MaxTorque = max_torque;
        Control.MaxRPM = max_rpm;
        Control.IdleRPM = idle_rpm;
        Control.BrakeEffect = brake_effect;
        Control.RevUpMOI = rev_up_moi;
        Control.RevDownRate = rev_down_rate;
        Control.DifferentialType = differential_type;
        Control.FrontRearSplit = front_rear_split;
        Control.bUseAutomaticGears = use_automatic_gears;
        Control.GearChangeTime = gear_change_time;
        Control.FinalRatio = final_ratio;
        Control.ForwardGearRatios.SetNum(forward_gear_ratios.size());
        for (size_t i = 0; i != Control.ForwardGearRatios.Num(); ++i)
          Control.ForwardGearRatios[i] = forward_gear_ratios[i];
        Control.ReverseGearRatios.SetNum(reverse_gear_ratios.size());
        for (size_t i = 0; i != Control.ReverseGearRatios.Num(); ++i)
          Control.ReverseGearRatios[i] = reverse_gear_ratios[i];
        Control.ChangeUpRPM = change_up_rpm;
        Control.ChangeDownRPM = change_down_rpm;
        Control.TransmissionEfficiency = transmission_efficiency;
        Control.Mass = mass;
        Control.DragCoefficient = drag_coefficient;
        Control.CenterOfMass = center_of_mass;
        Control.ChassisWidth = chassis_width;
        Control.ChassisHeight = chassis_height;
        Control.DownforceCoefficient = downforce_coefficient;
        Control.DragArea = drag_area;
        Control.InertiaTensorScale = FVector(
          inertia_tensor_scale.x,
          inertia_tensor_scale.y,
          inertia_tensor_scale.z);
        Control.SleepThreshold = sleep_threshold;
        Control.SleepSlopeLimit = sleep_slope_limit;
        for (auto [x, y] : steering_curve)
          Control.SteeringCurve.AddKey(x, y);
        Control.Wheels.SetNum(wheels.size());
        for (size_t i = 0; i != Control.Wheels.Num(); ++i)
          Control.Wheels[i] = wheels[i];
        Control.UseSweepWheelCollision = use_sweep_wheel_collision;
        return Control;
      }

#endif

      MSGPACK_DEFINE_ARRAY(
        torque_curve,
        max_torque,
        max_rpm,
        idle_rpm,
        brake_effect,
        rev_up_moi,
        rev_down_rate,
        differential_type,
        front_rear_split,
        use_automatic_gears,
        gear_change_time,
        final_ratio,
        forward_gear_ratios,
        reverse_gear_ratios,
        change_up_rpm,
        change_down_rpm,
        transmission_efficiency,
        mass,
        drag_coefficient,
        center_of_mass,
        chassis_width,
        chassis_height,
        downforce_coefficient,
        drag_area,
        inertia_tensor_scale,
        sleep_threshold,
        sleep_slope_limit,
        steering_curve,
        wheels,
        use_sweep_wheel_collision
      );
    };

  } // namespace rpc
} // namespace carla
