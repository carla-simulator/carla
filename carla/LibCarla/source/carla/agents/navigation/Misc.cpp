// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/agents/navigation/Misc.h"

#include "carla/client/Actor.h"
#include "carla/client/DebugHelper.h"
#include "carla/client/TrafficLight.h"
#include "carla/client/Waypoint.h"
#include "carla/client/World.h"
#include "carla/geom/BoundingBox.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace carla {
namespace agents {
namespace navigation {

namespace {

constexpr float kKmhPerMs = 3.6f;

// Replicates numpy's `np.finfo(float).eps` regularisation used in the Python
// helpers so we don't divide by zero on coincident points.
constexpr float kNormEps = std::numeric_limits<float>::epsilon();

} // namespace

float GetSpeed(SharedPtr<client::Actor> actor) {
  if (!actor) {
    return 0.0f;
  }
  const auto vel = actor->GetVelocity();
  return kKmhPerMs * std::sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z);
}

geom::Location GetTrafficLightTriggerLocation(
    SharedPtr<client::TrafficLight> traffic_light) {
  // Local helper that mirrors the lambda in misc.py. The Python code rotates
  // the (0, 0, extent.z) point by the traffic light's yaw angle and adds it
  // back onto the trigger volume's transformed location.
  auto rotate_point = [](const geom::Vector3D &point, float radians) {
    const float rotated_x =
        std::cos(radians) * point.x - std::sin(radians) * point.y;
    const float rotated_y =
        std::sin(radians) * point.x - std::cos(radians) * point.y;
    return geom::Vector3D(rotated_x, rotated_y, point.z);
  };

  const geom::Transform base_transform = traffic_light->GetTransform();
  const float base_rot = base_transform.rotation.yaw;
  const geom::BoundingBox &trigger = traffic_light->GetTriggerVolume();

  // Transform the trigger volume's local centre into world space.
  geom::Vector3D area_loc(trigger.location.x, trigger.location.y,
                          trigger.location.z);
  base_transform.TransformPoint(area_loc);

  const geom::Vector3D &area_ext = trigger.extent;

  const geom::Vector3D point = rotate_point(
      geom::Vector3D(0.0f, 0.0f, area_ext.z),
      geom::Math::ToRadians(base_rot));

  return geom::Location(area_loc.x + point.x,
                        area_loc.y + point.y,
                        area_loc.z);
}

bool IsWithinDistance(
    const geom::Transform &target,
    const geom::Transform &reference,
    float max_distance,
    std::optional<std::pair<float, float>> angle_interval) {
  const float dx = target.location.x - reference.location.x;
  const float dy = target.location.y - reference.location.y;
  const float norm_target = std::sqrt(dx * dx + dy * dy);

  // Match the early-out behaviour from misc.py.
  if (norm_target < 0.001f) {
    return true;
  }
  if (norm_target > max_distance) {
    return false;
  }
  if (!angle_interval.has_value()) {
    return true;
  }

  const float min_angle = angle_interval->first;
  const float max_angle = angle_interval->second;

  const auto fwd = reference.GetForwardVector();
  const float dot = (fwd.x * dx + fwd.y * dy) / norm_target;
  const float clamped = std::clamp(dot, -1.0f, 1.0f);
  const float angle_deg = geom::Math::ToDegrees(std::acos(clamped));

  return min_angle < angle_deg && angle_deg < max_angle;
}

std::pair<float, float> ComputeMagnitudeAngle(
    const geom::Location &target,
    const geom::Location &current,
    float orientation_deg) {
  const float dx = target.x - current.x;
  const float dy = target.y - current.y;
  const float norm_target = std::sqrt(dx * dx + dy * dy);

  const float orientation_rad = geom::Math::ToRadians(orientation_deg);
  const float fwd_x = std::cos(orientation_rad);
  const float fwd_y = std::sin(orientation_rad);

  // Avoid a divide-by-zero when target and current coincide.
  const float denom = norm_target > 0.0f ? norm_target : kNormEps;
  const float dot = (fwd_x * dx + fwd_y * dy) / denom;
  const float clamped = std::clamp(dot, -1.0f, 1.0f);
  const float angle_deg = geom::Math::ToDegrees(std::acos(clamped));

  return {norm_target, angle_deg};
}

float DistanceVehicle(
    SharedPtr<client::Waypoint> waypoint,
    const geom::Transform &vehicle_transform) {
  const auto &wp_loc = waypoint->GetTransform().location;
  const float dx = wp_loc.x - vehicle_transform.location.x;
  const float dy = wp_loc.y - vehicle_transform.location.y;
  return std::sqrt(dx * dx + dy * dy);
}

std::array<float, 3> UnitVector(
    const geom::Location &a,
    const geom::Location &b) {
  const float x = b.x - a.x;
  const float y = b.y - a.y;
  const float z = b.z - a.z;
  // Match Python's `np.linalg.norm(...) + np.finfo(float).eps`.
  const float norm = std::sqrt(x * x + y * y + z * z) + kNormEps;
  return {x / norm, y / norm, z / norm};
}

float ComputeDistance(
    const geom::Location &a,
    const geom::Location &b) {
  const float x = b.x - a.x;
  const float y = b.y - a.y;
  const float z = b.z - a.z;
  return std::sqrt(x * x + y * y + z * z) + kNormEps;
}

void DrawWaypoints(
    SharedPtr<client::World> world,
    const std::vector<SharedPtr<client::Waypoint>> &waypoints,
    float z) {
  if (!world) {
    return;
  }
  auto debug = world->MakeDebugHelper();
  for (const auto &wpt : waypoints) {
    if (!wpt) continue;
    const auto wpt_t = wpt->GetTransform();
    const geom::Location begin(
        wpt_t.location.x,
        wpt_t.location.y,
        wpt_t.location.z + z);
    const float angle = geom::Math::ToRadians(wpt_t.rotation.yaw);
    const geom::Location end(
        begin.x + std::cos(angle),
        begin.y + std::sin(angle),
        begin.z);
    debug.DrawArrow(
        begin,
        end,
        /*thickness=*/0.1f,
        /*arrow_size=*/0.3f,
        client::DebugHelper::Color{255u, 0u, 0u},
        /*life_time=*/1.0f);
  }
}

} // namespace navigation
} // namespace agents
} // namespace carla
