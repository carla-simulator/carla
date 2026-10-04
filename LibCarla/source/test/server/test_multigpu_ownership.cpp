// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/multigpu/sensorOwnership.h>

namespace {

using carla::multigpu::OwnsSensor;
using carla::multigpu::ProcessRole;

} // namespace

TEST(MultiGpuOwnership, standalone_owns_every_sensor) {
  for (const bool routed : {false, true}) {
    for (const bool primary_only : {false, true}) {
      for (const bool token_granted : {false, true}) {
        EXPECT_TRUE(OwnsSensor(ProcessRole::Standalone, routed, primary_only, token_granted));
      }
    }
  }
}

TEST(MultiGpuOwnership, primary_owns_sensors_not_routed_to_a_secondary) {
  EXPECT_TRUE(OwnsSensor(ProcessRole::Primary, false, false, false));
  EXPECT_FALSE(OwnsSensor(ProcessRole::Primary, true, false, false));
}

TEST(MultiGpuOwnership, primary_always_owns_primary_only_sensors) {
  EXPECT_TRUE(OwnsSensor(ProcessRole::Primary, false, true, false));
  EXPECT_TRUE(OwnsSensor(ProcessRole::Primary, true, true, false));
}

TEST(MultiGpuOwnership, primary_ignores_the_token_granted_flag) {
  EXPECT_TRUE(OwnsSensor(ProcessRole::Primary, false, false, true));
  EXPECT_FALSE(OwnsSensor(ProcessRole::Primary, true, false, true));
}

TEST(MultiGpuOwnership, secondary_owns_only_routed_sensors) {
  EXPECT_TRUE(OwnsSensor(ProcessRole::Secondary, false, false, true));
  EXPECT_FALSE(OwnsSensor(ProcessRole::Secondary, false, false, false));
}

TEST(MultiGpuOwnership, secondary_never_owns_primary_only_sensors) {
  EXPECT_FALSE(OwnsSensor(ProcessRole::Secondary, false, true, true));
  EXPECT_FALSE(OwnsSensor(ProcessRole::Secondary, false, true, false));
}

TEST(MultiGpuOwnership, secondary_ignores_the_routed_flag) {
  EXPECT_TRUE(OwnsSensor(ProcessRole::Secondary, true, false, true));
  EXPECT_FALSE(OwnsSensor(ProcessRole::Secondary, true, false, false));
}

TEST(MultiGpuOwnership, each_sensor_has_exactly_one_owner_among_primary_and_its_secondary) {
  for (const bool routed : {false, true}) {
    for (const bool primary_only : {false, true}) {
      // A secondary holds the token exactly when the primary routed the sensor there.
      const bool primary = OwnsSensor(ProcessRole::Primary, routed, primary_only, false);
      const bool secondary = OwnsSensor(ProcessRole::Secondary, false, primary_only, routed);
      EXPECT_NE(primary, secondary) << "routed=" << routed << " primary_only=" << primary_only;
    }
  }
}

static_assert(OwnsSensor(ProcessRole::Primary, true, true, false));
static_assert(!OwnsSensor(ProcessRole::Primary, true, false, false));
static_assert(!OwnsSensor(ProcessRole::Secondary, false, true, true));
