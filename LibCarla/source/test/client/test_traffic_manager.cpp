// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

#include <carla/geom/Location.h>
#include <carla/trafficmanager/Constants.h>
#include <carla/trafficmanager/DataStructures.h>
#include <carla/trafficmanager/PIDController.h>
#include <carla/trafficmanager/TrafficManagerGeometry.h>

namespace cg = carla::geom;
using carla::traffic_manager::ActuationSignal;
using carla::traffic_manager::StateEntry;
using carla::traffic_manager::GetPathSpeedLimit;
using carla::traffic_manager::GetThreePointCircleRadius;
using carla::traffic_manager::InterpolateBufferAt;
using carla::traffic_manager::IsOffsetSideOccupied;
using carla::traffic_manager::LargeVehicleJunctionOffsetProfile;
using carla::traffic_manager::LargeVehicleOffsetMagnitude;

namespace {

constexpr float kRadiusTolerance{1e-2f};
constexpr float kLocationTolerance{1e-3f};

void ExpectLocationNear(
    const cg::Location &actual,
    const cg::Location &expected,
    float tolerance) {
  EXPECT_NEAR(actual.x, expected.x, tolerance);
  EXPECT_NEAR(actual.y, expected.y, tolerance);
  EXPECT_NEAR(actual.z, expected.z, tolerance);
}

}  // namespace

// -----------------------------------------------------------------------------
// GetThreePointCircleRadius
// -----------------------------------------------------------------------------

TEST(TrafficManagerGeometry, ThreePointCircleRadius_EquilateralTriangle) {
  // Equilateral triangle with side s has circumradius s / sqrt(3).
  constexpr float side{6.0f};
  const float expected_radius{side / std::sqrt(3.0f)};

  const cg::Location p1{0.0f, 0.0f, 0.0f};
  const cg::Location p2{side, 0.0f, 0.0f};
  const cg::Location p3{side / 2.0f, side * std::sqrt(3.0f) / 2.0f, 0.0f};

  const float radius{GetThreePointCircleRadius(p1, p2, p3)};
  EXPECT_NEAR(radius, expected_radius, kRadiusTolerance);
}

TEST(TrafficManagerGeometry, ThreePointCircleRadius_RightTriangle) {
  // A right triangle's circumradius equals half the hypotenuse.
  // Triangle with legs 3 and 4, hypotenuse 5, expected radius 2.5.
  const cg::Location p1{0.0f, 0.0f, 0.0f};
  const cg::Location p2{3.0f, 0.0f, 0.0f};
  const cg::Location p3{0.0f, 4.0f, 0.0f};

  const float radius{GetThreePointCircleRadius(p1, p2, p3)};
  EXPECT_NEAR(radius, 2.5f, kRadiusTolerance);
}

TEST(TrafficManagerGeometry, ThreePointCircleRadius_NearlyCollinearReturnsLarge) {
  // Three nearly-collinear points (tiny y offset on the middle point) should
  // produce a radius above the junction gating threshold so callers fall
  // through the "no offset" branch, while staying finite (distinct from the
  // perfectly-collinear sentinel asserted in the next test).
  const cg::Location p1{0.0f, 0.0f, 0.0f};
  const cg::Location p2{1.0f, 1e-3f, 0.0f};
  const cg::Location p3{2.0f, 0.0f, 0.0f};

  const float radius{GetThreePointCircleRadius(p1, p2, p3)};
  EXPECT_GT(radius, carla::traffic_manager::constants::WaypointSelection::LARGE_VEHICLES_JUNCTION_MAX_RADIUS);
  EXPECT_LT(radius, std::numeric_limits<float>::max());
}

TEST(TrafficManagerGeometry, ThreePointCircleRadius_PerfectlyCollinearReturnsMax) {
  // Perfectly collinear points have no circumscribing circle. The
  // implementation returns std::numeric_limits<float>::max() as a sentinel
  // to flag the degenerate case.
  const cg::Location p1{0.0f, 0.0f, 0.0f};
  const cg::Location p2{5.0f, 0.0f, 0.0f};
  const cg::Location p3{10.0f, 0.0f, 0.0f};

  const float radius{GetThreePointCircleRadius(p1, p2, p3)};
  EXPECT_EQ(radius, std::numeric_limits<float>::max());
}

// -----------------------------------------------------------------------------
// InterpolateBufferAt (the SimpleWaypoint-free seam used by GetTargetData)
// -----------------------------------------------------------------------------

TEST(TrafficManagerInterpolation, InterpolateBufferAt_EmptyBufferReturnsVehicleLocation) {
  const std::vector<cg::Location> empty;
  const cg::Location vehicle_location{1.0f, 2.0f, 3.0f};

  const auto [location, index] = InterpolateBufferAt(empty, 5.0f, vehicle_location);
  ExpectLocationNear(location, vehicle_location, kLocationTolerance);
  EXPECT_EQ(index, 0u);
}

TEST(TrafficManagerInterpolation, InterpolateBufferAt_SingleElementReturnsThatElement) {
  const std::vector<cg::Location> single{{7.0f, 0.0f, 0.0f}};
  const cg::Location vehicle_location{0.0f, 0.0f, 0.0f};

  const auto [location, index] = InterpolateBufferAt(single, 5.0f, vehicle_location);
  ExpectLocationNear(location, single.front(), kLocationTolerance);
  EXPECT_EQ(index, 0u);
}

TEST(TrafficManagerInterpolation, InterpolateBufferAt_LinearInterpBetweenTwoPoints) {
  // Two waypoints at x=0 and x=10. Vehicle at origin. Asking for distance
  // 5 should land at the midpoint (5, 0, 0) with closest_index 0.
  const std::vector<cg::Location> buffer{
      {0.0f, 0.0f, 0.0f},
      {10.0f, 0.0f, 0.0f}};
  const cg::Location vehicle_location{0.0f, 0.0f, 0.0f};

  const auto [location, index] = InterpolateBufferAt(buffer, 5.0f, vehicle_location);
  ExpectLocationNear(location, cg::Location{5.0f, 0.0f, 0.0f}, kLocationTolerance);
  EXPECT_EQ(index, 0u);
}

