// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/LidarRawData.h"

#include <iostream>

namespace carla {
namespace sensor {
namespace s11n {


  void WriteDetection(std::ostream& out, const LidarRawDetection& detection) {
    out << detection.point.x << ' ' << detection.point.y << ' ' << detection.point.z << ' '
        << detection.cos_inc_angle << ' ' << detection.object_idx << ' ' << detection.object_tag;
  }

  void WriteHeader(std::ostream& out, const LidarRawDetection& detection) {
    static_assert(sizeof(LidarRawDetection) == 6u*sizeof(float), "The header is not compatible \
        with LidarRawDetection");

    out << detection.point.x << ' ' << detection.point.y << ' ' << detection.point.z << ' '
        << detection.cos_inc_angle << ' ' << detection.object_idx << ' ' << detection.object_tag;
  }

  void WriteDetection(std::ostream& out, const LidarDetection& detection) {
    out << detection.point.x << ' ' << detection.point.y << ' ' << detection.point.z << ' '
        << detection.intensity;
  }
/*
  void WriteHeader(std::ostream& out, const LidarDetection& detection) {
    static_assert(sizeof(LidarRawDetection) == 6u*sizeof(float), "The header is not compatible \
        with LidarDetection");

    out << detection.point.x << ' ' << detection.point.y << ' ' << detection.point.z << ' '
        << detection.cos_inc_angle << ' ' << detection.object_idx << ' ' << detection.object_tag;
  }
*/

} // namespace s11n
} // namespace sensor
} // namespace carla
