// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <cmath>
#include <limits>
#include <utility>
#include <vector>

#include <carla/geom/Location.h>
#include <carla/trafficmanager/Constants.h>
#include <carla/trafficmanager/TrafficManagerGeometry.h>

namespace cg = carla::geom;
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
