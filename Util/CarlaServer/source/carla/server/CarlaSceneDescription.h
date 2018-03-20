// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/server/CarlaServerAPI.h"

#include <memory>

namespace carla {
namespace server {

  /// This is the only message that for convenience it is encoded in the main
  /// thread.
  ///
  /// Since the messages are only sent once, it is safe to invalidate the
  /// encoded string on the first read.
  class CarlaSceneDescription : private NonCopyable {
  public:

    CarlaSceneDescription() = default;

    CarlaSceneDescription(std::string &&encoded_scene)
      : _encoded_scene(std::move(encoded_scene)) {}

    CarlaSceneDescription(CarlaSceneDescription &&rhs)
      : _encoded_scene(std::move(rhs._encoded_scene)) {}

    CarlaSceneDescription &operator=(CarlaSceneDescription &&rhs) {
      _encoded_scene = std::move(rhs._encoded_scene);
      return *this;
    }

    std::string pop_scene() const {
      return std::move(_encoded_scene);
    }

  private:

    mutable std::string _encoded_scene;
  };

} // namespace server
} // namespace carla
