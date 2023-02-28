// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef LIBCARLA_ENABLE_LIFETIME_PROFILER
#  define LIBCARLA_ENABLE_LIFETIME_PROFILER
#endif // LIBCARLA_ENABLE_LIFETIME_PROFILER

#include "carla/Debug.h"
#include "carla/Logging.h"
#include "carla/profiler/LifetimeProfiled.h"

#include <mutex>
#include <unordered_map>

namespace carla {
namespace profiler {

  template <typename ... Args>
  static inline void log(Args && ... args) {
    logging::write_to_stream(std::cerr, "LIFETIME:", std::forward<Args>(args) ..., '\n');
  }

  class LifetimeProfiler {
  public:

    ~LifetimeProfiler() {
      std::lock_guard<std::mutex> lock(_mutex);
      if (!_objects.empty()) {
        log("WARNING! the following objects were not destructed.");
        for (const auto &item : _objects) {
          log(item.second, "still alive.");
        }
        DEBUG_ERROR;
      }
    }

    void Register(void *object, std::string display_name) {
#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_DEBUG
      log('+', display_name);
#endif
      std::lock_guard<std::mutex> lock(_mutex);
      _objects.emplace(object, std::move(display_name));
    }

    void Deregister(void *object) {
      std::lock_guard<std::mutex> lock(_mutex);
      auto it = _objects.find(object);
      DEBUG_ASSERT(it != _objects.end());
#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_DEBUG
      log('-', it->second);
#endif
      _objects.erase(it);
    }

  private:

    std::mutex _mutex;

    std::unordered_map<void *, std::string> _objects;
  };

  static LifetimeProfiler PROFILER;

  LifetimeProfiled::LifetimeProfiled(std::string display_name) {
    PROFILER.Register(this, std::move(display_name));
  }

  LifetimeProfiled::~LifetimeProfiled() {
    PROFILER.Deregister(this);
  }

} // namespace profiler
} // namespace carla