TEST(TrafficManagerInterpolation, InterpolateBufferAt_DistanceShorterThanFirstWaypoint) {
  // Vehicle at origin; first waypoint at x=10. Asking for distance 3 falls
  // before the first waypoint, so the helper brackets with the first segment
  // (indices 0 and 1) and interpolates a point ahead of the vehicle along
  // the planned path.
  const std::vector<cg::Location> buffer{
      {10.0f, 0.0f, 0.0f},
      {20.0f, 0.0f, 0.0f}};
  const cg::Location vehicle_location{0.0f, 0.0f, 0.0f};

  const auto [location, index] = InterpolateBufferAt(buffer, 3.0f, vehicle_location);
  // closest=0, farthest=0 → fix-up to (0, 1):
  //   t = (3 - 10) / (20 - 10) = -0.7
  //   x = 10 + (20 - 10) * -0.7 = 3
  ExpectLocationNear(location, cg::Location{3.0f, 0.0f, 0.0f}, kLocationTolerance);
  EXPECT_EQ(index, 0u);
}

TEST(TrafficManagerInterpolation, InterpolateBufferAt_DistanceBeyondLastWaypoint) {
  // Every waypoint is closer than target_distance, so the helper clamps to
  // the last waypoint instead of extrapolating off the front segment.
  const std::vector<cg::Location> buffer{
      {1.0f, 0.0f, 0.0f},
      {2.0f, 0.0f, 0.0f},
      {3.0f, 0.0f, 0.0f}};
  const cg::Location vehicle_location{0.0f, 0.0f, 0.0f};

  const auto [location, index] = InterpolateBufferAt(buffer, 100.0f, vehicle_location);
  ExpectLocationNear(location, buffer.back(), kLocationTolerance);
  EXPECT_EQ(index, buffer.size() - 1u);
}

TEST(TrafficManagerInterpolation, InterpolateBufferAt_BracketBetweenLastTwoWaypoints) {
  // Target falls between the second-to-last and last waypoints. The
  // corrected loop must walk the full buffer to find the bracketing pair
  // (the previous implementation stopped at size - 1 and missed it).
  const std::vector<cg::Location> buffer{
      {1.0f, 0.0f, 0.0f},
      {5.0f, 0.0f, 0.0f},
      {100.0f, 0.0f, 0.0f}};
  const cg::Location vehicle_location{0.0f, 0.0f, 0.0f};

  const auto [location, index] = InterpolateBufferAt(buffer, 10.0f, vehicle_location);
  // closest=1 (dist 5), farthest=2 (dist 100):
  //   t = (10 - 5) / (100 - 5) ≈ 0.0526
  //   x ≈ 5 + (100 - 5) * 0.0526 = 10
  ExpectLocationNear(location, cg::Location{10.0f, 0.0f, 0.0f}, kLocationTolerance);
  EXPECT_EQ(index, 1u);
}

TEST(TrafficManagerInterpolation, InterpolateBufferAt_CoincidentBracketReturnsClosest) {
  // Two consecutive waypoints at the same location. The span is zero and
  // the implementation must return the close location to avoid divide-by-zero.
  const std::vector<cg::Location> buffer{
      {5.0f, 0.0f, 0.0f},
      {5.0f, 0.0f, 0.0f},
      {15.0f, 0.0f, 0.0f}};
  const cg::Location vehicle_location{0.0f, 0.0f, 0.0f};

  const auto [location, index] = InterpolateBufferAt(buffer, 5.0f, vehicle_location);
  ExpectLocationNear(location, cg::Location{5.0f, 0.0f, 0.0f}, kLocationTolerance);
  EXPECT_EQ(index, 0u);
}

// -----------------------------------------------------------------------------
// GetPathSpeedLimit
// -----------------------------------------------------------------------------

namespace {

constexpr float kSampleSpacing{5.0f};
constexpr float kLateralAcceleration{3.0f};
constexpr float kBrakingDeceleration{2.0f};
constexpr float kFreeSpeed{30.0f};

/// A straight run along +x followed by a left arc, sampled every `spacing` m.
std::vector<cg::Location> StraightThenArc(
    float straight_length,
    float radius,
    float arc_degrees,
    float spacing) {

  std::vector<cg::Location> path;
  for (float s = 0.0f; s < straight_length; s += spacing) {
    path.push_back(cg::Location{s, 0.0f, 0.0f});
  }
  const cg::Location centre{straight_length, radius, 0.0f};
  const float arc_radians = arc_degrees * 3.14159265f / 180.0f;
  for (float travelled = 0.0f; travelled <= radius * arc_radians; travelled += spacing) {
    const float angle = travelled / radius;
    path.push_back(cg::Location{centre.x + radius * std::sin(angle),
                                centre.y - radius * std::cos(angle),
                                0.0f});
  }
  return path;
}

}  // namespace

TEST(TrafficManagerPathSpeed, PathSpeedLimit_StraightPathIsNotLimited) {
  const std::vector<cg::Location> path{
      {0.0f, 0.0f, 0.0f}, {5.0f, 0.0f, 0.0f}, {10.0f, 0.0f, 0.0f},
      {15.0f, 0.0f, 0.0f}, {20.0f, 0.0f, 0.0f}};

  EXPECT_FLOAT_EQ(
      GetPathSpeedLimit(path, 0.0f, kSampleSpacing, kLateralAcceleration,
                        kBrakingDeceleration, kFreeSpeed),
      kFreeSpeed);
}

