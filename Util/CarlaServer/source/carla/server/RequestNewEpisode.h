// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/server/CarlaServerAPI.h"

namespace carla {
namespace server {

  /// Holds the data of a carla_request_new_episode. Since we cannot pass
  /// ownership of data throught the C interface we need to hold it internally.
  /// The data is hold in memory until the next call to
  /// carla_read_request_new_episode().
  class RequestNewEpisode {
  public:
    carla_request_new_episode values;
    std::unique_ptr<const char[]> data;
  };

} // namespace server
} // namespace carla
