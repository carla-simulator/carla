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

    const std::string &GetDisplayId() const {
      return _display_id;
    }

    World GetWorld() const {
      return _episode;
    }

  protected:

    const rpc::Actor &GetActorDescription() const {
      return _description;
    }

    Episode &GetEpisode() {
      return _episode;
    }

    const Episode &GetEpisode() const {
      return _episode;
    }

  private:

    friend class detail::Client;

    ActorState(rpc::Actor description, Episode episode);

    rpc::Actor _description;

    Episode _episode;

    std::string _display_id;
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
