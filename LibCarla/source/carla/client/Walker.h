// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/rpc/WalkerControl.h"
#include "carla/rpc/WalkerBoneControlIn.h"
#include "carla/rpc/WalkerBoneControlOut.h"

namespace carla {
namespace client {

  class Walker : public Actor {
  public:

    using Control = rpc::WalkerControl;
    using BoneControlIn = rpc::WalkerBoneControlIn;
    using BoneControlOut = rpc::WalkerBoneControlOut;

    explicit Walker(ActorInitializer init) : Actor(std::move(init)) {}

    /// Apply @a control to this Walker.
    void ApplyControl(const Control &control);

    /// Return the control last applied to this Walker.
    ///
    /// @note This function does not call the simulator, it returns the Control
    /// received in the last tick.
    Control GetWalkerControl() const;

    BoneControlOut GetBonesTransform();
    void SetBonesTransform(const BoneControlIn &bones);
    void BlendPose(float blend);
    void ShowPose() { BlendPose(1.0f); };
    void HidePose() { BlendPose(0.0f); };
    void GetPoseFromAnimation();

  private:

    Control _control;
  };

} // namespace client
} // namespace carla