TEST(TrafficManagerPathSpeed, PathSpeedLimit_ShortPathIsNotLimited) {
  const std::vector<cg::Location> path{{0.0f, 0.0f, 0.0f}, {5.0f, 0.0f, 0.0f}};

  EXPECT_FLOAT_EQ(
      GetPathSpeedLimit(path, 0.0f, kSampleSpacing, kLateralAcceleration,
                        kBrakingDeceleration, kFreeSpeed),
      kFreeSpeed);
}

TEST(TrafficManagerPathSpeed, PathSpeedLimit_ArcUnderfootAllowsItsOwnSpeed) {
  const float radius = 11.0f;
  const std::vector<cg::Location> path = StraightThenArc(0.0f, radius, 120.0f, kSampleSpacing);

  EXPECT_NEAR(
      GetPathSpeedLimit(path, 0.0f, kSampleSpacing, kLateralAcceleration,
                        kBrakingDeceleration, kFreeSpeed),
      std::sqrt(radius * kLateralAcceleration),
      0.1f);
}

TEST(TrafficManagerPathSpeed, PathSpeedLimit_HoldsSteadyAsAnArcIsConsumed) {
  const float radius = 11.0f;
  const std::vector<cg::Location> path = StraightThenArc(0.0f, radius, 180.0f, kSampleSpacing);

  std::vector<float> limits;
  for (size_t consumed = 0u; consumed < 4u; ++consumed) {
    for (float offset = 0.0f; offset < kSampleSpacing; offset += 1.0f) {
      const std::vector<cg::Location> remaining(path.begin() + static_cast<long>(consumed),
                                                path.end());
      limits.push_back(
          GetPathSpeedLimit(remaining, -offset, kSampleSpacing, kLateralAcceleration,
                            kBrakingDeceleration, kFreeSpeed));
    }
  }

  const auto [lowest, highest] = std::minmax_element(limits.begin(), limits.end());
  EXPECT_LT(*highest - *lowest, 0.35f) << "limit swings by " << (*highest - *lowest) << " m/s";
}

TEST(TrafficManagerPathSpeed, PathSpeedLimit_AllowsMoreSpeedFurtherFromTheTurn) {
  const float radius = 11.0f;
  const std::vector<cg::Location> near_path = StraightThenArc(10.0f, radius, 90.0f, kSampleSpacing);
  const std::vector<cg::Location> far_path = StraightThenArc(40.0f, radius, 90.0f, kSampleSpacing);

  const float near_limit = GetPathSpeedLimit(
      near_path, 0.0f, kSampleSpacing, kLateralAcceleration, kBrakingDeceleration, kFreeSpeed);
  const float far_limit = GetPathSpeedLimit(
      far_path, 0.0f, kSampleSpacing, kLateralAcceleration, kBrakingDeceleration, kFreeSpeed);

  EXPECT_GT(far_limit, near_limit);
  EXPECT_GT(near_limit, std::sqrt(radius * kLateralAcceleration));
}

TEST(TrafficManagerPathSpeed, PathSpeedLimit_TighterTurnAllowsLessSpeed) {
  // Junction connectors are stored more densely than the map resolution.
  constexpr float kJunctionSpacing{2.0f};
  const std::vector<cg::Location> tight = StraightThenArc(0.0f, 6.0f, 120.0f, kJunctionSpacing);
  const std::vector<cg::Location> open = StraightThenArc(0.0f, 25.0f, 120.0f, kJunctionSpacing);

  EXPECT_LT(
      GetPathSpeedLimit(tight, 0.0f, kSampleSpacing, kLateralAcceleration,
                        kBrakingDeceleration, kFreeSpeed),
      GetPathSpeedLimit(open, 0.0f, kSampleSpacing, kLateralAcceleration,
                        kBrakingDeceleration, kFreeSpeed));
}

TEST(TrafficManagerPathSpeed, PathSpeedLimit_PathShorterThanTwoSamplesIsNotLimited) {
  const std::vector<cg::Location> stub = StraightThenArc(0.0f, 6.0f, 60.0f, 2.0f);
  ASSERT_LT(stub.size() * 2.0f, 2.0f * kSampleSpacing);

  EXPECT_FLOAT_EQ(
      GetPathSpeedLimit(stub, 0.0f, kSampleSpacing, kLateralAcceleration,
                        kBrakingDeceleration, kFreeSpeed),
      kFreeSpeed);
}

TEST(TrafficManagerPathSpeed, PathSpeedLimit_NonPositiveSpacingIsNotLimited) {
  const std::vector<cg::Location> path = StraightThenArc(0.0f, 6.0f, 120.0f, 2.0f);

  EXPECT_FLOAT_EQ(
      GetPathSpeedLimit(path, 0.0f, 0.0f, kLateralAcceleration, kBrakingDeceleration, kFreeSpeed),
      kFreeSpeed);
}

TEST(TrafficManagerPathSpeed, PathSpeedLimit_ArcBehindTheVehicleIsNotCreditedWithBraking) {
  const float radius = 11.0f;
  const std::vector<cg::Location> path = StraightThenArc(0.0f, radius, 180.0f, kSampleSpacing);

  const float behind = GetPathSpeedLimit(
      path, -kSampleSpacing, kSampleSpacing, kLateralAcceleration, kBrakingDeceleration,
      kFreeSpeed);
  const float ahead = GetPathSpeedLimit(
      path, kSampleSpacing, kSampleSpacing, kLateralAcceleration, kBrakingDeceleration,
      kFreeSpeed);

  EXPECT_NEAR(behind, std::sqrt(radius * kLateralAcceleration), 0.1f);
  EXPECT_GT(ahead, behind);
}

// -----------------------------------------------------------------------------
// LargeVehicleJunctionOffsetProfile (Option C: bounded inboard excursion)
// -----------------------------------------------------------------------------

