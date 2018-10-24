// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

namespace carla {
namespace rpc {

  class MapInfo {
  public:

    std::string name;

    std::string open_drive_file;

    MSGPACK_DEFINE_ARRAY(name, open_drive_file);
  };

} // namespace rpc
} // namespace carla
