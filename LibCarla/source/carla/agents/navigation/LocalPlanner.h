// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/agents/navigation/PIDController.h"
#include "carla/agents/navigation/Types.h"
#include "carla/rpc/VehicleControl.h"

#include <deque>
#include <memory>
#include <random>
#include <utility>
#include <vector>

namespace carla {

namespace client {
  class Map;
  class Vehicle;
  class Waypoint;
} // namespace client

namespace agents {
namespace navigation {

  /// LocalPlanner reproduces `agents.navigation.local_planner.LocalPlanner`.
  /// It maintains a queue of (waypoint, RoadOption) pairs, refills it as
  /// needed, and drives the vehicle along it via a VehiclePIDController.
  class LocalPlanner {
  public:
    using PlanItem = std::pair<SharedPtr<client::Waypoint>, RoadOption>;

    /// Aggregated configuration. Mirrors the `opt_dict` in local_planner.py.
    struct Options {
      float dt = 1.0f / 20.0f;
      float target_speed = 20.0f;          // km/h
      float sampling_radius = 2.0f;
      // LateralArgs/LongitudinalArgs supply their own defaults (1.95/0.05/0.20
      // and 1.50/0.05/0.20 with dt=0.05) — leaving these value-initialized.
      // Earlier brace-init member defaults here triggered a GCC aggregate-init
      // quirk when Options was used as `= {}` default arg downstream.
      VehiclePIDController::LateralArgs       lateral{};
      VehiclePIDController::LongitudinalArgs  longitudinal{};
      float max_throttle = 0.75f;
      float max_brake = 0.30f;
      float max_steering = 0.80f;
      float offset = 0.0f;
      float base_min_distance = 3.0f;
      float distance_ratio = 0.5f;
      bool  follow_speed_limits = false;
    };

    /// Construct a LocalPlanner. If @a map is null we pull the map from the
    /// vehicle's world (matches the Python `map_inst` fallback).
    /// Returns a value-initialized Options. Defined out-of-line because GCC's
    /// default-arg parser refuses `= {}` / `= Options()` when the struct
    /// inherits from another aggregate with default member initializers.
    static Options DefaultOptions();

    LocalPlanner(
        SharedPtr<client::Vehicle> vehicle,
        const Options &opt = DefaultOptions(),
        SharedPtr<client::Map> map = nullptr);

    /// Drop the vehicle reference (mirrors `reset_vehicle`).
    void ResetVehicle();

    /// Override the target speed in km/h.
    void SetSpeed(float speed_kmh);

    /// Toggle automatic following of road speed limits.
    void FollowSpeedLimits(bool value = true);

    /// Replace (or append to) the waypoint queue.
    void SetGlobalPlan(
        const std::vector<PlanItem> &plan,
        bool stop_waypoint_creation = true,
        bool clean_queue = true);

    /// Apply a lateral offset to the underlying PID controller.
    void SetOffset(float offset);

    /// Drive one tick. Returns the VehicleControl to apply.
    rpc::VehicleControl RunStep(bool debug = false);

    /// Peek `steps` ahead in the queue, falling back to the last entry if
    /// the queue is too short. Returns (nullptr, RoadOption::Void) when
    /// the queue is empty.
    std::pair<SharedPtr<client::Waypoint>, RoadOption>
    GetIncomingWaypointAndDirection(int steps = 3) const;

    /// Read-only view of the queued plan.
    const std::deque<PlanItem> &GetPlan() const;

    /// True when the queue is empty.
    bool Done() const;

    /// Currently active target waypoint (mirrors `target_waypoint`).
    SharedPtr<client::Waypoint> TargetWaypoint() const;

    /// Currently active target RoadOption (mirrors `target_road_option`).
    RoadOption TargetRoadOption() const;

  private:
    /// Build the VehiclePIDController and seed the queue with the vehicle's
    /// current waypoint. Called from the constructor.
    void InitController();

    /// Append up to @a k waypoints to the back of the queue, branching with
    /// random choice on intersections (mirrors `_compute_next_waypoints`).
    void ComputeNextWaypoints(std::size_t k);

    SharedPtr<client::Vehicle> _vehicle;
    SharedPtr<client::Map>     _map;

    std::unique_ptr<VehiclePIDController> _vehicle_controller;

    SharedPtr<client::Waypoint> _target_waypoint;
    RoadOption                  _target_road_option = RoadOption::Void;

    std::deque<PlanItem> _waypoints_queue;
    std::size_t _max_queue_size       = 10000;
    std::size_t _min_queue_length     = 100;
    bool _stop_waypoint_creation      = false;

    // Tunables (mirror the Python instance fields).
    float _dt;
    float _target_speed;
    float _sampling_radius;
    VehiclePIDController::LateralArgs       _lateral_args;
    VehiclePIDController::LongitudinalArgs  _longitudinal_args;
    float _max_throttle;
    float _max_brake;
    float _max_steering;
    float _offset;
    float _base_min_distance;
    float _distance_ratio;
    bool  _follow_speed_limits;

    // Random branching. Seeded once per planner.
    mutable std::mt19937 _rng;
  };

  inline LocalPlanner::Options LocalPlanner::DefaultOptions() {
    return Options{};
  }

} // namespace navigation
} // namespace agents
} // namespace carla
