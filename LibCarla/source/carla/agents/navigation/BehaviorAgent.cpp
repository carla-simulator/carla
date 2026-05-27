// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/agents/navigation/BehaviorAgent.h"

#include "carla/agents/navigation/LocalPlanner.h"
#include "carla/agents/navigation/Misc.h"
#include "carla/client/Actor.h"
#include "carla/client/ActorList.h"
#include "carla/client/Map.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Waypoint.h"
#include "carla/client/World.h"
#include "carla/geom/BoundingBox.h"
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"
#include "carla/road/Lane.h"
#include "carla/road/element/LaneMarking.h"
#include "carla/rpc/VehicleControl.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <type_traits>

namespace carla {
namespace agents {
namespace navigation {

namespace {

  using LaneChange = road::element::LaneMarking::LaneChange;

  /// True if a `LaneChange` value contains the `flag` bit
  /// (mirrors Python `==` checks like `right_turn == LaneChange.Right` /
  /// `right_turn == LaneChange.Both`).
  bool HasLaneChange(LaneChange value, LaneChange flag) {
    using U = std::underlying_type_t<LaneChange>;
    return (static_cast<U>(value) & static_cast<U>(flag)) == static_cast<U>(flag);
  }

  BehaviorParameters BehaviorParamsFor(BehaviorAgent::Behavior b) {
    switch (b) {
      case BehaviorAgent::Behavior::Cautious:   return BehaviorParameters::Cautious();
      case BehaviorAgent::Behavior::Aggressive: return BehaviorParameters::Aggressive();
      case BehaviorAgent::Behavior::Normal:
      default:                                  return BehaviorParameters::Normal();
    }
  }

} // anonymous namespace

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

BehaviorAgent::BehaviorAgent(SharedPtr<client::Vehicle> vehicle,
                             Behavior behavior,
                             const Options &opts,
                             SharedPtr<client::Map> map,
                             SharedPtr<GlobalRoutePlanner> grp)
  : BasicAgent(vehicle, /*target_speed_kmh=*/20.0f, opts, map, grp),
    _behavior(BehaviorParamsFor(behavior)) {
  // Match the Python BehaviorAgent override of the sampling resolution.
  _sampling_resolution = 4.5f;
}

// ---------------------------------------------------------------------------
// Information refresh
// ---------------------------------------------------------------------------

void BehaviorAgent::UpdateInformation() {
  _speed = GetSpeed(_vehicle);
  _speed_limit = _vehicle->GetSpeedLimit();
  // Even when follow_speed_limits is on this seed mirrors the Python reference.
  _local_planner->SetSpeed(_speed_limit);

  _direction = _local_planner->TargetRoadOption();
  if (_direction == RoadOption::Void) {
    _direction = RoadOption::LaneFollow;
  }

  _look_ahead_steps = static_cast<int>(_speed_limit / 10.0f);

  auto incoming = _local_planner->GetIncomingWaypointAndDirection(_look_ahead_steps);
  _incoming_waypoint = incoming.first;
  _incoming_direction = incoming.second;
  if (_incoming_direction == RoadOption::Void) {
    _incoming_direction = RoadOption::LaneFollow;
  }
}

// ---------------------------------------------------------------------------
// Traffic light handling
// ---------------------------------------------------------------------------

bool BehaviorAgent::TrafficLightManager() {
  auto actor_list = _world->GetActors();
  auto lights_list = actor_list ? actor_list->Filter("*traffic_light*") : nullptr;
  auto result = AffectedByTrafficLight(lights_list);
  return result.first;
}

// ---------------------------------------------------------------------------
// Tailgating
// ---------------------------------------------------------------------------

void BehaviorAgent::Tailgating(SharedPtr<client::Waypoint> waypoint) {
  // Python passes the prefiltered vehicle list around; the base helper
  // `VehicleObstacleDetected` accepts a `SharedPtr<ActorList>` and applies
  // its own proximity gating via `max_distance`, so we hand it the
  // world-wide vehicle filter result here.
  auto left_marking = waypoint->GetLeftLaneMarking();
  auto right_marking = waypoint->GetRightLaneMarking();
  auto left_wpt = waypoint->GetLeft();
  auto right_wpt = waypoint->GetRight();

  // Build the world-vehicle ActorList passthrough (matches Python's reuse of
  // the filtered list).
  auto vehicles_alist = _world->GetActors()
      ? _world->GetActors()->Filter("*vehicle*")
      : nullptr;

  const float behind_max =
      std::max(_behavior.min_proximity_threshold, _speed_limit / 2.0f);

  auto behind = VehicleObstacleDetected(vehicles_alist,
                                        behind_max,
                                        /*up_angle_th=*/180.0f,
                                        /*low_angle_th=*/160.0f,
                                        /*lane_offset=*/0);

  if (!(behind.affected && behind.actor)) return;
  if (_speed >= GetSpeed(behind.actor)) return;

  const int wp_lane_id = waypoint->GetLaneId();

  // Right-lane tailgate avoidance.
  if (right_marking.has_value() && right_wpt &&
      (HasLaneChange(right_marking->lane_change, LaneChange::Right) ||
       HasLaneChange(right_marking->lane_change, LaneChange::Both)) &&
      (wp_lane_id * right_wpt->GetLaneId() > 0) &&
      right_wpt->GetType() == road::Lane::LaneType::Driving) {
    auto check = VehicleObstacleDetected(vehicles_alist,
                                         behind_max,
                                         /*up_angle_th=*/180.0f,
                                         /*low_angle_th=*/0.0f,
                                         /*lane_offset=*/1);
    if (!check.affected) {
      std::cout << "Tailgating, moving to the right!" << std::endl;
      auto end_waypoint = _local_planner->TargetWaypoint();
      _behavior.tailgate_counter = 200;
      if (end_waypoint) {
        SetDestination(end_waypoint->GetTransform().location,
                       right_wpt->GetTransform().location);
      }
    }
    return;
  }

  // Left-lane tailgate avoidance.
  if (left_marking.has_value() && left_wpt &&
      HasLaneChange(left_marking->lane_change, LaneChange::Left) &&
      (wp_lane_id * left_wpt->GetLaneId() > 0) &&
      left_wpt->GetType() == road::Lane::LaneType::Driving) {
    auto check = VehicleObstacleDetected(vehicles_alist,
                                         behind_max,
                                         /*up_angle_th=*/180.0f,
                                         /*low_angle_th=*/0.0f,
                                         /*lane_offset=*/-1);
    if (!check.affected) {
      std::cout << "Tailgating, moving to the left!" << std::endl;
      auto end_waypoint = _local_planner->TargetWaypoint();
      _behavior.tailgate_counter = 200;
      if (end_waypoint) {
        SetDestination(end_waypoint->GetTransform().location,
                       left_wpt->GetTransform().location);
      }
    }
  }
}

// ---------------------------------------------------------------------------
// Vehicle / pedestrian managers
// ---------------------------------------------------------------------------

BasicAgent::VehicleObstacle BehaviorAgent::CollisionAndCarAvoidManager(
    SharedPtr<client::Waypoint> waypoint) {
  auto vehicles_alist = _world->GetActors()
      ? _world->GetActors()->Filter("*vehicle*")
      : nullptr;

  // Python applies a 45 m prefilter around the waypoint here (excluding the
  // ego). The base helper `VehicleObstacleDetected` honors `max_distance`
  // internally, so we just route the world-wide vehicle list to it.
  VehicleObstacle obs;
  if (_direction == RoadOption::ChangeLaneLeft) {
    obs = VehicleObstacleDetected(
        vehicles_alist,
        std::max(_behavior.min_proximity_threshold, _speed_limit / 2.0f),
        /*up_angle_th=*/180.0f,
        /*low_angle_th=*/0.0f,
        /*lane_offset=*/-1);
  } else if (_direction == RoadOption::ChangeLaneRight) {
    obs = VehicleObstacleDetected(
        vehicles_alist,
        std::max(_behavior.min_proximity_threshold, _speed_limit / 2.0f),
        /*up_angle_th=*/180.0f,
        /*low_angle_th=*/0.0f,
        /*lane_offset=*/1);
  } else {
    obs = VehicleObstacleDetected(
        vehicles_alist,
        std::max(_behavior.min_proximity_threshold, _speed_limit / 3.0f),
        /*up_angle_th=*/30.0f,
        /*low_angle_th=*/0.0f,
        /*lane_offset=*/0);

    // Tailgating check.
    if (!obs.affected &&
        _direction == RoadOption::LaneFollow &&
        !waypoint->IsJunction() &&
        _speed > 10.0f &&
        _behavior.tailgate_counter == 0) {
      Tailgating(waypoint);
    }
  }

  return obs;
}

BasicAgent::VehicleObstacle BehaviorAgent::PedestrianAvoidManager(
    SharedPtr<client::Waypoint> waypoint) {
  auto walkers_alist = _world->GetActors()
      ? _world->GetActors()->Filter("*walker.pedestrian*")
      : nullptr;

  // Python applies a 10 m prefilter around the waypoint here. The base
  // `VehicleObstacleDetected` honors `max_distance` internally, so we let
  // it do the proximity gating from the world-wide walker list.
  (void)waypoint;  // location consumed indirectly via VehicleObstacleDetected
  VehicleObstacle obs;
  if (_direction == RoadOption::ChangeLaneLeft) {
    obs = VehicleObstacleDetected(
        walkers_alist,
        std::max(_behavior.min_proximity_threshold, _speed_limit / 2.0f),
        /*up_angle_th=*/90.0f,
        /*low_angle_th=*/0.0f,
        /*lane_offset=*/-1);
  } else if (_direction == RoadOption::ChangeLaneRight) {
    obs = VehicleObstacleDetected(
        walkers_alist,
        std::max(_behavior.min_proximity_threshold, _speed_limit / 2.0f),
        /*up_angle_th=*/90.0f,
        /*low_angle_th=*/0.0f,
        /*lane_offset=*/1);
  } else {
    obs = VehicleObstacleDetected(
        walkers_alist,
        std::max(_behavior.min_proximity_threshold, _speed_limit / 3.0f),
        /*up_angle_th=*/60.0f,
        /*low_angle_th=*/0.0f,
        /*lane_offset=*/0);
  }
  return obs;
}

// ---------------------------------------------------------------------------
// Car-following control law
// ---------------------------------------------------------------------------

rpc::VehicleControl BehaviorAgent::CarFollowingManager(
    SharedPtr<client::Actor> vehicle, float distance, bool debug) {
  const float vehicle_speed = GetSpeed(vehicle);
  const float delta_v = std::max(1.0f, (_speed - vehicle_speed) / 3.6f);
  const float ttc = (delta_v != 0.0f)
      ? distance / delta_v
      : distance / std::nextafter(0.0f, 1.0f);

  float target_speed = 0.0f;
  if (_behavior.safety_time > ttc && ttc > 0.0f) {
    target_speed = std::min({
        Positive(vehicle_speed - _behavior.speed_decrease),
        _behavior.max_speed,
        _speed_limit - _behavior.speed_lim_dist});
  } else if (2.0f * _behavior.safety_time > ttc && ttc >= _behavior.safety_time) {
    target_speed = std::min({
        std::max(_min_speed, vehicle_speed),
        _behavior.max_speed,
        _speed_limit - _behavior.speed_lim_dist});
  } else {
    target_speed = std::min({
        _behavior.max_speed,
        _speed_limit - _behavior.speed_lim_dist});
  }
  _local_planner->SetSpeed(target_speed);
  return _local_planner->RunStep(debug);
}

// ---------------------------------------------------------------------------
// Main run-step
// ---------------------------------------------------------------------------

rpc::VehicleControl BehaviorAgent::RunStep(bool debug) {
  UpdateInformation();

  if (_behavior.tailgate_counter > 0) {
    _behavior.tailgate_counter -= 1;
  }

  const auto ego_vehicle_loc = _vehicle->GetLocation();
  auto ego_vehicle_wp = _map->GetWaypoint(ego_vehicle_loc);

  // 1. Red lights / stops.
  if (TrafficLightManager()) {
    return EmergencyStop();
  }

  // 2.1: Pedestrian avoidance.
  auto walker_obs = PedestrianAvoidManager(ego_vehicle_wp);
  if (walker_obs.affected && walker_obs.actor) {
    const auto &walker_extent = walker_obs.actor->GetBoundingBox().extent;
    const auto &ego_extent    = _vehicle->GetBoundingBox().extent;
    const float distance = walker_obs.distance
        - std::max(walker_extent.y, walker_extent.x)
        - std::max(ego_extent.y,    ego_extent.x);
    if (distance < _behavior.braking_distance) {
      return EmergencyStop();
    }
  }

  // 2.2: Vehicle avoidance / car following.
  auto vehicle_obs = CollisionAndCarAvoidManager(ego_vehicle_wp);
  rpc::VehicleControl control{};
  if (vehicle_obs.affected && vehicle_obs.actor) {
    const auto &v_extent  = vehicle_obs.actor->GetBoundingBox().extent;
    const auto &ego_extent = _vehicle->GetBoundingBox().extent;
    const float distance = vehicle_obs.distance
        - std::max(v_extent.y,   v_extent.x)
        - std::max(ego_extent.y, ego_extent.x);
    if (distance < _behavior.braking_distance) {
      return EmergencyStop();
    }
    control = CarFollowingManager(vehicle_obs.actor, distance, debug);
  }
  // 3: Intersection behaviour (left/right turn).
  else if (_incoming_waypoint && _incoming_waypoint->IsJunction() &&
           (_incoming_direction == RoadOption::Left ||
            _incoming_direction == RoadOption::Right)) {
    const float target_speed = std::min(_behavior.max_speed,
                                        _speed_limit - 5.0f);
    _local_planner->SetSpeed(target_speed);
    control = _local_planner->RunStep(debug);
  }
  // 4: Normal behaviour.
  else {
    const float target_speed = std::min(_behavior.max_speed,
                                        _speed_limit - _behavior.speed_lim_dist);
    _local_planner->SetSpeed(target_speed);
    control = _local_planner->RunStep(debug);
  }
  return control;
}

// ---------------------------------------------------------------------------
// Emergency stop
// ---------------------------------------------------------------------------

rpc::VehicleControl BehaviorAgent::EmergencyStop() const {
  rpc::VehicleControl control{};
  control.throttle = 0.0f;
  control.brake = _max_brake;
  control.hand_brake = false;
  return control;
}

} // namespace navigation
} // namespace agents
} // namespace carla
