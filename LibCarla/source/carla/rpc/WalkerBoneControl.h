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
      : bone_transforms(bone_transforms) {}

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    explicit WalkerBoneControl(const FWalkerBoneControl& Control) {
      bone_transforms.reserve(Control.BoneTransforms.Num());
      for (const TPair<FString, FTransform> &bone_transform : Control.BoneTransforms) {
        bone_transforms.push_back(rpc::BoneTransformData{TCHAR_TO_UTF8(*bone_transform.Key), bone_transform.Value});
      }
    }

    operator FWalkerBoneControl() const {
      FWalkerBoneControl Control;
      for (auto &bone_transform : bone_transforms) {
        Control.BoneTransforms.Add(ToFString(bone_transform.first), bone_transform.second);
      }
      return Control;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    std::vector<rpc::BoneTransformData> bone_transforms;

    MSGPACK_DEFINE_ARRAY(bone_transforms);
  };

} // namespace rpc
} // namespace carla
