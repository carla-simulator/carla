// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/rpc/WalkerControl.h"
#include "carla/rpc/WalkerBoneControl.h"

namespace carla {
namespace client {

  class Walker : public Actor {
  public:

    using Control = rpc::WalkerControl;
    using BoneControl = rpc::WalkerBoneControl;

    explicit Walker(ActorInitializer init) : Actor(std::move(init)) {}

    /// Apply @a control to this Walker.
    void ApplyControl(const Control &control);

    void ApplyControl(const BoneControl &bone_control);

    /// Return the control last applied to this Walker.
    ///
    /// @note This function does not call the simulator, it returns the Control
    /// received in the last tick.
    Control GetWalkerControl() const;

    /// Return the bone positions applied to this Walker by calling the simulator.
    BoneControl GetWalkerBoneState();

  private:

    Control _control;
  };

} // namespace client
} // namespace carla
