// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/ros2/TopicNames.h>
#include <gtest/gtest.h>

namespace {

class Ros2TopicNames : public testing::Test {
protected:
  int actors[3] = {};
  std::unordered_map<void *, void *> parents;
  std::unordered_map<void *, std::string> names;

  std::string Topic(int actor) {
    return carla::ros2::detail::BuildActorBaseTopicName(&actors[actor], parents, names);
  }
};

TEST_F(Ros2TopicNames, RegisteredRootKeepsPrefix) {
  names[&actors[0]] = "camera";
  EXPECT_EQ(Topic(0), "rt/carla/camera");
}

TEST_F(Ros2TopicNames, RegisteredParentKeepsHierarchy) {
  names[&actors[0]] = "hero2";
  names[&actors[1]] = "camera";
  parents[&actors[1]] = &actors[0];
  EXPECT_EQ(Topic(1), "rt/carla/hero2/camera");
}

TEST_F(Ros2TopicNames, UnregisteredParentDoesNotAddEmptySegment) {
  names[&actors[1]] = "camera";
  parents[&actors[1]] = &actors[0];
  EXPECT_EQ(Topic(1), "rt/carla/camera");
}

TEST_F(Ros2TopicNames, EmptyParentNameDoesNotAddEmptySegment) {
  names[&actors[0]] = "";
  names[&actors[1]] = "camera";
  parents[&actors[1]] = &actors[0];
  EXPECT_EQ(Topic(1), "rt/carla/camera");
}

TEST_F(Ros2TopicNames, EmptyIntermediateAncestorIsSkipped) {
  names[&actors[0]] = "hero1";
  names[&actors[1]] = "";
  names[&actors[2]] = "camera";
  parents[&actors[1]] = &actors[0];
  parents[&actors[2]] = &actors[1];
  EXPECT_EQ(Topic(2), "rt/carla/hero1/camera");
}

TEST_F(Ros2TopicNames, EmptyRootKeepsExistingPrefixBehavior) {
  EXPECT_EQ(Topic(0), "rt/carla/");
  names[&actors[0]] = "";
  EXPECT_EQ(Topic(0), "rt/carla/");
}

TEST_F(Ros2TopicNames, NonemptyNamesAreNotNormalized) {
  names[&actors[0]] = "hero/";
  names[&actors[1]] = "camera";
  parents[&actors[1]] = &actors[0];
  EXPECT_EQ(Topic(1), "rt/carla/hero//camera");
  names[&actors[0]] = "hero";
  names[&actors[1]] = "/camera";
  EXPECT_EQ(Topic(1), "rt/carla/hero//camera");
}

TEST_F(Ros2TopicNames, NamedAncestorsKeepOrder) {
  names[&actors[0]] = "hero";
  names[&actors[1]] = "roof";
  names[&actors[2]] = "lidar/front";
  parents[&actors[1]] = &actors[0];
  parents[&actors[2]] = &actors[1];
  EXPECT_EQ(Topic(2), "rt/carla/hero/roof/lidar/front");
}

} // namespace
