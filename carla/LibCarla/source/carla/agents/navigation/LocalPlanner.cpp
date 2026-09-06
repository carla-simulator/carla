// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/agents/navigation/LocalPlanner.h"

#include "carla/agents/navigation/Misc.h"
#include "carla/client/Map.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Waypoint.h"
#include "carla/client/World.h"
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace carla {
namespace agents {
namespace navigation {

namespace {

constexpr float kKmhToMs = 1.0f / 3.6f;

/// Compute the topological connection between @a current and @a next_wp,
/// matching `_compute_connection` in local_planner.py.
RoadOption ComputeConnection(
    SharedPtr<client::Waypoint> current,
    SharedPtr<client::Waypoint> next_wp,
    float threshold = 35.0f) {
  // Python uses `% 360` and `% 180` on yaws to wrap them. Replicate with
  // `std::fmod` and a positive-fixup, since C++ `%` doesn't accept floats.
  auto wrap = [](float v, float mod) {
    float r = std::fmod(v, mod);
    if (r < 0.0f) r += mod;
    return r;
  };

  const float n = wrap(next_wp->GetTransform().rotation.yaw, 360.0f);
  const float c = wrap(current->GetTransform().rotation.yaw, 360.0f);
  const float diff = wrap(n - c, 180.0f);

  if (diff < threshold || diff > (180.0f - threshold)) {
    return RoadOption::Straight;
  }
  if (diff > 90.0f) {
    return RoadOption::Left;
  }
  return RoadOption::Right;
}

/// Mirror of `_retrieve_options` in local_planner.py.
std::vector<RoadOption> RetrieveOptions(
    const std::vector<SharedPtr<client::Waypoint>> &candidates,
    SharedPtr<client::Waypoint> current) {
  std::vector<RoadOption> options;
  options.reserve(candidates.size());
  for (const auto &next_wp : candidates) {
    // The Python code probes 3 m further to disambiguate intersections that
    // share an entry waypoint.
    auto next_next = next_wp->GetNext(3.0);
    if (next_next.empty()) {
      options.push_back(ComputeConnection(current, next_wp));
    } else {
      options.push_back(ComputeConnection(current, next_next.front()));
    }
  }
  return options;
}

} // namespace

// =========================================================================
// LocalPlanner
// =========================================================================

LocalPlanner::LocalPlanner(
    SharedPtr<client::Vehicle> vehicle,
    const Options &opt,
    SharedPtr<client::Map> map)
  : _vehicle(std::move(vehicle)),
    _map(std::move(map)),
    _dt(opt.dt),
    _target_speed(opt.target_speed),
    _sampling_radius(opt.sampling_radius),
    _lateral_args(opt.lateral),
    _longitudinal_args(opt.longitudinal),
    _max_throttle(opt.max_throttle),
    _max_brake(opt.max_brake),
    _max_steering(opt.max_steering),
    _offset(opt.offset),
    _base_min_distance(opt.base_min_distance),
    _distance_ratio(opt.distance_ratio),
    _follow_speed_limits(opt.follow_speed_limits),
    _rng(std::random_device{}()) {
  // If no map was supplied, pull it from the vehicle's world (mirrors the
  // `map_inst` fallback in local_planner.py).
  if (!_map && _vehicle) {
    _map = _vehicle->GetWorld().GetMap();
  }
  InitController();
}

void LocalPlanner::InitController() {
  _vehicle_controller = std::make_unique<VehiclePIDController>(
      _vehicle,
      _lateral_args,
      _longitudinal_args,
      _offset,
      _max_throttle,
      _max_brake,
      _max_steering);

  if (_vehicle && _map) {
    auto current_waypoint = _map->GetWaypoint(_vehicle->GetLocation());
    _target_waypoint = current_waypoint;
    _target_road_option = RoadOption::LaneFollow;
    _waypoints_queue.emplace_back(_target_waypoint, _target_road_option);
  }
}

void LocalPlanner::ResetVehicle() {
  _vehicle.reset();
}

void LocalPlanner::SetSpeed(float speed_kmh) {
  if (_follow_speed_limits) {
    std::cout << "WARNING: The max speed is currently set to follow the speed"
                 " limits. Use 'follow_speed_limits' to deactivate this\n";
  }
  _target_speed = speed_kmh;
}

void LocalPlanner::FollowSpeedLimits(bool value) {
  _follow_speed_limits = value;
}

void LocalPlanner::ComputeNextWaypoints(std::size_t k) {
  // Bound k by remaining queue capacity, like Python's
  // `available_entries = self._waypoints_queue.maxlen - len(...)`.
  const std::size_t available = (_waypoints_queue.size() < _max_queue_size)
      ? (_max_queue_size - _waypoints_queue.size())
      : 0;
  k = std::min(k, available);

  for (std::size_t i = 0; i < k; ++i) {
    if (_waypoints_queue.empty()) break;
    auto last_waypoint = _waypoints_queue.back().first;
    if (!last_waypoint) break;
    auto next_waypoints = last_waypoint->GetNext(_sampling_radius);

    if (next_waypoints.empty()) {
      break;
    }

    SharedPtr<client::Waypoint> next_waypoint;
    RoadOption road_option;
    if (next_waypoints.size() == 1) {
      next_waypoint = next_waypoints.front();
      road_option = RoadOption::LaneFollow;
    } else {
      // Random choice among the available branches.
      auto road_options_list = RetrieveOptions(next_waypoints, last_waypoint);
      std::uniform_int_distribution<std::size_t> dist(
          0, road_options_list.size() - 1);
      const std::size_t idx = dist(_rng);
      road_option = road_options_list[idx];
      // Python pulls `next_waypoints[road_options_list.index(road_option)]`,
      // i.e. the *first* candidate that shares the chosen RoadOption. With a
      // pre-randomised idx that's just the same as picking idx directly,
      // because there's a 1:1 alignment between the lists (the first option
      // matching the random pick is the random pick itself).
      next_waypoint = next_waypoints[idx];
    }

    _waypoints_queue.emplace_back(next_waypoint, road_option);
  }
}

void LocalPlanner::SetGlobalPlan(
    const std::vector<PlanItem> &plan,
    bool stop_waypoint_creation,
    bool clean_queue) {
  if (clean_queue) {
    _waypoints_queue.clear();
  }

  // The Python implementation grows the deque's `maxlen` whenever the new
  // plan exceeds it; we keep a soft cap and just raise it as needed.
  const std::size_t new_plan_length =
      plan.size() + _waypoints_queue.size();
  if (new_plan_length > _max_queue_size) {
    _max_queue_size = new_plan_length;
  }

  for (const auto &elem : plan) {
    _waypoints_queue.push_back(elem);
  }
  _stop_waypoint_creation = stop_waypoint_creation;
}

void LocalPlanner::SetOffset(float offset) {
  _offset = offset;
  if (_vehicle_controller) {
    _vehicle_controller->SetOffset(offset);
  }
}

rpc::VehicleControl LocalPlanner::RunStep(bool debug) {
  if (_follow_speed_limits && _vehicle) {
    _target_speed = _vehicle->GetSpeedLimit();
  }

  if (!_stop_waypoint_creation
      && _waypoints_queue.size() < _min_queue_length) {
    ComputeNextWaypoints(_min_queue_length);
  }

  // Purge waypoints we've already passed.
  if (_vehicle) {
    const auto veh_location = _vehicle->GetLocation();
    const float vehicle_speed_ms = GetSpeed(_vehicle) * kKmhToMs;
    const float min_distance =
        _base_min_distance + _distance_ratio * vehicle_speed_ms;

    std::size_t num_removed = 0;
    for (const auto &item : _waypoints_queue) {
      // Don't remove the very last waypoint until we're almost on top of it
      // (mirrors Python's `min_distance = 1` in the same situation).
      const float threshold =
          (_waypoints_queue.size() - num_removed == 1) ? 1.0f : min_distance;

      const auto &wp_loc = item.first->GetTransform().location;
      if (veh_location.Distance(wp_loc) < threshold) {
        ++num_removed;
      } else {
        break;
      }
    }
    for (std::size_t i = 0; i < num_removed; ++i) {
      _waypoints_queue.pop_front();
    }
  }

  rpc::VehicleControl control;
  if (_waypoints_queue.empty()) {
    control.steer = 0.0f;
    control.throttle = 0.0f;
    control.brake = 1.0f;
    control.hand_brake = false;
    control.manual_gear_shift = false;
  } else {
    _target_waypoint = _waypoints_queue.front().first;
    _target_road_option = _waypoints_queue.front().second;
    control = _vehicle_controller->RunStep(_target_speed, _target_waypoint);
  }

  if (debug && _vehicle && _target_waypoint) {
    auto world = std::make_shared<client::World>(_vehicle->GetWorld());
    DrawWaypoints(world, {_target_waypoint}, 1.0f);
  }

  return control;
}

std::pair<SharedPtr<client::Waypoint>, RoadOption>
LocalPlanner::GetIncomingWaypointAndDirection(int steps) const {
  if (steps >= 0
      && static_cast<std::size_t>(steps) < _waypoints_queue.size()) {
    return _waypoints_queue[static_cast<std::size_t>(steps)];
  }
  if (!_waypoints_queue.empty()) {
    return _waypoints_queue.back();
  }
  return {nullptr, RoadOption::Void};
}

const std::deque<LocalPlanner::PlanItem> &LocalPlanner::GetPlan() const {
  return _waypoints_queue;
}

bool LocalPlanner::Done() const {
  return _waypoints_queue.empty();
}

SharedPtr<client::Waypoint> LocalPlanner::TargetWaypoint() const {
  return _target_waypoint;
}

RoadOption LocalPlanner::TargetRoadOption() const {
  return _target_road_option;
}

} // namespace navigation
} // namespace agents
} // namespace carla
