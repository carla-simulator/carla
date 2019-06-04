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
#  include "Carla/Walker/WalkerBoneControl.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  class WalkerBoneControl {
  public:

    WalkerBoneControl() = default;

    bool modify_bones;

    geom::Location world_position;

    geom::Rotation world_rotation;

    std::vector<rpc::BoneTransformData> bone_transforms;
    WalkerBoneControl(
        bool modify_bones,
        geom::Location world_position,
        geom::Rotation world_rotation,
        std::vector<rpc::BoneTransformData> bone_transforms)
      : modify_bones(modify_bones),
        world_position(world_position),
        world_rotation(world_rotation),
        bone_transforms(bone_transforms) {}

    void SetBoneTransformData(std::vector<rpc::BoneTransformData> &in_bone_Tranform_data) {
      bone_transforms = in_bone_Tranform_data;
    }

    const std::vector<rpc::BoneTransformData> &GetBoneTransformData() const {
      return bone_transforms;
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    // WalkerBoneControl(const FWalkerBoneControl &Control)
    //   : modify_bones(Control.bModifyBones),
    //     world_position(Control.WorldPosition.x, Control.WorldPosition.y,
    // Control.WorldPosition.z),
    //     world_rotation(Control.WorldRotation.x, Control.WorldRotation.y,
    // Control.WorldRotation.z) {
    //       bone_transforms
    //       for (const TPair<FString, FTransform>& pair :
    // Control.BoneTransforms)
    //       {
    //         geom::Location tmp_location =
    // geom::Location(Vector3D((FTransform)pair.Value))
    //         geom::Transform tmp_transform =
    //         std::pair<std::string, geom::Tranform> tmp_pair =
    // std::make_pair(pair.Key);
    //       }
    //     }

    operator FWalkerBoneControl() const {
      FWalkerBoneControl Control;
      Control.bModifyBones = modify_bones;
      Control.WorldPosition = {world_position.x, world_position.y, world_position.z};
      Control.WorldRotation = {world_rotation.yaw, world_rotation.pitch, world_rotation.roll};
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
      return modify_bones != rhs.modify_bones;
    }

    bool operator==(const WalkerBoneControl &rhs) const {
      return !(*this != rhs);
    }

    MSGPACK_DEFINE_ARRAY(modify_bones, world_position, world_rotation, bone_transforms);
  };

} // namespace rpc
} // namespace carla
