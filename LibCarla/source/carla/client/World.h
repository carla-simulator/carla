// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/NonCopyable.h"
#include "carla/client/Client.h"
#include "carla/client/Memory.h"

namespace carla {
namespace client {

  class Actor;

  class World
    : public EnableSharedFromThis<World>,
      private NonCopyable {
  public:

    SharedPtr<BlueprintLibrary> GetBlueprintLibrary() const {
      return _parent->GetBlueprintLibrary();
    }

    SharedPtr<Actor> GetSpectator() const {
      return _parent->GetSpectator();
    }

    SharedPtr<Actor> TrySpawnActor(
        const ActorBlueprint &blueprint,
        const Transform &transform,
        Actor *parent = nullptr);

    SharedPtr<Actor> SpawnActor(
        const ActorBlueprint &blueprint,
        const Transform &transform,
        Actor *parent = nullptr) {
      return _parent->SpawnActor(blueprint, transform, parent);
    }

    Client &GetClient() const {
      DEBUG_ASSERT(_parent != nullptr);
      return *_parent;
    }

  private:

    friend class Client;

    explicit World(SharedPtr<Client> parent)
      : _parent(std::move(parent)) {
      DEBUG_ASSERT(_parent != nullptr);
    }

    SharedPtr<Client> _parent;
  };

} // namespace client
} // namespace carla
