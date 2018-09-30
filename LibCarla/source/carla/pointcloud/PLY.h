// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/OutputFile.h"

#include <iterator>

namespace carla {
namespace pointcloud {

  class PLY {
  public:

    template <typename PointIt>
    static void SaveToDisk(const std::string &path, PointIt begin, PointIt end) {
      OutputFile out(path, ".ply");
      WriteHeader(out, std::distance(begin, end));
      for (; begin != end; ++begin) {
        out << begin->x << ' ' << begin->y << ' ' << begin->z << '\n';
      }
    }

  private:

    static void WriteHeader(OutputFile &out, size_t number_of_points);
  };

} // namespace pointcloud
} // namespace carla
