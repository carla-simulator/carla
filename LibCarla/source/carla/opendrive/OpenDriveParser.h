// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/road/Map.h"

#include <string>

namespace carla {
namespace opendrive {

  class OpenDriveParser {
  public:

    static SharedPtr<road::Map> Load(const std::string &opendrive);

  };

} // namespace opendrive
} // namespace carla
