
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
static const float HYBRID_MODE_DT_FL = 0.1f;
static const double HYBRID_MODE_DT = 0.1;
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
static const float TARGET_WAYPOINT_TIME_HORIZON = 0.3f;
static const float MIN_TARGET_WAYPOINT_DISTANCE = 3.0f;
static const float JUNCTION_LOOK_AHEAD = 5.0f;
static const float SAFE_DISTANCE_AFTER_JUNCTION = 4.0f;
static const float MIN_JUNCTION_LENGTH = 8.0f;
static const float MIN_SAFE_INTERVAL_LENGTH = 0.5f * SAFE_DISTANCE_AFTER_JUNCTION;
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
} // namespace Map

namespace TrafficLight {
static const uint64_t NO_SIGNAL_PASSTHROUGH_INTERVAL = 5u;
static const double DOUBLE_NO_SIGNAL_PASSTHROUGH_INTERVAL = 5.0;
} // namespace TrafficLight

namespace MotionPlan {
static const float RELATIVE_APPROACH_SPEED = 12.0f / 3.6f;
static const float MIN_FOLLOW_LEAD_DISTANCE = 2.0f;
static const float CRITICAL_BRAKING_MARGIN = 0.2f;
static const float EPSILON_RELATIVE_SPEED = 0.001f;
static const float MAX_JUNCTION_BLOCK_DISTANCE = 1.0f * WaypointSelection::SAFE_DISTANCE_AFTER_JUNCTION;
static const float TWO_KM = 2000.0f;
static const uint16_t ATTEMPTS_TO_TELEPORT = 5u;
static const float LANDMARK_DETECTION_TIME = 2.5f;
static const float TL_GREEN_TARGET_VELOCITY = 20.0f / 3.6f;
static const float TL_RED_TARGET_VELOCITY = 15.0f / 3.6f;
static const float TL_UNKNOWN_TARGET_VELOCITY = TL_RED_TARGET_VELOCITY;
static const float STOP_TARGET_VELOCITY = 15.0f / 3.6f;
static const float YIELD_TARGET_VELOCITY = 15.0f / 3.6f;
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
static const float DT = 0.05f;
static const float INV_DT = 1.0f / DT;
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
