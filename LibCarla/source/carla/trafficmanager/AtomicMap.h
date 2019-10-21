#pragma once

#include <mutex>
#include <unordered_map>

template <typename Key, typename Value>
class AtomicMap {

  private:

  std::mutex map_mutex;
  std::unordered_map<Key, Value> map;

  public:

  AtomicMap() {}

  void AddEntry(const std::pair<Key, Value> &entry) {

    std::lock_guard<std::mutex> lock(map_mutex);
    map.insert(entry);
  }

  bool Contains(const Key &key) {

    std::lock_guard<std::mutex> lock(map_mutex);
    return map.find(key) != map.end();
  }

  Value &GetValue(const Key &key) {

    std::lock_guard<std::mutex> lock(map_mutex);
    return map.at(key);
  }

  void RemoveEntry(const Key &key) {

    std::lock_guard<std::mutex> lock(map_mutex);
    map.erase(key);
  }

};
