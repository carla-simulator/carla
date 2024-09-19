// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector2D.h"
#include "carla/geom/Vector3D.h"
#include "carla/geom/Location.h"
#include "carla/MsgPack.h"



namespace carla {
  namespace rpc {

    struct WheelPhysicsControl {

      uint8_t axle_type = 0; // @TODO INTRODUCE ENUM
      geom::Vector3D offset = geom::Vector3D(0, 0, 0);
      float wheel_radius = 30.0f;
      float wheel_width = 30.0f;
      float wheel_mass = 30.0f;
      float cornering_stiffness = 1000.0f;
      float friction_force_multiplier = 3.0f;
      float side_slip_modifier = 1.0f;
      float slip_threshold = 20.0f;
      float skid_threshold = 20.0f;
      float max_steer_angle = 70.0f;
      bool affected_by_steering = true;
      bool affected_by_brake = true;
      bool affected_by_handbrake = true;
      bool affected_by_engine = true;
      bool abs_enabled = false;
      bool traction_control_enabled = false;
      float max_wheelspin_rotation = 30;
      uint8_t external_torque_combine_method = 0; // @TODO INTRODUCE ENUM
      std::vector<geom::Vector2D> lateral_slip_graph = {};
      geom::Vector3D suspension_axis = geom::Vector3D(0, 0, -1);
      geom::Vector3D suspension_force_offset = geom::Vector3D(0, 0, 0);
      float suspension_max_raise = 10.0f;
      float suspension_max_drop = 10.0f;
      float suspension_damping_ratio = 0.5f;
      float wheel_load_ratio = 0.5f;
      float spring_rate = 250.0f;
      float spring_preload = 50.0f;
      int suspension_smoothing = 0;
      float rollbar_scaling = 0.15f;
      uint8_t sweep_shape = 0; // @TODO INTRODUCE ENUM
      uint8_t sweep_type = 0; // @TODO INTRODUCE ENUM
      float max_brake_torque = 1500.0f;
      float max_hand_brake_torque = 3000.0f;
      int32_t wheel_index = -1;
      geom::Location location = geom::Location(0, 0, 0);
      geom::Location old_location = geom::Location(0, 0, 0);
      geom::Location velocity = geom::Location(0, 0, 0);

      inline bool operator==(const WheelPhysicsControl& rhs) const {
        const bool cmp[] =
        {
          axle_type == rhs.axle_type,
          offset == rhs.offset,
          wheel_radius == rhs.wheel_radius,
          wheel_width == rhs.wheel_width,
          wheel_mass == rhs.wheel_mass,
          cornering_stiffness == rhs.cornering_stiffness,
          friction_force_multiplier == rhs.friction_force_multiplier,
          side_slip_modifier == rhs.side_slip_modifier,
          slip_threshold == rhs.slip_threshold,
          skid_threshold == rhs.skid_threshold,
          max_steer_angle == rhs.max_steer_angle,
          affected_by_steering == rhs.affected_by_steering,
          affected_by_brake == rhs.affected_by_brake,
          affected_by_handbrake == rhs.affected_by_handbrake,
          affected_by_engine == rhs.affected_by_engine,
          abs_enabled == rhs.abs_enabled,
          traction_control_enabled == rhs.traction_control_enabled,
          max_wheelspin_rotation == rhs.max_wheelspin_rotation,
          external_torque_combine_method == rhs.external_torque_combine_method,
          lateral_slip_graph == rhs.lateral_slip_graph,
          suspension_axis == rhs.suspension_axis,
          suspension_force_offset == rhs.suspension_force_offset,
          suspension_max_raise == rhs.suspension_max_raise,
          suspension_max_drop == rhs.suspension_max_drop,
          suspension_damping_ratio == rhs.suspension_damping_ratio,
          wheel_load_ratio == rhs.wheel_load_ratio,
          spring_rate == rhs.spring_rate,
          spring_preload == rhs.spring_preload,
          suspension_smoothing == rhs.suspension_smoothing,
          rollbar_scaling == rhs.rollbar_scaling,
          sweep_shape == rhs.sweep_shape,
          sweep_type == rhs.sweep_type,
          max_brake_torque == rhs.max_brake_torque,
          max_hand_brake_torque == rhs.max_hand_brake_torque,
          wheel_index == rhs.wheel_index,
          location == rhs.location,
          old_location == rhs.old_location,
          velocity == rhs.velocity
        };
        return std::all_of(std::begin(cmp), std::end(cmp), std::identity());
      }

