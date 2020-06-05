// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <mutex>
#include <unordered_map>

namespace carla {
namespace traffic_manager {

  template <typename Key, typename Value>
  class AtomicMap {

    private:

    mutable std::mutex map_mutex;
    std::unordered_map<Key, Value> map;

    public:

    AtomicMap() {}

    void AddEntry(const std::pair<Key, Value> &entry) {

      std::lock_guard<std::mutex> lock(map_mutex);
      map.insert(entry);
    }

    bool Contains(const Key &key) const {

      std::lock_guard<std::mutex> lock(map_mutex);
      return map.find(key) != map.end();
    }

    const Value &GetValue(const Key &key) const {

      std::lock_guard<std::mutex> lock(map_mutex);
      return map.at(key);
    }

    void RemoveEntry(const Key &key) {

      std::lock_guard<std::mutex> lock(map_mutex);
      map.erase(key);
    }

  };

} // namespace traffic_manager
} // namespace carla