namespace {

constexpr float kProfileMaxOffset{2.0f};
constexpr float kProfileMaxOffsetPoint{0.3f};
constexpr float kProfileInboardScale{0.25f};
constexpr float kProfileTolerance{1e-3f};

float SampleProfile(float t, float inboard_scale) {
  return LargeVehicleJunctionOffsetProfile(
      t, kProfileMaxOffset, kProfileMaxOffsetPoint, inboard_scale);
}

}  // namespace

TEST(TrafficManagerWideTurn, OffsetProfile_ZeroAtEntryAndExit) {
  // t = 0 is the junction exit, t = 1 is the entry; both must produce no
  // offset so the manoeuvre starts and ends on the centreline.
  EXPECT_NEAR(SampleProfile(0.0f, kProfileInboardScale), 0.0f, kProfileTolerance);
  EXPECT_NEAR(SampleProfile(1.0f, kProfileInboardScale), 0.0f, kProfileTolerance);
}

TEST(TrafficManagerWideTurn, OffsetProfile_OutboardSwingReachesFullMagnitude) {
  // The outboard (negative) swing near the entry is preserved at full
  // magnitude so the vehicle still opens the turn up. Its peak sits at
  // t = 1 - max_offset_point.
  const float outboard{SampleProfile(1.0f - kProfileMaxOffsetPoint, kProfileInboardScale)};
  EXPECT_NEAR(outboard, -kProfileMaxOffset, kProfileTolerance);
}

TEST(TrafficManagerWideTurn, OffsetProfile_InboardExcursionIsCapped) {
  // The inboard (positive) excursion is the exit cut-in that drives the rear
  // into the inside shoulder. Across the whole junction it must never exceed
  // inboard_scale * max_offset, and never fall below -max_offset.
  const float inboard_cap{kProfileInboardScale * kProfileMaxOffset};
  for (int i = 0; i <= 100; ++i) {
    const float t{static_cast<float>(i) / 100.0f};
    const float offset{SampleProfile(t, kProfileInboardScale)};
    EXPECT_LE(offset, inboard_cap + kProfileTolerance);
    EXPECT_GE(offset, -kProfileMaxOffset - kProfileTolerance);
  }
}

TEST(TrafficManagerWideTurn, OffsetProfile_ZeroInboardScaleRemovesCutIn) {
  // With inboard_scale = 0 the exit cut-in is removed entirely: the profile is
  // never positive (it only swings outboard, then returns to the centreline).
  for (int i = 0; i <= 100; ++i) {
    const float t{static_cast<float>(i) / 100.0f};
    EXPECT_LE(SampleProfile(t, 0.0f), kProfileTolerance);
  }
}

// -----------------------------------------------------------------------------
// LargeVehicleOffsetMagnitude (Option B: dimension-aware magnitude)
// -----------------------------------------------------------------------------

namespace {

constexpr float kRefLength{6.0f};
constexpr float kOffsetGain{0.25f};
constexpr float kOffsetCap{1.5f};

float SampleMagnitude(float length) {
  return LargeVehicleOffsetMagnitude(length, kRefLength, kOffsetGain, kOffsetCap);
}

}  // namespace

TEST(TrafficManagerWideTurn, OffsetMagnitude_ZeroAtOrBelowReferenceLength) {
  EXPECT_NEAR(SampleMagnitude(kRefLength), 0.0f, kProfileTolerance);
  EXPECT_NEAR(SampleMagnitude(kRefLength - 2.0f), 0.0f, kProfileTolerance);
}

TEST(TrafficManagerWideTurn, OffsetMagnitude_RampsThenClampsToCap) {
  // length 8 -> 0.25 * 2 = 0.5; length 12 -> 0.25 * 6 = 1.5 (cap); beyond is clamped.
  EXPECT_NEAR(SampleMagnitude(8.0f), 0.5f, kProfileTolerance);
  EXPECT_NEAR(SampleMagnitude(12.0f), kOffsetCap, kProfileTolerance);
  EXPECT_NEAR(SampleMagnitude(20.0f), kOffsetCap, kProfileTolerance);
}

TEST(TrafficManagerWideTurn, OffsetMagnitude_MonotonicNonDecreasingInLength) {
  float previous{SampleMagnitude(0.0f)};
  for (float length = 0.0f; length <= 20.0f; length += 0.5f) {
    const float current{SampleMagnitude(length)};
    EXPECT_GE(current, previous - kProfileTolerance);
    previous = current;
  }
}

// -----------------------------------------------------------------------------
// IsOffsetSideOccupied (Option A: collision-aware gating decision)
// -----------------------------------------------------------------------------

namespace {

// Ego at the origin, heading +x, swinging toward +y.
const cg::Location kEgoLocation{0.0f, 0.0f, 0.0f};
const cg::Vector3D kEgoForward{1.0f, 0.0f, 0.0f};
const cg::Vector3D kOffsetDirection{0.0f, 1.0f, 0.0f};
constexpr float kOffsetMagnitude{1.5f};
constexpr float kLateralClearance{1.0f};
constexpr float kLongitudinalWindow{5.0f};

bool SampleOccupied(const std::vector<std::pair<cg::Location, float>> &neighbours) {
  return IsOffsetSideOccupied(
      kEgoLocation,
      kEgoForward,
      kOffsetDirection,
      kOffsetMagnitude,
      kLateralClearance,
      kLongitudinalWindow,
      neighbours);
}

}  // namespace

TEST(TrafficManagerWideTurn, OffsetSideOccupied_EmptyNeighboursIsClear) {
  EXPECT_FALSE(SampleOccupied({}));
}

TEST(TrafficManagerWideTurn, OffsetSideOccupied_NeighbourInBandBlocks) {
  // 2 m to the offset side, alongside the ego: within the swing band.
  EXPECT_TRUE(SampleOccupied({{{0.0f, 2.0f, 0.0f}, 1.0f}}));
}