      inline bool operator!=(const WheelPhysicsControl& rhs) const {
        return !(*this == rhs);
      }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

      static WheelPhysicsControl FromFWheelPhysicsControl(
        const FWheelPhysicsControl& Wheel)
      {
        WheelPhysicsControl Out = { };
        Out.axle_type = (uint8_t)Wheel.AxleType;
        Out.offset = geom::Vector3D(
          Wheel.Offset.X,
          Wheel.Offset.Y,
          Wheel.Offset.Z);
        Out.wheel_radius = Wheel.WheelRadius;
        Out.wheel_width = Wheel.WheelWidth;
        Out.wheel_mass = Wheel.WheelMass;
        Out.cornering_stiffness = Wheel.CorneringStiffness;
        Out.friction_force_multiplier = Wheel.FrictionForceMultiplier;
        Out.side_slip_modifier = Wheel.SideSlipModifier;
        Out.slip_threshold = Wheel.SlipThreshold;
        Out.skid_threshold = Wheel.SkidThreshold;
        Out.max_steer_angle = Wheel.MaxSteerAngle;
        Out.affected_by_steering = Wheel.bAffectedBySteering;
        Out.affected_by_brake = Wheel.bAffectedByBrake;
        Out.affected_by_handbrake = Wheel.bAffectedByHandbrake;
        Out.affected_by_engine = Wheel.bAffectedByEngine;
        Out.abs_enabled = Wheel.bABSEnabled;
        Out.traction_control_enabled = Wheel.bTractionControlEnabled;
        Out.max_wheelspin_rotation = Wheel.MaxWheelspinRotation;
        Out.external_torque_combine_method = (uint8_t)Wheel.ExternalTorqueCombineMethod;
        Out.lateral_slip_graph.reserve(Wheel.LateralSlipGraph.GetNumKeys());
        for (auto& e : Wheel.LateralSlipGraph.GetConstRefOfKeys())
          Out.lateral_slip_graph.push_back({ e.Time, e.Value });
        Out.suspension_axis = geom::Vector3D(
          Wheel.SuspensionAxis.X,
          Wheel.SuspensionAxis.Y,
          Wheel.SuspensionAxis.Z);
        Out.suspension_force_offset = geom::Vector3D(
          Wheel.SuspensionForceOffset.X,
          Wheel.SuspensionForceOffset.Y,
          Wheel.SuspensionForceOffset.Z);
        Out.suspension_max_raise = Wheel.SuspensionMaxRaise;
        Out.suspension_max_drop = Wheel.SuspensionMaxDrop;
        Out.suspension_damping_ratio = Wheel.SuspensionDampingRatio;
        Out.wheel_load_ratio = Wheel.WheelLoadRatio;
        Out.spring_rate = Wheel.SpringRate;
        Out.spring_preload = Wheel.SpringPreload;
        Out.suspension_smoothing = Wheel.SuspensionSmoothing;
        Out.rollbar_scaling = Wheel.RollbarScaling;
        Out.sweep_shape = (uint8_t)Wheel.SweepShape;
        Out.sweep_type = (uint8_t)Wheel.SweepType;
        Out.max_brake_torque = Wheel.MaxBrakeTorque;
        Out.max_hand_brake_torque = Wheel.MaxHandBrakeTorque;
        Out.wheel_index = Wheel.WheelIndex;
        Out.location = Wheel.Location;
        Out.old_location = Wheel.OldLocation;
        Out.velocity = Wheel.Velocity;
        return Out;
      }

