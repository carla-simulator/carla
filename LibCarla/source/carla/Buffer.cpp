#include "carla/Buffer.h"

#include "carla/BufferPool.h"

namespace carla {

  void Buffer::ReuseThisBuffer() {
    auto pool = _parent_pool.lock();
    if (pool != nullptr) {
      log_debug("Buffer[", static_cast<void*>(_data.get()), ":", _size, "]::ReuseThisBuffer() returning buffer to pool:", pool.get());
      pool->Push(std::move(*this));
    }
  }

} // namespace carla