TEST(TrafficManagerWideTurn, OffsetSideOccupied_FarLateralNeighbourIsClear) {
  // 10 m to the side is well beyond offset_magnitude + clearance + radius.
  EXPECT_FALSE(SampleOccupied({{{0.0f, 10.0f, 0.0f}, 1.0f}}));
}

TEST(TrafficManagerWideTurn, OffsetSideOccupied_OppositeSideNeighbourIsClear) {
  // A neighbour on the far side of the offset direction does not block.
  EXPECT_FALSE(SampleOccupied({{{0.0f, -3.0f, 0.0f}, 1.0f}}));
}

TEST(TrafficManagerWideTurn, OffsetSideOccupied_NeighbourAheadOutsideWindowIsClear) {
  // On the offset side but 20 m ahead: outside the longitudinal window.
  EXPECT_FALSE(SampleOccupied({{{20.0f, 1.0f, 0.0f}, 1.0f}}));
}

// -----------------------------------------------------------------------------
// IsRefreshDue
// -----------------------------------------------------------------------------

namespace {

namespace tmgr = carla::traffic_manager;
namespace pid_constants = carla::traffic_manager::constants::PID;
namespace refresh = carla::traffic_manager::constants::WorldInfoRefresh;

constexpr double kNeverRead{-std::numeric_limits<double>::infinity()};

}  // namespace

TEST(TrafficManagerRefreshCadence, IsRefreshDue_NothingReadYetIsDue) {
  EXPECT_TRUE(refresh::IsRefreshDue(0.0, kNeverRead, refresh::EPISODE_SETTINGS_REFRESH_PERIOD));
  EXPECT_TRUE(refresh::IsRefreshDue(1234.5, kNeverRead, refresh::VEHICLE_LIGHT_STATES_REFRESH_PERIOD));
}

TEST(TrafficManagerRefreshCadence, IsRefreshDue_SameFrameIsNotDue) {
  EXPECT_FALSE(refresh::IsRefreshDue(10.0, 10.0, refresh::VEHICLE_LIGHT_STATES_REFRESH_PERIOD));
}

TEST(TrafficManagerRefreshCadence, IsRefreshDue_WithinThePeriodIsNotDue) {
  EXPECT_FALSE(refresh::IsRefreshDue(10.24, 10.0, 0.25));
}

TEST(TrafficManagerRefreshCadence, IsRefreshDue_AtOrPastThePeriodIsDue) {
  EXPECT_TRUE(refresh::IsRefreshDue(10.25, 10.0, 0.25));
  EXPECT_TRUE(refresh::IsRefreshDue(11.0, 10.0, 0.25));
}

TEST(TrafficManagerRefreshCadence, IsRefreshDue_ClockGoingBackwardsIsDue) {
  // A new episode restarts elapsed_seconds at zero.
  EXPECT_TRUE(refresh::IsRefreshDue(0.05, 300.0, 1.0));
}

// -----------------------------------------------------------------------------
// IsEarlyRefreshDue
// -----------------------------------------------------------------------------

namespace {

struct LightRefreshTrace {
  int refreshes{0};
  int steps_from_registration_to_a_refresh{-1};
};

/// Mirrors VehicleLightStage's refresh cadence for a vehicle registered on
/// @a registration_step that the server never reports; pass @a steps for none.
LightRefreshTrace TraceLightRefreshes(
    const int steps,
    const double step_period,
    const int registration_step) {

  LightRefreshTrace trace{};
  double last_update{kNeverRead};
  bool missing_from_the_last_refresh{false};

  for (int step{0}; step < steps; ++step) {
    const double now{step_period * static_cast<double>(step + 1)};
    const bool refreshed{
        refresh::IsRefreshDue(now, last_update, refresh::VEHICLE_LIGHT_STATES_REFRESH_PERIOD)};
    if (refreshed) {
      last_update = now;
      missing_from_the_last_refresh = false;
      ++trace.refreshes;
    }

    if (step < registration_step) {
      continue;
    }
    if (refreshed && trace.steps_from_registration_to_a_refresh < 0) {
      trace.steps_from_registration_to_a_refresh = step - registration_step;
    }
    if (refresh::IsEarlyRefreshDue(refreshed, missing_from_the_last_refresh)) {
      last_update = kNeverRead;
    }
    if (refreshed) {
      missing_from_the_last_refresh = true;
    }
  }
  return trace;
}

constexpr int kThirtyFpsSteps{300};
constexpr double kThirtyFpsPeriod{1.0 / 30.0};

}  // namespace

TEST(TrafficManagerRefreshCadence, IsEarlyRefreshDue_VehicleSeenForTheFirstTimeIsWorthARead) {
  EXPECT_TRUE(refresh::IsEarlyRefreshDue(false, false));
}

TEST(TrafficManagerRefreshCadence, IsEarlyRefreshDue_VehicleTheServerOmitsIsNotWorthAnother) {
  EXPECT_FALSE(refresh::IsEarlyRefreshDue(false, true));
}

TEST(TrafficManagerRefreshCadence, IsEarlyRefreshDue_StepThatAlreadyReadNeedsNoRead) {
  EXPECT_FALSE(refresh::IsEarlyRefreshDue(true, false));
  EXPECT_FALSE(refresh::IsEarlyRefreshDue(true, true));
}

TEST(TrafficManagerRefreshCadence, EarlyRefresh_VehicleTheServerNeverReportsKeepsTheRegularCadence) {
  const LightRefreshTrace reported{
      TraceLightRefreshes(kThirtyFpsSteps, kThirtyFpsPeriod, kThirtyFpsSteps)};
  const LightRefreshTrace omitted{TraceLightRefreshes(kThirtyFpsSteps, kThirtyFpsPeriod, 3)};

  EXPECT_LE(omitted.refreshes, reported.refreshes + 1);
  EXPECT_LE(
      static_cast<double>(reported.refreshes),
      1.0 + static_cast<double>(kThirtyFpsSteps) * kThirtyFpsPeriod /
                refresh::VEHICLE_LIGHT_STATES_REFRESH_PERIOD);
}

