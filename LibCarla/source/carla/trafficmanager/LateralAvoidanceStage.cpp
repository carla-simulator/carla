// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <algorithm>
#include <cmath>
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
    case ManeuverState::IN_LANE_BYPASS:
      return p.bypass_feasible;
    case ManeuverState::SIDE_CLEARANCE:
      return p.side_obstacle && p.blocker_stopped;
    default:
      return false;
  }
}

// --- Transition guards (pure functions of context + perception) ------------

bool GuardEnterSideClearance(const AvoidContext &, const AvoidPerception &p) {
  return p.side_obstacle && std::fabs(p.target_offset) >= ENTER_OFFSET_THRESHOLD;
}

bool GuardEnterBypass(const AvoidContext &, const AvoidPerception &p) {
  return p.blocker_ahead && p.blocker_stopped && p.bypass_feasible;
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

struct Transition {
  ManeuverState from;
  ManeuverState to;
  bool (*guard)(const AvoidContext &, const AvoidPerception &);
};

// Priority-ordered: the first row whose `from` matches the current state and
// whose guard passes wins. A stopped blocker (bypass) takes priority over
// side-clearance.
const Transition kTransitions[] = {
  {ManeuverState::IN_LANE_BYPASS, ManeuverState::FOLLOW,         &GuardExitBypass},
  {ManeuverState::SIDE_CLEARANCE, ManeuverState::IN_LANE_BYPASS, &GuardEnterBypass},
  {ManeuverState::SIDE_CLEARANCE, ManeuverState::FOLLOW,         &GuardExitToFollow},
  {ManeuverState::FOLLOW,         ManeuverState::IN_LANE_BYPASS, &GuardEnterBypass},
  {ManeuverState::FOLLOW,         ManeuverState::SIDE_CLEARANCE, &GuardEnterSideClearance},
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
        const float lat = (frac * 2.0f - 1.0f) * lane_half;  // [-half, +half]
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

      if (min_lat > 0.0f) {
        const float allowed = min_lat - CLEARANCE_MARGIN - ego_half_width;
        if (allowed < 0.0f) {
          req_left = std::min(req_left, allowed);
          side_stopped = side_stopped || obs.stopped;
        }
      } else if (max_lat < 0.0f) {
        const float allowed = max_lat + CLEARANCE_MARGIN + ego_half_width;
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
      if (has_blocker) {
        // Straddling blocker: try an in-lane bypass on the wider free side.
        perception.blocker_stopped = blocker_stopped;
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
      } else {
        // Side obstacle too far in to clear within the lane -> a lane borrow is
        // needed (added in a later iteration); for now the ego holds.
        perception.blocker_stopped = side_stopped;
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

void LateralAvoidanceStage::Update(const unsigned long index) {
  const ActorId actor_id = vehicle_id_list.at(index);

  // Default-OFF short-circuit. Never-enabled vehicles never touch the context
  // map, keeping this path byte-identical to a build without the stage.
  if (!parameters.GetLateralAvoidance(actor_id)) {
    output_array.at(index) = AvoidanceCommand{};
    return;
  }

  AvoidContext &ctx = avoidance_context[actor_id];

  AvoidPerception perception = Perceive(actor_id, ctx);
  // Fold in the collision stage's own hazard flag (authoritative for whether
  // motion planning is currently stopping). Perception keeps the stopped/side
  // determination from its window scan.
  const CollisionHazardData &hazard = collision_frame.at(index);
  perception.blocker_ahead = perception.blocker_ahead || hazard.hazard;

  // Advance the behaviour graph: first matching transition wins.
  for (const Transition &t : kTransitions) {
    if (t.from == ctx.state && t.guard(ctx, perception)) {
      ctx.state = t.to;
      ctx.state_entry_location = simulation_state.GetLocation(actor_id);
      break;
    }
  }

  // Rate-limit the emitted offset towards the active node's target.
  const float target_offset = NodeTargetOffset(ctx.state, perception);
  const float delta = std::max(-MAX_OFFSET_RATE,
                               std::min(MAX_OFFSET_RATE, target_offset - ctx.committed_offset));
  ctx.committed_offset += delta;

  // Latch the hazard release across the maneuver. It engages once a stopped
  // obstacle with a feasible plan is verified, and stays engaged while a
  // feasible lateral plan exists (side clearance or bypass), so the noisy
  // per-frame stopped flag cannot toggle the collision stop. It disengages when
  // no feasible plan remains or the ego returns to lane following.
  if (NodeClearsHazard(ctx.state, perception)) {
    ctx.hold_clear = true;
  }
  // Hold the release for the whole maneuver: it only disengages when the ego
  // has returned to lane following. The maneuver state itself persists across
  // gaps between obstacles (its offset stays applied until the path is clear),
  // so this bridges those gaps without the collision stop pulsing back on.
  if (ctx.state == ManeuverState::FOLLOW) {
    ctx.hold_clear = false;
  }

  AvoidanceCommand command;
  command.lateral_offset = ctx.committed_offset;
  command.clear_hazard = ctx.hold_clear;
  // While releasing the collision stop, creep at the maneuver's speed factor;
  // otherwise run at full speed. Derived from the latched state so it does not
  // flicker frame-to-frame.
  if (command.clear_hazard) {
    command.speed_factor = (ctx.state == ManeuverState::IN_LANE_BYPASS)
                               ? BYPASS_SPEED_FACTOR
                               : SIDE_CLEARANCE_SPEED_FACTOR;
  } else {
    command.speed_factor = 1.0f;
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
