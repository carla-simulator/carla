// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/ROS2Time.h>

#include <cmath>
#include <cstdint>

namespace {

constexpr std::uint32_t kNanosecondsPerSecond = 1000000000u;
constexpr std::int32_t kSweepSteps = 200000;

}  // namespace

TEST(ros2_time_conversion, whole_seconds_have_zero_nanoseconds) {
  const carla::ros2::msg::Time stamp = carla::ros2::ToRosTime(139.0);
  ASSERT_EQ(stamp.sec, 139);
  ASSERT_EQ(stamp.nanosec, 0u);
}

TEST(ros2_time_conversion, decimal_fraction_139_407) {
  const carla::ros2::msg::Time stamp = carla::ros2::ToRosTime(139.407);
  ASSERT_EQ(stamp.sec, 139);
  ASSERT_EQ(stamp.nanosec, 407000000u);
}

TEST(ros2_time_conversion, truncates_instead_of_rounding) {
  const carla::ros2::msg::Time stamp = carla::ros2::ToRosTime(2.675);
  ASSERT_EQ(stamp.sec, 2);
  ASSERT_EQ(stamp.nanosec, 674999999u);
}

TEST(ros2_time_conversion, nanoseconds_stay_below_one_billion) {
  const double just_below_one = std::nextafter(1.0, 0.0);
  const double just_below_five = std::nextafter(5.0, 0.0);
  ASSERT_LT(carla::ros2::ToRosTime(just_below_one).nanosec, kNanosecondsPerSecond);
  ASSERT_LT(carla::ros2::ToRosTime(just_below_five).nanosec, kNanosecondsPerSecond);

  for (std::int32_t k = 0; k < kSweepSteps; ++k) {
    const double seconds = static_cast<double>(k) * 0.05;
    ASSERT_LT(carla::ros2::ToRosTime(seconds).nanosec, kNanosecondsPerSecond) << "k=" << k;
  }

  const float step = 0.05f;
  double elapsed = 0.0;
  for (std::int32_t k = 0; k < kSweepSteps; ++k) {
    elapsed += static_cast<double>(step);
    ASSERT_LT(carla::ros2::ToRosTime(elapsed).nanosec, kNanosecondsPerSecond) << "k=" << k;
  }
}