TEST(TrafficManagerRefreshCadence, EarlyRefresh_NewlyRegisteredVehicleIsReadPromptly) {
  const LightRefreshTrace omitted{TraceLightRefreshes(kThirtyFpsSteps, kThirtyFpsPeriod, 3)};
  EXPECT_EQ(omitted.steps_from_registration_to_a_refresh, 1);
}

// -----------------------------------------------------------------------------
// IsBatchSyncDue
// -----------------------------------------------------------------------------

namespace {

struct BatchSyncTrace {
  int syncs{0};
  uint64_t deepest_queue{0u};
};

BatchSyncTrace TraceBatchSyncs(const int steps, const uint64_t limit) {
  BatchSyncTrace trace{};
  uint64_t unwaited{limit};
  for (int step{0}; step < steps; ++step) {
    if (refresh::IsBatchSyncDue(unwaited, limit)) {
      unwaited = 0u;
      ++trace.syncs;
    } else {
      ++unwaited;
    }
    trace.deepest_queue = std::max(trace.deepest_queue, unwaited);
  }
  return trace;
}

/// The same queue bounded by simulation time instead.
BatchSyncTrace TraceTimeGatedBatchSyncs(
    const int steps,
    const double fixed_delta_seconds,
    const double period) {

  BatchSyncTrace trace{};
  uint64_t unwaited{0u};
  double last_sync{kNeverRead};
  for (int step{0}; step < steps; ++step) {
    const double now{fixed_delta_seconds * static_cast<double>(step + 1)};
    if (refresh::IsRefreshDue(now, last_sync, period)) {
      unwaited = 0u;
      last_sync = now;
      ++trace.syncs;
    } else {
      ++unwaited;
    }
    trace.deepest_queue = std::max(trace.deepest_queue, unwaited);
  }
  return trace;
}

constexpr int kBatchSteps{1000};

}  // namespace

TEST(TrafficManagerBatchSync, IsBatchSyncDue_WithinTheBoundNothingIsWaitedFor) {
  EXPECT_FALSE(refresh::IsBatchSyncDue(0u, refresh::MAX_UNWAITED_CONTROL_BATCHES));
  EXPECT_FALSE(refresh::IsBatchSyncDue(refresh::MAX_UNWAITED_CONTROL_BATCHES - 1u,
                                       refresh::MAX_UNWAITED_CONTROL_BATCHES));
}

TEST(TrafficManagerBatchSync, IsBatchSyncDue_AtOrPastTheBoundOneIsWaitedFor) {
  EXPECT_TRUE(refresh::IsBatchSyncDue(refresh::MAX_UNWAITED_CONTROL_BATCHES,
                                      refresh::MAX_UNWAITED_CONTROL_BATCHES));
  EXPECT_TRUE(refresh::IsBatchSyncDue(refresh::MAX_UNWAITED_CONTROL_BATCHES + 5u,
                                      refresh::MAX_UNWAITED_CONTROL_BATCHES));
}

TEST(TrafficManagerBatchSync, QueueNeverHoldsMoreThanThePermittedBatches) {
  const BatchSyncTrace trace{TraceBatchSyncs(kBatchSteps, refresh::MAX_UNWAITED_CONTROL_BATCHES)};
  EXPECT_LE(trace.deepest_queue, refresh::MAX_UNWAITED_CONTROL_BATCHES);
  EXPECT_EQ(trace.syncs, kBatchSteps / static_cast<int>(refresh::MAX_UNWAITED_CONTROL_BATCHES + 1u));
}

TEST(TrafficManagerBatchSync, SlowServerWithAFixedDeltaIsBoundedTheSameWayAsAFastOne) {
  // With a fixed delta a simulation-time bound scales with the delta, not the
  // queue: at 0.05 s a one-second period is twenty batches.
  const BatchSyncTrace counted{TraceBatchSyncs(kBatchSteps, refresh::MAX_UNWAITED_CONTROL_BATCHES)};
  const BatchSyncTrace timed{TraceTimeGatedBatchSyncs(kBatchSteps, 0.05, 1.0)};

  EXPECT_LE(counted.deepest_queue, refresh::MAX_UNWAITED_CONTROL_BATCHES);
  EXPECT_GT(timed.deepest_queue, refresh::MAX_UNWAITED_CONTROL_BATCHES);
}

// -----------------------------------------------------------------------------
// ShapeReferenceVelocity
// -----------------------------------------------------------------------------

namespace {

constexpr float kControlDt{0.05f};
constexpr float kCruiseTarget{60.0f / 3.6f};

float Shape(
    float previous_reference,
    float vehicle_speed,
    float target_velocity,
    float control_dt,
    bool emergency_stop = false) {

  return tmgr::PID::ShapeReferenceVelocity(
      previous_reference, vehicle_speed, target_velocity, control_dt, emergency_stop);
}

}  // namespace

TEST(TrafficManagerComfortConstants, ReferenceLeadOutrunsTheThrottleSaturationError) {
  for (const auto &longitudinal : {pid_constants::LONGITUDIAL_PARAM,
                                   pid_constants::LONGITUDIAL_HIGHWAY_PARAM}) {
    EXPECT_GT(pid_constants::REFERENCE_LEAD_FRACTION,
              pid_constants::MAX_THROTTLE / longitudinal[0]);
  }
}

TEST(TrafficManagerReferenceVelocity, ShapeReference_RampsAwayFromRestAtComfortAcceleration) {
  EXPECT_FLOAT_EQ(
      Shape(0.0f, 0.0f, kCruiseTarget, kControlDt),
      pid_constants::COMFORT_ACCELERATION * kControlDt);
}

