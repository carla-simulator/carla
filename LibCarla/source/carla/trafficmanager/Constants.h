
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
const int64_t TM_WAIT_FOR_TICK_TIMEOUT = 1000;
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
static const float HIGHWAY_SPEED = 50.0f / 3.6f;
static const float ARBITRARY_MAX_SPEED = 100.0f / 3.6f;
static const float AFTER_JUNCTION_MIN_SPEED = 5.0f / 3.6f;
static const float INITIAL_PERCENTAGE_SPEED_DIFFERENCE = 30.0f;
} // namespace SpeedThreshold

namespace PathBufferUpdate {
static const float MAX_START_DISTANCE = 30.0f;
static const float MINIMUM_HORIZON_LENGTH = 30.0f;
static const float MAXIMUM_HORIZON_LENGTH = 60.0f;
static const float HORIZON_RATE = RATE(MAXIMUM_HORIZON_LENGTH,
                                       MINIMUM_HORIZON_LENGTH,
                                       SpeedThreshold::ARBITRARY_MAX_SPEED);
} // namespace PathBufferUpdate

namespace WaypointSelection {
static const float TARGET_WAYPOINT_TIME_HORIZON = 0.3f;
static const float TARGET_WAYPOINT_HORIZON_LENGTH = 0.8f;
static const float JUNCTION_LOOK_AHEAD = 6.0f;
static const float SAFE_DISTANCE_AFTER_JUNCTION = 6.0f;
static const float MIN_JUNCTION_LENGTH = 8.0f;
static const float MIN_SAFE_INTERVAL_LENGTH = 1.5f * SAFE_DISTANCE_AFTER_JUNCTION;
} // namespace WaypointSelection

namespace LaneChange {
static const float MINIMUM_LANE_CHANGE_DISTANCE = 15.0f;
static const float MAXIMUM_LANE_OBSTACLE_DISTANCE = 50.0f;
static const float MAXIMUM_LANE_OBSTACLE_CURVATURE = 0.6f;
static const float INTER_LANE_CHANGE_DISTANCE = 10.0f;
} // namespace LaneChange

namespace Collision {
static const float BOUNDARY_EXTENSION_MAXIMUM = 50.0f;
static const float BOUNDARY_EXTENSION_MINIMUM = 2.0f;
static const float BOUNDARY_EXTENSION_RATE = RATE(BOUNDARY_EXTENSION_MAXIMUM,
                                                  BOUNDARY_EXTENSION_MINIMUM,
                                                  SpeedThreshold::ARBITRARY_MAX_SPEED);
static const float COS_10_DEGREES = 0.9848f;
static const float EPSILON_VELOCITY = 0.1f;
static const float LOCKING_DISTANCE_PADDING = 4.0f;
static const float MAX_COLLISION_RADIUS = 100.0f;
static const float MAX_LOCKING_EXTENSION = 10.0f;
static const float WALKER_TIME_EXTENSION = 1.5f;
static const float SQUARE_ROOT_OF_TWO = 1.414f;
static const float VERTICAL_OVERLAP_THRESHOLD = 4.0f;
static const float EPSILON = 2.0f * std::numeric_limits<float>::epsilon();
} // namespace Collision

namespace FrameMemory {
static const uint64_t INITIAL_SIZE = 50u;
static const uint64_t GROWTH_STEP_SIZE = 50u;
static const float INV_GROWTH_STEP_SIZE = 1.0f / static_cast<float>(GROWTH_STEP_SIZE);
} // namespace FrameMemory

namespace Map {
static const float INFINITE_DISTANCE = std::numeric_limits<float>::max();
static const float GRID_SIZE = 4.0f;
static const float PED_GRID_SIZE = 10.0f;
static const float MAX_GEODESIC_GRID_LENGTH = 20.0f;
static const double MAP_RESOLUTION = 0.1;
static const float INV_MAP_RESOLUTION = 10.0f;
} // namespace Map

namespace TrafficLight {
static const uint64_t NO_SIGNAL_PASSTHROUGH_INTERVAL = 5u;
static const double DOUBLE_NO_SIGNAL_PASSTHROUGH_INTERVAL = 5.0;
} // namespace TrafficLight

namespace MotionPlan {
static const float RELATIVE_APPROACH_SPEED = 10.0f / 3.6f;
static const float MIN_FOLLOW_LEAD_DISTANCE = 5.0f;
static const float MAX_FOLLOW_LEAD_DISTANCE = 10.0f;
static const float FOLLOW_DISTANCE_RATE = RATE(MAX_FOLLOW_LEAD_DISTANCE,
                                               MIN_FOLLOW_LEAD_DISTANCE,
                                               SpeedThreshold::ARBITRARY_MAX_SPEED);
static const float CRITICAL_BRAKING_MARGIN = 0.25f;
static const float EPSILON_RELATIVE_SPEED = 0.001f;
static const float MAX_JUNCTION_BLOCK_DISTANCE = 0.5f * WaypointSelection::SAFE_DISTANCE_AFTER_JUNCTION;
} // namespace MotionPlan

namespace PID {
static const float MAX_THROTTLE = 0.7f;
static const float MAX_BRAKE = 1.0f;
static const float STEERING_LIMIT = 0.8f;
static const float VELOCITY_INTEGRAL_MAX = 5.0f;
static const float VELOCITY_INTEGRAL_MIN = -5.0f;
static const float DT = 0.05f;
static const float INV_DT = 1.0f / DT;
static const std::vector<float> LONGITUDIAL_PARAM = {2.0f, 0.01f, 0.4f};
static const std::vector<float> LONGITUDIAL_HIGHWAY_PARAM = {4.0f, 0.02f, 0.2f};
static const std::vector<float> LATERAL_PARAM = {9.0f, 0.02f, 1.0f};
static const std::vector<float> LATERAL_HIGHWAY_PARAM = {7.0f, 0.02f, 1.0f};
} // namespace PID

namespace TrackTraffic {
static const uint64_t BUFFER_STEP_THROUGH = 10;
static const float INV_BUFFER_STEP_THROUGH = 0.1f;
} // namespace TrackTraffic

} // namespace constants
} // namespace traffic_manager
} // namespace carla
