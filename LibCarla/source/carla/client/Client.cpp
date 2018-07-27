// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Client.h"

#include "carla/client/Actor.h"
#include "carla/client/Control.h"
#include "carla/client/Sensor.h"
#include "carla/client/Vehicle.h"
#include "carla/client/World.h"

namespace carla {
namespace client {

  SharedPtr<World> Client::GetWorld() {
    if (_active_world == nullptr) {
      _active_world.reset(new World(shared_from_this()));
    }
    return _active_world;
  }

  SharedPtr<Actor> Client::SpawnActor(
      const ActorBlueprint &blueprint,
      const Transform &transform,
      Actor *parent) {
    auto actor = parent != nullptr ?
        Call<carla::rpc::Actor>("spawn_actor_with_parent", transform, blueprint.MakeActorDescription(), parent->Serialize()) :
        Call<carla::rpc::Actor>("spawn_actor", transform, blueprint.MakeActorDescription());
    if (actor.IsASensor()) {
      return SharedPtr<Actor>(new Sensor{actor, GetWorld()});
    }
    return SharedPtr<Actor>(new Vehicle{actor, GetWorld()});
  }

  void Client::ApplyControlToActor(
      const Actor &actor,
      const VehicleControl &control) {
    _client.call("apply_control_to_actor", actor.Serialize(), control);
  }

} // namespace client
} // namespace carla
