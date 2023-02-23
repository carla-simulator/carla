#include "BufferPool.h"



namespace carla {

    Buffer BufferPool::Pop() {
      Buffer item;
      _queue.try_dequeue(item); // we don't care if it fails.
#if __cplusplus >= 201703L // C++17
      item._parent_pool = weak_from_this();
#else
      item._parent_pool = shared_from_this();
#endif
      return item;
    }

    void BufferPool::Push(Buffer &&buffer) {
      _queue.enqueue(std::move(buffer));
    }

}