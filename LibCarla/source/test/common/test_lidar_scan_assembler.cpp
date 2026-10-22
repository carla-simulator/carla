// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/publishers/LidarScanAssembler.h>

#include <cmath>
#include <cstdint>
#include <vector>

namespace {

struct TestPoint {
  float x{0.0f};
  float y{0.0f};
  std::uint16_t channel{0u};
  std::uint32_t time_stamp{0u};
};

using Assembler = carla::ros2::LidarScanAssembler<TestPoint>;
using carla::ros2::LidarSliceInfo;

constexpr double kPi = 3.14159265358979323846;

// One step of a CARLA ray-cast lidar: `rays` rays per channel from sweep
// position `start` over `sweep` degrees, channel-major as SimulateLidar
// serializes them, in the ROS frame (y mirrored).
std::vector<TestPoint> MakeSlice(double start, double sweep, std::uint32_t rays, std::uint16_t channels) {
  std::vector<TestPoint> points;
  for (std::uint16_t channel = 0u; channel < channels; ++channel) {
    for (std::uint32_t i = 0u; i < rays; ++i) {
      const double sweep_pos = std::fmod(start + sweep * i / rays, 360.0);
      const double yaw = (sweep_pos - 180.0) * kPi / 180.0;  // UE yaw of the ray
      TestPoint point;
      point.x = static_cast<float>(10.0 * std::cos(yaw));
      point.y = static_cast<float>(-10.0 * std::sin(yaw));
      point.channel = channel;
      points.push_back(point);
    }
  }
  return points;
}

LidarSliceInfo MakeInfo(int step, double dt, double start, double sweep, std::uint32_t rays, double span = 0.0) {
  LidarSliceInfo info;
  info.stamp = 100.0 + step * dt;
  info.delta_time = dt;
  info.time_span = span;
  info.start_angle = static_cast<float>(start);
  info.sweep_angle = static_cast<float>(sweep);
  info.rays_per_channel = rays;
  return info;
}

// Sweep position [deg] of a ROS-frame point, unwrapped from `from`.
double SweepPos(const TestPoint &point, double from) {
  const double pos = 180.0 - std::atan2(point.y, point.x) * 180.0 / kPi;
  return from + std::fmod(std::fmod(pos - from, 360.0) + 360.0, 360.0);
}

}  // namespace

// 10 Hz sensor, 50 Hz simulation: five steps of 72 deg per rotation; with the
// default cut (behind the sensor, sweep position 0) and the sweep starting at
// 0, every fifth step completes a rotation.
TEST(LidarScanAssembler, full_rotations_in_firing_order) {
  constexpr double dt = 0.02;
  constexpr double sweep = 72.0;
  constexpr std::uint32_t rays = 100u;
  constexpr std::uint16_t channels = 4u;
  Assembler assembler(180.0f);
  std::vector<Assembler::Scan> scans;
  double start = 0.0;
  for (int step = 0; step < 16; ++step) {
    auto scan = assembler.Add(MakeInfo(step, dt, start, sweep, rays), MakeSlice(start, sweep, rays, channels));
    if (scan.has_value()) {
      scans.push_back(std::move(*scan));
    }
    start = std::fmod(start + sweep, 360.0);
  }
  ASSERT_EQ(scans.size(), 3u);
  for (std::size_t k = 0; k < scans.size(); ++k) {
    const auto &scan = scans[k];
    ASSERT_EQ(scan.points.size(), 5u * rays * channels);
    // Stamped at the first point: the first step of the rotation.
    EXPECT_NEAR(scan.stamp, 100.0 + 5.0 * k * dt, 1e-9);
    // Firing order: per ray all channels, sweep position growing.
    double previous = SweepPos(scan.points.front(), -1.0);
    EXPECT_NEAR(previous, 0.0, 1e-3);
    for (std::size_t i = 0; i < scan.points.size(); ++i) {
      const auto &point = scan.points[i];
      EXPECT_EQ(point.channel, i % channels);
      const double pos = SweepPos(point, previous - 1.0);
      EXPECT_GE(pos, previous - 1e-3);
      previous = pos;
      // Every ray of a step is cast at its stamp.
      EXPECT_EQ(point.time_stamp, static_cast<std::uint32_t>(std::llround((i / (rays * channels)) * dt * 1e9)));
    }
    EXPECT_LT(previous, 360.0);
  }
}

