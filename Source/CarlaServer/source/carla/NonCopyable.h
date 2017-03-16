#pragma once

namespace carla {

  class NonCopyable {
  public:

    NonCopyable() = default;

    NonCopyable(const NonCopyable &) = delete;

    void operator=(const NonCopyable &x) = delete;

  };

} // namespace carla
