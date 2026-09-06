// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//
// Server-free unit tests for the C++ port of agents.navigation.* (issue
// carla-simulator/carla#9554). The tests exercise the parts that don't need
// a live CARLA server: the Types.h enums and behavior factories, the pure
// helpers in Misc.h, and the Default*Options() factories on the planner /
// agent classes. Behaviour that requires a Vehicle / Map / TrafficLight is
// covered by integration tests run against a real server.

#include "test.h"

#include <carla/agents/navigation/Types.h>
#include <carla/agents/navigation/Misc.h>
#include <carla/agents/navigation/PIDController.h>
#include <carla/agents/navigation/LocalPlanner.h>
#include <carla/agents/navigation/GlobalRoutePlanner.h>
#include <carla/agents/navigation/BasicAgent.h>
#include <carla/agents/navigation/BehaviorAgent.h>
#include <carla/agents/navigation/ConstantVelocityAgent.h>

#include <carla/geom/Location.h>
#include <carla/geom/Rotation.h>
#include <carla/geom/Transform.h>

#include <cmath>
#include <type_traits>

using namespace carla::agents::navigation;
using carla::geom::Location;
using carla::geom::Rotation;
using carla::geom::Transform;

// =====================================================================
// Types.h — enum sanity + behavior factory parity with Python.
// =====================================================================

TEST(agents_types, road_option_values_match_python) {
  EXPECT_EQ(static_cast<int>(RoadOption::Void),            -1);
  EXPECT_EQ(static_cast<int>(RoadOption::Left),             1);
  EXPECT_EQ(static_cast<int>(RoadOption::Right),            2);
  EXPECT_EQ(static_cast<int>(RoadOption::Straight),         3);
  EXPECT_EQ(static_cast<int>(RoadOption::LaneFollow),       4);
  EXPECT_EQ(static_cast<int>(RoadOption::ChangeLaneLeft),   5);
  EXPECT_EQ(static_cast<int>(RoadOption::ChangeLaneRight),  6);
}

TEST(agents_types, behavior_cautious_matches_python) {
  const auto p = BehaviorParameters::Cautious();
  EXPECT_FLOAT_EQ(p.max_speed,               40.0f);
  EXPECT_FLOAT_EQ(p.speed_lim_dist,           6.0f);
  EXPECT_FLOAT_EQ(p.speed_decrease,          12.0f);
  EXPECT_FLOAT_EQ(p.safety_time,              3.0f);
  EXPECT_FLOAT_EQ(p.min_proximity_threshold, 12.0f);
  EXPECT_FLOAT_EQ(p.braking_distance,         6.0f);
  EXPECT_EQ     (p.tailgate_counter,           0);
}

TEST(agents_types, behavior_normal_matches_python) {
  const auto p = BehaviorParameters::Normal();
  EXPECT_FLOAT_EQ(p.max_speed,               50.0f);
  EXPECT_FLOAT_EQ(p.speed_lim_dist,           3.0f);
  EXPECT_FLOAT_EQ(p.speed_decrease,          10.0f);
  EXPECT_FLOAT_EQ(p.safety_time,              3.0f);
  EXPECT_FLOAT_EQ(p.min_proximity_threshold, 10.0f);
  EXPECT_FLOAT_EQ(p.braking_distance,         5.0f);
  EXPECT_EQ     (p.tailgate_counter,           0);
}

TEST(agents_types, behavior_aggressive_matches_python) {
  const auto p = BehaviorParameters::Aggressive();
  EXPECT_FLOAT_EQ(p.max_speed,               70.0f);
  EXPECT_FLOAT_EQ(p.speed_lim_dist,           1.0f);
  EXPECT_FLOAT_EQ(p.speed_decrease,           8.0f);
  EXPECT_FLOAT_EQ(p.safety_time,              3.0f);
  EXPECT_FLOAT_EQ(p.min_proximity_threshold,  8.0f);
  EXPECT_FLOAT_EQ(p.braking_distance,         4.0f);
  // Aggressive disables tailgating by setting the counter to -1, so the
  // `> 0` test in BehaviorAgent::Tailgating always fails.
  EXPECT_EQ     (p.tailgate_counter,          -1);
}

// =====================================================================
// Misc.h — pure helpers.
// =====================================================================

TEST(agents_misc, positive_clamps_at_zero) {
  EXPECT_FLOAT_EQ(Positive(-3.5f), 0.0f);
  EXPECT_FLOAT_EQ(Positive(0.0f),  0.0f);
  EXPECT_FLOAT_EQ(Positive(2.5f),  2.5f);
}

TEST(agents_misc, compute_distance_basic) {
  Location a(0.0f, 0.0f, 0.0f);
  Location b(3.0f, 4.0f, 0.0f);
  EXPECT_NEAR(ComputeDistance(a, b), 5.0f, 1e-3f);

  Location c(1.0f, 2.0f, 2.0f);
  Location d(1.0f, 2.0f, 2.0f);
  // Identical points: epsilon-regularised result is small but nonzero.
  EXPECT_NEAR(ComputeDistance(c, d), 0.0f, 1e-3f);
}

