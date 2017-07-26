// CARLA, Copyright (C) 2017 Computer Vision Center (CVC) Project Settings.

#pragma once

#include <carla/carla_server.h>

#include "carla/server/Protobuf.h"
#include "carla/server/RequestNewEpisode.h"

namespace carla {
namespace server {

  class CarlaEncoder {
  public:

    // =========================================================================
    /// @name string encoders (for testing only)
    // =========================================================================
    /// @{

    std::string Encode(const std::string &values) {
      return Protobuf::Encode(values);
    }

    bool Decode(std::string message, std::string &values) {
      values = std::move(message);
      return true;
    }

    /// @}
    // =========================================================================
    /// @name Protobuf encodings
    // =========================================================================
    /// @{

    std::string Encode(const carla_scene_description &values);

    std::string Encode(const carla_episode_ready &values);

    std::string Encode(const carla_measurements &values);

    bool Decode(const std::string &message, RequestNewEpisode &values);

    bool Decode(const std::string &message, carla_episode_start &values);

    bool Decode(const std::string &message, carla_control &values);

    /// @}

  private:

    Protobuf _protobuf;
  };

} // namespace server
} // namespace carla
