
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
// Lookahead time for the steering target. ue5-dev halved this to 0.5 s,
// which at 30 km/h puts the target ~4 m ahead: any lateral transient (lane
// change, resume offset) then reads as a huge angular error, saturates the
// PID and produces an underdamped weave. 1.0 s matches upstream and keeps
// the pure-pursuit geometry well damped.
static const float TARGET_WAYPOINT_TIME_HORIZON = 1.0f;
static const float MIN_TARGET_WAYPOINT_DISTANCE = 3.0f;
// Cap on the pursuit-target distance. Chasing a point d ahead cuts a curve
// of radius R by the chord sagitta ~d^2/(8R): uncapped at 60 km/h (16.7 m)
// on Town10's R~25 m corners that is a ~1.4 m apex cut, which lands the
// vehicle in the oncoming lane on left curves. 10 m keeps the cut under
// ~0.6 m on the tightest urban corners.
static const float MAX_TARGET_WAYPOINT_DISTANCE = 10.0f;
// Curvature-aware bound on the same chord cut. The fixed 10 m cap encodes the
// R~25 m / 60 km/h case, but junction connecting roads on Town10 are R~11 m
// taken at ~8 m/s where the surviving d = speed * 1 s chord still cuts
// d^2/(8R) ~ 0.8 m (measured ~1.4 m at the apex with vehicle dynamics): the
// vehicle turns in before the lane does and sweeps the adjacent lane. Capping
// d = sqrt(8 * R_local * MAX_PURSUIT_CHORD_SAGITTA) keeps the geometric cut
// below ~0.2 m at any local path radius; straight paths (R -> inf) are
// unaffected. R_local is measured over the buffer up to the pursuit target.
static const float MAX_PURSUIT_CHORD_SAGITTA = 0.2f;
static const float JUNCTION_LOOK_AHEAD = 5.0f;
static const float SAFE_DISTANCE_AFTER_JUNCTION = 4.0f;
static const float MIN_JUNCTION_LENGTH = 8.0f;
static const float MIN_SAFE_INTERVAL_LENGTH = 0.5f * SAFE_DISTANCE_AFTER_JUNCTION;
static const float LARGE_VEHICLES_JUNCTION_OFFSET = 1.5f;
static const float LARGE_VEHICLES_JUNCTION_POINT = 0.3f;
static const float LARGE_VEHICLES_JUNCTION_MAX_RADIUS = 20.0f;
// Fraction of the inboard (exit "cut-in") excursion retained for large
// vehicles. The cut-in is what drives a long vehicle's rear into the inside
// shoulder; 0.0f removes it entirely, 1.0f keeps the original profile.
static const float LARGE_VEHICLES_JUNCTION_INBOARD_SCALE = 0.25f;
// Vehicle length (m) at or below which the wide-turn offset is zero. Above it
// the offset ramps up linearly at OFFSET_GAIN per metre, capped by
// LARGE_VEHICLES_JUNCTION_OFFSET. Sim-tuned.
static const float LARGE_VEHICLES_JUNCTION_REF_LENGTH = 6.0f;
static const float LARGE_VEHICLES_JUNCTION_OFFSET_GAIN = 0.25f;
// Lateral clearance (m) added beyond the swing band when checking whether the
// offset side is occupied, and longitudinal margin (m) added to the vehicle's
// half-length to define the "alongside" window.
static const float LARGE_VEHICLES_JUNCTION_CLEARANCE = 1.0f;
static const float LARGE_VEHICLES_JUNCTION_SIDE_MARGIN = 2.0f;
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
// Speed-scaled steering envelope: |steer| <= STEER_LIMIT_GAIN / speed^2
// (speed in m/s). Guards against a saturated PID output knifing the vehicle
// across lanes at speed. Inactive below ~6 m/s (envelope > 0.8).
// A/B note (Town10 matrix): raising the gain to 45 to let vehicles steer
// through corners taken at 60 km/h regressed badly (saturated transients
// became hard swerves into obstacles); the correct fix for running wide in
// curves is slowing down before them (GetTurnTargetVelocity), not granting
// more steering authority at speed.
static const float STEER_LIMIT_GAIN = 30.0f;
// Vehicle steering-authority normalization. A normalized steer command maps
// to path curvature as kappa = tan(steer * max_steer_angle) / wheelbase, so
// for the same command a long vehicle yields proportionally less curvature
// than the car-class fleet the lateral gains and the STEER_LIMIT_GAIN
// envelope were tuned on (measured: 8.5 m trucks pinned at the envelope on
// 45% of junction ticks, ran R~13 m on R~11 m connecting roads and invaded
// the adjacent lane on every junction turn while cars tracked cleanly).
// Both the lateral P/D terms and the envelope are therefore scaled by
//   (bbox_length / REF_VEHICLE_LENGTH) * (REF_MAX_STEER_ANGLE / max_steer)
// which equalizes commanded CURVATURE across the fleet; wheelbase tracks
// bounding-box length closely, so the unknown wheelbase fraction cancels
// against the car-anchored reference length. Clamped to
// [1, MAX_STEER_AUTHORITY_CORRECTION] so car-class vehicles and the
// physical lateral-acceleration guard stay exactly as tuned.
static const float REF_VEHICLE_LENGTH = 4.9f;    // m, tuning-fleet car bbox length
static const float REF_MAX_STEER_ANGLE = 70.0f;  // deg, tuning-fleet wheel lock
static const float MAX_STEER_AUTHORITY_CORRECTION = 2.5f;
// Ceiling for the COMBINED lateral P/D scale (pursuit-distance schedule x
// steering-authority correction). Both factors approximate the pure-pursuit
// equivalent gain, but their product on a long vehicle at a short pursuit
// distance overshoots it (measured: 8.5 m trucks settled ~1.0 m INSIDE their
// reference through junction turns at a composed scale of ~4.6). The ceiling
// is the largest scale the schedule alone produces on the tuned car fleet
// (MAX/MIN pursuit distance = 10/3), which validated cleanly; the authority
// correction fills toward that ceiling for long vehicles instead of
// exceeding it. The steering ENVELOPE keeps the full authority correction:
// it is a physical curvature cap, not a loop gain.
static const float MAX_LATERAL_GAIN_SCALE =
    WaypointSelection::MAX_TARGET_WAYPOINT_DISTANCE /
    WaypointSelection::MIN_TARGET_WAYPOINT_DISTANCE;
