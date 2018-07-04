// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/Version.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Control.h"
#include "carla/client/Memory.h"
#include "carla/client/Transform.h"
#include "carla/rpc/Client.h"

#include <string>

namespace carla {
namespace client {

  class Actor;
  class ActorBlueprint;
  class World;

  class Client
    : public EnableSharedFromThis<Client>,
      private NonCopyable {
  public:

    template <typename ... Args>
    explicit Client(Args && ... args)
      : _client(std::forward<Args>(args) ...) {
      SetTimeout(10'000);
    }

    void SetTimeout(int64_t milliseconds) {
      _client.set_timeout(milliseconds);
    }

    template <typename T, typename ... Args>
    T Call(const std::string &function, Args && ... args) {
      return _client.call(function, std::forward<Args>(args) ...).template as<T>();
    }

    std::string GetClientVersion() const {
      return ::carla::version();
    }

    std::string GetServerVersion() {
      return Call<std::string>("version");
    }

    bool Ping() {
      return Call<bool>("ping");
    }

    SharedPtr<World> GetWorld();

    BlueprintLibrary GetBlueprintLibrary() {
      return Call<std::vector<carla::rpc::ActorBlueprint>>("get_blueprints");
    }

    SharedPtr<Actor> SpawnActor(const ActorBlueprint &blueprint, const Transform &transform);

    void ApplyControlToActor(
        const Actor &actor,
        const VehicleControl &control);

  private:

    carla::rpc::Client _client;

    SharedPtr<World> _active_world;
  };

} // namespace client
} // namespace carla
