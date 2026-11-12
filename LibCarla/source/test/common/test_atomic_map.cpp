// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/trafficmanager/AtomicMap.h>

#include <gtest/gtest.h>

#include <memory>
#include <string>

using carla::traffic_manager::AtomicMap;

TEST(AtomicMap, value_snapshot_survives_replacement) {
  AtomicMap<int, std::string> values;
  values.AddEntry({1, "original"});

  const auto &snapshot = values.GetValue(1);
  values.AddEntry({1, "replacement"});

  EXPECT_EQ(snapshot, "original");
  EXPECT_EQ(values.GetValue(1), "replacement");
}

TEST(AtomicMap, value_snapshot_keeps_shared_ownership_after_removal) {
  AtomicMap<int, std::shared_ptr<int>> values;
  auto value = std::make_shared<int>(42);
  const std::weak_ptr<int> lifetime = value;
  values.AddEntry({1, value});
  value.reset();

  const auto &snapshot = values.GetValue(1);
  values.RemoveEntry(1);

  EXPECT_FALSE(values.Contains(1));
  ASSERT_FALSE(lifetime.expired());
  EXPECT_EQ(*snapshot, 42);
}
