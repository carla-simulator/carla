// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {

  /// Inherit (privately) to suppress copy-construction and copy-assignment.
  class NonCopyable {
  public:

    NonCopyable() = default;

    NonCopyable(const NonCopyable &) = delete;

    void operator=(const NonCopyable &) = delete;
  };

} // namespace carla