TEST(TrafficManagerReferenceVelocity, ShapeReference_NeverExceedsTheTarget) {
  EXPECT_FLOAT_EQ(
      Shape(kCruiseTarget - 0.05f, kCruiseTarget - 0.05f, kCruiseTarget,
            pid_constants::MAX_CONTROL_DT),
      kCruiseTarget);
}

TEST(TrafficManagerReferenceVelocity, ShapeReference_DecelerationIsPassedThroughUnshaped) {
  EXPECT_FLOAT_EQ(Shape(20.0f, 20.0f, 5.0f, kControlDt), 5.0f);
}

TEST(TrafficManagerReferenceVelocity, ShapeReference_StaleLowReferenceIsRecoveredInOneStep) {
  // e.g. a flushed state entry or a K-turn recovery.
  EXPECT_FLOAT_EQ(Shape(0.0f, 20.0f, 25.0f, kControlDt), 20.0f);
}

TEST(TrafficManagerReferenceVelocity, ShapeReference_StaleHighReferenceIsCappedToTheLead) {
  EXPECT_FLOAT_EQ(
      Shape(100.0f, 5.0f, 25.0f, kControlDt),
      5.0f + pid_constants::REFERENCE_LEAD_FRACTION * 25.0f);
}

TEST(TrafficManagerReferenceVelocity, ShapeReference_LeadStaysBoundedWhileTheVehicleCannotFollow) {
  // Uphill, or wedged against a kerb: the vehicle never gains speed.
  float reference{0.0f};
  for (int step = 0; step < 400; ++step) {
    reference = Shape(reference, 0.0f, kCruiseTarget, kControlDt);
  }
  EXPECT_FLOAT_EQ(reference, pid_constants::REFERENCE_LEAD_FRACTION * kCruiseTarget);
}

TEST(TrafficManagerReferenceVelocity, ShapeReference_EmergencyStopDiscardsTheStoredReference) {
  const float held{Shape(25.0f, 5.0f, 20.0f, kControlDt, true)};
  const float not_held{Shape(25.0f, 5.0f, 20.0f, kControlDt, false)};

  EXPECT_FLOAT_EQ(held, 5.0f + pid_constants::COMFORT_ACCELERATION * kControlDt);
  EXPECT_LT(held, not_held);
}

TEST(TrafficManagerReferenceVelocity, ShapeReference_SlowServerStillRampsAtTheCompensatedRate) {
  // The step is bounded by MAX_CONTROL_DT, as the controller is.
  EXPECT_FLOAT_EQ(
      Shape(0.0f, 0.0f, kCruiseTarget, 1.0f),
      pid_constants::COMFORT_ACCELERATION * pid_constants::MAX_CONTROL_DT);
}

TEST(TrafficManagerReferenceVelocity, ShapeReference_ShortFrameStillAdvancesTheReference) {
  EXPECT_FLOAT_EQ(
      Shape(0.0f, 0.0f, kCruiseTarget, 0.0001f),
      pid_constants::COMFORT_ACCELERATION * pid_constants::MIN_CONTROL_DT);
}

// -----------------------------------------------------------------------------
// RelativeVelocityDeviation
// -----------------------------------------------------------------------------

TEST(TrafficManagerReferenceVelocity, VelocityDeviation_MatchesThePlainRatioOnceCaughtUp) {
  EXPECT_FLOAT_EQ(tmgr::PID::RelativeVelocityDeviation(20.0f, 10.0f, 20.0f), 0.5f);
}

TEST(TrafficManagerReferenceVelocity, VelocityDeviation_ZeroTargetBehindAStoppedVehicleIsFinite) {
  const float reference{Shape(0.0f, 0.0f, 0.0f, kControlDt)};
  const float deviation{tmgr::PID::RelativeVelocityDeviation(reference, 0.0f, 0.0f)};

  EXPECT_TRUE(std::isfinite(deviation));
  EXPECT_FLOAT_EQ(deviation, 0.0f);
}

TEST(TrafficManagerReferenceVelocity, VelocityDeviation_ZeroTargetDoesNotPoisonTheNextStep) {
  // A NaN deviation would reach the steering through the derivative term.
  StateEntry previous{};
  StateEntry current{};
  const float reference{Shape(0.0f, 0.0f, 0.0f, kControlDt)};
  current.velocity_deviation = tmgr::PID::RelativeVelocityDeviation(reference, 0.0f, 0.0f);
  current.reference_velocity = reference;

  const ActuationSignal first{tmgr::PID::RunStep(
      current, previous, pid_constants::LONGITUDIAL_PARAM, pid_constants::LATERAL_PARAM,
      0.0f, kControlDt)};
  EXPECT_TRUE(std::isfinite(first.throttle));
  EXPECT_TRUE(std::isfinite(first.brake));
  EXPECT_TRUE(std::isfinite(first.steer));

  const ActuationSignal second{tmgr::PID::RunStep(
      current, current, pid_constants::LONGITUDIAL_PARAM, pid_constants::LATERAL_PARAM,
      0.0f, kControlDt)};
  EXPECT_TRUE(std::isfinite(second.steer));
  EXPECT_TRUE(std::isfinite(second.throttle));
}

// -----------------------------------------------------------------------------
// SmoothActuation
// -----------------------------------------------------------------------------

namespace {

StateEntry PreviousCommand(float throttle, float brake, float steer) {
  StateEntry state{};
  state.throttle = throttle;
  state.brake = brake;
  state.steer = steer;
  return state;
}

ActuationSignal Smooth(
    const StateEntry &previous,
    float control_dt,
    float vehicle_speed,
    ActuationSignal demand) {

  tmgr::PID::SmoothActuation(previous, control_dt, vehicle_speed, demand);
  return demand;
}

}  // namespace

