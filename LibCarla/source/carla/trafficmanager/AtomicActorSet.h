// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <mutex>
#include <map>
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
    std::map<ActorId, ActorPtr> actor_set;
    int state_counter;

  public:

    AtomicActorSet(): state_counter(0) {}

    std::vector<ActorPtr> GetList() {

      std::scoped_lock<std::mutex> lock(modification_mutex);
      std::vector<ActorPtr> actor_list;
      for (auto it = actor_set.begin(); it != actor_set.end(); ++it) {
        actor_list.push_back(it->second);
      }
      return actor_list;
    }

    std::vector<ActorId> GetIDList() {

      std::scoped_lock<std::mutex> lock(modification_mutex);
      std::vector<ActorId> actor_list;
      for (auto it = actor_set.begin(); it != actor_set.end(); ++it) {
        actor_list.push_back(it->first);
      }
      return actor_list;
    }

    void Insert(std::vector<ActorPtr> actor_list) {

      std::scoped_lock<std::mutex> lock(modification_mutex);
      for (auto &actor: actor_list) {
        actor_set.insert({actor->GetId(), actor});
      }
      ++state_counter;
    }

    void Remove(std::vector<ActorId> actor_id_list) {

      std::scoped_lock<std::mutex> lock(modification_mutex);
      for (auto& actor_id: actor_id_list) {
        if (actor_set.find(actor_id) != actor_set.end()){
          actor_set.erase(actor_id);
        }
      }
      ++state_counter;
    }

    void Destroy(ActorId actor_id) {

      std::scoped_lock<std::mutex> lock(modification_mutex);
      if (actor_set.find(actor_id) != actor_set.end()) {
        ActorPtr actor = actor_set.at(actor_id);
        actor->Destroy();
        actor_set.erase(actor_id);
        ++state_counter;
      }
    }

    int GetState() {

      std::scoped_lock<std::mutex> lock(modification_mutex);
      return state_counter;
    }

    bool Contains(ActorId id) {

      std::scoped_lock<std::mutex> lock(modification_mutex);
      return actor_set.find(id) != actor_set.end();
    }

    size_t Size() {

      std::scoped_lock<std::mutex> lock(modification_mutex);
      return actor_set.size();
    }

    void Clear() {

      std::scoped_lock<std::mutex> lock(modification_mutex);
      return actor_set.clear();
    }

  };

} // namespace traffic_manager
} // namespace carla
