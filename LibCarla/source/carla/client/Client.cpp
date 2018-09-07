// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Client.h"

#include "carla/client/Actor.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Control.h"
#include "carla/client/Sensor.h"
#include "carla/client/Vehicle.h"
#include "carla/client/World.h"

#include <thread>

namespace carla {
namespace client {

  Client::Client(const std::string &host, uint16_t port, size_t worker_threads)
    : _client(host, port),
      _streaming_client(host) {
    _streaming_client.AsyncRun(
        worker_threads > 0u ? worker_threads : std::thread::hardware_concurrency());
  }

  SharedPtr<World> Client::GetWorld() {
    if (_active_world == nullptr) {
      _active_world.reset(new World(shared_from_this()));
    }
    return _active_world;
  }

  SharedPtr<BlueprintLibrary> Client::GetBlueprintLibrary() {
    return MakeShared<BlueprintLibrary>(
        Call<std::vector<carla::rpc::ActorDefinition>>("get_actor_definitions"));
  }

  SharedPtr<Actor> Client::GetSpectator() {
    auto spectator = Call<carla::rpc::Actor>("get_spectator");
    return SharedPtr<Actor>(new Actor{spectator, GetWorld()});
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

  void Client::DestroyActor(Actor &actor) {
    _client.call("destroy_actor", actor.Serialize());
  }

  Location Client::GetActorLocation(Actor &actor) {
    return Call<Location>("get_actor_location", actor.Serialize());
  }

  Transform Client::GetActorTransform(Actor &actor) {
    return Call<Transform>("get_actor_transform", actor.Serialize());
  }

  bool Client::SetActorLocation(Actor &actor, const Location &location) {
    return Call<bool>("set_actor_location", actor.Serialize(), location);
  }

  bool Client::SetActorTransform(Actor &actor, const Transform &transform) {
    return Call<bool>("set_actor_transform", actor.Serialize(), transform);
  }

  void Client::ApplyControlToActor(
      Actor &actor,
      const VehicleControl &control) {
    _client.call("apply_control_to_actor", actor.Serialize(), control);
  }

  void Client::SetActorAutopilot(
      Actor &actor,
      const bool enabled) {
    _client.call("set_actor_autopilot", actor.Serialize(), enabled);
  }

} // namespace client
} // namespace carla