TEST(agents_misc, unit_vector_is_normalized) {
  Location a(0.0f, 0.0f, 0.0f);
  Location b(3.0f, 4.0f, 0.0f);
  const auto u = UnitVector(a, b);
  // 3-4-5 triangle: expect (0.6, 0.8, 0.0).
  EXPECT_NEAR(u[0], 0.6f, 1e-3f);
  EXPECT_NEAR(u[1], 0.8f, 1e-3f);
  EXPECT_NEAR(u[2], 0.0f, 1e-3f);
  // Magnitude is unit.
  const float mag = std::sqrt(u[0]*u[0] + u[1]*u[1] + u[2]*u[2]);
  EXPECT_NEAR(mag, 1.0f, 1e-3f);
}

TEST(agents_misc, is_within_distance_no_angle_filter) {
  Transform ref(Location(0, 0, 0), Rotation(0, 0, 0));
  Transform near_t(Location(5, 0, 0), Rotation(0, 0, 0));
  Transform far_t(Location(50, 0, 0), Rotation(0, 0, 0));

  EXPECT_TRUE (IsWithinDistance(near_t, ref, 10.0f));
  EXPECT_FALSE(IsWithinDistance(far_t,  ref, 10.0f));
}

TEST(agents_misc, is_within_distance_too_close_returns_true) {
  // Per the Python helper, when target is essentially co-located with the
  // reference (norm < 0.001), the function returns true regardless of
  // distance/angle (avoids divide-by-zero on the angle calculation).
  Transform ref(Location(10, 10, 0), Rotation(0, 90, 0));
  Transform overlap(Location(10, 10, 0), Rotation(0, 0, 0));
  EXPECT_TRUE(IsWithinDistance(overlap, ref, 0.001f, std::make_pair(0.0f, 90.0f)));
}

TEST(agents_misc, is_within_distance_with_forward_arc_filter) {
  // Reference looks down +x. Python uses a strict open interval
  // (min < angle < max), so a target exactly on the forward axis (angle=0°)
  // is excluded from the (0°, 90°) cone — matched here by placing the
  // target diagonally forward-right at 45°.
  Transform ref(Location(0, 0, 0), Rotation(0, 0, 0));
  Transform front_diag(Location(5, 5, 0), Rotation(0, 0, 0));   // 45° from +x
  Transform behind(Location(-5, 0, 0), Rotation(0, 0, 0));      // 180° (behind)

  EXPECT_TRUE (IsWithinDistance(front_diag, ref, 10.0f, std::make_pair(0.0f, 90.0f)));
  EXPECT_FALSE(IsWithinDistance(behind,     ref, 10.0f, std::make_pair(0.0f, 90.0f)));

  // Strict-open boundary: target exactly on the forward axis (angle=0°) is
  // NOT inside the (0°, 90°) interval. Matches Python parity.
  Transform front_axial(Location(5, 0, 0), Rotation(0, 0, 0));
  EXPECT_FALSE(IsWithinDistance(front_axial, ref, 10.0f, std::make_pair(0.0f, 90.0f)));
}

TEST(agents_misc, compute_magnitude_angle_zero_yaw) {
  // Looking down +x, target also down +x → angle ~ 0°.
  Location origin(0, 0, 0);
  Location forward(10, 0, 0);
  const auto [mag, angle] = ComputeMagnitudeAngle(forward, origin, /*orientation_deg=*/0.0f);
  EXPECT_NEAR(mag,   10.0f, 1e-3f);
  EXPECT_NEAR(angle,  0.0f, 1e-2f);
}

TEST(agents_misc, compute_magnitude_angle_perpendicular) {
  // Looking down +x, target up +y → angle = 90°.
  Location origin(0, 0, 0);
  Location side(0, 5, 0);
  const auto [mag, angle] = ComputeMagnitudeAngle(side, origin, /*orientation_deg=*/0.0f);
  EXPECT_NEAR(mag,   5.0f, 1e-3f);
  EXPECT_NEAR(angle, 90.0f, 1e-2f);
}

// =====================================================================
// Default*Options factories — confirm they value-initialize and the
// downstream defaults flow through (Python parity for the opt_dict).
// =====================================================================

