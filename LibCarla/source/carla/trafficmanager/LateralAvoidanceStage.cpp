// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <algorithm>
#include <cmath>
#include <limits>

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/LateralAvoidanceStage.h"

namespace carla {
namespace traffic_manager {

using namespace constants::LateralAvoidance;
using constants::VehicleRemoval::STOPPED_VELOCITY_THRESHOLD;

namespace {

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

// The target-velocity multiplier a node applies while active.
float NodeSpeedFactor(const ManeuverState state) {
  switch (state) {
    case ManeuverState::IN_LANE_BYPASS:
      return BYPASS_SPEED_FACTOR;
    case ManeuverState::SIDE_CLEARANCE:
    case ManeuverState::FOLLOW:
    default:
      return 1.0f;
  }
}

// Whether a node releases the collision emergency-stop for the blocker it is
// steering around. Only while the lateral clearance is currently verified.
bool NodeClearsHazard(const ManeuverState state, const AvoidPerception &p) {
  return state == ManeuverState::IN_LANE_BYPASS && p.bypass_feasible;
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

AvoidPerception LateralAvoidanceStage::Perceive(const ActorId actor_id) const {
  AvoidPerception perception;

  const auto buffer_it = buffer_map.find(actor_id);
  if (buffer_it == buffer_map.end() || buffer_it->second.empty()) {
    return perception;
  }
  const Buffer &buffer = buffer_it->second;
  const SimpleWaypointPtr &reference = buffer.front();

  // Lane frame at the reference point.
  const cg::Location ref_location = reference->GetLocation();
  const cg::Vector3D forward = reference->GetForwardVector();
  const cg::Vector3D right = reference->GetTransform().GetRightVector();

  const float lane_half_width =
      static_cast<float>(reference->GetWaypoint()->GetLaneWidth()) * 0.5f;
  const float ego_half_width = simulation_state.GetDimensions(actor_id).y;
  const float drivable_half = std::max(0.0f, lane_half_width - ego_half_width);
  perception.lane_half_width = lane_half_width;
  perception.ego_half_width = ego_half_width;

  // The most demanding shift required by obstacles on each side.
  // req_left <= 0 (shift left to clear a right-side obstacle),
  // req_right >= 0 (shift right to clear a left-side obstacle).
  float req_left = 0.0f;
  float req_right = 0.0f;

  // Nearest stopped obstacle straddling the centerline (an in-lane blocker
  // that might be passed within the lane).
  bool has_blocker = false;
  float blocker_min_lat = 0.0f;
  float blocker_max_lat = 0.0f;
  float blocker_longitudinal = LOOKAHEAD_DISTANCE + 1.0f;

  const ActorIdSet overlapping = track_traffic.GetOverlappingVehicles(actor_id);
  for (const ActorId other_id : overlapping) {
    if (other_id == actor_id || !simulation_state.ContainsActor(other_id)) {
      continue;
    }

    const cg::Location other_location = simulation_state.GetLocation(other_id);
    const cg::Vector3D to_other = other_location - ref_location;

    // Longitudinal position along the ego lane; only look ahead.
    const float longitudinal = to_other.x * forward.x + to_other.y * forward.y;
    if (longitudinal < -2.0f || longitudinal > LOOKAHEAD_DISTANCE) {
      continue;
    }

    // Signed lateral span of the obstacle's oriented bounding box.
    const cg::Vector3D other_heading = simulation_state.GetHeading(other_id);
    const cg::Vector3D other_dims = simulation_state.GetDimensions(other_id);
    const cg::Vector3D other_perp(-other_heading.y, other_heading.x, 0.0f);
    float min_lat = std::numeric_limits<float>::max();
    float max_lat = std::numeric_limits<float>::lowest();
    for (int sx = -1; sx <= 1; sx += 2) {
      for (int sy = -1; sy <= 1; sy += 2) {
        const cg::Vector3D corner_offset =
            other_heading * (other_dims.x * static_cast<float>(sx)) +
            other_perp * (other_dims.y * static_cast<float>(sy));
        const cg::Vector3D corner_rel = to_other + corner_offset;
        const float lat = corner_rel.x * right.x + corner_rel.y * right.y;
        min_lat = std::min(min_lat, lat);
        max_lat = std::max(max_lat, lat);
      }
    }

    // Ignore obstacles that are not near the lane at all.
    if (min_lat > LATERAL_CONSIDERATION || max_lat < -LATERAL_CONSIDERATION) {
      continue;
    }

    if (min_lat > 0.0f) {
      // Obstacle entirely to the right of the centerline: clear it by keeping
      // the ego's right side margin-clear of the obstacle's inner (left) edge.
      const float allowed = min_lat - CLEARANCE_MARGIN - ego_half_width;
      if (allowed < 0.0f) {
        req_left = std::min(req_left, allowed);
      }
    } else if (max_lat < 0.0f) {
      // Obstacle entirely to the left of the centerline.
      const float allowed = max_lat + CLEARANCE_MARGIN + ego_half_width;
      if (allowed > 0.0f) {
        req_right = std::max(req_right, allowed);
      }
    } else {
      // Obstacle straddles the centerline: an in-lane blocker. If it is stopped
      // and the nearest one ahead, record it as a bypass candidate.
      const bool other_stopped =
          simulation_state.GetVelocity(other_id).SquaredLength() <
          SQUARE(STOPPED_VELOCITY_THRESHOLD);
      if (other_stopped && longitudinal > 0.0f && longitudinal < blocker_longitudinal) {
        has_blocker = true;
        blocker_longitudinal = longitudinal;
        blocker_min_lat = min_lat;
        blocker_max_lat = max_lat;
      }
    }
  }

  float target = 0.0f;
  const bool need_left = req_left < 0.0f;
  const bool need_right = req_right > 0.0f;
  if (need_left && !need_right) {
    target = std::max(req_left, -drivable_half);
  } else if (need_right && !need_left) {
    target = std::min(req_right, drivable_half);
  }
  // If both sides intrude, the corridor cannot be cleared laterally; stay
  // centered and let car-following / bypass handle it (target stays 0).

  perception.target_offset = target;
  perception.side_obstacle = std::fabs(target) > 0.0f;

  // In-lane bypass of a stopped blocker: pick the side with the wider free gap
  // that still fits the ego plus margin, and hug that lane boundary.
  if (has_blocker) {
    const float right_gap = lane_half_width - blocker_max_lat;
    const float left_gap = lane_half_width + blocker_min_lat;
    const float needed = 2.0f * ego_half_width + CLEARANCE_MARGIN;
    const bool right_ok = right_gap >= needed;
    const bool left_ok = left_gap >= needed;
    if (right_ok && (right_gap >= left_gap || !left_ok)) {
      perception.bypass_feasible = true;
      perception.bypass_offset = drivable_half;
    } else if (left_ok) {
      perception.bypass_feasible = true;
      perception.bypass_offset = -drivable_half;
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

  AvoidPerception perception = Perceive(actor_id);
  // Blocker-ahead signals from the collision stage (reserved for the bypass
  // nodes; unused by SIDE_CLEARANCE).
  const CollisionHazardData &hazard = collision_frame.at(index);
  perception.blocker_ahead = hazard.hazard;
  if (hazard.hazard && simulation_state.ContainsActor(hazard.hazard_actor_id)) {
    perception.blocker_stopped =
        simulation_state.GetVelocity(hazard.hazard_actor_id).SquaredLength() <
        SQUARE(STOPPED_VELOCITY_THRESHOLD);
  }

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

  AvoidanceCommand command;
  command.lateral_offset = ctx.committed_offset;
  command.speed_factor = NodeSpeedFactor(ctx.state);
  command.clear_hazard = NodeClearsHazard(ctx.state, perception);
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
