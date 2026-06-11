// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/TrafficManagerGeometry.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "carla/trafficmanager/Constants.h"

namespace carla {
namespace traffic_manager {

using constants::Collision::EPSILON;
using constants::MotionPlan::PI;

float GetThreePointCircleRadius(
    cg::Location first_location,
    cg::Location middle_location,
    cg::Location last_location) {

  const float x1{first_location.x};
  const float y1{first_location.y};
  const float x2{middle_location.x};
  const float y2{middle_location.y};
  const float x3{last_location.x};
  const float y3{last_location.y};

  const float x12{x1 - x2};
  const float x13{x1 - x3};
  const float y12{y1 - y2};
  const float y13{y1 - y3};
  const float y31{y3 - y1};
  const float y21{y2 - y1};
  const float x31{x3 - x1};
  const float x21{x2 - x1};

  const float sx13{x1 * x1 - x3 * x3};
  const float sy13{y1 * y1 - y3 * y3};
  const float sx21{x2 * x2 - x1 * x1};
  const float sy21{y2 * y2 - y1 * y1};

  const float f_denom{2.0f * (y31 * x12 - y21 * x13)};
  if (std::abs(f_denom) <= EPSILON) {
    return std::numeric_limits<float>::max();
  }
  const float f{(sx13 * x12 + sy13 * x12 + sx21 * x13 + sy21 * x13) / f_denom};

  const float g_denom{2.0f * (x31 * y12 - x21 * y13)};
  if (std::abs(g_denom) <= EPSILON) {
    return std::numeric_limits<float>::max();
  }
  const float g{(sx13 * y12 + sy13 * y12 + sx21 * y13 + sy21 * y13) / g_denom};

  const float c{-(x1 * x1 + y1 * y1) - 2.0f * g * x1 - 2.0f * f * y1};
  const float h{-g};
  const float k{-f};

  return std::sqrt(h * h + k * k - c);
}

std::pair<cg::Location, uint64_t> InterpolateBufferAt(
    const std::vector<cg::Location> &waypoint_locations,
    float target_distance,
    cg::Location vehicle_location) {

  if (waypoint_locations.empty()) {
    return std::make_pair(vehicle_location, 0u);
  }

  if (waypoint_locations.size() == 1u) {
    return std::make_pair(waypoint_locations.front(), 0u);
  }

  const float target_square_distance{target_distance * target_distance};

  // Walk every waypoint and find the first one whose distance is greater than
  // or equal to target_distance. That waypoint is the upper bracket; the one
  // before it is the lower bracket.
  size_t closest_index{0u};
  size_t farthest_index{0u};
  bool found_bracket{false};
  for (size_t i = 0u; i < waypoint_locations.size(); ++i) {
    const float close_dist_square{vehicle_location.DistanceSquared(waypoint_locations.at(i))};

    if (close_dist_square < target_square_distance) {
      closest_index = i;
    } else {
      farthest_index = i;
      found_bracket = true;
      break;
    }
  }

  // Target distance is beyond every waypoint in the buffer: clamp to the
  // last waypoint instead of extrapolating off the front segment.
  if (!found_bracket) {
    const size_t last_index{waypoint_locations.size() - 1u};
    return std::make_pair(waypoint_locations.at(last_index), last_index);
  }

  // Target distance is shorter than the distance to the first waypoint:
  // bracket with the first segment so the interpolation produces a point
  // ahead of the vehicle along the planned path.
  if (closest_index == 0u && farthest_index == 0u) {
    farthest_index = 1u;
  }

  const cg::Location target_close_location{waypoint_locations.at(closest_index)};
  const cg::Location target_far_location{waypoint_locations.at(farthest_index)};
  const float target_close_distance{vehicle_location.Distance(target_close_location)};
  const float target_far_distance{vehicle_location.Distance(target_far_location)};

  // Guard against the bracketing pair coinciding (zero denominator).
  const float span{target_far_distance - target_close_distance};
  if (std::abs(span) <= EPSILON) {
    return std::make_pair(target_close_location, closest_index);
  }

  const float t{(target_distance - target_close_distance) / span};
  const cg::Location target_location{
      target_close_location.x + (target_far_location.x - target_close_location.x) * t,
      target_close_location.y + (target_far_location.y - target_close_location.y) * t,
      target_close_location.z + (target_far_location.z - target_close_location.z) * t};

  return std::make_pair(target_location, closest_index);
}

std::pair<cg::Location, uint64_t> GetTargetData(
    const Buffer &waypoint_buffer,
    float target_distance,
    cg::Location vehicle_location) {

  std::vector<cg::Location> locations;
  locations.reserve(waypoint_buffer.size());
  for (const SimpleWaypointPtr &waypoint : waypoint_buffer) {
    locations.emplace_back(waypoint->GetLocation());
  }
  return InterpolateBufferAt(locations, target_distance, vehicle_location);
}

float LargeVehicleJunctionOffsetProfile(
    float t,
    float max_offset,
    float max_offset_point,
    float inboard_scale) {

  t = std::clamp(t, 0.0f, 1.0f);

  // From +offset (inboard, near the exit) to -offset (outboard, near the
  // entry); entry and exit stay at 0 for a smooth transition. The vehicle
  // opens up at the entry to perform a wider turn, then returns along a
  // straighter trajectory.
  float offset{0.0f};
  if (t < max_offset_point) {
    const float a{t / max_offset_point};
    offset = max_offset * 0.5f * (1.0f - std::cos(PI * a));
  } else if (t < 1.0f - max_offset_point) {
    const float a{(t - max_offset_point) / (1.0f - 2.0f * max_offset_point)};
    offset = max_offset * std::cos(PI * a);
  } else {
    const float a{(t - (1.0f - max_offset_point)) / max_offset_point};
    offset = -max_offset * 0.5f * (1.0f + std::cos(PI * a));
  }

  // Attenuate the inboard (positive) excursion — the exit "cut-in" that drives
  // a long vehicle's rear into the inside shoulder — while preserving the
  // outboard (negative) swing that opens the turn up.
  if (offset > 0.0f) {
    offset *= inboard_scale;
  }
  return offset;
}

float LargeVehicleOffsetMagnitude(
    float vehicle_length,
    float reference_length,
    float gain,
    float offset_cap) {

  const float scaled{gain * (vehicle_length - reference_length)};
  return std::clamp(scaled, 0.0f, offset_cap);
}

bool IsOffsetSideOccupied(
    cg::Location ego_location,
    cg::Vector3D ego_forward,
    cg::Vector3D offset_direction,
    float offset_magnitude,
    float lateral_clearance,
    float longitudinal_window,
    const std::vector<std::pair<cg::Location, float>> &neighbours) {

  for (const auto &[location, radius] : neighbours) {
    const cg::Vector3D relative{
        location.x - ego_location.x,
        location.y - ego_location.y,
        0.0f};
    const float lateral{relative.x * offset_direction.x + relative.y * offset_direction.y};
    const float longitudinal{relative.x * ego_forward.x + relative.y * ego_forward.y};

    const bool on_offset_side{
        lateral > -radius
        && lateral <= offset_magnitude + lateral_clearance + radius};
    const bool alongside{std::abs(longitudinal) <= longitudinal_window + radius};

    if (on_offset_side && alongside) {
      return true;
    }
  }
  return false;
}

}  // namespace traffic_manager
}  // namespace carla