// Bounding-box length to wheelbase ratio, stable across the CARLA fleet
// (0.55-0.62 on the shipped vehicles); used with the physical wheel lock to
// invert the steering-to-curvature map tan(steer * lock) / wheelbase exactly
// instead of linearizing it. The linearized P law was tuned at car-scale
// commands (|steer| ~ 0.1, where tan is linear); long vehicles need
// |steer| ~ 0.4-0.5 through junctions, where the uncompensated tan() makes
// the linear law over-curve and settle ~1 m inside its reference.
static const float WHEELBASE_FRACTION = 0.6f;
// Margin over the exact pure-pursuit curvature (kappa = 2 sin(alpha) / d).
// Chosen so the anchor car at the cruise pursuit distance reproduces the
// validated linear gain P = 2: P_pp = 2 * L_wb * pi / (d * lock_rad) = 1.49
// at d = 10 m, and 1.32 * 1.49 * dev matches 2 * dev in the small-command
// regime the cruise tuning was measured in.
static const float PURSUIT_CURVATURE_MARGIN = 1.32f;
// Nominal controller period: the sync-mode fixed_delta_seconds the gains were
// tuned at. In asynchronous mode the real period is one server frame, which
// under render load (e.g. a path-traced rt_lens sensor halving the frame
// rate) can be 2-4x this. RunStep therefore takes the measured period and
// compensates; DT is only the design point and the fallback when no valid
// measurement exists (first tick after registration or a state reseed).
static const float DT = 0.05f;
static const float INV_DT = 1.0f / DT;
// Valid range for the measured controller period. Below MIN the derivative
// division gets noisy; above MAX the sim is hitching so badly that reacting
// to the full elapsed time would command huge one-tick corrections.
static const float MIN_CONTROL_DT = 0.01f;
static const float MAX_CONTROL_DT = 0.2f;
// Steering slew budget, per second of simulation time (0.15 per 0.05 s tick
// at the design rate). Applying it per second instead of per tick keeps the
// physical steering rate constant when the tick rate changes.
static const float MAX_STEERING_RATE = MAX_STEERING_DIFF / DT;
// Max change in normalised angular deviation per tick accepted by the
// lateral derivative term: 0.05 * 180 deg / 0.05 s = 180 deg/s, the upper
// bound of real vehicle yaw. Larger jumps are target discontinuities.
static const float MAX_DEVIATION_DELTA = 0.05f;
// The same bound expressed as a rate, so it scales with the measured tick
// period: 1.0 normalised units/s = 180 deg/s.
static const float MAX_DEVIATION_RATE = MAX_DEVIATION_DELTA / DT;
static const std::vector<float> LONGITUDIAL_PARAM = {12.0f, 0.05f, 0.02f};
static const std::vector<float> LONGITUDIAL_HIGHWAY_PARAM = {20.0f, 0.05f, 0.01f};
// Lateral gains, step-response tuned against the LINEAR Chaos steering
// mapping (the previous values were tuned while the engine still applied a
// squared input curve, which hid a ~2-5x effective gain increase at small
// commands). Key constraint: at cruise speeds the P term must stay inside
// the STEER_LIMIT_GAIN/v^2 envelope for typical errors (~1.5 m offset ->
// deviation ~0.05); P=2 keeps the loop linear at 60 km/h where P>=3
// saturates the envelope and hunts around the lane center without settling
// (measured: settle 1.1 s vs >6 s). D=0.24 damps the recovery to <=0.25 m
// overshoot at 30-60 km/h.
static const std::vector<float> LATERAL_PARAM = {2.0f, 0.03f, 0.24f};
static const std::vector<float> LATERAL_HIGHWAY_PARAM = {2.0f, 0.02f, 0.24f};
} // namespace PID

