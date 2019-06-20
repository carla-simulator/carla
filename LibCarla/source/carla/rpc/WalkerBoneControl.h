// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/rpc/BoneTransformData.h"
#include "carla/rpc/String.h"
#include "carla/rpc/Transform.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Carla/Walker/WalkerBoneControl.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

#include <vector>

namespace carla {
namespace rpc {

  class WalkerBoneControl {
  public:

    WalkerBoneControl() = default;

    explicit WalkerBoneControl(
        std::vector<rpc::BoneTransformData> bone_transforms)
      : _bone_transforms(bone_transforms) {}

    void SetBoneTransformData(std::vector<rpc::BoneTransformData> &bone_tranform_data) {
      _bone_transforms = bone_tranform_data;
    }

    const std::vector<rpc::BoneTransformData> &GetBoneTransformData() const {
      return _bone_transforms;
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    operator FWalkerBoneControl() const {
      FWalkerBoneControl Control;
      for (auto &bone_transform : _bone_transforms) {
        Control.BoneTransforms.Add(ToFString(bone_transform.first), bone_transform.second);
      }
      return Control;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

  private:

    std::vector<rpc::BoneTransformData> _bone_transforms;

  public:

    MSGPACK_DEFINE_ARRAY(_bone_transforms);
  };

} // namespace rpc
} // namespace carla
