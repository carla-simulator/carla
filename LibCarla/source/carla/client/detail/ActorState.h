// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/client/World.h"
#include "carla/rpc/Actor.h"

namespace carla {
namespace client {
namespace detail {

  class ActorFactory;

  class ActorState : private MovableNonCopyable {
  public:

    auto GetId() const {
      return _description.id;
    }

    const std::string &GetTypeId() const {
      return _description.description.id;
    }

    std::string GetDisplayId() const;

    World GetWorld() const {
      return _parent;
    }

  protected:

    const rpc::Actor &GetActorDescription() const {
      return _description;
    }

  private:

    ActorState(rpc::Actor description, World parent)
      : _description(std::move(description)),
        _parent(std::move(parent)) {}

    rpc::Actor _description;

    World _parent;
  };

} // namespace detail

  class ActorInitializer : public detail::ActorState {
  public:
    ActorInitializer(ActorInitializer &&) = default;
  private:
    friend class detail::ActorFactory;
    using detail::ActorState::ActorState;
  };

} // namespace client
} // namespace carla
