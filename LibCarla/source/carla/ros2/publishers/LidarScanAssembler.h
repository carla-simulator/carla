// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <utility>
#include <vector>

namespace carla {
namespace ros2 {

// One simulation step of a rotating ray-cast lidar, as SimulateLidar cast it.
struct LidarSliceInfo {
  // Sim time of the step [s]: the ROS stamp of the step.
  double stamp{0.0};
  // Length of the step [s].
  double delta_time{0.0};
  // Span of the ray times within the step [s]: the rays are spread over
  // [stamp - time_span, stamp] in sweep order. 0 when every ray is cast from
  // the sensor pose at the stamp.
  double time_span{0.0};
  // Sweep position of the first ray [deg, 0..360): CARLA's horizontal angle at
  // the start of the step (0 = ray along -X of the sensor).
  float start_angle{0.0f};
  // Horizontal angle swept during the step [deg].
  float sweep_angle{0.0f};
  // Rays cast per channel during the step.
  std::uint32_t rays_per_channel{0u};
};

// Driver-like output of a rotating ray-cast lidar (ros_driver_mode).
struct LidarDriverOptions {
  bool enabled{false};
  // Azimuth where a rotation starts [deg], in the ROS sensor frame
  // (counter-clockwise from +X): the cut angle of a lidar driver.
  float cut_angle{180.0f};
  LidarSliceInfo slice{};
};

// Assembles the per-step slices of a rotating ray-cast lidar into full
// rotations, as a lidar driver publishes them: one cloud per rotation, cut at a
// fixed azimuth, points in firing order (sweep position, then channel), stamped
// at the first point with a per-point time_stamp offset [ns] from that stamp.
//
// PointT needs float x, y (ROS sensor frame), an integral channel and a uint32
// time_stamp. The sensor turns clockwise seen from above (CARLA's sweep
// direction): the ROS azimuth atan2(y, x) decreases with time.
template <typename PointT>
class LidarScanAssembler {
public:
  struct Scan {
    double stamp{0.0};  // time of the first point [s]
    std::vector<PointT> points;
  };

  explicit LidarScanAssembler(float cut_angle_deg)
    : _cut(Wrap360(180.0 - static_cast<double>(cut_angle_deg))) {}

  // Adds the points of one step (any order). Returns the rotation completed by
  // this step, if any; the points past the cut start the next one.
  std::optional<Scan> Add(const LidarSliceInfo &slice, std::vector<PointT> points) {
    std::optional<Scan> completed;
    if (!(slice.sweep_angle > 0.0f) || slice.rays_per_channel == 0u) {
      return completed;
    }
    // A step back in time (episode reload) or a gap of more than two rotations
    // (sensor paused): drop the partial rotation and start again.
    const double max_gap = 2.0 * 360.0 / slice.sweep_angle * slice.delta_time;
    if (_has_slice && (slice.stamp <= _last_stamp || slice.stamp - _last_stamp > max_gap)) {
      Reset();
    }

    // Unwrapped sweep position of the step start, continuous across steps.
    const double start = slice.start_angle;
    double base = start;
    if (_has_slice) {
      base = _last_end + std::remainder(start - _last_end, 360.0);
    }
    const double sweep = slice.sweep_angle;
    const double ray_step = sweep / slice.rays_per_channel;
    const std::uint64_t sequence = _sequence++;

    std::vector<Entry> entries;
    std::vector<std::int64_t> rotations;
    entries.reserve(points.size());
    rotations.reserve(points.size());
    for (auto &point : points) {
      // CARLA casts the ray at yaw (sweep position - 180 deg) in its
      // left-handed frame, i.e. at ROS azimuth (180 deg - sweep position).
      const double sweep_pos = Wrap360(180.0 - Degrees(std::atan2(point.y, point.x)));
      double phase = Wrap360(sweep_pos - start);
      if (phase > sweep + 0.5 * (360.0 - sweep)) {
        phase -= 360.0;  // a hair before the step start (float noise)
      }
      const auto ray = static_cast<std::int64_t>(std::llround(phase / ray_step));
      const auto ray_index = static_cast<std::uint32_t>(
          std::clamp<std::int64_t>(ray, 0, static_cast<std::int64_t>(slice.rays_per_channel) - 1));
      Entry entry;
      entry.key = (sequence << 32) | ray_index;
      entry.time = slice.stamp - slice.time_span
          + slice.time_span * (ray_index + 1.0) / slice.rays_per_channel;
      entry.point = std::move(point);
      entries.push_back(std::move(entry));
      // Rotation from the ray's nominal position, not the noisy one: a ray on the cut belongs to the new rotation
      const double position = base + ray_index * ray_step;
      rotations.push_back(static_cast<std::int64_t>(std::floor((position - _cut) / 360.0 + 1e-9)));
    }
    if (!_has_rotation && !rotations.empty()) {
      _rotation = *std::min_element(rotations.begin(), rotations.end());
      _has_rotation = true;
    }

    std::vector<Entry> current;
    std::vector<Entry> next;
    std::int64_t next_rotation = _rotation;
    current.reserve(entries.size());
    for (std::size_t i = 0; i < entries.size(); ++i) {
      if (rotations[i] > _rotation) {
        next_rotation = next.empty() ? rotations[i] : std::min(next_rotation, rotations[i]);
        next.push_back(std::move(entries[i]));
      } else {
        current.push_back(std::move(entries[i]));
      }
    }

    _pending.insert(
        _pending.end(),
        std::make_move_iterator(current.begin()),
        std::make_move_iterator(current.end()));
    if (!next.empty()) {
      if (!_pending.empty()) {
        completed = Emit();
      }
      _pending = std::move(next);
      _rotation = next_rotation;
    }

    _last_stamp = slice.stamp;
    _last_end = base + sweep;
    _has_slice = true;
    return completed;
  }

  void Reset() {
    _pending.clear();
    _has_slice = false;
    _has_rotation = false;
  }

private:
  struct Entry {
    std::uint64_t key{0u};  // step sequence (high 32 bits), ray index (low 32 bits)
    double time{0.0};
    PointT point{};
  };

  static double Degrees(double radians) {
    return radians * (180.0 / 3.14159265358979323846);
  }

  static double Wrap360(double degrees) {
    const double wrapped = std::fmod(degrees, 360.0);
    return wrapped < 0.0 ? wrapped + 360.0 : wrapped;
  }

  Scan Emit() {
    std::vector<std::size_t> order(_pending.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::stable_sort(order.begin(), order.end(), [this](std::size_t a, std::size_t b) {
      const Entry &ea = _pending[a];
      const Entry &eb = _pending[b];
      return ea.key != eb.key ? ea.key < eb.key : ea.point.channel < eb.point.channel;
    });

    Scan scan;
    scan.stamp = _pending[order.front()].time;
    for (const Entry &entry : _pending) {
      scan.stamp = std::min(scan.stamp, entry.time);
    }
    scan.points.reserve(_pending.size());
    for (std::size_t index : order) {
      Entry &entry = _pending[index];
      const double offset = std::max(0.0, entry.time - scan.stamp);
      entry.point.time_stamp = static_cast<std::uint32_t>(std::llround(offset * 1e9));
      scan.points.push_back(std::move(entry.point));
    }
    _pending.clear();
    return scan;
  }

  // Cut angle as a sweep position [deg, 0..360).
  double _cut;
  std::vector<Entry> _pending;
  std::uint64_t _sequence{0u};
  std::int64_t _rotation{0};
  bool _has_rotation{false};
  double _last_stamp{0.0};
  double _last_end{0.0};
  bool _has_slice{false};
};

}  // namespace ros2
}  // namespace carla
