// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

namespace carla {

  class NonCopyable {
  public:

    NonCopyable() = default;

    NonCopyable(const NonCopyable &) = delete;

    void operator=(const NonCopyable &x) = delete;
  };

} // namespace carla
