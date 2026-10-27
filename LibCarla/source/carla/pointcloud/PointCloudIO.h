// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/FileSystem.h"
#include "carla/Exception.h"

#include <fstream>
#include <iterator>
#include <iomanip>
#include <stdexcept>
#include <type_traits>

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
      if (!out) {
        throw_exception(std::runtime_error("Failed to open point cloud output: " + path));
      }
      Dump(out, begin, end);
      out.close();
      if (!out) {
        throw_exception(std::runtime_error("Failed to write point cloud output: " + path));
      }
      return path;
    }

  private:
    template <typename PointIt> static void WriteHeader(std::ostream &out, PointIt begin, PointIt end) {
      DEBUG_ASSERT(std::distance(begin, end) >= 0);
      out << "ply\n"
           "format ascii 1.0\n"
           "element vertex " << std::to_string(static_cast<size_t>(std::distance(begin, end))) << "\n";
      using Detection = typename std::iterator_traits<PointIt>::value_type;
      if constexpr (requires { Detection::WritePlyHeaderInfo(out); }) {
        Detection::WritePlyHeaderInfo(out);
      } else if (begin != end) {
        // Retain support for custom point types with instance header methods.
        begin->WritePlyHeaderInfo(out);
      } else if constexpr (std::is_default_constructible_v<Detection>) {
        Detection{}.WritePlyHeaderInfo(out);
      } else {
        throw_exception(std::invalid_argument(
            "An empty point cloud requires an instance-independent PLY schema"));
      }
      out << "\nend_header\n";
      out << std::fixed << std::setprecision(4u);
    }
  };

} // namespace pointcloud
} // namespace carla
