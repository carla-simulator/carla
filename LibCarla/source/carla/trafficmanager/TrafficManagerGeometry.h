// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "carla/geom/Location.h"

#include "carla/trafficmanager/SimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

namespace cg = carla::geom;
using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
using Buffer = std::deque<SimpleWaypointPtr>;

// Pure geometry helpers shared between the LocalizationStage (junction radius
// gating) and the MotionPlanStage (target waypoint selection). Free functions
// so the unit-test suite can exercise them directly without a class instance.

/// Returns the radius of the circle defined by three points in the XY plane.
/// Returns std::numeric_limits<float>::max() when the three points are
/// collinear (no circumscribing circle exists).
[[nodiscard]] float GetThreePointCircleRadius(
    cg::Location first_location,
    cg::Location middle_location,
    cg::Location last_location);

/// Locations-only variant of GetTargetData. Takes a flat list of waypoint
/// locations and walks them to find the bracketing pair around
/// `target_distance` from `vehicle_location`, then linearly interpolates.
/// Exposed as a free function so the unit-test suite can exercise it
/// without constructing SimpleWaypoint instances (which require a real
/// carla::client::Waypoint and therefore a road map).
///
/// Defensive: an empty list returns {vehicle_location, 0u}; a single-element
/// list returns that element.
[[nodiscard]] std::pair<cg::Location, uint64_t> InterpolateBufferAt(
    const std::vector<cg::Location> &waypoint_locations,
    float target_distance,
    cg::Location vehicle_location);

/// Returns the location reached by walking `target_distance` along the
/// waypoint buffer from `vehicle_location`, plus the index of the bracketing
/// waypoint. Thin adapter that extracts each waypoint's location and
/// delegates to InterpolateBufferAt.
[[nodiscard]] std::pair<cg::Location, uint64_t> GetTargetData(
    const Buffer &waypoint_buffer,
    float target_distance,
    cg::Location vehicle_location);

}  // namespace traffic_manager
}  // namespace carla
