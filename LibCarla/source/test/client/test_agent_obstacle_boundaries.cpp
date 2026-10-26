// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/agents/navigation/Misc.h>
#include <carla/geom/Transform.h>

namespace {
using carla::agents::navigation::IsWithinDistance;
using carla::geom::Location;
using carla::geom::Transform;

TEST(agent_obstacle_boundaries, angle_interval_includes_its_documented_limits) {
  const Transform reference;
  EXPECT_TRUE(IsWithinDistance(Transform(Location{5.f, 0.f, 0.f}), reference, 10.f, {{0.f, 90.f}}));
  EXPECT_TRUE(IsWithinDistance(Transform(Location{0.f, 5.f, 0.f}), reference, 10.f, {{0.f, 90.f}}));
  EXPECT_TRUE(IsWithinDistance(Transform(Location{-5.f, 0.f, 0.f}), reference, 10.f, {{90.f, 180.f}}));
}

TEST(agent_obstacle_boundaries, outside_angle_distance_and_interior_controls) {
  const Transform reference;
  EXPECT_FALSE(IsWithinDistance(Transform(Location{-5.f, 0.f, 0.f}), reference, 10.f, {{0.f, 90.f}}));
  EXPECT_FALSE(IsWithinDistance(Transform(Location{5.f, 1.f, 0.f}), reference, 10.f, {{20.f, 40.f}}));
  EXPECT_FALSE(IsWithinDistance(Transform(Location{15.f, 1.f, 0.f}), reference, 10.f, {{0.f, 90.f}}));
  EXPECT_TRUE(IsWithinDistance(Transform(Location{5.f, 1.f, 0.f}), reference, 10.f, {{0.f, 90.f}}));
}
} // namespace
