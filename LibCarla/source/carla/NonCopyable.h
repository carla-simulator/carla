// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {

  /// Inherit (privately) to suppress copy/move construction and assignment.
  class NonCopyable {
  public:

    NonCopyable() = default;

    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;

    NonCopyable(NonCopyable &&) = delete;
    NonCopyable &operator=(NonCopyable &&) = delete;
  };

  /// Inherit (privately) to suppress copy construction and assignment.
  class MovableNonCopyable {
  public:

    MovableNonCopyable() = default;

    MovableNonCopyable(const MovableNonCopyable &) = delete;
    MovableNonCopyable &operator=(const MovableNonCopyable &) = delete;

    MovableNonCopyable(MovableNonCopyable &&) = default;
    MovableNonCopyable &operator=(MovableNonCopyable &&) = default;
  };

} // namespace carla
