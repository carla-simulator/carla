// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"
#include <carla/agents/navigation/Misc.h>
#include <carla/geom/Transform.h>

TEST(agent_planar_angles, normalizes_the_projected_forward_direction) {
  for (const float pitch : {-30.f, -11.f, 11.f, 30.f}) {
    const carla::geom::Transform reference({}, {pitch, 0.f, 0.f});
    const carla::geom::Transform target({8.f, 0.1f, 1.6f});
    EXPECT_TRUE(carla::agents::navigation::IsWithinDistance(target, reference, 15.f, {{0.f, 5.f}}));
  }
}

TEST(agent_planar_angles, flat_and_outside_controls) {
  const carla::geom::Transform target({8.f, 0.1f, 1.6f});
  EXPECT_TRUE(carla::agents::navigation::IsWithinDistance(target, {}, 15.f, {{0.f, 5.f}}));
  EXPECT_FALSE(carla::agents::navigation::IsWithinDistance(target, {}, 5.f, {{0.f, 5.f}}));
  EXPECT_FALSE(carla::agents::navigation::IsWithinDistance(
      carla::geom::Transform({8.f, 5.f, 1.6f}), {{}, {11.f, 0.f, 0.f}}, 15.f, {{0.f, 5.f}}));
}
