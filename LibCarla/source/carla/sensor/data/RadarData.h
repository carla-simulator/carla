// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <vector>
#include <cstdio>

namespace carla {
namespace sensor {

namespace s11n {
  class RadarSerializer;
}

namespace data {

  struct RadarDetection {
    float velocity; // m/s
    float azimuth;  // rad
    float altitude; // rad
    float depth;    // m
  };

  class RadarData {
    static_assert(sizeof(float) == sizeof(uint32_t), "Invalid float size");
    static_assert(sizeof(float) * 4 == sizeof(RadarDetection), "Invalid RadarDetection size");

  public:
    explicit RadarData() = default;

    constexpr static auto detection_size = sizeof(RadarDetection);

    RadarData &operator=(RadarData &&) = default;

    /// Set a new resolution for the RadarData.
    /// Allocates / Deallocates space in memory if needed.
    ///
    /// @warning This is expensive, not to be called each tick!
    void SetResolution(uint32_t resolution) {
      // Remove the capacity of the vector to zero
      _detections.clear();
      _detections.shrink_to_fit();
      // Set the new vector's capacity
      _detections.reserve(resolution);
    }

    /// Returns the number of current detections.
    size_t GetDetectionCount() const {
      return _detections.size();
    }

    /// Deletes the current detections.
    /// It doesn't change the resolution nor the allocated memory.
    void Reset() {
      _detections.clear();
    }

    /// Adds a new detection.
    void WriteDetection(RadarDetection detection) {
      _detections.push_back(detection);
    }

  private:
    std::vector<RadarDetection> _detections;

  friend class s11n::RadarSerializer;
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
