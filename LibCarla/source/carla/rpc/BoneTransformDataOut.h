// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/Transform.h"

#include <string>
#include <utility>

namespace carla {
namespace rpc {

  // using BoneTransformDataOut = std::pair<std::string, geom::Transform>;
  class BoneTransformDataOut
  {
    public:
    std::string bone_name;
    Transform world;
    Transform component;
    Transform relative;
  
    bool operator!=(const BoneTransformDataOut &rhs) const {
      return
      bone_name != rhs.bone_name ||
      world != rhs.world ||
      component != rhs.component ||
      relative != rhs.relative;
    }

    bool operator==(const BoneTransformDataOut &rhs) const {
      return !(*this != rhs);
    }


    MSGPACK_DEFINE_ARRAY(bone_name, world, component, relative);
  };

} // namespace rpc
} // namespace carla
