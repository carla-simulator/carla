// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Memory.h"
#include "carla/client/detail/ActorState.h"

namespace carla {
namespace client {

  class Actor
    : public EnableSharedFromThis<Actor>,
      public detail::ActorState {
    using Super = detail::ActorState;
  public:

    explicit Actor(ActorInitializer init) : Super(std::move(init)) {}

    virtual ~Actor() = default;


    geom::Location GetLocation() const;

    geom::Transform GetTransform() const;

    void SetLocation(const geom::Location &location);

    void SetTransform(const geom::Transform &transform);

    const auto &Serialize() const {
      return Super::GetActorDescription();
    }

    bool IsAlive() const {
      return _is_alive;
    }

    virtual void Destroy();

  private:

    bool _is_alive = true;
  };

} // namespace client
} // namespace carla
