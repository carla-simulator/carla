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

			VehiclePhysicsControl() = default;

			VehiclePhysicsControl(
				// Engine Setup:
				const std::vector<carla::geom::Vector2D>& in_torque_curve,
				float in_max_torque,
				float in_max_rpm,
				float in_idle_rpm,
				float in_brake_effect,
				float in_rev_up_moi,
				float in_rev_down_rate,

				uint8_t in_differential_type,
				float in_front_rear_split,

				bool in_use_gear_automatic_gears,
				float in_gear_switch_time,
				float in_final_ratio,
				std::vector<float>& in_forward_gear_ratios,
				std::vector<float>& in_reverse_gear_ratios,
				float in_change_up_rpm,
				float in_change_down_rpm,
				float in_transmission_efficiency,

				float in_mass,
				float in_drag_coefficient,
				geom::Location in_center_of_mass,
				float in_chassis_width,
				float in_chassis_height,
				float in_downforce_coefficient,
				float in_drag_area,
				float in_debug_drag_magnitude,
				geom::Vector3D in_inertia_tensor_scale,
				float in_sleep_threshold,
				float in_sleep_slope_limit,

				const std::vector<carla::geom::Vector2D>& in_steering_curve,
				std::vector<WheelPhysicsControl>& in_wheels,
				bool in_use_sweep_wheel_collision) :
			torque_curve(in_torque_curve),
				max_torque(in_max_torque),
				max_rpm(in_max_rpm),
				idle_rpm(in_idle_rpm),
				brake_effect(in_brake_effect),
				rev_up_moi(in_rev_up_moi),
				rev_down_rate(in_rev_down_rate),

				differential_type(in_differential_type),
				front_rear_split(in_front_rear_split),

				use_automatic_gears(in_use_gear_automatic_gears),
				gear_change_time(in_gear_switch_time),
				final_ratio(in_final_ratio),
				forward_gear_ratios(in_forward_gear_ratios),
				reverse_gear_ratios(in_reverse_gear_ratios),
				change_up_rpm(in_change_up_rpm),
				change_down_rpm(in_change_down_rpm),
				transmission_efficiency(in_transmission_efficiency),

				mass(in_mass),
				drag_coefficient(in_drag_coefficient),
				center_of_mass(in_center_of_mass),
				chassis_width(in_chassis_width),
				chassis_height(in_chassis_height),
				downforce_coefficient(in_downforce_coefficient),
				drag_area(in_drag_area),
				inertia_tensor_scale(in_inertia_tensor_scale),
				sleep_threshold(in_sleep_threshold),
				sleep_slope_limit(in_sleep_slope_limit),

				steering_curve(in_steering_curve),
				wheels(in_wheels),
				use_sweep_wheel_collision(in_use_sweep_wheel_collision) {}

			const std::vector<float>& GetForwardGearRatios() const {
				return forward_gear_ratios;
			}

			void SetForwardGearRatios(std::vector<float>& in_forward_gear_ratios) {
				forward_gear_ratios = in_forward_gear_ratios;
			}

			const std::vector<float>& GetReverseGearRatios() const {
				return reverse_gear_ratios;
			}

			void SetReverseGearRatios(std::vector<float>& in_reverse_gear_ratios) {
				reverse_gear_ratios = in_reverse_gear_ratios;
			}

			const std::vector<WheelPhysicsControl>& GetWheels() const {
				return wheels;
			}

			void SetWheels(std::vector<WheelPhysicsControl>& in_wheels) {
				wheels = in_wheels;
			}

			const std::vector<geom::Vector2D>& GetTorqueCurve() const {
				return torque_curve;
			}

			void SetTorqueCurve(std::vector<geom::Vector2D>& in_torque_curve) {
				torque_curve = in_torque_curve;
			}

			const std::vector<geom::Vector2D>& GetSteeringCurve() const {
				return steering_curve;
			}

			void SetSteeringCurve(std::vector<geom::Vector2D>& in_steering_curve) {
				steering_curve = in_steering_curve;
			}

			void SetUseSweepWheelCollision(bool in_sweep) {
				use_sweep_wheel_collision = in_sweep;
			}

			bool GetUseSweepWheelCollision() {
				return use_sweep_wheel_collision;
			}

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
				const bool cmp_results[] = {
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
					std::begin(cmp_results),
					std::end(cmp_results),
					std::identity());
			}

			inline bool operator!=(const VehiclePhysicsControl& rhs) const {
				return !(*this == rhs);
			}

#ifdef LIBCARLA_INCLUDED_FROM_UE4

			VehiclePhysicsControl(const FVehiclePhysicsControl& Control) {
				new (this) VehiclePhysicsControl();
				torque_curve.reserve(Control.TorqueCurve.GetNumKeys());
				for (auto& Key : Control.TorqueCurve.GetConstRefOfKeys())
					torque_curve.push_back(geom::Vector2D(Key.Time, Key.Value));
				max_torque = Control.MaxTorque;
				max_rpm = Control.MaxRPM;
				idle_rpm = Control.IdleRPM;
				brake_effect = Control.BrakeEffect;
				rev_up_moi = Control.RevUpMOI;
				rev_down_rate = Control.RevDownRate;
				differential_type = Control.DifferentialType;
				front_rear_split = Control.FrontRearSplit;
				use_automatic_gears = Control.bUseAutomaticGears;
				gear_change_time = Control.GearChangeTime;
				final_ratio = Control.FinalRatio;
				forward_gear_ratios.resize(Control.ForwardGearRatios.Num());
				std::copy(Control.ForwardGearRatios.begin(), Control.ForwardGearRatios.end(), forward_gear_ratios.begin());
				reverse_gear_ratios.resize(Control.ReverseGearRatios.Num());
				std::copy(Control.ReverseGearRatios.begin(), Control.ReverseGearRatios.end(), reverse_gear_ratios.begin());
				change_up_rpm = Control.ChangeUpRPM;
				change_down_rpm = Control.ChangeDownRPM;
				transmission_efficiency = Control.TransmissionEfficiency;
				mass = Control.Mass;
				drag_coefficient = Control.DragCoefficient;
				center_of_mass = Control.CenterOfMass;
				chassis_width = Control.ChassisWidth;
				chassis_height = Control.ChassisHeight;
				downforce_coefficient = Control.DownforceCoefficient;
				drag_area = Control.DragArea;
				inertia_tensor_scale = geom::Vector3D(
					Control.InertiaTensorScale.X,
					Control.InertiaTensorScale.Y,
					Control.InertiaTensorScale.Z);
				sleep_threshold = Control.SleepThreshold;
				sleep_slope_limit = Control.SleepSlopeLimit;
				steering_curve.reserve(Control.SteeringCurve.GetNumKeys());
				for (auto& Key : Control.SteeringCurve.GetConstRefOfKeys())
					steering_curve.push_back(geom::Vector2D(Key.Time, Key.Value));
				wheels.resize(Control.Wheels.Num());
				std::copy(Control.Wheels.begin(), Control.Wheels.end(), wheels.begin());
				use_sweep_wheel_collision = Control.UseSweepWheelCollision;
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
				std::copy(forward_gear_ratios.begin(), forward_gear_ratios.end(), Control.ForwardGearRatios.begin());
				Control.ReverseGearRatios.SetNum(reverse_gear_ratios.size());
				std::copy(reverse_gear_ratios.begin(), reverse_gear_ratios.end(), Control.ReverseGearRatios.begin());
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
				std::copy(wheels.begin(), wheels.end(), Control.Wheels.begin());
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
