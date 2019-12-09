// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/pointcloud/PointCloudIO.h"

#include <iomanip>

namespace carla {
namespace pointcloud {

  void PointCloudIO::WriteHeader(std::ostream &out, size_t number_of_points) {
    out << "ply\n"
           "format ascii 1.0\n"
           "element vertex " << std::to_string(number_of_points) << "\n"
           "property float32 x\n"
           "property float32 y\n"
           "property float32 z\n"
           // "property uchar diffuse_red\n"
           // "property uchar diffuse_green\n"
           // "property uchar diffuse_blue\n"
           "end_header\n";
    out << std::fixed << std::setprecision(4u);
  }

} // namespace pointcloud
} // namespace carla
