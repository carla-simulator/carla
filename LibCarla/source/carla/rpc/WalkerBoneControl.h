// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/rpc/Transform.h"
#include "carla/rpc/BoneTransformData.h"
#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include "Carla/Walker/WalkerBoneControl.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  class WalkerBoneControl {
  public:

    WalkerBoneControl() = default;

    std::vector<rpc::BoneTransformData> bone_transforms;
    WalkerBoneControl(
        std::vector<rpc::BoneTransformData> bone_transforms)
      : bone_transforms(bone_transforms) {}

    void SetBoneTransformData(std::vector<rpc::BoneTransformData> &in_bone_Tranform_data) {
      bone_transforms = in_bone_Tranform_data;
    }

    const std::vector<rpc::BoneTransformData> &GetBoneTransformData() const {
      return bone_transforms;
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    operator FWalkerBoneControl() const {
      FWalkerBoneControl Control;
      for (auto bone_transform : bone_transforms) {
        Control.BoneTransforms.Add(bone_transform.first.c_str(), bone_transform.second);
      }
      return Control;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    template <typename Map>
    bool map_compare(Map const &lhs, Map const &rhs) {
      // No predicate needed because there is operator== for pairs already.
      return lhs.size() == rhs.size() &&
             std::equal(lhs.begin(), lhs.end(),
          rhs.begin());
    }

    bool operator!=(const WalkerBoneControl &rhs) const {
      if (bone_transforms.size() != rhs.bone_transforms.size()) {
        return true;
      }
      for (uint32_t i = 0; i < bone_transforms.size(); ++i) {
        if (bone_transforms[i].first.compare(rhs.bone_transforms[i].first) != 0 ||
            bone_transforms[i].second != rhs.bone_transforms[i].second) {
          return true;
        }
      }
      return false;
    }

    bool operator==(const WalkerBoneControl &rhs) const {
      return !(*this != rhs);
    }

    MSGPACK_DEFINE_ARRAY(bone_transforms);
  };

} // namespace rpc
} // namespace carla
