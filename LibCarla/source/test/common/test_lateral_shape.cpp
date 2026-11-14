// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/road/MapBuilder.h"

#include <algorithm>
#include <array>
#include <vector>

namespace {

  struct Shape {
    double s, t, a, b, c, d;
  };

  std::optional<carla::road::Map> BuildSurface(const std::vector<Shape> &shapes) {
    carla::road::MapBuilder builder;
    auto *road = builder.AddRoad(1, "surface", 100.0, -1, 0, 0, true);
    builder.AddRoadGeometryLine(road, 0.0, 0.0, 0.0, 0.0, 100.0);
    builder.AddRoadElevationProfile(road, 0.0, 0.0, 0.0, 0.0, 0.0);
    for (const auto &shape : shapes) {
      builder.AddRoadLateralShape(road, shape.s, shape.a, shape.b, shape.c, shape.d, shape.t);
    }
    return builder.Build();
  }

} // namespace

TEST(LateralShape, EmptySurface) {
  auto map = BuildSurface({});
  ASSERT_TRUE(map.has_value());
  const auto &road = map->GetData().GetRoad(1);
  for (double s : {-10.0, 0.0, 50.0, 100.0, 110.0}) {
    for (double t : {-10.0, 0.0, 10.0}) {
      EXPECT_DOUBLE_EQ(road.GetLateralShapeZ(s, t), 0.0);
    }
  }
}

TEST(LateralShape, SingleStationWithUnorderedCubicPieces) {
  auto map = BuildSurface({
      {10.0, 4.0, 20.0, -1.0, 0.5, -0.25},
      {10.0, -2.0, 1.0, 2.0, 3.0, 4.0},
      {10.0, 1.0, 5.0, -0.5, 0.25, 0.125}});
  ASSERT_TRUE(map.has_value());
  const auto &road = map->GetData().GetRoad(1);
  const std::array<double, 7> positions{-3.0, -2.0, 0.0, 1.0, 2.0, 4.0, 5.0};
  const std::array<double, 7> heights{-2.0, 1.0, 49.0, 5.0, 4.875, 20.0, 19.25};
  for (double s : {-10.0, 0.0, 10.0, 30.0, 120.0}) {
    for (size_t i = 0; i < positions.size(); ++i) {
      EXPECT_DOUBLE_EQ(road.GetLateralShapeZ(s, positions[i]), heights[i]);
    }
  }
}

TEST(LateralShape, InterpolatesBetweenDifferentPieceBoundaries) {
  // Deliberately interleave both station and lateral orders. Each station
  // selects its own piece before the heights are interpolated along the road.
  auto map = BuildSurface({
      {60.0, 1.0, 25.0, 0.5, 0.0, 0.0},
      {20.0, 1.0, 10.0, 2.0, 0.0, 0.0},
      {60.0, 4.0, 30.0, -0.5, 0.0, 0.0},
      {20.0, -2.0, 4.0, -1.0, 0.0, 0.0},
      {60.0, -3.0, 20.0, 1.0, 0.0, 0.0}});
  ASSERT_TRUE(map.has_value());
  const auto &road = map->GetData().GetRoad(1);
  const std::array<double, 6> positions{-4.0, 0.0, 1.0, 3.0, 4.0, 5.0};
  const std::array<double, 6> first{6.0, 2.0, 10.0, 14.0, 16.0, 18.0};
  const std::array<double, 6> last{19.0, 23.0, 25.0, 26.0, 30.0, 29.5};
  for (double s : {-5.0, 0.0, 10.0, 20.0, 30.0, 40.0, 60.0, 90.0, 110.0}) {
    const double u = std::clamp((s - 20.0) / 40.0, 0.0, 1.0);
    for (size_t i = 0; i < positions.size(); ++i) {
      const double expected = (1.0 - u) * first[i] + u * last[i];
      EXPECT_DOUBLE_EQ(road.GetLateralShapeZ(s, positions[i]), expected);
    }
  }
}

TEST(LateralShape, ManyStationsPreservePiecewiseSurface) {
  std::vector<Shape> shapes;
  for (int station = 10; station >= 0; --station) {
    const double s = 10.0 * station;
    for (double t : {4.0, -4.0, 0.0, 2.0, -2.0}) {
      shapes.push_back({s, t, 0.25 * s + t * t, 0.5, 0.0, 0.0});
    }
  }
  auto map = BuildSurface(shapes);
  ASSERT_TRUE(map.has_value());
  const auto &road = map->GetData().GetRoad(1);
  for (double s : {-5.0, 0.0, 5.0, 10.0, 37.5, 90.0, 95.0, 100.0, 105.0}) {
    for (double t : {-6.0, -4.0, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 6.0}) {
      const double piece = std::clamp(2.0 * std::floor(t / 2.0), -4.0, 4.0);
      const double expected = 0.25 * std::clamp(s, 0.0, 100.0) +
          piece * piece + 0.5 * (t - piece);
      EXPECT_DOUBLE_EQ(road.GetLateralShapeZ(s, t), expected);
    }
  }
}