namespace StuckRecovery {
// A vehicle commanded to move that stays below STUCK_SPEED for STUCK_TIME
// while no hazard justifies the stop is considered wedged (curb, wall,
// fence); a vehicle whose target sits more than ALIGN_ENTER_DEVIATION
// off-heading at low speed is facing the wrong way (spun, wrong-way
// resume). Both run the same K-turn maneuver: alternate reverse and
// forward phases with full steering toward the target until the heading
// deviation drops below ALIGN_EXIT_DEVIATION, then resume the PID.
static const float STUCK_SPEED = 0.3f;         // m/s
static const double STUCK_TIME = 3.0;          // s immobile before recovery
static const double PHASE_DURATION = 1.5;      // s per K-turn phase
static const float REVERSE_THROTTLE = 0.5f;
static const float FORWARD_THROTTLE = 0.3f;
static const float RECOVERY_STEER = 0.8f;
static const float ALIGN_ENTER_DEVIATION = 0.6f;  // normalised angle (~108 deg)
static const float ALIGN_EXIT_DEVIATION = 0.1f;   // normalised angle (~18 deg)
static const float MISALIGN_MAX_SPEED = 3.5f;     // m/s
} // namespace StuckRecovery

namespace TrackTraffic {
static const uint64_t BUFFER_STEP_THROUGH = 5;
static const float INV_BUFFER_STEP_THROUGH = 1.0f / static_cast<float>(BUFFER_STEP_THROUGH);
} // namespace TrackTraffic

} // namespace constants
} // namespace traffic_manager
} // namespace carla
