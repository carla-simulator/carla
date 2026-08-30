
/// This file contains definitions of common data structures used in traffic manager.

#pragma once

#include <chrono>
#include <cstdint>
#include <deque>
#include <vector>

#include "carla/client/Actor.h"
#include "carla/geom/Location.h"
#include "carla/geom/Rotation.h"
#include "carla/geom/Vector3D.h"
#include "carla/rpc/ActorId.h"
#include "carla/rpc/Command.h"
#include "carla/rpc/TrafficLightState.h"

#include "carla/trafficmanager/SimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

namespace chr = std::chrono;
namespace cc = carla::client;
namespace cg = carla::geom;

using ActorId = carla::ActorId;
using ActorPtr = carla::SharedPtr<cc::Actor>;
using JunctionID = carla::road::JuncId;
using Junction = carla::SharedPtr<carla::client::Junction>;
using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
using Buffer = std::deque<SimpleWaypointPtr>;
using BufferMap = std::unordered_map<carla::ActorId, Buffer>;
using TimeInstance = chr::time_point<chr::system_clock, chr::nanoseconds>;
using TLS = carla::rpc::TrafficLightState;

struct LocalizationData {
  SimpleWaypointPtr junction_end_point;
  SimpleWaypointPtr safe_point;
  bool is_at_junction_entrance;
};
using LocalizationFrame = std::vector<LocalizationData>;

struct CollisionHazardData {
  float available_distance_margin;
  ActorId hazard_actor_id;
  bool hazard;
};
using CollisionFrame = std::vector<CollisionHazardData>;

/// Nodes of the lateral-avoidance behaviour graph. Each is a semantically
/// distinct maneuver; transitions between them are data-driven.
enum class ManeuverState : uint8_t {
  FOLLOW = 0,             ///< Lane-keeping; no lateral perturbation.
  SIDE_CLEARANCE,         ///< In-lane offset away from a static side obstacle.
  IN_LANE_BYPASS,         ///< Squeeze past a partly-blocking stopped vehicle.
  LANE_BORROW_SAME,       ///< Borrow a same-direction neighbour lane, then return.
  LANE_BORROW_ONCOMING,   ///< Borrow the oncoming lane (gated), then return.
  WAIT_BLOCKED,           ///< No lateral option; hold (emergency stop).
  JUNCTION_APPROACH,      ///< Approaching a crossing junction; creep to the hold line.
  JUNCTION_YIELD,         ///< At the hold line; oncoming gap too small; hold.
  JUNCTION_CROSS,         ///< Gap accepted (TTC clear); commit through the junction.
};

/// Side selector for a lateral maneuver.
enum class ManeuverSide : uint8_t { NONE = 0, LEFT, RIGHT };

/// Coarse driving-situation estimate for a vehicle, inferred each cycle purely
/// from world state the traffic manager already holds (actor kinematics, lane
/// graph, junction geometry, traffic-light state) -- never from sensors or
/// scenario/route metadata. The estimate is deliberately coarse: many distinct
/// leaderboard scenarios collapse to the same *action*, so the label answers
/// "how should I behave here", not "which scripted scenario is this". It is an
/// estimate (with a confidence), expected to be imperfect.
enum class SituationLabel : uint8_t {
  CLEAR = 0,             ///< Nothing actionable ahead in the lane.
  STATIC_BLOCKER,        ///< Stopped obstacle intruding the lane -> pass it
                         ///< (covers parked / accident / construction / open
                         ///< door; the estimator does not distinguish which).
  ONCOMING_BLOCK,        ///< Lane fully blocked, pass requires borrowing the
                         ///< oncoming lane (verified clear over the horizon).
  PEDESTRIAN_CROSSING,   ///< A walker is in / entering the path -> hold, never
                         ///< steer around.
  JUNCTION_YIELD,        ///< Junction ahead with conflicting traffic -> creep
                         ///< and yield until the gap is acceptable.
  ONCOMING_INTRUSION,    ///< An opposing vehicle occupies / invades the ego
                         ///< lane -> hold / edge away, never borrow oncoming.
  LEAD_BRAKING,          ///< A moving lead in-lane is decelerating -> follow.
};

/// Human-readable name of a situation label (diagnostics only).
inline const char *ToString(const SituationLabel s) {
  switch (s) {
    case SituationLabel::CLEAR:               return "CLEAR";
    case SituationLabel::STATIC_BLOCKER:      return "STATIC_BLOCKER";
    case SituationLabel::ONCOMING_BLOCK:      return "ONCOMING_BLOCK";
    case SituationLabel::PEDESTRIAN_CROSSING: return "PEDESTRIAN_CROSSING";
    case SituationLabel::JUNCTION_YIELD:      return "JUNCTION_YIELD";
    case SituationLabel::ONCOMING_INTRUSION:  return "ONCOMING_INTRUSION";
    case SituationLabel::LEAD_BRAKING:        return "LEAD_BRAKING";
  }
  return "?";
}

/// Per-vehicle output of the lateral-avoidance stage. Consumed by the
/// motion-planning stage (lateral_offset + speed_factor) and, for lane
/// borrows, by the localization stage (borrow_request + return_waypoint).
struct AvoidanceCommand {
  float lateral_offset = 0.0f;   ///< Metres, signed; + shifts right of centerline.
  float speed_factor = 1.0f;     ///< Multiplies target velocity; 1.0 = unchanged.
  /// When true, motion planning releases the collision emergency-stop for the
  /// stopped blocker being bypassed (the ego has verified lateral clearance and
  /// is steering around it). The collision stage reasons about the lane
  /// centerline and cannot see the lateral offset, so it would otherwise brake
  /// the whole way past.
  bool clear_hazard = false;
  /// When true, clear_hazard may release the stop even for a *moving* hazard.
  /// Set only by the junction-crossing commit (which has already accepted the
  /// conflicting-traffic gap via TTC). Lateral maneuvers leave this false, so
  /// clear_hazard then only releases the stop for a stopped obstacle -- a moving
  /// crosser/turner keeps the collision stop authoritative.
  bool allow_moving_hazard = false;
  bool borrow_request = false;   ///< Request a lane borrow from localization.
  ManeuverSide borrow_side = ManeuverSide::NONE;
  SimpleWaypointPtr return_waypoint = nullptr; ///< Where to rejoin the origin lane.
};
using AvoidanceFrame = std::vector<AvoidanceCommand>;

using ControlFrame = std::vector<carla::rpc::Command>;

using TLFrame = std::vector<bool>;

/// Structure to hold the actuation signals.
struct ActuationSignal {
  float throttle;
  float brake;
  float steer;
};

/// Structure to hold the controller state.
struct StateEntry {
  cc::Timestamp time_instance;
  float angular_deviation;
  float velocity_deviation;
  ActuationSignal actuation_signal;
};

} // namespace traffic_manager
} // namespace carla
