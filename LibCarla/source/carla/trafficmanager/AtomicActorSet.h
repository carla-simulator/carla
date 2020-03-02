// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

#include "carla/client/Actor.h"
#include "carla/Memory.h"

namespace carla {
namespace traffic_manager {

  namespace cc = carla::client;
  using ActorPtr = carla::SharedPtr<cc::Actor>;
  using ActorId = carla::ActorId;

  class AtomicActorSet {

  private:

    std::mutex modification_mutex;
    std::unordered_map<ActorId, ActorPtr> actor_set;
    int state_counter;

  public:

    AtomicActorSet(): state_counter(0) {}

    std::vector<ActorPtr> GetList() {

      std::lock_guard<std::mutex> lock(modification_mutex);
      std::vector<ActorPtr> actor_list;
      for (auto it = actor_set.begin(); it != actor_set.end(); ++it) {
        actor_list.push_back(it->second);
      }
      return actor_list;
    }

    std::vector<ActorId> GetIDList() {

      std::lock_guard<std::mutex> lock(modification_mutex);
      std::vector<ActorId> actor_list;
      for (auto it = actor_set.begin(); it != actor_set.end(); ++it) {
        actor_list.push_back(it->first);
      }
      return actor_list;
    }

    void Insert(std::vector<ActorPtr> actor_list) {

      std::lock_guard<std::mutex> lock(modification_mutex);
      for (auto &actor: actor_list) {
        actor_set.insert({actor->GetId(), actor});
      }
      ++state_counter;
    }

    void Remove(std::vector<ActorPtr> actor_list) {

      std::lock_guard<std::mutex> lock(modification_mutex);
      for (auto& actor: actor_list) {
        actor_set.erase(actor->GetId());
      }
      ++state_counter;
    }

    void Destroy(ActorPtr actor) {

      std::lock_guard<std::mutex> lock(modification_mutex);
      actor_set.erase(actor->GetId());
      actor->Destroy();
      ++state_counter;
    }

    int GetState() {

      std::lock_guard<std::mutex> lock(modification_mutex);
      return state_counter;
    }

    bool Contains(ActorId id) {

      std::lock_guard<std::mutex> lock(modification_mutex);
      return actor_set.find(id) != actor_set.end();
    }

    size_t Size() {

      std::lock_guard<std::mutex> lock(modification_mutex);
      return actor_set.size();
    }

  };

} // namespace traffic_manager
} // namespace carla
