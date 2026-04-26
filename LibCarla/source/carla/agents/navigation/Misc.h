// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"

#include <array>
#include <optional>
#include <utility>
#include <vector>

namespace carla {

namespace client {
  class Actor;
  class TrafficLight;
  class Waypoint;
  class World;
} // namespace client

namespace geom {
  class Location;
  class Transform;
} // namespace geom

namespace agents {
namespace navigation {

  /// Compute speed of an actor in km/h.
  float GetSpeed(SharedPtr<client::Actor> actor);

  /// Compute the world-space trigger location of a traffic light, mirroring
  /// the helper in `agents.tools.misc.get_trafficlight_trigger_location`.
  geom::Location GetTrafficLightTriggerLocation(
      SharedPtr<client::TrafficLight> traffic_light);

  /// Returns true when @a target is within @a max_distance of @a reference.
  /// When @a angle_interval is provided, the angle (in degrees) between the
  /// reference's forward vector and the vector reference -> target must lie
  /// in the open interval (min, max).
  bool IsWithinDistance(
      const geom::Transform &target,
      const geom::Transform &reference,
      float max_distance,
      std::optional<std::pair<float, float>> angle_interval = std::nullopt);

  /// Returns the (distance, angle in degrees) from @a current to @a target
  /// relative to a forward orientation @a orientation_deg in degrees.
  std::pair<float, float> ComputeMagnitudeAngle(
      const geom::Location &target,
      const geom::Location &current,
      float orientation_deg);

  /// 2-D distance between a waypoint and a vehicle transform.
  float DistanceVehicle(
      SharedPtr<client::Waypoint> waypoint,
      const geom::Transform &vehicle_transform);

  /// Unit vector going from @a a to @a b, including the z component.
  std::array<float, 3> UnitVector(
      const geom::Location &a,
      const geom::Location &b);

  /// Euclidean distance between two locations (matches numpy's regularised
  /// version that adds an epsilon to avoid 0).
  float ComputeDistance(
      const geom::Location &a,
      const geom::Location &b);

  /// Clamp @a v at zero. Inline because it's tiny.
  inline float Positive(float v) { return v > 0.0f ? v : 0.0f; }

  /// Draw the supplied waypoints in the world for a fixed lifetime.
  void DrawWaypoints(
      SharedPtr<client::World> world,
      const std::vector<SharedPtr<client::Waypoint>> &waypoints,
      float z = 0.5f);

} // namespace navigation
} // namespace agents
} // namespace carla
