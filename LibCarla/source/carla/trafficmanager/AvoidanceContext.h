// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

/// Data types for the lateral-avoidance behaviour graph. The graph is made of
/// stateless maneuver nodes (see ManeuverState in DataStructures.h); all
/// per-vehicle mutable state lives in AvoidContext, and each cycle's perception
/// is gathered into AvoidPerception. Keeping node logic stateless and the state
/// here is what lets each maneuver be developed and tuned independently.

#pragma once

#include "carla/geom/Location.h"
#include "carla/trafficmanager/DataStructures.h"

namespace carla {
namespace traffic_manager {

/// Per-vehicle persistent state of the behaviour graph.
struct AvoidContext {
  /// Current node of the behaviour graph.
  ManeuverState state = ManeuverState::FOLLOW;
  /// Lateral offset actually applied last cycle (metres, signed, + = right).
  /// The graph rate-limits the emitted offset towards each node's target, so
  /// this is the smoothed value carried between cycles.
  float committed_offset = 0.0f;
  /// Ego location at the last state transition, for distance-based hysteresis.
  cg::Location state_entry_location;
};

/// Per-cycle perception feeding the behaviour graph (recomputed every frame,
/// never persisted).
struct AvoidPerception {
  /// A near-stationary obstacle intrudes the driving corridor on one side.
  bool side_obstacle = false;
  /// Signed lateral offset (metres, + = right) that would clear the obstacle
  /// with the configured margin, already clamped to the drivable lane width.
  float target_offset = 0.0f;
  /// Half lane width (metres) at the look-ahead reference point.
  float lane_half_width = 0.0f;
  /// Ego half width (metres).
  float ego_half_width = 0.0f;
  /// A blocker sits ahead in-lane (from the collision stage).
  bool blocker_ahead = false;
  /// The blocker ahead is stopped (velocity below threshold).
  bool blocker_stopped = false;
  /// A stopped blocker can be passed within the lane on one side with the
  /// configured margin.
  bool bypass_feasible = false;
  /// Signed lateral offset (metres, + = right) that hugs the free side to pass
  /// the blocker within the lane.
  float bypass_offset = 0.0f;
};

} // namespace traffic_manager
} // namespace carla
