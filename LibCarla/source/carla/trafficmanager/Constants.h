
/// This file contains various constants used in traffic manager
/// arranged into sensible namespaces for re-usability across
/// different files.

#pragma once

#include <limits>
#include <stdint.h>
#include <iostream>
#include <vector>

#define SQUARE(a) ((a) * (a))
#define RATE(MaxY, MinY, DiffX) (((MaxY) - (MinY)) / (DiffX))

namespace carla {
namespace traffic_manager {
namespace constants {

namespace Networking {
static const uint64_t MIN_TRY_COUNT = 20u;
static const unsigned short TM_DEFAULT_PORT = 8000u;
static const int64_t TM_TIMEOUT = 2000; // ms
} // namespace Networking

namespace VehicleRemoval {
static const float STOPPED_VELOCITY_THRESHOLD = 0.8f;
static const double BLOCKED_TIME_THRESHOLD = 90.0;
static const double RED_TL_BLOCKED_TIME_THRESHOLD = 180.0;
static const double DELTA_TIME_BETWEEN_DESTRUCTIONS = 10.0;
} // namespace VehicleRemoval

namespace HybridMode {
static const float HYBRID_MODE_DT_FL = 0.05f;
static const double HYBRID_MODE_DT = 0.05;
static const double INV_HYBRID_DT = 1.0 / HYBRID_MODE_DT;
static const float PHYSICS_RADIUS = 50.0f;
} // namespace HybridMode

namespace LateralAvoidance {
// Longitudinal window (m) ahead of the ego along its path in which side
// obstacles are considered for in-lane clearance.
static const float LOOKAHEAD_DISTANCE = 20.0f;
// Lateral gap (m) to keep between the ego's side and an obstacle's edge.
static const float CLEARANCE_MARGIN = 0.5f;
// Extra lateral intrusion (m) attributed to a *stopped* obstacle when deciding
// whether it blocks the corridor. A parked/stopped vehicle often has an open
// door, mirror or protruding load that reaches past its bounding box into the
// lane; treating its near edge as this much closer makes a car hugging the lane
// boundary register as a blocker every frame (stable) instead of flickering at
// the exact threshold. Moving traffic gets no such bonus.
static const float STOPPED_INTRUSION_MARGIN = 0.4f;
// Maximum change in applied lateral offset per cycle (m), rate-limiting the
// steering target to avoid chatter.
static const float MAX_OFFSET_RATE = 0.10f;
// Offset magnitude (m) needed to enter SIDE_CLEARANCE, and the smaller value
// to fall back to FOLLOW (asymmetric thresholds give hysteresis).
static const float ENTER_OFFSET_THRESHOLD = 0.15f;
static const float EXIT_OFFSET_THRESHOLD = 0.05f;
// Only obstacles whose bounding box comes within this lateral distance (m) of
// the lane centerline are treated as intruding the driving corridor.
static const float LATERAL_CONSIDERATION = 4.0f;
// Longitudinal half-window (m) around a path station within which an obstacle
// is considered "beside" that station when scanning the future waypoints.
static const float STATION_TOLERANCE = 3.0f;
// Target-velocity multiplier while squeezing past a stopped blocker in-lane
// (a tight maneuver -> a firmer slowdown).
static const float BYPASS_SPEED_FACTOR = 0.6f;
// Target-velocity multiplier while edging past a stopped side obstacle. The
// ego keeps its clearance within the lane, so only a mild slowdown is needed --
// a hard crawl looks unnatural.
static const float SIDE_CLEARANCE_SPEED_FACTOR = 0.9f;
// Target-velocity multiplier while borrowing the oncoming lane to pass a full
// blockage (crossing into opposing traffic -> a firm, cautious crawl).
static const float BORROW_SPEED_FACTOR = 0.4f;
// Horizon (m) ahead over which the oncoming lane must be free of opposing
// traffic before the ego commits to borrowing it.
static const float BORROW_ONCOMING_CLEAR_DISTANCE = 40.0f;
// Horizon (m) ahead over which a same-direction borrow lane must be free before
// committing (shorter than the oncoming horizon -- no closing-speed doubling).
static const float BORROW_SAME_DIR_CLEAR_DISTANCE = 20.0f;
// Extra width (m) beyond each lane edge the static-prop raycast fan reaches, so
// a barrier sitting in an adjacent borrow lane is seen before the ego commits to
// borrowing into it (otherwise the borrow is blind to its own path).
static const float BORROW_PROBE_WIDTH = 3.5f;
// Longitudinal window (m) ahead over which a candidate borrow lane must be free
// of static obstacles for the borrow path itself to count as clear.
static const float BORROW_PATH_CLEAR_DISTANCE = 18.0f;
// Hard cap (m/s) on the creep velocity used while steering around a stopped
// obstacle with the collision stop released.
static const float LATERAL_CREEP_MAX_SPEED = 8.0f;
// Opt-in static-obstacle raycast: number of lateral sample rays cast across the
// lane plus the adjacent borrow corridors (BORROW_PROBE_WIDTH beyond each edge).
// Each is a forward ray at a fixed lateral offset; an odd count keeps one ray on
// the centerline. Kept modest -- rays are server RPCs, and the cast is throttled.
static const int RAYCAST_LATERAL_SAMPLES = 15;
// Height (m) above the sampled path point at which rays are cast, so they clear
// the road surface and strike upright props (cones, barriers) rather than the
// ground plane.
static const float RAYCAST_HEIGHT = 0.4f;
// Longitudinal reach (m) of the raycast fan ahead of the ego.
static const float RAYCAST_RANGE = LOOKAHEAD_DISTANCE;
// Half-extent (m) of the synthetic point-obstacle a ray hit is turned into, so
// a thin prop reads as a small footprint the station scan can clear around.
static const float RAYCAST_OBSTACLE_HALF = 0.25f;
// Ticks between raycast refreshes. Static props do not move, so hits are cached
// and only re-cast this often, cutting the server RPC load ~Nx (at ~20 Hz, 10
// ticks ~= 0.5 s -- well within the look-ahead horizon at road speeds).
static const int RAYCAST_REFRESH_TICKS = 10;

// --- Junction gap-acceptance (unprotected-turn creep-and-commit) -----------
// Distance (m) ahead along the path within which an upcoming junction is
// considered for the creep-and-commit maneuver.
static const float JUNCTION_SCAN_DISTANCE = 20.0f;
// Heading change (dot of approach vs exit forward, below this cosine) that marks
// the junction maneuver as a turn crossing conflicting traffic (~25 deg).
static const float JUNCTION_TURN_COS = 0.9f;
// Distance (m) to the junction entrance at or inside which the ego holds and
// waits rather than continuing to creep.
static const float JUNCTION_HOLD_LINE = 6.0f;
// Distance (m) to the junction within which -- once stopped/slowed at its yield
// point -- the ego may accept a gap and commit. Covers the collision stage's
// natural stop point so the commit can actually fire there.
static const float JUNCTION_COMMIT_DISTANCE = 12.0f;
// Radius (m) around the junction within which an approaching vehicle is treated
// as potential conflicting traffic.
static const float JUNCTION_CONFLICT_RANGE = 45.0f;
// A conflicting vehicle must be moving faster than this (m/s) to matter; slower
// ones are treated as stopped and negotiated by the normal collision logic.
static const float JUNCTION_MOVING_THRESHOLD = 0.8f;
// Minimum time-to-junction (s) of the nearest conflicting vehicle for the gap
// to be accepted and the crossing committed. Sized with margin for the ego to
// accelerate from a stop and clear the conflict zone.
static const float JUNCTION_COMMIT_TTC = 5.0f;
// Target-velocity multiplier while creeping up to the hold line (a slow roll
// that signals intent without entering the conflict zone).
static const float JUNCTION_CREEP_SPEED_FACTOR = 0.35f;
// Same-direction rejection: a candidate whose heading aligns with the ego's
// approach above this cosine is a leader/follower, not crossing traffic.
static const float JUNCTION_SAME_DIR_COS = 0.3f;
} // namespace LateralAvoidance

namespace SpeedThreshold {
static const float HIGHWAY_SPEED = 60.0f / 3.6f;
static const float AFTER_JUNCTION_MIN_SPEED = 5.0f / 3.6f;
static const float INITIAL_PERCENTAGE_SPEED_DIFFERENCE = 0.0f;
} // namespace SpeedThreshold

namespace PathBufferUpdate {
static const float MAX_START_DISTANCE = 20.0f;
static const float MINIMUM_HORIZON_LENGTH = 15.0f;
static const float HORIZON_RATE = 2.0f;
static const float HIGH_SPEED_HORIZON_RATE = 4.0f;
} // namespace PathBufferUpdate

namespace WaypointSelection {
static const float TARGET_WAYPOINT_TIME_HORIZON = 0.3f;
static const float MIN_TARGET_WAYPOINT_DISTANCE = 3.0f;
static const float JUNCTION_LOOK_AHEAD = 5.0f;
static const float SAFE_DISTANCE_AFTER_JUNCTION = 4.0f;
static const float MIN_JUNCTION_LENGTH = 8.0f;
static const float MIN_SAFE_INTERVAL_LENGTH = 0.5f * SAFE_DISTANCE_AFTER_JUNCTION;
static const float LARGE_VEHICLES_JUNCTION_OFFSET = 1.5f;
static const float LARGE_VEHICLES_JUNCTION_POINT = 0.3f;
static const float LARGE_VEHICLES_JUNCTION_MAX_RADIUS = 20.0f;
} // namespace WaypointSelection

namespace LaneChange {
static const float MINIMUM_LANE_CHANGE_DISTANCE = 20.0f;
static const float MAXIMUM_LANE_OBSTACLE_DISTANCE = 50.0f;
static const float MAXIMUM_LANE_OBSTACLE_CURVATURE = 0.6f;
static const float INTER_LANE_CHANGE_DISTANCE = 10.0f;
static const float MIN_WPT_DISTANCE = 5.0f;
static const float MAX_WPT_DISTANCE = 20.0f;
static const float MIN_LANE_CHANGE_SPEED = 5.0f;
static const float FIFTYPERC = 50.0f;
} // namespace LaneChange

namespace Collision {
static const float BOUNDARY_EXTENSION_MINIMUM = 2.5f;
static const float BOUNDARY_EXTENSION_RATE = 4.35f;
static const float COS_10_DEGREES = 0.9848f;
static const float OVERLAP_THRESHOLD = 0.1f;
static const float LOCKING_DISTANCE_PADDING = 4.0f;
static const float COLLISION_RADIUS_STOP = 8.0f;
static const float COLLISION_RADIUS_MIN = 20.0f;
static const float COLLISION_RADIUS_RATE = 2.65f;
static const float MAX_LOCKING_EXTENSION = 10.0f;
static const float WALKER_TIME_EXTENSION = 1.5f;
static const float SQUARE_ROOT_OF_TWO = 1.414f;
static const float VERTICAL_OVERLAP_THRESHOLD = 4.0f;
static const float EPSILON = 2.0f * std::numeric_limits<float>::epsilon();
static const float MIN_REFERENCE_DISTANCE = 0.5f;
static const float MIN_VELOCITY_COLL_RADIUS = 2.0f;
static const float VEL_EXT_FACTOR = 0.36f;
} // namespace Collision

namespace FrameMemory {
static const uint64_t INITIAL_SIZE = 50u;
static const uint64_t GROWTH_STEP_SIZE = 50u;
static const float INV_GROWTH_STEP_SIZE = 1.0f / static_cast<float>(GROWTH_STEP_SIZE);
} // namespace FrameMemory

namespace Map {
static const float INFINITE_DISTANCE = std::numeric_limits<float>::max();
static const float MAX_GEODESIC_GRID_LENGTH = 20.0f;
static const float MAP_RESOLUTION = 5.0f;
static const float INV_MAP_RESOLUTION = 1.0f / MAP_RESOLUTION;
static const double MAX_WPT_DISTANCE = MAP_RESOLUTION/2.0 + SQUARE(MAP_RESOLUTION);
static const float MAX_WPT_RADIANS = 0.087f;  // 5º
static float const DELTA = 25.0f;
static float const Z_DELTA = 500.0f;
static float const STRAIGHT_DEG = 19.0f;
static const double MIN_LANE_WIDTH = 1.0f;
} // namespace Map

namespace TrafficLight {
static const double MINIMUM_STOP_TIME = 2.0;
static const double EXIT_JUNCTION_THRESHOLD = 0;  // Dot product of 90º
} // namespace TrafficLight

namespace MotionPlan {
static const float RELATIVE_APPROACH_SPEED = 12.0f / 3.6f;
static const float MIN_FOLLOW_LEAD_DISTANCE = 2.0f;
static const float CRITICAL_BRAKING_MARGIN = 0.2f;
static const float EPSILON_RELATIVE_SPEED = 0.001f;
static const float MAX_JUNCTION_BLOCK_DISTANCE = 1.0f * WaypointSelection::SAFE_DISTANCE_AFTER_JUNCTION;
static const float TWO_KM = 2000.0f;
static const uint16_t ATTEMPTS_TO_TELEPORT = 5u;
static const float LANDMARK_DETECTION_TIME = 3.5f;
static const float TL_TARGET_VELOCITY = 15.0f / 3.6f;
static const float STOP_TARGET_VELOCITY = 10.0f / 3.6f;
static const float YIELD_TARGET_VELOCITY = 10.0f / 3.6f;
static const float FRICTION = 0.6f;
static const float GRAVITY = 9.81f;
static const float PI = 3.1415927f;
static const float PERC_MAX_SLOWDOWN = 0.08f;
static const float FOLLOW_LEAD_FACTOR = 2.0f;
} // namespace MotionPlan

namespace VehicleLight {
static const float SUN_ALTITUDE_DEGREES_BEFORE_DAWN = 15.0f;
static const float SUN_ALTITUDE_DEGREES_AFTER_SUNSET = 165.0f;
static const float SUN_ALTITUDE_DEGREES_JUST_AFTER_DAWN = 35.0f;
static const float SUN_ALTITUDE_DEGREES_JUST_BEFORE_SUNSET = 145.0f;
static const float HEAVY_PRECIPITATION_THRESHOLD = 80.0f;
static const float FOG_DENSITY_THRESHOLD = 20.0f;
static const float MAX_DISTANCE_LIGHT_CHECK = 225.0f;
} // namespace VehicleLight

namespace PID {
static const float MAX_THROTTLE = 0.85f;
static const float MAX_BRAKE = 0.7f;
static const float MAX_STEERING = 0.8f;
static const float MAX_STEERING_DIFF = 0.15f;
static const std::vector<float> LONGITUDIAL_PARAM = {12.0f, 0.05f, 0.02f};
static const std::vector<float> LONGITUDIAL_HIGHWAY_PARAM = {20.0f, 0.05f, 0.01f};
static const std::vector<float> LATERAL_PARAM = {4.0f, 0.02f, 0.08f};
static const std::vector<float> LATERAL_HIGHWAY_PARAM = {2.0f, 0.02f, 0.04f};
} // namespace PID

namespace TrackTraffic {
static const uint64_t BUFFER_STEP_THROUGH = 5;
static const float INV_BUFFER_STEP_THROUGH = 1.0f / static_cast<float>(BUFFER_STEP_THROUGH);
} // namespace TrackTraffic

} // namespace constants
} // namespace traffic_manager
} // namespace carla
