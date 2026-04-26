// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/agents/navigation/LocalPlanner.h"
#include "carla/agents/navigation/Types.h"
#include "carla/geom/Location.h"
#include "carla/rpc/VehicleControl.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace carla {

namespace client {
  class ActorList;
  class Map;
  class TrafficLight;
  class Vehicle;
  class Waypoint;
  class World;
  class Actor;
} // namespace client

namespace agents {
namespace navigation {

  // Forward decls — heavy headers only pulled in BasicAgent.cpp.
  class GlobalRoutePlanner;

  /// C++ port of agents.navigation.basic_agent.BasicAgent.
  ///
  /// Roams the scene following random or scripted waypoints while obeying
  /// traffic lights and avoiding other vehicles. Stop signs are ignored.
  class BasicAgent {
  public:

    /// Tunable knobs. Inherits LocalPlanner::Options so the same dictionary
    /// can drive the local planner.
    struct Options : LocalPlanner::Options {
      bool  ignore_traffic_lights  = false;
      bool  ignore_stop_signs      = false;
      bool  ignore_vehicles        = false;
      bool  use_bbs_detection      = false;
      float sampling_resolution    = 2.0f;
      float base_tlight_threshold  = 5.0f;
      float base_vehicle_threshold = 5.0f;
      float detection_speed_ratio  = 1.0f;
      float max_brake              = 0.6f;   // emergency brake (overrides LocalPlanner.max_brake)
    };

    /// Out-of-line default factory; see LocalPlanner::DefaultOptions().
    static Options DefaultOptions();

    BasicAgent(SharedPtr<client::Vehicle> vehicle,
               float target_speed_kmh = 20.0f,
               const Options &options = DefaultOptions(),
               SharedPtr<client::Map> map = nullptr,
               SharedPtr<GlobalRoutePlanner> grp = nullptr);

    virtual ~BasicAgent() = default;

    /// Overwrites throttle/brake to perform an emergency stop. Steering is
    /// left intact so the agent doesn't drift mid-turn.
    rpc::VehicleControl AddEmergencyStop(rpc::VehicleControl c) const;

    void SetTargetSpeed(float speed_kmh);
    void FollowSpeedLimits(bool value = true);

    LocalPlanner       *GetLocalPlanner();
    GlobalRoutePlanner *GetGlobalPlanner();

    /// Computes a route from `start` (or the current target waypoint) to
    /// `end` and forwards it to the local planner.
    void SetDestination(const geom::Location &end,
                        std::optional<geom::Location> start = std::nullopt);

    void SetGlobalPlan(const std::vector<LocalPlanner::PlanItem> &plan,
                       bool stop_waypoint_creation = true,
                       bool clean_queue = true);

    std::vector<LocalPlanner::PlanItem>
        TraceRoute(SharedPtr<client::Waypoint> start,
                   SharedPtr<client::Waypoint> end);

    virtual rpc::VehicleControl RunStep();

    bool Done() const;

    void IgnoreTrafficLights(bool active = true);
    void IgnoreStopSigns(bool active = true);
    void IgnoreVehicles(bool active = true);
    void SetOffset(float offset);

    /// Replaces the current plan with one that performs a lane change.
    /// `direction` must be "left" or "right".
    void LaneChange(const std::string &direction,
                    float same_lane_time = 0.0f,
                    float other_lane_time = 0.0f,
                    float lane_change_time = 2.0f);

  protected:

    // ---- helpers exposed to BehaviorAgent ----------------------------------

    /// Returns (affected, light) similar to Python `_affected_by_traffic_light`.
    /// `lights` may be null — in which case all *traffic_light* actors are
    /// pulled from the world. `max_distance` defaults to `_base_tlight_threshold`.
    std::pair<bool, SharedPtr<client::TrafficLight>>
        AffectedByTrafficLight(SharedPtr<client::ActorList> lights = nullptr,
                               std::optional<float> max_distance = std::nullopt);

    struct VehicleObstacle {
      bool                     affected = false;
      SharedPtr<client::Actor> actor;
      float                    distance = -1.0f;
    };

    /// Mirrors Python `_vehicle_obstacle_detected`.
    VehicleObstacle VehicleObstacleDetected(
        SharedPtr<client::ActorList> vehicles = nullptr,
        std::optional<float> max_distance = std::nullopt,
        float up_angle_th = 90.0f,
        float low_angle_th = 0.0f,
        int   lane_offset = 0);

    /// Mirrors Python `_generate_lane_change_path`. Returns an empty vector
    /// if no valid path exists.
    std::vector<LocalPlanner::PlanItem> GenerateLaneChangePath(
        SharedPtr<client::Waypoint> waypoint,
        const std::string &direction = "left",
        float distance_same_lane = 10.0f,
        float distance_other_lane = 25.0f,
        float lane_change_distance = 25.0f,
        bool  check = true,
        int   lane_changes = 1,
        float step_distance = 2.0f);

    // ---- members BehaviorAgent reads/touches -------------------------------
    SharedPtr<client::Vehicle>          _vehicle;
    SharedPtr<client::World>            _world;
    SharedPtr<client::Map>              _map;
    SharedPtr<client::TrafficLight>     _last_traffic_light;
    std::unique_ptr<LocalPlanner>       _local_planner;
    SharedPtr<GlobalRoutePlanner>       _global_planner;
    SharedPtr<client::ActorList>        _lights_list;
    std::unordered_map<uint32_t, SharedPtr<client::Waypoint>> _lights_map;

    bool  _ignore_traffic_lights;
    bool  _ignore_stop_signs;
    bool  _ignore_vehicles;
    bool  _use_bbs_detection;
    float _target_speed;
    float _sampling_resolution;
    float _base_tlight_threshold;
    float _base_vehicle_threshold;
    float _speed_ratio;
    float _max_brake;
    float _offset;
  };

  inline BasicAgent::Options BasicAgent::DefaultOptions() {
    return Options{};
  }

} // namespace navigation
} // namespace agents
} // namespace carla
