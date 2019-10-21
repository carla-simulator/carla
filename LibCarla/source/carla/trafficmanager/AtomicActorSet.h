#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

#include "carla/client/Actor.h"
#include "carla/Memory.h"

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

        void Insert(std::vector<ActorPtr> actor_list) {

            std::lock_guard<std::mutex> lock(modification_mutex);
            for (auto &actor: actor_list) {
                actor_set.insert({actor->GetId(), actor});
            }
            ++state_counter;
        }

        void Remove(std::vector<ActorId> actor_ids) {

            std::lock_guard<std::mutex> lock(modification_mutex);
            for (auto& id: actor_ids) {
                actor_set.erase(id);
            }
            ++state_counter;
        }

        int GetState() {

            std::lock_guard<std::mutex> lock(modification_mutex);
            return state_counter;
        }

        bool Contains(ActorId id) {

            return actor_set.find(id) != actor_set.end();
        }
    };

}