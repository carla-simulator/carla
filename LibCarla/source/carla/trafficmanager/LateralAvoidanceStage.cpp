// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <unordered_set>

#include "carla/client/World.h"
#include "carla/rpc/LabelledPoint.h"
#include "carla/rpc/ObjectLabel.h"
#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/LateralAvoidanceStage.h"

namespace carla {
namespace traffic_manager {

using namespace constants::LateralAvoidance;
using constants::VehicleRemoval::STOPPED_VELOCITY_THRESHOLD;

namespace {

// Semantic labels a lateral ray treats as a solid, avoidable obstacle. Road
// surface, lane lines, sidewalks, terrain and vegetation are passable/irrelevant
// and excluded; vehicle/pedestrian labels are already handled as actors, so they
// are excluded here to avoid double-counting.
bool IsBlockingStaticLabel(const rpc::CityObjectLabel label) {
  switch (label) {
    case rpc::CityObjectLabel::Static:
    case rpc::CityObjectLabel::Dynamic:
    case rpc::CityObjectLabel::Other:
    case rpc::CityObjectLabel::Poles:
    case rpc::CityObjectLabel::Fences:
    case rpc::CityObjectLabel::Walls:
    case rpc::CityObjectLabel::GuardRail:
      return true;
    default:
      return false;
  }
}

// ===========================================================================
// Behaviour graph: stateless maneuver nodes + a data-driven transition table.
// Each node contributes a target lateral offset and speed factor; the stage
// rate-limits the emitted offset towards that target. Adding a maneuver means
// adding a node case below and one (or more) rows to kTransitions — existing
// nodes are untouched.
// ===========================================================================

// The signed lateral offset (metres, + = right) a node steers the ego towards.
float NodeTargetOffset(const ManeuverState state, const AvoidPerception &p) {
  switch (state) {
    case ManeuverState::LANE_BORROW_ONCOMING:
      return p.borrow_offset;
    case ManeuverState::IN_LANE_BYPASS:
      return p.bypass_offset;
    case ManeuverState::SIDE_CLEARANCE:
      return p.target_offset;
    case ManeuverState::FOLLOW:
    default:
      return 0.0f;
  }
}

// Whether a node releases the collision emergency-stop for the stopped blocker
// it is steering around. The collision stage reasons about the lane centerline
// and cannot see the lateral offset, so without this the ego brakes the whole
// way and can never edge past. Gated on the blocker being stopped and a
// feasible lateral plan existing; a moving lead still causes a normal stop.
bool NodeClearsHazard(const ManeuverState state, const AvoidPerception &p) {
  switch (state) {
    case ManeuverState::LANE_BORROW_ONCOMING:
      // Only release the stop while the borrow is actually feasible (oncoming
      // still clear). If an opposing vehicle appears mid-maneuver this drops to
      // false, restoring the collision stop until the gap reopens.
      return p.borrow_oncoming;
    case ManeuverState::IN_LANE_BYPASS:
      return p.bypass_feasible;
    case ManeuverState::SIDE_CLEARANCE:
      return p.side_obstacle && p.blocker_stopped;
    default:
      return false;
  }
}

// --- Transition guards (pure functions of context + perception) ------------

// A situation in which steering around is unsafe: a walker in the path or an
// opposing vehicle occupying the lane. The ego must hold (the collision stage
// keeps braking for the actor), never edge or borrow around it.
bool SituationBlocksLateral(const AvoidPerception &p) {
  return p.situation == SituationLabel::PEDESTRIAN_CROSSING ||
         p.situation == SituationLabel::ONCOMING_INTRUSION;
}

bool GuardEnterSideClearance(const AvoidContext &, const AvoidPerception &p) {
  return !SituationBlocksLateral(p) && p.side_obstacle &&
         std::fabs(p.target_offset) >= ENTER_OFFSET_THRESHOLD;
}

bool GuardEnterBypass(const AvoidContext &, const AvoidPerception &p) {
  return !SituationBlocksLateral(p) && p.blocker_ahead && p.blocker_stopped &&
         p.bypass_feasible;
}

bool GuardExitBypass(const AvoidContext &, const AvoidPerception &p) {
  // Leave the bypass once the blocker is no longer ahead (passed or gone).
  return !p.blocker_ahead;
}

bool GuardExitToFollow(const AvoidContext &ctx, const AvoidPerception &p) {
  // Hysteresis: leave SIDE_CLEARANCE only once the obstacle is gone and the
  // applied offset has decayed close to zero.
  return !p.side_obstacle && std::fabs(ctx.committed_offset) <= EXIT_OFFSET_THRESHOLD;
}

// --- Oncoming lane-borrow guards -------------------------------------------

bool GuardEnterBorrowOncoming(const AvoidContext &, const AvoidPerception &p) {
  // The lane is fully blocked (no in-lane bypass), and an oncoming Driving lane
  // is present and clear over the commit horizon. Never borrow when a walker or
  // an opposing vehicle is the situation -- that lane is exactly where the
  // danger is.
  return !SituationBlocksLateral(p) && p.blocker_ahead && !p.bypass_feasible &&
         p.borrow_oncoming;
}

bool GuardExitBorrowOncoming(const AvoidContext &, const AvoidPerception &p) {
  // Return to the lane once the blockage is no longer ahead (passed or gone).
  return !p.blocker_ahead;
}

// --- Junction gap-acceptance guards ----------------------------------------

bool GuardEnterJunction(const AvoidContext &, const AvoidPerception &p) {
  return p.at_junction;
}

bool GuardJunctionCommit(const AvoidContext &, const AvoidPerception &p) {
  // Accept the gap and cross only once near the junction (at/inside the commit
  // distance, i.e. at the yield point) AND the conflicting-traffic gap is large
  // enough. Requiring proximity prevents committing while still far out, where
  // the gap looks clear only because the oncoming has not yet arrived.
  return p.at_junction && p.junction_gap_clear &&
         p.dist_to_junction <= JUNCTION_COMMIT_DISTANCE;
}

bool GuardJunctionExit(const AvoidContext &, const AvoidPerception &p) {
  // The junction scenario is over (crossed it, or the route no longer crosses).
  return !p.at_junction;
}

// True for the junction gap-acceptance nodes, whose hazard release is derived
// per-frame from the node (not latched like the lateral maneuvers).
bool IsJunctionState(const ManeuverState s) {
  return s == ManeuverState::JUNCTION_APPROACH ||
         s == ManeuverState::JUNCTION_YIELD ||
         s == ManeuverState::JUNCTION_CROSS;
}

struct Transition {
  ManeuverState from;
  ManeuverState to;
  bool (*guard)(const AvoidContext &, const AvoidPerception &);
};

// Priority-ordered: the first row whose `from` matches the current state and
// whose guard passes wins. Junction crossing takes priority over the lateral
// maneuvers (you negotiate the junction, not a parked car); within the lateral
// group a stopped blocker (bypass) takes priority over side-clearance.
const Transition kTransitions[] = {
  // Junction gap-acceptance. APPROACH holds at the yield point (baseline
  // collision-stage behaviour); it commits to CROSS only once near the junction
  // with an accepted gap, and CROSS latches until the junction is passed.
  {ManeuverState::JUNCTION_CROSS,    ManeuverState::FOLLOW,            &GuardJunctionExit},
  {ManeuverState::JUNCTION_APPROACH, ManeuverState::JUNCTION_CROSS,    &GuardJunctionCommit},
  {ManeuverState::JUNCTION_APPROACH, ManeuverState::FOLLOW,            &GuardJunctionExit},
  {ManeuverState::FOLLOW,            ManeuverState::JUNCTION_APPROACH, &GuardEnterJunction},
  // Lateral avoidance. Oncoming lane-borrow is the fallback when the lane is
  // fully blocked and no in-lane option exists; it is reachable from any lateral
  // node (a side-clearance or in-lane bypass that turns out infeasible escalates
  // to a borrow) and latches until the blockage is passed.
  {ManeuverState::LANE_BORROW_ONCOMING, ManeuverState::FOLLOW,               &GuardExitBorrowOncoming},
  {ManeuverState::IN_LANE_BYPASS, ManeuverState::LANE_BORROW_ONCOMING, &GuardEnterBorrowOncoming},
  {ManeuverState::IN_LANE_BYPASS, ManeuverState::FOLLOW,               &GuardExitBypass},
  {ManeuverState::SIDE_CLEARANCE, ManeuverState::LANE_BORROW_ONCOMING, &GuardEnterBorrowOncoming},
  {ManeuverState::SIDE_CLEARANCE, ManeuverState::IN_LANE_BYPASS,       &GuardEnterBypass},
  {ManeuverState::SIDE_CLEARANCE, ManeuverState::FOLLOW,               &GuardExitToFollow},
  {ManeuverState::FOLLOW,         ManeuverState::LANE_BORROW_ONCOMING, &GuardEnterBorrowOncoming},
  {ManeuverState::FOLLOW,         ManeuverState::IN_LANE_BYPASS,       &GuardEnterBypass},
  {ManeuverState::FOLLOW,         ManeuverState::SIDE_CLEARANCE,       &GuardEnterSideClearance},
};

} // namespace

LateralAvoidanceStage::LateralAvoidanceStage(
  const std::vector<ActorId> &vehicle_id_list,
  const SimulationState &simulation_state,
  const BufferMap &buffer_map,
  const TrackTraffic &track_traffic,
  const Parameters &parameters,
  const LocalMapPtr &local_map,
  const cc::World &world,
  const CollisionFrame &collision_frame,
  AvoidanceFrame &output_array,
  UniformPRNG &random_device)
    : vehicle_id_list(vehicle_id_list),
      simulation_state(simulation_state),
      buffer_map(buffer_map),
      track_traffic(track_traffic),
      parameters(parameters),
      local_map(local_map),
      world(world),
      collision_frame(collision_frame),
      output_array(output_array),
      random_device(random_device) {}

AvoidPerception LateralAvoidanceStage::Perceive(const ActorId actor_id, AvoidContext &ctx) const {
  AvoidPerception perception;

  const auto buffer_it = buffer_map.find(actor_id);
  if (buffer_it == buffer_map.end() || buffer_it->second.empty()) {
    return perception;
  }
  const Buffer &buffer = buffer_it->second;
  const float ego_half_width = simulation_state.GetDimensions(actor_id).y;
  perception.ego_half_width = ego_half_width;

  // Gather the candidate obstacles once (actors whose paths overlap ours).
  struct Obs {
    cg::Location location;
    cg::Vector3D heading;
    cg::Vector3D dimensions;
    bool stopped;
  };
  // Collect candidate obstacle ids from the window's stations and their lane
  // neighbours. Geodesic-grid overlap (GetOverlappingVehicles) registers a
  // vehicle by its centre, so an obstacle whose centre sits in an adjacent lane
  // while its body intrudes ours is missed. We therefore also sweep the
  // passing-vehicle sets of each station and of its left/right lane-change
  // waypoints.
  std::unordered_set<ActorId> candidate_ids;
  {
    const ActorIdSet overlapping = track_traffic.GetOverlappingVehicles(actor_id);
    candidate_ids.insert(overlapping.begin(), overlapping.end());
    float gather_accum = 0.0f;
    for (size_t i = 0; i < buffer.size(); ++i) {
      const SimpleWaypointPtr &wp = buffer.at(i);
      const SimpleWaypointPtr neighbours[3] = {wp, wp->GetRightWaypoint(),
                                               wp->GetLeftWaypoint()};
      for (const SimpleWaypointPtr &n : neighbours) {
        if (n != nullptr) {
          const ActorIdSet passing = track_traffic.GetPassingVehicles(n->GetId());
          candidate_ids.insert(passing.begin(), passing.end());
        }
      }
      if (i + 1 < buffer.size()) {
        gather_accum += wp->Distance(buffer.at(i + 1));
      }
      if (gather_accum > LOOKAHEAD_DISTANCE) {
        break;
      }
    }
  }

  std::vector<Obs> obstacles;
  obstacles.reserve(candidate_ids.size());
  for (const ActorId other_id : candidate_ids) {
    if (other_id == actor_id || !simulation_state.ContainsActor(other_id)) {
      continue;
    }
    const bool stopped = simulation_state.GetVelocity(other_id).SquaredLength() <
                         SQUARE(STOPPED_VELOCITY_THRESHOLD);
    obstacles.push_back({simulation_state.GetLocation(other_id),
                         simulation_state.GetHeading(other_id),
                         simulation_state.GetDimensions(other_id), stopped});
  }

  // Opt-in: augment with static props (cones, barriers, debris) that are not
  // registered actors and so are invisible to the vehicle perception above.
  // Cast a small lateral fan of forward rays across the lane at the ego's near
  // frame; each solid-static hit becomes a small synthetic point-obstacle fed
  // to the same station scan, so it is cleared with the identical geometry as a
  // stopped vehicle. Non-opt-in vehicles issue zero CastRay calls (no server
  // cost) -- this is also the only channel that can see static props at all.
  //
  // Rays are server RPCs, so casting a fan every tick saturates the RPC server.
  // Static props do not move, so the hits are cached in the context and only
  // re-cast every RAYCAST_REFRESH_TICKS; the small per-tick work (frame vectors,
  // rebuilding synthetic obstacles from the cache) stays local.
  if (parameters.GetLateralAvoidanceRaycast(actor_id)) {
    const SimpleWaypointPtr &near = buffer.front();
    const cg::Location origin = near->GetLocation();
    const cg::Vector3D forward = near->GetForwardVector();
    const cg::Vector3D right = near->GetTransform().GetRightVector();
    const float lane_half =
        static_cast<float>(near->GetWaypoint()->GetLaneWidth()) * 0.5f;

    if (ctx.raycast_countdown <= 0) {
      ctx.static_hits.clear();
      for (int s = 0; s < RAYCAST_LATERAL_SAMPLES; ++s) {
        const float frac = (RAYCAST_LATERAL_SAMPLES <= 1) ? 0.5f
            : static_cast<float>(s) /
              static_cast<float>(RAYCAST_LATERAL_SAMPLES - 1);
        // Span the lane plus a borrow-probe margin beyond each edge, so a barrier
        // sitting in an adjacent lane is seen before the ego borrows into it.
        const float half_span = lane_half + BORROW_PROBE_WIDTH;
        const float lat = (frac * 2.0f - 1.0f) * half_span;  // [-span, +span]
        const cg::Location start(origin.x + right.x * lat,
                                 origin.y + right.y * lat,
                                 origin.z + RAYCAST_HEIGHT);
        const cg::Location end(start.x + forward.x * RAYCAST_RANGE,
                               start.y + forward.y * RAYCAST_RANGE,
                               start.z + forward.z * RAYCAST_RANGE);
        const std::vector<rpc::LabelledPoint> hits = world.CastRay(start, end);
        for (const rpc::LabelledPoint &hit : hits) {
          if (!IsBlockingStaticLabel(hit._label)) {
            continue;
          }
          const cg::Location hloc = hit._location;
          const float along = (hloc.x - start.x) * forward.x +
                              (hloc.y - start.y) * forward.y;
          if (along < 0.0f || along > RAYCAST_RANGE) {
            continue;  // crossing beyond our reach or behind us
          }
          ctx.static_hits.push_back(hloc);
          break;  // nearest blocking hit on this ray is enough
        }
      }
      ctx.raycast_countdown = RAYCAST_REFRESH_TICKS;
    } else {
      --ctx.raycast_countdown;
    }

    // Feed the cached hits as small synthetic point-obstacles (a static prop
    // never moves -> always "stopped"). Symmetric half-extents make the along-
    // lane heading immaterial; hits now behind the ego are dropped by the
    // station scan's longitudinal window.
    for (const cg::Location &hloc : ctx.static_hits) {
      obstacles.push_back({hloc, forward,
          cg::Vector3D(RAYCAST_OBSTACLE_HALF, RAYCAST_OBSTACLE_HALF,
                       RAYCAST_OBSTACLE_HALF),
          true});
    }
  }

  if (obstacles.empty()) {
    return perception;
  }

  // Walk a window of future waypoints. At each station we scan the sides for
  // intruding obstacles and compute the lateral shift that station needs; the
  // nearest station that requires action drives the command. This deforms the
  // path locally around the obstacle and lets it return to center afterwards,
  // rather than offsetting the whole path from one ego-frame projection.
  float accumulated = 0.0f;
  for (size_t i = 0; i < buffer.size(); ++i) {
    const SimpleWaypointPtr &wp = buffer.at(i);
    const cg::Location station = wp->GetLocation();
    const cg::Vector3D forward = wp->GetForwardVector();
    const cg::Vector3D right = wp->GetTransform().GetRightVector();
    const float lane_half_width =
        static_cast<float>(wp->GetWaypoint()->GetLaneWidth()) * 0.5f;
    const float drivable_half = std::max(0.0f, lane_half_width - ego_half_width);

    // req_left <= 0 (shift left to clear a right-side obstacle),
    // req_right >= 0 (shift right to clear a left-side obstacle).
    float req_left = 0.0f;
    float req_right = 0.0f;
    bool side_stopped = false;
    bool has_blocker = false;
    float blocker_min_lat = 0.0f, blocker_max_lat = 0.0f;
    bool blocker_stopped = false;

    for (const Obs &obs : obstacles) {
      const cg::Vector3D to_obs = obs.location - station;
      const float longitudinal = to_obs.x * forward.x + to_obs.y * forward.y;
      if (std::fabs(longitudinal) > STATION_TOLERANCE) {
        continue;  // obstacle is not beside this station
      }

      // Signed lateral span of the obstacle's oriented bounding box at this
      // station's lane frame.
      const cg::Vector3D perp(-obs.heading.y, obs.heading.x, 0.0f);
      float min_lat = std::numeric_limits<float>::max();
      float max_lat = std::numeric_limits<float>::lowest();
      for (int sx = -1; sx <= 1; sx += 2) {
        for (int sy = -1; sy <= 1; sy += 2) {
          const cg::Vector3D corner = to_obs +
              obs.heading * (obs.dimensions.x * static_cast<float>(sx)) +
              perp * (obs.dimensions.y * static_cast<float>(sy));
          const float lat = corner.x * right.x + corner.y * right.y;
          min_lat = std::min(min_lat, lat);
          max_lat = std::max(max_lat, lat);
        }
      }
      if (min_lat > LATERAL_CONSIDERATION || max_lat < -LATERAL_CONSIDERATION) {
        continue;
      }

      // A stopped obstacle is treated as reaching STOPPED_INTRUSION_MARGIN
      // further into the lane (open door / mirror / load past its bbox), so one
      // hugging the lane boundary registers stably instead of flickering.
      const float intrusion = obs.stopped ? STOPPED_INTRUSION_MARGIN : 0.0f;
      if (min_lat > 0.0f) {
        const float allowed = min_lat - intrusion - CLEARANCE_MARGIN - ego_half_width;
        if (allowed < 0.0f) {
          req_left = std::min(req_left, allowed);
          side_stopped = side_stopped || obs.stopped;
        }
      } else if (max_lat < 0.0f) {
        const float allowed = max_lat + intrusion + CLEARANCE_MARGIN + ego_half_width;
        if (allowed > 0.0f) {
          req_right = std::max(req_right, allowed);
          side_stopped = side_stopped || obs.stopped;
        }
      } else if (!has_blocker) {
        // Obstacle straddles the centerline at this station: an in-lane blocker.
        has_blocker = true;
        blocker_min_lat = min_lat;
        blocker_max_lat = max_lat;
        blocker_stopped = obs.stopped;
      }
    }

    // A side obstacle that can be cleared *within* the drivable lane width wins
    // at the nearest station. The clearance must actually be achievable — a
    // shift clamped to the lane edge that still overlaps the obstacle is not a
    // valid side clearance (that is a blocker needing a lane borrow).
    // Feasible if the minimum clearance can be achieved within the lane; the
    // target then hugs the far lane boundary so the ego leaves as much distance
    // to the obstacle as the lane allows (rather than the bare minimum).
    const bool need_left = req_left < 0.0f;
    const bool need_right = req_right > 0.0f;
    bool feasible_side = false;
    float shift = 0.0f;
    if (need_left && !need_right && req_left >= -drivable_half) {
      shift = -drivable_half;
      feasible_side = true;
    } else if (need_right && !need_left && req_right <= drivable_half) {
      shift = drivable_half;
      feasible_side = true;
    }
    if (feasible_side && std::fabs(shift) > 0.0f) {
      perception.side_obstacle = true;
      perception.target_offset = shift;
      perception.lane_half_width = lane_half_width;
      perception.blocker_stopped = side_stopped;
      return perception;
    }

    // Otherwise there is an obstacle at this station we cannot side-clear: a
    // straddling in-lane blocker, or a side obstacle too far into the lane.
    const bool blocked = has_blocker || need_left || need_right;
    if (blocked) {
      perception.blocker_ahead = true;
      perception.lane_half_width = lane_half_width;
      perception.blocker_stopped = has_blocker ? blocker_stopped : side_stopped;
      if (has_blocker) {
        // Straddling blocker: try an in-lane bypass on the wider free side.
        const float right_gap = lane_half_width - blocker_max_lat;
        const float left_gap = lane_half_width + blocker_min_lat;
        const float needed = 2.0f * ego_half_width + CLEARANCE_MARGIN;
        if (right_gap >= needed && (right_gap >= left_gap || left_gap < needed)) {
          perception.bypass_feasible = true;
          perception.bypass_offset = drivable_half;
        } else if (left_gap >= needed) {
          perception.bypass_feasible = true;
          perception.bypass_offset = -drivable_half;
        }
      }

      // No in-lane option for a stopped/static full blockage: consider borrowing
      // an adjacent Driving lane (the TwoWays obstacle-passing / lane-closure
      // case). Safety: a borrow is only offered when the target corridor is
      // *positively verified clear* -- of static barriers (seen via the widened
      // raycast fan) and of traffic. If no side verifies clear the ego holds and
      // the collision stop stays authoritative, rather than driving blind into
      // the closed side.
      if (!perception.bypass_feasible && perception.blocker_stopped) {
        // Is a candidate corridor (centred at lateral offset `off`, half-width
        // `nhalf`) free enough to borrow? Static obstacles block over the path
        // window; traffic blocks over the direction-appropriate horizon.
        auto corridor_clear = [&](float off, float nhalf, bool oncoming) -> bool {
          const float band = nhalf + ego_half_width;
          const float dyn_horizon = oncoming ? BORROW_ONCOMING_CLEAR_DISTANCE
                                             : BORROW_SAME_DIR_CLEAR_DISTANCE;
          for (const Obs &o : obstacles) {
            const cg::Vector3D to_o = o.location - station;
            const float lon = to_o.x * forward.x + to_o.y * forward.y;
            const float lat = to_o.x * right.x + to_o.y * right.y;
            if (std::fabs(lat - off) > band) {
              continue;  // not in this corridor
            }
            if (o.stopped) {
              // A static barrier / parked car in the borrow lane blocks the path.
              if (lon > -ego_half_width && lon < BORROW_PATH_CLEAR_DISTANCE) {
                return false;
              }
            } else {
              const bool opposing =
                  (o.heading.x * forward.x + o.heading.y * forward.y) < 0.0f;
              // Oncoming: only opposing traffic matters. Same-direction: any
              // vehicle ahead in the corridor we would merge into.
              if ((!oncoming || opposing) && lon > -ego_half_width &&
                  lon < dyn_horizon) {
                return false;
              }
            }
          }
          return true;
        };

        const auto cwp = wp->GetWaypoint();
        bool have_same = false, have_onc = false;
        float off_same = 0.0f, off_onc = 0.0f;
        auto consider = [&](const decltype(cwp) &lane, float sign) {
          if (lane == nullptr ||
              lane->GetType() != carla::road::Lane::LaneType::Driving) {
            return;
          }
          const float nhalf = static_cast<float>(lane->GetLaneWidth()) * 0.5f;
          const float off = sign * (lane_half_width + nhalf);
          const cg::Vector3D nf = lane->GetTransform().GetForwardVector();
          const bool oncoming = (nf.x * forward.x + nf.y * forward.y) < 0.0f;
          if (!corridor_clear(off, nhalf, oncoming)) {
            return;
          }
          if (oncoming) {
            if (!have_onc) { have_onc = true; off_onc = off; }
          } else {
            if (!have_same) { have_same = true; off_same = off; }
          }
        };
        if (cwp != nullptr) {
          consider(cwp->GetRight(), 1.0f);  // right first: prefer a same-dir lane
          consider(cwp->GetLeft(), -1.0f);
        }
        // Prefer a same-direction lane; fall back to the oncoming lane.
        if (have_same) {
          perception.borrow_oncoming = true;
          perception.borrow_offset = off_same;
        } else if (have_onc) {
          perception.borrow_oncoming = true;
          perception.borrow_offset = off_onc;
        }
      }
      return perception;
    }

    if (i + 1 < buffer.size()) {
      accumulated += wp->Distance(buffer.at(i + 1));
    }
    if (accumulated > LOOKAHEAD_DISTANCE) {
      break;
    }
  }

  return perception;
}

void LateralAvoidanceStage::PerceiveJunction(const ActorId actor_id,
                                             AvoidPerception &perception) const {
  const auto buffer_it = buffer_map.find(actor_id);
  if (buffer_it == buffer_map.end() || buffer_it->second.empty()) {
    return;
  }
  const Buffer &buffer = buffer_it->second;

  // Find the first junction waypoint within the scan horizon and the along-path
  // distance to it.
  float accum = 0.0f;
  size_t j_index = 0;
  bool found = false;
  for (size_t i = 0; i < buffer.size(); ++i) {
    if (buffer.at(i)->CheckJunction()) {
      j_index = i;
      found = true;
      break;
    }
    if (i + 1 < buffer.size()) {
      accum += buffer.at(i)->Distance(buffer.at(i + 1));
    }
    if (accum > JUNCTION_SCAN_DISTANCE) {
      break;
    }
  }
  if (!found) {
    return;  // no junction ahead within the horizon
  }
  const float dist_to_junction = accum;

  // Require a turn (crossing) maneuver: compare the approach heading with the
  // heading where the path leaves the junction. A straight pass-through does not
  // cross oncoming traffic and is left to the normal collision logic.
  const cg::Vector3D approach_fwd = buffer.front()->GetForwardVector();
  cg::Vector3D exit_fwd = approach_fwd;
  bool was_in_junction = false;
  bool exit_found = false;
  for (size_t i = j_index; i < buffer.size(); ++i) {
    const bool jn = buffer.at(i)->CheckJunction();
    if (jn) {
      was_in_junction = true;
    } else if (was_in_junction) {
      exit_fwd = buffer.at(i)->GetForwardVector();
      exit_found = true;
      break;
    }
  }
  // Reject only a *confirmed* straight-through pass (priority road) -- it does
  // not cross oncoming traffic. When the buffer is too short to see the exit
  // lane (a stopped ego has only a ~15 m horizon), fail open and treat it as a
  // crossing so the gap-acceptance still engages; a genuine no-conflict case
  // just commits immediately anyway.
  const float turn_cos = approach_fwd.x * exit_fwd.x + approach_fwd.y * exit_fwd.y;
  if (exit_found && turn_cos > JUNCTION_TURN_COS) {
    return;  // confirmed straight through, not a crossing turn
  }

  perception.at_junction = true;
  perception.dist_to_junction = dist_to_junction;
  perception.at_hold_line = dist_to_junction <= JUNCTION_HOLD_LINE;

  // Conflict reference point ~ the junction entrance.
  const cg::Location jx = buffer.at(j_index)->GetLocation();

  // Gather candidate conflicting vehicles: those overlapping the ego and those
  // passing the junction-region waypoints and their lane neighbours (to sweep
  // the crossing lanes).
  std::unordered_set<ActorId> candidate_ids;
  {
    const ActorIdSet overlapping = track_traffic.GetOverlappingVehicles(actor_id);
    candidate_ids.insert(overlapping.begin(), overlapping.end());
    const size_t lo = (j_index > 2) ? j_index - 2 : 0;
    const size_t hi = std::min(buffer.size(), j_index + 6);
    for (size_t i = lo; i < hi; ++i) {
      const SimpleWaypointPtr &wp = buffer.at(i);
      const SimpleWaypointPtr neighbours[3] = {wp, wp->GetRightWaypoint(),
                                               wp->GetLeftWaypoint()};
      for (const SimpleWaypointPtr &n : neighbours) {
        if (n != nullptr) {
          const ActorIdSet passing = track_traffic.GetPassingVehicles(n->GetId());
          candidate_ids.insert(passing.begin(), passing.end());
        }
      }
    }
  }

  // Time-to-junction of the nearest conflicting vehicle.
  float min_ttc = std::numeric_limits<float>::max();
  for (const ActorId other_id : candidate_ids) {
    if (other_id == actor_id || !simulation_state.ContainsActor(other_id)) {
      continue;
    }
    const cg::Vector3D vel = simulation_state.GetVelocity(other_id);
    const float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
    if (speed < JUNCTION_MOVING_THRESHOLD) {
      continue;  // stopped/slow -> normal collision negotiation handles it
    }
    const cg::Vector3D heading = simulation_state.GetHeading(other_id);
    // Same-direction traffic (a leader/follower) is not crossing conflict.
    const float head_align =
        approach_fwd.x * heading.x + approach_fwd.y * heading.y;
    if (head_align > JUNCTION_SAME_DIR_COS) {
      continue;
    }
    const cg::Location opos = simulation_state.GetLocation(other_id);
    const float to_jx_x = jx.x - opos.x;
    const float to_jx_y = jx.y - opos.y;
    // Must be approaching the junction, not leaving it.
    if (vel.x * to_jx_x + vel.y * to_jx_y <= 0.0f) {
      continue;
    }
    const float d = std::sqrt(to_jx_x * to_jx_x + to_jx_y * to_jx_y);
    if (d > JUNCTION_CONFLICT_RANGE) {
      continue;
    }
    min_ttc = std::min(min_ttc, d / speed);
  }

  perception.conflict_ttc = min_ttc;
  // No conflict (min_ttc stays +inf) or all conflicts far enough away -> accept.
  perception.junction_gap_clear = (min_ttc >= JUNCTION_COMMIT_TTC);
}

namespace {
// Situation-estimator scan windows (metres / dimensionless). Local to this
// translation unit -- these are estimator tuning, not shared behaviour limits.
constexpr float SIT_PED_LOOKAHEAD = 14.0f;     ///< how far ahead a walker matters
constexpr float SIT_PED_SIDE_MARGIN = 1.5f;    ///< extra half-corridor for walkers
constexpr float SIT_PED_LATERAL_SPEED = 0.3f;  ///< m/s to count as "crossing"
constexpr float SIT_ONC_LOOKAHEAD = 30.0f;     ///< how far ahead an oncoming car matters
constexpr float SIT_ONC_SIDE_MARGIN = 0.5f;    ///< tolerance inside the ego corridor
constexpr float SIT_ONC_OPPOSING_COS = -0.5f;  ///< heading dot < this => opposing
constexpr float SIT_LEAD_SLOWER_FRAC = 0.85f;  ///< lead slower than this * ego speed
constexpr float SIT_MOVING_SPEED = 0.5f;       ///< m/s above which an actor "moves"
constexpr int SIT_CONFIRM_FRAMES = 3;          ///< frames a new raw label must hold
constexpr int SIT_PASS_HOLD = 12;              ///< frames a pass label is force-held
}  // namespace

void LateralAvoidanceStage::ClassifySituation(const ActorId actor_id,
                                              AvoidContext &ctx,
                                              AvoidPerception &perception) const {
  const cg::Location ego_loc = simulation_state.GetLocation(actor_id);
  const cg::Vector3D fwd = simulation_state.GetHeading(actor_id);
  const cg::Vector3D right(-fwd.y, fwd.x, 0.0f);  // right of heading (CollisionStage convention)
  const cg::Vector3D ego_vel = simulation_state.GetVelocity(actor_id);
  const float ego_speed = std::sqrt(ego_vel.x * ego_vel.x + ego_vel.y * ego_vel.y);
  const float half_corridor =
      (perception.lane_half_width > 0.0f ? perception.lane_half_width : 1.75f) +
      perception.ego_half_width;

  // Confidence accumulators for the two actor-scan situations.
  float ped_conf = 0.0f;
  float onc_conf = 0.0f;

  const ActorIdSet overlapping = track_traffic.GetOverlappingVehicles(actor_id);
  for (const ActorId other_id : overlapping) {
    if (other_id == actor_id || !simulation_state.ContainsActor(other_id)) {
      continue;
    }
    const cg::Location oloc = simulation_state.GetLocation(other_id);
    const cg::Vector3D to = oloc - ego_loc;
    const float lon = to.x * fwd.x + to.y * fwd.y;      // ahead > 0
    const float lat = to.x * right.x + to.y * right.y;  // right > 0
    if (lon <= 0.0f) {
      continue;  // beside or behind -> not an obstacle we are driving into
    }
    const ActorType type = simulation_state.GetType(other_id);
    const cg::Vector3D ovel = simulation_state.GetVelocity(other_id);

    if (type == ActorType::Pedestrian) {
      // A walker in or entering the path ahead. Presence in the corridor is
      // enough to flag; lateral motion across the path raises confidence.
      if (lon < SIT_PED_LOOKAHEAD &&
          std::fabs(lat) < half_corridor + SIT_PED_SIDE_MARGIN) {
        perception.pedestrian_in_path = true;
        const float lat_speed = std::fabs(ovel.x * right.x + ovel.y * right.y);
        const float proximity = 1.0f - lon / SIT_PED_LOOKAHEAD;  // closer => higher
        const float crossing = lat_speed >= SIT_PED_LATERAL_SPEED ? 0.3f : 0.0f;
        ped_conf = std::max(ped_conf, std::min(1.0f, 0.5f + 0.2f * proximity + crossing));
      }
      continue;
    }

    if (type == ActorType::Vehicle) {
      // An opposing-heading vehicle sitting in / invading the ego lane ahead.
      const cg::Vector3D ohead = simulation_state.GetHeading(other_id);
      const float head_dot = ohead.x * fwd.x + ohead.y * fwd.y;
      const bool opposing = head_dot < SIT_ONC_OPPOSING_COS;
      if (opposing && lon < SIT_ONC_LOOKAHEAD &&
          std::fabs(lat) < half_corridor + SIT_ONC_SIDE_MARGIN) {
        perception.oncoming_intrusion = true;
        const float centred = 1.0f - std::min(1.0f, std::fabs(lat) / half_corridor);
        const float proximity = 1.0f - lon / SIT_ONC_LOOKAHEAD;
        onc_conf = std::max(onc_conf, std::min(1.0f, 0.4f + 0.3f * centred + 0.3f * proximity));
      }
    }
  }

  // Lead-braking: a moving in-lane lead we are closing on (the collision stage
  // flags the blocker; here we require it moving and slower than the ego). No
  // acceleration history is kept, so this approximates "decelerating lead" with
  // "slower moving lead ahead" -- the action (ease off / follow) is the same.
  if (perception.blocker_ahead && !perception.blocker_stopped &&
      ego_speed > SIT_MOVING_SPEED) {
    perception.lead_braking = true;
  }

  // Priority resolution, safety first: a walker or an opposing car in the lane
  // outranks any pass; junction negotiation outranks a static pass; a pass that
  // needs the oncoming lane outranks an in-lane one; a moving lead is lowest.
  SituationLabel label = SituationLabel::CLEAR;
  float conf = 0.0f;
  if (perception.pedestrian_in_path) {
    label = SituationLabel::PEDESTRIAN_CROSSING;
    conf = ped_conf;
  } else if (perception.oncoming_intrusion) {
    label = SituationLabel::ONCOMING_INTRUSION;
    conf = onc_conf;
  } else if (perception.at_junction) {
    label = SituationLabel::JUNCTION_YIELD;
    conf = perception.junction_gap_clear ? 0.6f : 0.9f;  // more sure when it must yield
  } else if (perception.borrow_oncoming) {
    label = SituationLabel::ONCOMING_BLOCK;
    conf = 0.8f;
  } else if ((perception.blocker_ahead && perception.blocker_stopped) ||
             perception.side_obstacle) {
    label = SituationLabel::STATIC_BLOCKER;
    conf = perception.bypass_feasible ? 0.85f : 0.7f;
  } else if (perception.lead_braking) {
    label = SituationLabel::LEAD_BRAKING;
    conf = 0.6f;
  }
  // Debounce: the raw label is noisy (blocker/borrow flags toggle frame to
  // frame). A new label must persist SIT_CONFIRM_FRAMES before it is adopted --
  // except the two safety-urgent labels, adopted at once. A pass label
  // (STATIC_BLOCKER / ONCOMING_BLOCK), once stable, is force-held for
  // SIT_PASS_HOLD frames so it does not evaporate on a single dropped blocker
  // frame before it has gated action.
  const SituationLabel raw = label;
  const bool urgent = (raw == SituationLabel::PEDESTRIAN_CROSSING ||
                       raw == SituationLabel::ONCOMING_INTRUSION);
  const int confirm_needed = urgent ? 1 : SIT_CONFIRM_FRAMES;
  const bool stable_is_pass =
      (ctx.stable_situation == SituationLabel::STATIC_BLOCKER ||
       ctx.stable_situation == SituationLabel::ONCOMING_BLOCK);
  if (raw == ctx.stable_situation) {
    ctx.sit_candidate = raw;
    ctx.sit_confirm = 0;
    ctx.stable_confidence = conf;
  } else {
    if (raw == ctx.sit_candidate) {
      ++ctx.sit_confirm;
    } else {
      ctx.sit_candidate = raw;
      ctx.sit_confirm = 1;
    }
    const bool hold_protects_pass =
        stable_is_pass && ctx.sit_hold > 0 && !urgent &&
        raw == SituationLabel::CLEAR;
    if (ctx.sit_confirm >= confirm_needed && !hold_protects_pass) {
      ctx.stable_situation = raw;
      ctx.stable_confidence = conf;
      ctx.sit_confirm = 0;
    }
  }
  // Maintain the pass-label hold counter (refresh while the raw label agrees).
  if (ctx.stable_situation == SituationLabel::STATIC_BLOCKER ||
      ctx.stable_situation == SituationLabel::ONCOMING_BLOCK) {
    if (raw == ctx.stable_situation) {
      ctx.sit_hold = SIT_PASS_HOLD;
    } else if (ctx.sit_hold > 0) {
      --ctx.sit_hold;
    }
  } else {
    ctx.sit_hold = 0;
  }

  perception.situation = ctx.stable_situation;
  perception.situation_confidence = ctx.stable_confidence;
}

SituationLabel LateralAvoidanceStage::GetSituation(const ActorId actor_id) const {
  const auto it = avoidance_context.find(actor_id);
  return it == avoidance_context.end() ? SituationLabel::CLEAR
                                       : it->second.stable_situation;
}

void LateralAvoidanceStage::Update(const unsigned long index) {
  const ActorId actor_id = vehicle_id_list.at(index);

  const bool lateral_enabled = parameters.GetLateralAvoidance(actor_id);
  const bool junction_enabled = parameters.GetJunctionGapAcceptance(actor_id);

  // Default-OFF short-circuit. Vehicles with neither feature never touch the
  // context map, keeping this path byte-identical to a build without the stage.
  if (!lateral_enabled && !junction_enabled) {
    output_array.at(index) = AvoidanceCommand{};
    return;
  }

  AvoidContext &ctx = avoidance_context[actor_id];

  AvoidPerception perception;
  if (lateral_enabled) {
    perception = Perceive(actor_id, ctx);
    // Fold in the collision stage's own hazard flag (authoritative for whether
    // motion planning is currently stopping). Perception keeps the stopped/side
    // determination from its window scan.
    const CollisionHazardData &hazard = collision_frame.at(index);
    perception.blocker_ahead = perception.blocker_ahead || hazard.hazard;
  }
  if (junction_enabled) {
    PerceiveJunction(actor_id, perception);
  }

  // Estimate the coarse driving situation from the perception gathered above
  // plus the pedestrian / oncoming / lead scans. The label gates unsafe lateral
  // maneuvers (see the enter-guards) and is exposed for diagnostics + the API.
  const SituationLabel prev_situation = ctx.stable_situation;
  ClassifySituation(actor_id, ctx, perception);
  if (perception.situation != prev_situation &&
      std::getenv("TM_LOG_SIT") != nullptr) {
    const cg::Location el = simulation_state.GetLocation(actor_id);
    std::cerr << "[TMSIT] actor=" << actor_id << " loc=(" << el.x << "," << el.y
              << ") est=" << ToString(perception.situation)
              << " conf=" << perception.situation_confidence
              << " (was " << ToString(prev_situation) << ")" << std::endl;
  }

  // Advance the behaviour graph: first matching transition wins.
  for (const Transition &t : kTransitions) {
    if (t.from == ctx.state && t.guard(ctx, perception)) {
      ctx.state = t.to;
      ctx.state_entry_location = simulation_state.GetLocation(actor_id);
      break;
    }
  }

  // Rate-limit the emitted offset towards the active node's target (junction
  // nodes target zero offset, so any residual lateral shift decays out).
  const float target_offset = NodeTargetOffset(ctx.state, perception);
  const float delta = std::max(-MAX_OFFSET_RATE,
                               std::min(MAX_OFFSET_RATE, target_offset - ctx.committed_offset));
  ctx.committed_offset += delta;

  AvoidanceCommand command;
  command.lateral_offset = ctx.committed_offset;

  if (IsJunctionState(ctx.state)) {
    // Junction gap-acceptance: the hazard release is derived directly from the
    // node each frame (no latch). APPROACH holds at the yield point by leaving
    // the collision stop in force (baseline behaviour); CROSS commits by
    // releasing it. The commit is latched by the state machine (CROSS only
    // exits once the junction is passed), so it does not flip back mid-crossing.
    ctx.hold_clear = false;  // keep the lateral latch clear across junctions
    if (ctx.state == ManeuverState::JUNCTION_CROSS) {
      command.clear_hazard = true;
      command.allow_moving_hazard = true;  // gap already accepted via TTC
      command.speed_factor = 1.0f;
    } else {
      // JUNCTION_APPROACH / JUNCTION_YIELD: defer to the collision stage.
      command.clear_hazard = false;
      command.speed_factor = 1.0f;
    }
  } else {
    // Lateral maneuvers: latch the hazard release across the maneuver. It engages
    // once a stopped obstacle with a feasible plan is verified and stays engaged
    // while a feasible lateral plan exists, so the noisy per-frame stopped flag
    // cannot toggle the collision stop. It disengages on return to lane
    // following.
    if (NodeClearsHazard(ctx.state, perception)) {
      ctx.hold_clear = true;
    }
    if (ctx.state == ManeuverState::FOLLOW) {
      ctx.hold_clear = false;
    }
    command.clear_hazard = ctx.hold_clear;
    if (ctx.state == ManeuverState::LANE_BORROW_ONCOMING &&
        std::getenv("TM_LOG_PATH") != nullptr) {
      const cg::Location el = simulation_state.GetLocation(actor_id);
      std::cerr << "[TMBORROW] actor=" << actor_id << " loc=(" << el.x << "," << el.y
                << ") offset=" << ctx.committed_offset << " target=" << perception.borrow_offset
                << " clear=" << perception.borrow_oncoming
                << " hazard_released=" << command.clear_hazard << std::endl;
    }
    if (command.clear_hazard) {
      command.speed_factor = (ctx.state == ManeuverState::LANE_BORROW_ONCOMING)
                                 ? BORROW_SPEED_FACTOR
                                 : (ctx.state == ManeuverState::IN_LANE_BYPASS)
                                       ? BYPASS_SPEED_FACTOR
                                       : SIDE_CLEARANCE_SPEED_FACTOR;
    } else {
      command.speed_factor = 1.0f;
    }
  }
  output_array.at(index) = command;
}

void LateralAvoidanceStage::RemoveActor(const ActorId actor_id) {
  avoidance_context.erase(actor_id);
}

void LateralAvoidanceStage::Reset() {
  avoidance_context.clear();
}

} // namespace traffic_manager
} // namespace carla
