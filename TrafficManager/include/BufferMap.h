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

      if (
        data_map.find(key) == data_map.end()
        or
          (
            data_map.at(key) == nullptr
            and
            data != nullptr
          )
      ) {
        std::unique_lock<std::mutex> lock(put_mutex);
        if (
          data_map.find(key) == data_map.end()
          or
          (
            data_map.at(key) == nullptr
            and
            data != nullptr
          )
        ) {
          data_map.insert(std::pair<K, D>(key, data));
        }
      }
    }

    D get(K key) const {
      if (data_map.find(key) != data_map.end()) {
        return data_map.at(key);
      } else {
        return D();
      }
    }

    bool contains(K key) const {
      return data_map.find(key) != data_map.end();
    }

  };

}
