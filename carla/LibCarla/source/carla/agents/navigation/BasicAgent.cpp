// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/agents/navigation/BasicAgent.h"

#include "carla/agents/navigation/GlobalRoutePlanner.h"
#include "carla/agents/navigation/LocalPlanner.h"
#include "carla/agents/navigation/Misc.h"
#include "carla/agents/navigation/Types.h"

#include "carla/client/Actor.h"
#include "carla/client/ActorList.h"
#include "carla/client/Map.h"
#include "carla/client/TrafficLight.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Waypoint.h"
#include "carla/client/World.h"
#include "carla/geom/BoundingBox.h"
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"
#include "carla/road/Lane.h"
#include "carla/road/element/LaneMarking.h"
#include "carla/rpc/TrafficLightState.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <utility>

namespace carla {
namespace agents {
namespace navigation {

namespace bg = boost::geometry;
using BgPoint   = bg::model::d2::point_xy<double>;
using BgPolygon = bg::model::polygon<BgPoint>;

using LaneType   = road::Lane::LaneType;
using LaneChange = road::element::LaneMarking::LaneChange;

namespace {

inline bool LaneChangeAllows(LaneChange lc, bool want_left) {
  // bitwise: Right=0x01, Left=0x02, Both=0x03.
  if (want_left) {
    return lc == LaneChange::Left || lc == LaneChange::Both;
  }
  return lc == LaneChange::Right || lc == LaneChange::Both;
}

} // namespace

// ---------------------------------------------------------------------------

BasicAgent::BasicAgent(SharedPtr<client::Vehicle> vehicle,
                       float target_speed_kmh,
                       const Options &options,
                       SharedPtr<client::Map> map,
                       SharedPtr<GlobalRoutePlanner> grp)
    : _vehicle(std::move(vehicle)),
      _last_traffic_light(nullptr),
      _ignore_traffic_lights(options.ignore_traffic_lights),
      _ignore_stop_signs(options.ignore_stop_signs),
      _ignore_vehicles(options.ignore_vehicles),
      _use_bbs_detection(options.use_bbs_detection),
      _target_speed(target_speed_kmh),
      _sampling_resolution(options.sampling_resolution),
      _base_tlight_threshold(options.base_tlight_threshold),
      _base_vehicle_threshold(options.base_vehicle_threshold),
      _speed_ratio(options.detection_speed_ratio),
      _max_brake(options.max_brake),
      _offset(options.offset) {

  // World is a value type in LibCarla — wrap it in a SharedPtr to match the
  // Python-side `self._world` reference semantics that BehaviorAgent expects.
  _world = SharedPtr<client::World>(new client::World(_vehicle->GetWorld()));

  _map = map ? map : _world->GetMap();

  // Forward the (possibly customised) options into the local planner. The
  // caller passes target_speed via the dedicated arg, so propagate it through
  // a copy of the LocalPlanner::Options slice.
  LocalPlanner::Options lp_opts = static_cast<const LocalPlanner::Options &>(options);
  lp_opts.target_speed = target_speed_kmh;
  _local_planner = std::make_unique<LocalPlanner>(_vehicle, lp_opts, _map);

  if (grp) {
    _global_planner = grp;
  } else {
    _global_planner = SharedPtr<GlobalRoutePlanner>(
        new GlobalRoutePlanner(_map, static_cast<double>(_sampling_resolution)));
  }

  // Cache the static traffic-light list (mirrors Python __init__).
  _lights_list = _world->GetActors()->Filter("*traffic_light*");
}

// ---------------------------------------------------------------------------

rpc::VehicleControl BasicAgent::AddEmergencyStop(rpc::VehicleControl c) const {
  c.throttle   = 0.0f;
  c.brake      = _max_brake;
  c.hand_brake = false;
  return c;
}

void BasicAgent::SetTargetSpeed(float speed_kmh) {
  _target_speed = speed_kmh;
  _local_planner->SetSpeed(speed_kmh);
}

void BasicAgent::FollowSpeedLimits(bool value) {
  _local_planner->FollowSpeedLimits(value);
}

LocalPlanner *BasicAgent::GetLocalPlanner() {
  return _local_planner.get();
}

GlobalRoutePlanner *BasicAgent::GetGlobalPlanner() {
  return _global_planner.get();
}

void BasicAgent::SetDestination(const geom::Location &end,
                                std::optional<geom::Location> start) {
  geom::Location start_location;
  bool clean_queue;
  if (!start.has_value()) {
    auto target_wpt = _local_planner->TargetWaypoint();
    // LocalPlanner::_target_waypoint is seeded via Map::GetWaypoint(), which
    // can return nullptr (e.g. vehicle starts off-road, or the planner has
    // not been re-targeted since construction). Fall back to the vehicle's
    // current location so a route can still be planned.
    if (target_wpt) {
      start_location = target_wpt->GetTransform().location;
    } else {
      start_location = _vehicle->GetLocation();
    }
    clean_queue = true;
  } else {
    start_location = *start;  // Use the provided start location
    clean_queue = false;
  }

  auto start_wp = _map->GetWaypoint(start_location);
  auto end_wp   = _map->GetWaypoint(end);

  auto plan = TraceRoute(start_wp, end_wp);
  _local_planner->SetGlobalPlan(plan, /*stop_waypoint_creation=*/true, clean_queue);
}

void BasicAgent::SetGlobalPlan(const std::vector<LocalPlanner::PlanItem> &plan,
                               bool stop_waypoint_creation,
                               bool clean_queue) {
  _local_planner->SetGlobalPlan(plan, stop_waypoint_creation, clean_queue);
}

std::vector<LocalPlanner::PlanItem>
BasicAgent::TraceRoute(SharedPtr<client::Waypoint> start,
                       SharedPtr<client::Waypoint> end) {
  if (!start || !end) return {};  // Null check before dereference
  const auto &start_loc = start->GetTransform().location;
  const auto &end_loc   = end->GetTransform().location;
  return _global_planner->TraceRoute(start_loc, end_loc);
}

rpc::VehicleControl BasicAgent::RunStep() {
  bool hazard_detected = false;

  // Pull all relevant actors from the world.
  auto vehicle_list = _world->GetActors()->Filter("*vehicle*");

  const float vehicle_speed_ms = GetSpeed(_vehicle) / 3.6f;

  // Vehicles in front?
  const float max_vehicle_distance =
      _base_vehicle_threshold + _speed_ratio * vehicle_speed_ms;
  auto v_obs = VehicleObstacleDetected(vehicle_list, max_vehicle_distance);
  if (v_obs.affected) {
    hazard_detected = true;
  }

  // Red light?
  const float max_tlight_distance =
      _base_tlight_threshold + _speed_ratio * vehicle_speed_ms;
  auto tl_pair = AffectedByTrafficLight(_lights_list, max_tlight_distance);
  if (tl_pair.first) {
    hazard_detected = true;
  }

  rpc::VehicleControl control = _local_planner->RunStep();
  if (hazard_detected) {
    control = AddEmergencyStop(control);
  }
  return control;
}

bool BasicAgent::Done() const {
  return _local_planner->Done();
}

void BasicAgent::IgnoreTrafficLights(bool active) { _ignore_traffic_lights = active; }
void BasicAgent::IgnoreStopSigns(bool active)     { _ignore_stop_signs = active; }
void BasicAgent::IgnoreVehicles(bool active)      { _ignore_vehicles = active; }

void BasicAgent::SetOffset(float offset) {
  _local_planner->SetOffset(offset);
}

void BasicAgent::LaneChange(const std::string &direction,
                            float same_lane_time,
                            float other_lane_time,
                            float lane_change_time) {
  const float speed = _vehicle->GetVelocity().Length();
  auto path = GenerateLaneChangePath(
      _map->GetWaypoint(_vehicle->GetLocation()),
      direction,
      same_lane_time * speed,
      other_lane_time * speed,
      lane_change_time * speed,
      /*check=*/false,
      /*lane_changes=*/1,
      _sampling_resolution);
  if (path.empty()) {
    // Match the Python warning behaviour: log and still call set_global_plan
    // (Python falls through). Keeping silent here — no logger dep needed.
  }
  SetGlobalPlan(path);
}

// ---------------------------------------------------------------------------

std::pair<bool, SharedPtr<client::TrafficLight>>
BasicAgent::AffectedByTrafficLight(SharedPtr<client::ActorList> lights_list,
                                   std::optional<float> max_distance_opt) {
  if (_ignore_traffic_lights) {
    return {false, nullptr};
  }

  if (!lights_list) {
    lights_list = _world->GetActors()->Filter("*traffic_light*");
  }

  const float max_distance =
      max_distance_opt.value_or(_base_tlight_threshold);

  if (_last_traffic_light) {
    if (_last_traffic_light->GetState() != rpc::TrafficLightState::Red) {
      _last_traffic_light = nullptr;
    } else {
      return {true, _last_traffic_light};
    }
  }

  const auto ego_location = _vehicle->GetLocation();
  auto ego_wpt = _map->GetWaypoint(ego_location);
  if (!ego_wpt) {
    // Vehicle is not on a road waypoint (off-road, mid-air, etc.); no
    // road-based traffic-light association can be computed.
    return {false, nullptr};
  }

  for (auto actor : *lights_list) {
    auto traffic_light =
        std::dynamic_pointer_cast<client::TrafficLight>(actor);
    if (!traffic_light) {
      continue;
    }

    const uint32_t tl_id = static_cast<uint32_t>(traffic_light->GetId());

    SharedPtr<client::Waypoint> trigger_wp;
    auto it = _lights_map.find(tl_id);
    if (it != _lights_map.end()) {
      trigger_wp = it->second;
    } else {
      const auto trigger_location =
          GetTrafficLightTriggerLocation(traffic_light);
      trigger_wp = _map->GetWaypoint(trigger_location);
      _lights_map.emplace(tl_id, trigger_wp);
    }

    if (!trigger_wp) {
      // Trigger box for this light isn't on the road network — it can't
      // affect an on-road vehicle. Skip (and the null cache entry above
      // makes subsequent lookups short-circuit at the same point).
      continue;
    }

    if (trigger_wp->GetTransform().location.Distance(ego_location) > max_distance) {
      continue;
    }

    if (trigger_wp->GetRoadId() != ego_wpt->GetRoadId()) {
      continue;
    }

    const auto ve_dir = ego_wpt->GetTransform().GetForwardVector();
    const auto wp_dir = trigger_wp->GetTransform().GetForwardVector();
    const float dot_ve_wp = ve_dir.x * wp_dir.x + ve_dir.y * wp_dir.y + ve_dir.z * wp_dir.z;
    if (dot_ve_wp < 0.0f) {
      continue;
    }

    if (traffic_light->GetState() != rpc::TrafficLightState::Red) {
      continue;
    }

    if (IsWithinDistance(trigger_wp->GetTransform(),
                         _vehicle->GetTransform(),
                         max_distance,
                         std::make_pair(0.0f, 90.0f))) {
      _last_traffic_light = traffic_light;
      return {true, traffic_light};
    }
  }

  return {false, nullptr};
}

// ---------------------------------------------------------------------------

BasicAgent::VehicleObstacle BasicAgent::VehicleObstacleDetected(
    SharedPtr<client::ActorList> vehicle_list,
    std::optional<float> max_distance_opt,
    float up_angle_th,
    float low_angle_th,
    int lane_offset) {

  VehicleObstacle result;
  if (_ignore_vehicles) {
    return result;
  }

  if (!vehicle_list) {
    vehicle_list = _world->GetActors()->Filter("*vehicle*");
  }

  const float max_distance =
      max_distance_opt.value_or(_base_vehicle_threshold);

  const auto ego_transform = _vehicle->GetTransform();
  const auto ego_location  = ego_transform.location;
  auto ego_wpt = _map->GetWaypoint(ego_location);

  // Sign correction for left-driving roads.
  if (ego_wpt->GetLaneId() < 0 && lane_offset != 0) {
    lane_offset *= -1;
  }

  // Front-of-ego transform (push location forward by half the bbox).
  geom::Transform ego_front_transform = ego_transform;
  {
    const auto fv = ego_transform.GetForwardVector();
    const float ext_x = _vehicle->GetBoundingBox().extent.x;
    ego_front_transform.location += geom::Location(ext_x * fv.x,
                                                   ext_x * fv.y,
                                                   ext_x * fv.z);
  }

  const float ego_extent_y = _vehicle->GetBoundingBox().extent.y;
  const bool opposite_invasion =
      std::abs(_offset) + ego_extent_y > ego_wpt->GetLaneWidth() / 2.0;
  const bool use_bbs =
      _use_bbs_detection || opposite_invasion || ego_wpt->IsJunction();

  // ---- Build the route polygon (use_bbs path).
  auto build_route_polygon = [&]() -> std::optional<BgPolygon> {
    std::vector<std::array<double, 3>> route_bb;
    const float r_ext = ego_extent_y + _offset;
    const float l_ext = -ego_extent_y + _offset;

    auto append_pair = [&](const geom::Transform &t) {
      const auto rv = t.GetRightVector();
      const auto loc = t.location;
      const geom::Location p1(loc.x + r_ext * rv.x, loc.y + r_ext * rv.y, loc.z);
      const geom::Location p2(loc.x + l_ext * rv.x, loc.y + l_ext * rv.y, loc.z);
      route_bb.push_back({{p1.x, p1.y, p1.z}});
      route_bb.push_back({{p2.x, p2.y, p2.z}});
    };

    append_pair(ego_transform);

    for (const auto &item : _local_planner->GetPlan()) {
      const auto &wp = item.first;
      if (ego_location.Distance(wp->GetTransform().location) > max_distance) {
        break;
      }
      append_pair(wp->GetTransform());
    }

    if (route_bb.size() < 3) {
      return std::nullopt;
    }

    BgPolygon poly;
    for (const auto &v : route_bb) {
      bg::append(poly.outer(), BgPoint(v[0], v[1]));
    }
    bg::correct(poly);
    return poly;
  };

  std::optional<BgPolygon> route_polygon = build_route_polygon();

  for (auto actor : *vehicle_list) {
    if (!actor) continue;
    if (actor->GetId() == _vehicle->GetId()) {
      continue;
    }

    const auto target_transform = actor->GetTransform();
    if (target_transform.location.Distance(ego_location) > max_distance) {
      continue;
    }

    auto target_wpt = _map->GetWaypoint(target_transform.location,
                                        /*project_to_road=*/true,
                                        static_cast<int32_t>(LaneType::Any));

    // Junction / bbox path.
    if ((use_bbs || target_wpt->IsJunction()) && route_polygon.has_value()) {
      const auto &target_bb = actor->GetBoundingBox();
      const auto vertices = target_bb.GetWorldVertices(actor->GetTransform());

      BgPolygon target_poly;
      for (const auto &v : vertices) {
        bg::append(target_poly.outer(), BgPoint(v.x, v.y));
      }
      bg::correct(target_poly);

      if (bg::intersects(*route_polygon, target_poly)) {
        result.affected = true;
        result.actor    = actor;
        result.distance = ComputeDistance(actor->GetLocation(), ego_location);
        return result;
      }
    } else {
      // Simplified plan-waypoint approach.
      if (target_wpt->GetRoadId() != ego_wpt->GetRoadId() ||
          target_wpt->GetLaneId() != ego_wpt->GetLaneId() + lane_offset) {
        auto next_pair = _local_planner->GetIncomingWaypointAndDirection(3);
        auto next_wp = next_pair.first;
        if (!next_wp) {
          continue;
        }
        if (target_wpt->GetRoadId() != next_wp->GetRoadId() ||
            target_wpt->GetLaneId() != next_wp->GetLaneId() + lane_offset) {
          continue;
        }
      }

      const auto target_forward = target_transform.GetForwardVector();
      const float target_extent_x = actor->GetBoundingBox().extent.x;
      geom::Transform target_rear_transform = target_transform;
      target_rear_transform.location -= geom::Location(
          target_extent_x * target_forward.x,
          target_extent_x * target_forward.y,
          0.0f);

      if (IsWithinDistance(target_rear_transform,
                           ego_front_transform,
                           max_distance,
                           std::make_pair(low_angle_th, up_angle_th))) {
        result.affected = true;
        result.actor    = actor;
        result.distance = ComputeDistance(target_transform.location,
                                          ego_transform.location);
        return result;
      }
    }
  }

  return result;
}

// ---------------------------------------------------------------------------

std::vector<LocalPlanner::PlanItem> BasicAgent::GenerateLaneChangePath(
    SharedPtr<client::Waypoint> waypoint,
    const std::string &direction,
    float distance_same_lane,
    float distance_other_lane,
    float lane_change_distance,
    bool  check,
    int   lane_changes,
    float step_distance) {

  distance_same_lane    = std::max(distance_same_lane, 0.1f);
  distance_other_lane   = std::max(distance_other_lane, 0.1f);
  lane_change_distance  = std::max(lane_change_distance, 0.1f);

  std::vector<LocalPlanner::PlanItem> plan;
  plan.emplace_back(waypoint, RoadOption::LaneFollow);

  RoadOption option = RoadOption::LaneFollow;

  // Same lane stretch.
  {
    float distance = 0.0f;
    while (distance < distance_same_lane) {
      auto next_wps = plan.back().first->GetNext(step_distance);
      if (next_wps.empty()) return {};
      auto next_wp = next_wps.front();
      distance += static_cast<float>(
          next_wp->GetTransform().location.Distance(
              plan.back().first->GetTransform().location));
      plan.emplace_back(next_wp, RoadOption::LaneFollow);
    }
  }

  bool want_left;
  if (direction == "left") {
    option = RoadOption::ChangeLaneLeft;
    want_left = true;
  } else if (direction == "right") {
    option = RoadOption::ChangeLaneRight;
    want_left = false;
  } else {
    return {};
  }

  int lane_changes_done = 0;
  const float per_change_distance =
      lane_change_distance / static_cast<float>(std::max(lane_changes, 1));

  // Lane change steps.
  while (lane_changes_done < lane_changes) {
    auto next_wps = plan.back().first->GetNext(per_change_distance);
    if (next_wps.empty()) return {};
    auto next_wp = next_wps.front();

    SharedPtr<client::Waypoint> side_wp;
    if (want_left) {
      if (check && !LaneChangeAllows(next_wp->GetLaneChange(), /*want_left=*/true)) {
        return {};
      }
      side_wp = next_wp->GetLeft();
    } else {
      if (check && !LaneChangeAllows(next_wp->GetLaneChange(), /*want_left=*/false)) {
        return {};
      }
      side_wp = next_wp->GetRight();
    }

    if (!side_wp || side_wp->GetType() != LaneType::Driving) {
      return {};
    }

    plan.emplace_back(side_wp, option);
    ++lane_changes_done;
  }

  // Other lane stretch.
  {
    float distance = 0.0f;
    while (distance < distance_other_lane) {
      auto next_wps = plan.back().first->GetNext(step_distance);
      if (next_wps.empty()) return {};
      auto next_wp = next_wps.front();
      distance += static_cast<float>(
          next_wp->GetTransform().location.Distance(
              plan.back().first->GetTransform().location));
      plan.emplace_back(next_wp, RoadOption::LaneFollow);
    }
  }

  return plan;
}

} // namespace navigation
} // namespace agents
} // namespace carla
