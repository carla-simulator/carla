// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/client/World.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/rpc/Actor.h"

namespace carla {
namespace client {
namespace detail {

  class ActorFactory;

  /// Internal state of an Actor.
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

    const geom::BoundingBox &GetBoundingBox() const {
      return _description.bounding_box;
    }

    const std::vector<uint8_t> &GetSemanticTags() const {
      return _description.semantic_tags;
    }

    SharedPtr<Actor> GetParent() const {
      return _parent;
    }

    World GetWorld() const {
      return World{_episode};
    }

  protected:

    const rpc::Actor &GetActorDescription() const {
      return _description;
    }

    EpisodeProxy &GetEpisode() {
      return _episode;
    }

    const EpisodeProxy &GetEpisode() const {
      return _episode;
    }

  private:

    friend class Simulator;

    ActorState(
        rpc::Actor description,
        EpisodeProxy episode,
        SharedPtr<Actor> parent);

    rpc::Actor _description;

    EpisodeProxy _episode;

    SharedPtr<Actor> _parent;

    std::string _display_id;
  };

} // namespace detail

  /// Used to initialize Actor classes. Only the ActorFactory can create this
  /// object, thus only the ActorFactory can create actors.
  class ActorInitializer : public detail::ActorState {
  public:
    ActorInitializer(ActorInitializer &&) = default;
  private:
    friend class detail::ActorFactory;
    using detail::ActorState::ActorState;
  };

} // namespace client
} // namespace carla