// A cut that falls inside a step splits it between two rotations.
TEST(LidarScanAssembler, cut_inside_a_step) {
  constexpr double dt = 0.02;
  constexpr double sweep = 72.0;
  constexpr std::uint32_t rays = 72u;
  Assembler assembler(90.0f);  // ROS azimuth 90 deg (left): sweep position 90
  std::vector<Assembler::Scan> scans;
  double start = 0.0;
  for (int step = 0; step < 11; ++step) {
    auto scan = assembler.Add(MakeInfo(step, dt, start, sweep, rays), MakeSlice(start, sweep, rays, 1u));
    if (scan.has_value()) {
      scans.push_back(std::move(*scan));
    }
    start = std::fmod(start + sweep, 360.0);
  }
  // First rotation: sweep 0..90 (partial), then full ones from 90.
  ASSERT_GE(scans.size(), 2u);
  EXPECT_EQ(scans[0].points.size(), 90u);
  EXPECT_EQ(scans[1].points.size(), 360u);
  EXPECT_NEAR(SweepPos(scans[1].points.front(), 0.0), 90.0, 1e-3);
  EXPECT_NEAR(SweepPos(scans[1].points.back(), 90.0), 449.0, 1e-3);
  // The step holding sweep 72..144 started at stamp 100.02.
  EXPECT_NEAR(scans[1].stamp, 100.02, 1e-9);
}

// Rays spread over the step: per-ray times, header at the first ray.
TEST(LidarScanAssembler, per_ray_times) {
  constexpr double dt = 0.02;
  constexpr double sweep = 72.0;
  constexpr std::uint32_t rays = 10u;
  Assembler assembler(180.0f);
  std::optional<Assembler::Scan> scan;
  double start = 0.0;
  for (int step = 0; step < 6 && !scan.has_value(); ++step) {
    scan = assembler.Add(MakeInfo(step, dt, start, sweep, rays, dt), MakeSlice(start, sweep, rays, 1u));
    start = std::fmod(start + sweep, 360.0);
  }
  ASSERT_TRUE(scan.has_value());
  ASSERT_EQ(scan->points.size(), 50u);
  // First ray of step 0 at 100 - dt + dt / rays.
  EXPECT_NEAR(scan->stamp, 100.0 - dt + dt / rays, 1e-9);
  for (std::size_t i = 0; i < scan->points.size(); ++i) {
    EXPECT_NEAR(scan->points[i].time_stamp, i * dt / rays * 1e9, 1.0);
  }
}

// Time going back (episode reload) drops the partial rotation.
TEST(LidarScanAssembler, reset_on_time_jump) {
  constexpr double dt = 0.02;
  constexpr double sweep = 72.0;
  constexpr std::uint32_t rays = 10u;
  Assembler assembler(180.0f);
  double start = 0.0;
  for (int step = 0; step < 3; ++step) {
    EXPECT_FALSE(assembler.Add(MakeInfo(step, dt, start, sweep, rays), MakeSlice(start, sweep, rays, 1u)).has_value());
    start = std::fmod(start + sweep, 360.0);
  }
  // Reload: stamps restart, sweep restarts at 0.
  std::optional<Assembler::Scan> scan;
  start = 0.0;
  for (int step = -1000; step < -994 && !scan.has_value(); ++step) {
    scan = assembler.Add(MakeInfo(step, dt, start, sweep, rays), MakeSlice(start, sweep, rays, 1u));
    start = std::fmod(start + sweep, 360.0);
  }
  ASSERT_TRUE(scan.has_value());
  EXPECT_EQ(scan->points.size(), 50u);
  EXPECT_NEAR(scan->stamp, 100.0 - 1000 * dt, 1e-9);
}
