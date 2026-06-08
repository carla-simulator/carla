// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <deque>
#include <memory>
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

/// Lateral offset profile for the large-vehicle wide turn, as a function of
/// `t`, the fraction of the junction still ahead of the target waypoint
/// (1 at the entry, 0 at the exit). Returns the *unsigned* offset in the
/// right-turn convention: positive values are inboard (toward the inside of
/// the turn, the exit "cut-in"), negative values are outboard (the entry
/// swing that opens the turn up). The caller flips the sign for left turns.
///
/// The raw profile is a cosine blend that is 0 at both entry and exit. The
/// inboard (positive) lobe is then scaled by `inboard_scale` in [0, 1] so the
/// cut-in that drives a long vehicle's rear into the inside shoulder can be
/// attenuated (0) or kept (1) independently of the outboard swing.
[[nodiscard]] float LargeVehicleJunctionOffsetProfile(
    float t,
    float max_offset,
    float max_offset_point,
    float inboard_scale);

/// Wide-turn offset magnitude scaled by vehicle length:
/// clamp(gain * (vehicle_length - reference_length), 0, offset_cap).
/// Short vehicles (at or below the reference length) get no offset; longer
/// vehicles ramp up to the cap.
[[nodiscard]] float LargeVehicleOffsetMagnitude(
    float vehicle_length,
    float reference_length,
    float gain,
    float offset_cap);

/// Returns true when a neighbouring vehicle sits in the lateral band that the
/// wide-turn offset would swing the ego vehicle into. `offset_direction` is a
/// unit vector pointing to the swing side; `ego_forward` is the ego heading
/// unit vector. A neighbour counts as blocking when it is on the offset side
/// within `offset_magnitude + lateral_clearance` (plus its own radius) and
/// roughly alongside the ego within `longitudinal_window` (plus its radius).
/// Each neighbour is a {location, radius} pair.
[[nodiscard]] bool IsOffsetSideOccupied(
    cg::Location ego_location,
    cg::Vector3D ego_forward,
    cg::Vector3D offset_direction,
    float offset_magnitude,
    float lateral_clearance,
    float longitudinal_window,
    const std::vector<std::pair<cg::Location, float>> &neighbours);

}  // namespace traffic_manager
}  // namespace carla
