// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/agents/navigation/BasicAgent.h"
#include "carla/agents/navigation/Types.h"
#include "carla/rpc/VehicleControl.h"

namespace carla {

namespace client {
  class Actor;
  class ActorList;
  class Map;
  class Vehicle;
  class Waypoint;
} // namespace client

namespace agents {
namespace navigation {

  class GlobalRoutePlanner;

  /// Behavior-driven navigation agent. Mirrors
  /// agents.navigation.behavior_agent.BehaviorAgent in the Python API.
  class BehaviorAgent : public BasicAgent {
  public:
    enum class Behavior { Cautious, Normal, Aggressive };

    BehaviorAgent(SharedPtr<client::Vehicle> vehicle,
                  Behavior behavior = Behavior::Normal,
                  const Options &opts = BasicAgent::DefaultOptions(),
                  SharedPtr<client::Map> map = nullptr,
                  SharedPtr<GlobalRoutePlanner> grp = nullptr);

    /// Returns true if the agent is affected by a traffic light.
    bool TrafficLightManager();

    /// Execute one step of navigation. The bool overload mirrors the Python
    /// `run_step(debug=False)` signature.
    rpc::VehicleControl RunStep(bool debug);

    /// Polymorphic entry point: keeps dispatch through `BasicAgent*` correct.
    rpc::VehicleControl RunStep() override { return RunStep(false); }

    /// Build an emergency-stop control: throttle=0, brake=_max_brake,
    /// hand_brake=false, steer left untouched (default 0).
    rpc::VehicleControl EmergencyStop() const;

  private:
    /// Refresh cached scene information consumed by RunStep.
    void UpdateInformation();

    /// Tailgating-avoidance lane change, called from CollisionAndCarAvoidManager.
    void Tailgating(SharedPtr<client::Waypoint> waypoint);

    /// Detect collision/car-avoidance opportunities ahead.
    VehicleObstacle CollisionAndCarAvoidManager(SharedPtr<client::Waypoint> waypoint);

    /// Detect pedestrians ahead.
    VehicleObstacle PedestrianAvoidManager(SharedPtr<client::Waypoint> waypoint);

    /// Car-following control law applied when there is a vehicle in front.
    rpc::VehicleControl CarFollowingManager(SharedPtr<client::Actor> vehicle,
                                            float distance,
                                            bool debug = false);

    BehaviorParameters _behavior;
    int   _look_ahead_steps = 0;
    float _speed = 0.0f;            // km/h
    float _speed_limit = 0.0f;      // km/h
    float _min_speed = 5.0f;        // km/h
    RoadOption _direction = RoadOption::LaneFollow;
    RoadOption _incoming_direction = RoadOption::LaneFollow;
    SharedPtr<client::Waypoint> _incoming_waypoint;
  };

} // namespace navigation
} // namespace agents
} // namespace carla
