// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/subscribers/AckermannControlSubscriber.h>
#include <carla/ros2/subscribers/CarlaEgoVehicleControlSubscriber.h>

#include <type_traits>

// The ROS 2 control subscribers are polymorphic and always owned through a
// std::shared_ptr<BaseSubscriber>, so copying or moving one would slice the derived
// object. BaseSubscriber deliberately = deletes its copy/move members; these concrete
// subscribers are non-abstract, so the assertions below only hold while that contract
// propagates. A revert of the base back to = default would make them copyable again and
// trip this test at build and run time.
static_assert(
    !std::is_copy_constructible_v<carla::ros2::AckermannControlSubscriber>,
    "AckermannControlSubscriber must stay non-copyable to avoid slicing");
static_assert(
    !std::is_move_constructible_v<carla::ros2::AckermannControlSubscriber>,
    "AckermannControlSubscriber must stay non-movable to avoid slicing");
static_assert(
    !std::is_copy_constructible_v<carla::ros2::CarlaEgoVehicleControlSubscriber>,
    "CarlaEgoVehicleControlSubscriber must stay non-copyable to avoid slicing");
static_assert(
    !std::is_move_constructible_v<carla::ros2::CarlaEgoVehicleControlSubscriber>,
    "CarlaEgoVehicleControlSubscriber must stay non-movable to avoid slicing");

TEST(ros2_subscriber_lifetime, control_subscribers_are_non_copyable) {
  EXPECT_FALSE(std::is_copy_constructible_v<carla::ros2::AckermannControlSubscriber>);
  EXPECT_FALSE(std::is_move_constructible_v<carla::ros2::AckermannControlSubscriber>);
  EXPECT_FALSE(std::is_copy_constructible_v<carla::ros2::CarlaEgoVehicleControlSubscriber>);
  EXPECT_FALSE(std::is_move_constructible_v<carla::ros2::CarlaEgoVehicleControlSubscriber>);
}
