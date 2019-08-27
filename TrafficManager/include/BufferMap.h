#include <memory>
#include <map>
#include <mutex>
#include <utility>

namespace traffic_manager {

  template <typename K, typename D>
  class BufferMap {

  private:

    std::map<K, D> data_map;
    std::mutex put_mutex;

  public:

    BufferMap() {}
    ~BufferMap() {}

    void put(K key, D data) {

      if (data_map.find(key) == data_map.end()) {
        std::lock_guard<std::mutex> lock(put_mutex);

        if (data_map.find(key) == data_map.end()) {
          data_map.insert(std::pair<K, D>(key, data));
        }
      }
    }

    D get(K key) const {
      D data;
      if (data_map.find(key) != data_map.end()) {
        data = data_map.at(key);
      }

      return data;
    }

    bool contains(K key) const {
      return data_map.find(key) != data_map.end();
    }

  };

}
