// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <memory>
#include <thread>

namespace carla {
namespace thread {

  template<typename T>
  class JoinAndDeletePointer {
  public:

    void operator()(T *ptr) {
      if (ptr) {
        if (ptr->joinable()) {
          ptr->join();
        }
        delete ptr;
      }
    }

  };

  using ThreadUniquePointer =
      std::unique_ptr<std::thread, JoinAndDeletePointer<std::thread>>;

} // namespace thread
} // namespace carla
