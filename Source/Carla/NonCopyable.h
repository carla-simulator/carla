#pragma once

class CARLA_API NonCopyable {
public:

  NonCopyable() = default;

  NonCopyable(const NonCopyable &) = delete;

  void operator=(const NonCopyable &x) = delete;
};