      operator FWheelPhysicsControl() const {
        FWheelPhysicsControl Wheel;
        Wheel.AxleType = (EAxleType)axle_type;
        Wheel.Offset = FVector(offset.x, offset.y, offset.z);
        Wheel.WheelRadius = wheel_radius;
        Wheel.WheelWidth = wheel_width;
        Wheel.WheelMass = wheel_mass;
        Wheel.CorneringStiffness = cornering_stiffness;
        Wheel.FrictionForceMultiplier = friction_force_multiplier;
        Wheel.SideSlipModifier = side_slip_modifier;
        Wheel.SlipThreshold = slip_threshold;
        Wheel.SkidThreshold = skid_threshold;
        Wheel.MaxSteerAngle = max_steer_angle;
        Wheel.bAffectedBySteering = affected_by_steering;
        Wheel.bAffectedByBrake = affected_by_brake;
        Wheel.bAffectedByHandbrake = affected_by_handbrake;
        Wheel.bAffectedByEngine = affected_by_engine;
        Wheel.bABSEnabled = abs_enabled;
        Wheel.bTractionControlEnabled = traction_control_enabled;
        Wheel.MaxWheelspinRotation = max_wheelspin_rotation;
        Wheel.ExternalTorqueCombineMethod = (ETorqueCombineMethod)external_torque_combine_method;
        for (auto [x, y] : lateral_slip_graph)
          Wheel.LateralSlipGraph.AddKey(x, y);
        Wheel.SuspensionAxis = FVector(
          suspension_axis.x,
          suspension_axis.y,
          suspension_axis.z);
        Wheel.SuspensionForceOffset = FVector(
          suspension_force_offset.x,
          suspension_force_offset.y,
          suspension_force_offset.z);
        Wheel.SuspensionMaxRaise = suspension_max_raise;
        Wheel.SuspensionMaxDrop = suspension_max_drop;
        Wheel.SuspensionDampingRatio = suspension_damping_ratio;
        Wheel.WheelLoadRatio = wheel_load_ratio;
        Wheel.SpringRate = spring_rate;
        Wheel.SpringPreload = spring_preload;
        Wheel.SuspensionSmoothing = suspension_smoothing;
        Wheel.RollbarScaling = rollbar_scaling;
        Wheel.SweepShape = (ESweepShape)sweep_shape;
        Wheel.SweepType = (ESweepType)sweep_type;
        Wheel.MaxBrakeTorque = max_brake_torque;
        Wheel.MaxHandBrakeTorque = max_hand_brake_torque;
        Wheel.WheelIndex = wheel_index;
        Wheel.Location = location;
        Wheel.OldLocation = old_location;
        Wheel.Velocity = velocity;
        return Wheel;
      }
#endif

      MSGPACK_DEFINE_ARRAY(
        axle_type,
        offset,
        wheel_radius,
        wheel_width,
        wheel_mass,
        cornering_stiffness,
        friction_force_multiplier,
        side_slip_modifier,
        slip_threshold,
        skid_threshold,
        max_steer_angle,
        affected_by_steering,
        affected_by_brake,
        affected_by_handbrake,
        affected_by_engine,
        abs_enabled,
        traction_control_enabled,
        max_wheelspin_rotation,
        external_torque_combine_method,
        lateral_slip_graph,
        suspension_axis,
        suspension_force_offset,
        suspension_max_raise,
        suspension_max_drop,
        suspension_damping_ratio,
        wheel_load_ratio,
        spring_rate,
        spring_preload,
        suspension_smoothing,
        rollbar_scaling,
        sweep_shape,
        sweep_type,
        max_brake_torque,
        max_hand_brake_torque,
        wheel_index,
        location,
        old_location,
        velocity)
    };

  }
}
