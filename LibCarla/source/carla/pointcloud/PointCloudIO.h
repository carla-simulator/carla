// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/FileSystem.h"

#include <fstream>
#include <iterator>
#include <iomanip>

namespace carla {
namespace pointcloud {

  class PointCloudIO {

  public:
    template <typename PointIt>
    static void Dump(std::ostream &out, PointIt begin, PointIt end) {
      WriteHeader(out, begin, end);
      for (; begin != end; ++begin) {
        begin->WriteDetection(out);
        out << '\n';
      }
    }

    template <typename PointIt>
    static std::string SaveToDisk(std::string path, PointIt begin, PointIt end) {
      FileSystem::ValidateFilePath(path, ".ply");
      std::ofstream out(path);
      Dump(out, begin, end);
      return path;
    }

  private:
    template <typename PointIt> static void WriteHeader(std::ostream &out, PointIt begin, PointIt end) {
      DEBUG_ASSERT(std::distance(begin, end) >= 0);
      out << "ply\n"
           "format ascii 1.0\n"
           "element vertex " << std::to_string(static_cast<size_t>(std::distance(begin, end))) << "\n";
      begin->WritePlyHeaderInfo(out);
      out << "\nend_header\n";
      out << std::fixed << std::setprecision(4u);
    }
  };

} // namespace pointcloud
} // namespace carla
