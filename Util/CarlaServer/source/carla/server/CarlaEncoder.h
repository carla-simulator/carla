// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/server/CarlaServerAPI.h"
#include "carla/server/Protobuf.h"

namespace carla {
namespace server {

  class CarlaSceneDescription;
  class RequestNewEpisode;

  /// Converts the data between the C interface types and the Protobuf message
  /// that is going to be sent and received through the socket.
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

    std::string Encode(const CarlaSceneDescription &values);

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
