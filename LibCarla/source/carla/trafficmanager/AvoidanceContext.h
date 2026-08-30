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

#include <vector>

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
  /// Latched hazard-release: set once the maneuver verifies a stopped obstacle
  /// with a feasible lateral plan, held for the rest of the maneuver so a noisy
  /// per-frame "stopped" reading (parked cars jitter on their suspension) does
  /// not toggle the collision stop and produce stop-and-go motion.
  bool hold_clear = false;
  /// Ticks remaining until the next static-obstacle raycast. The raycast is an
  /// RPC to the server and static props do not move, so hits are cached between
  /// casts (see static_hits) and only refreshed every RAYCAST_REFRESH_TICKS to
  /// keep the server RPC load low.
  int raycast_countdown = 0;
  /// Cached world-space locations of static-prop ray hits, reused between casts.
  std::vector<cg::Location> static_hits;
  /// Debounced ("stable") situation label -- what the estimator publishes and
  /// the behaviour graph / API sees. The raw per-frame classification is noisy
  /// (a blocker flag or a borrow test can toggle frame to frame), so the raw
  /// label must persist before it is adopted here.
  SituationLabel stable_situation = SituationLabel::CLEAR;
  /// The raw label currently accumulating confirmation frames, and how many
  /// consecutive frames it has held. When it reaches the confirm threshold it
  /// is promoted to stable_situation.
  SituationLabel sit_candidate = SituationLabel::CLEAR;
  int sit_confirm = 0;
  /// Frames the stable label is force-held after being set, so a briefly-seen
  /// pass situation (STATIC_BLOCKER / ONCOMING_BLOCK) does not evaporate the
  /// moment the noisy blocker flag drops -- it stays long enough to gate action.
  int sit_hold = 0;
  /// Confidence carried with stable_situation.
  float stable_confidence = 0.0f;
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
  /// The lane is fully blocked (no in-lane bypass), an adjacent oncoming Driving
  /// lane exists, and it is clear of opposing traffic over the commit horizon:
  /// the ego may borrow the oncoming lane to pass, then return.
  bool borrow_oncoming = false;
  /// Signed lateral offset (metres, + = right; negative here) that places the
  /// ego in the oncoming lane to pass the blockage.
  float borrow_offset = 0.0f;

  // --- Junction gap-acceptance (unprotected-turn creep-and-commit) ----------
  /// The ego is approaching a junction whose maneuver crosses conflicting
  /// traffic (a turn), within the scan horizon.
  bool at_junction = false;
  /// Longitudinal distance (metres) along the path to the junction entrance.
  float dist_to_junction = 0.0f;
  /// The ego is at/inside the hold line: close enough to the junction that it
  /// should stop and wait rather than keep creeping.
  bool at_hold_line = false;
  /// No conflicting vehicle arrives at the junction within the commit horizon:
  /// the gap is large enough to accept and cross.
  bool junction_gap_clear = false;
  /// Smallest time-to-junction (seconds) over conflicting vehicles; +inf when
  /// none (bring-up diagnostic).
  float conflict_ttc = 0.0f;

  // --- Situation estimate ---------------------------------------------------
  /// A walker is in / entering the ego path ahead (from the pedestrian scan).
  bool pedestrian_in_path = false;
  /// An opposing-heading vehicle occupies / invades the ego lane ahead.
  bool oncoming_intrusion = false;
  /// A moving in-lane lead is decelerating (approximated from relative speed).
  bool lead_braking = false;
  /// Coarse driving-situation estimate for this cycle (see SituationLabel).
  SituationLabel situation = SituationLabel::CLEAR;
  /// Confidence in [0,1] of the situation estimate (margin of the deciding
  /// test); low values mean "acted defensively on a weak signal".
  float situation_confidence = 0.0f;
};

} // namespace traffic_manager
} // namespace carla