TEST(TrafficManagerActuationSmoothing, LaunchRamp_LimitsTheThrottleRiseFromRest) {
  const ActuationSignal smoothed{Smooth(
      PreviousCommand(0.0f, 0.0f, 0.0f), kControlDt, 0.0f,
      ActuationSignal{pid_constants::MAX_THROTTLE, 0.0f, 0.0f})};

  EXPECT_FLOAT_EQ(smoothed.throttle, pid_constants::MAX_LAUNCH_THROTTLE_RISE_RATE * kControlDt);
}

TEST(TrafficManagerActuationSmoothing, LaunchRamp_AlwaysClearsTheThrottleDeadband) {
  const ActuationSignal smoothed{Smooth(
      PreviousCommand(0.0f, 0.0f, 0.0f), 0.0001f, 0.0f,
      ActuationSignal{pid_constants::MAX_THROTTLE, 0.0f, 0.0f})};

  EXPECT_GT(smoothed.throttle, pid_constants::THROTTLE_DEADBAND);
  EXPECT_FLOAT_EQ(
      smoothed.throttle,
      pid_constants::MAX_LAUNCH_THROTTLE_RISE_RATE * pid_constants::MIN_CONTROL_DT);
}

TEST(TrafficManagerActuationSmoothing, LaunchRamp_ReachesFullThrottleInABoundedNumberOfSteps) {
  StateEntry previous{PreviousCommand(0.0f, 0.0f, 0.0f)};
  for (int step = 0; step < 200; ++step) {
    const ActuationSignal smoothed{Smooth(
        previous, kControlDt, 0.0f,
        ActuationSignal{pid_constants::MAX_THROTTLE, 0.0f, 0.0f})};
    previous.throttle = smoothed.throttle;
  }
  EXPECT_FLOAT_EQ(previous.throttle, pid_constants::MAX_THROTTLE);
}

TEST(TrafficManagerActuationSmoothing, LaunchRamp_IsReleasedAboveTheLaunchSpeed) {
  const ActuationSignal smoothed{Smooth(
      PreviousCommand(0.0f, 0.0f, 0.0f), kControlDt,
      pid_constants::LAUNCH_RAMP_SPEED + 0.01f,
      ActuationSignal{pid_constants::MAX_THROTTLE, 0.0f, 0.0f})};

  EXPECT_FLOAT_EQ(smoothed.throttle, pid_constants::MAX_THROTTLE);
}

TEST(TrafficManagerActuationSmoothing, LaunchRamp_NeverRaisesAFallingThrottle) {
  const ActuationSignal smoothed{Smooth(
      PreviousCommand(0.5f, 0.0f, 0.0f), kControlDt, 1.0f,
      ActuationSignal{0.1f, 0.0f, 0.0f})};

  EXPECT_FLOAT_EQ(smoothed.throttle, 0.1f);
}

TEST(TrafficManagerActuationSmoothing, Deadband_HoldsTheThrottleDitherAndPassesARealStep) {
  const StateEntry previous{PreviousCommand(0.5f, 0.0f, 0.0f)};

  EXPECT_FLOAT_EQ(
      Smooth(previous, kControlDt, 10.0f, ActuationSignal{0.505f, 0.0f, 0.0f}).throttle, 0.5f);
  EXPECT_FLOAT_EQ(
      Smooth(previous, kControlDt, 10.0f, ActuationSignal{0.52f, 0.0f, 0.0f}).throttle, 0.52f);
}

TEST(TrafficManagerActuationSmoothing, Deadband_HoldsTheBrakeDitherAndPassesARealStep) {
  const StateEntry previous{PreviousCommand(0.0f, 0.4f, 0.0f)};

  EXPECT_FLOAT_EQ(
      Smooth(previous, kControlDt, 10.0f, ActuationSignal{0.0f, 0.4005f, 0.0f}).brake, 0.4f);
  EXPECT_FLOAT_EQ(
      Smooth(previous, kControlDt, 10.0f, ActuationSignal{0.0f, 0.45f, 0.0f}).brake, 0.45f);
}

TEST(TrafficManagerActuationSmoothing, Deadband_ZeroPedalDemandIsAlwaysHonoured) {
  const ActuationSignal smoothed{Smooth(
      PreviousCommand(0.005f, 0.005f, 0.0f), kControlDt, 10.0f,
      ActuationSignal{0.0f, 0.0f, 0.0f})};

  EXPECT_FLOAT_EQ(smoothed.throttle, 0.0f);
  EXPECT_FLOAT_EQ(smoothed.brake, 0.0f);
}

TEST(TrafficManagerActuationSmoothing, Deadband_HoldsTheSteeringDitherAtUrbanSpeed) {
  const StateEntry previous{PreviousCommand(0.0f, 0.0f, 0.1f)};

  EXPECT_FLOAT_EQ(
      Smooth(previous, kControlDt, 5.0f, ActuationSignal{0.0f, 0.0f, 0.1015f}).steer, 0.1f);
  EXPECT_FLOAT_EQ(
      Smooth(previous, kControlDt, 5.0f, ActuationSignal{0.0f, 0.0f, 0.105f}).steer, 0.105f);
}

TEST(TrafficManagerActuationSmoothing, Deadband_SteeringBandShrinksWithSpeed) {
  const StateEntry previous{PreviousCommand(0.0f, 0.0f, 0.1f)};
  const ActuationSignal demand{0.0f, 0.0f, 0.1005f};

  EXPECT_FLOAT_EQ(
      Smooth(previous, kControlDt, pid_constants::STEER_DEADBAND_REF_SPEED, demand).steer, 0.1f);
  EXPECT_FLOAT_EQ(
      Smooth(previous, kControlDt, 4.0f * pid_constants::STEER_DEADBAND_REF_SPEED, demand).steer,
      0.1005f);
}