TEST(agents_factories, local_planner_default_options) {
  const auto opt = LocalPlanner::DefaultOptions();
  // Python defaults: dt = 1/20, target_speed = 20 km/h, sampling = 2 m.
  EXPECT_FLOAT_EQ(opt.dt,                1.0f / 20.0f);
  EXPECT_FLOAT_EQ(opt.target_speed,      20.0f);
  EXPECT_FLOAT_EQ(opt.sampling_radius,    2.0f);
  EXPECT_FLOAT_EQ(opt.max_throttle,       0.75f);
  EXPECT_FLOAT_EQ(opt.max_brake,          0.30f);
  EXPECT_FLOAT_EQ(opt.max_steering,       0.80f);
  EXPECT_FLOAT_EQ(opt.offset,             0.0f);
  EXPECT_FLOAT_EQ(opt.base_min_distance,  3.0f);
  EXPECT_FLOAT_EQ(opt.distance_ratio,     0.5f);
  EXPECT_FALSE   (opt.follow_speed_limits);

  // The PID arg defaults come from VehiclePIDController::*Args definitions.
  EXPECT_FLOAT_EQ(opt.lateral.K_P,       1.95f);
  EXPECT_FLOAT_EQ(opt.lateral.K_I,       0.05f);
  EXPECT_FLOAT_EQ(opt.lateral.K_D,       0.20f);
  EXPECT_FLOAT_EQ(opt.longitudinal.K_P,  1.50f);
  EXPECT_FLOAT_EQ(opt.longitudinal.K_I,  0.05f);
  EXPECT_FLOAT_EQ(opt.longitudinal.K_D,  0.20f);
}

TEST(agents_factories, basic_agent_default_options_inherits_local_planner) {
  const auto opt = BasicAgent::DefaultOptions();
  // BasicAgent-specific defaults.
  EXPECT_FALSE(opt.ignore_traffic_lights);
  EXPECT_FALSE(opt.ignore_stop_signs);
  EXPECT_FALSE(opt.ignore_vehicles);
  EXPECT_FALSE(opt.use_bbs_detection);
  EXPECT_FLOAT_EQ(opt.sampling_resolution,    2.0f);
  EXPECT_FLOAT_EQ(opt.base_tlight_threshold,  5.0f);
  EXPECT_FLOAT_EQ(opt.base_vehicle_threshold, 5.0f);
  EXPECT_FLOAT_EQ(opt.detection_speed_ratio,  1.0f);
  EXPECT_FLOAT_EQ(opt.max_brake,              0.6f);
  // Inherited from LocalPlanner::Options.
  EXPECT_FLOAT_EQ(opt.target_speed,          20.0f);
  EXPECT_FLOAT_EQ(opt.dt,                     1.0f / 20.0f);
}

TEST(agents_factories, constant_velocity_default_options) {
  const auto opt = ConstantVelocityAgent::DefaultConstantOptions();
  EXPECT_FALSE(opt.use_basic_behavior);
  EXPECT_TRUE (std::isinf(opt.restart_time));
  // Inherited from BasicAgent::Options.
  EXPECT_FALSE(opt.ignore_traffic_lights);
  EXPECT_FLOAT_EQ(opt.max_brake, 0.6f);
}

// =====================================================================
// Compile-time API surface checks. These don't run any logic — they just
// pin the expected types so a future header refactor that drifts from the
// public contract fails to compile.
// =====================================================================

TEST(agents_api, surface_pins) {
  using PlanItem = LocalPlanner::PlanItem;
  static_assert(std::is_same_v<PlanItem,
                std::pair<carla::SharedPtr<carla::client::Waypoint>, RoadOption>>,
                "LocalPlanner::PlanItem must be (Waypoint, RoadOption)");

  using RouteItem = GlobalRoutePlanner::RouteItem;
  static_assert(std::is_same_v<RouteItem,
                std::pair<carla::SharedPtr<carla::client::Waypoint>, RoadOption>>,
                "GlobalRoutePlanner::RouteItem must be (Waypoint, RoadOption)");

  using Behavior = BehaviorAgent::Behavior;
  static_assert(static_cast<int>(Behavior::Cautious)   != static_cast<int>(Behavior::Normal),
                "BehaviorAgent::Behavior values must be distinct");
  static_assert(static_cast<int>(Behavior::Normal)     != static_cast<int>(Behavior::Aggressive),
                "BehaviorAgent::Behavior values must be distinct");

  // BasicAgent::Options derives from LocalPlanner::Options.
  static_assert(std::is_base_of_v<LocalPlanner::Options, BasicAgent::Options>,
                "BasicAgent::Options must inherit LocalPlanner::Options");
  // ConstantOptions derives from BasicAgent::Options.
  static_assert(std::is_base_of_v<BasicAgent::Options,
                ConstantVelocityAgent::ConstantOptions>,
                "ConstantOptions must inherit BasicAgent::Options");

  // BehaviorAgent / ConstantVelocityAgent inherit BasicAgent.
  static_assert(std::is_base_of_v<BasicAgent, BehaviorAgent>,
                "BehaviorAgent must inherit BasicAgent");
  static_assert(std::is_base_of_v<BasicAgent, ConstantVelocityAgent>,
                "ConstantVelocityAgent must inherit BasicAgent");

  // BasicAgent has a virtual destructor (so subclasses are safely deletable
  // through a base pointer).
  static_assert(std::has_virtual_destructor_v<BasicAgent>,
                "BasicAgent must have a virtual destructor");

  SUCCEED();
}
