// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/FileSystem.h"

#include <fstream>
#include <iterator>

namespace carla {
namespace eventarray {

  class EventArrayIO {
  public:

    template <typename PointIt>
    static void Dump(std::ostream &out, PointIt begin, PointIt end) {
      DEBUG_ASSERT(std::distance(begin, end) >= 0);
      WriteHeader(out, static_cast<size_t>(std::distance(begin, end)));
      for (; begin != end; ++begin) {
        out << begin->x << ' ' << begin->y << ' ' << begin->t << ' ' << begin->pol << '\n';
      }
    }

    template <typename PointIt>
    static std::string SaveToDisk(std::string path, PointIt begin, PointIt end) {
      FileSystem::ValidateFilePath(path, ".txt");
      std::ofstream out(path);
      Dump(out, begin, end);
      return path;
    }

  private:

    static void WriteHeader(std::ostream &out, size_t number_of_events);
  };

} // namespace eventarray
} // namespace carla
