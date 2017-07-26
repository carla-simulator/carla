// CARLA, Copyright (C) 2017 Computer Vision Center (CVC) Project Settings.

#pragma once

#include <memory>

#include <carla/carla_server.h>

namespace carla {
namespace server {

  /// Holds the data of a carla_request_new_episode. Since we cannot pass
  /// ownership of data throught the C interface we need to hold it internally.
  /// The data is hold in memory until the next call to
  /// carla_read_request_new_episode().
  struct RequestNewEpisode {
    carla_request_new_episode values;
    std::unique_ptr<const char[]> data;
  };

} // namespace server
} // namespace carla
