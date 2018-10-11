// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/Map.h"

#include <istream>
#include <ostream>

namespace carla {
namespace opendrive {

    class OpenDrive {
    public:

      static const road::Map &Load(std::istream &input);
      static const road::Map &Load(const std::string &file);

      static void Dump(const road::Map &map, std::ostream &output);
    };

} // namespace opendrive
} // namespace carla
