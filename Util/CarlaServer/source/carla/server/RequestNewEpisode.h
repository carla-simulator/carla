// CARLA, Copyright (C) 2017 Computer Vision Center (CVC) Project Settings.

#pragma once

#include <memory>

#include <carla/carla_server.h>

namespace carla {
namespace server {

  struct RequestNewEpisode {
    carla_request_new_episode values;
    std::unique_ptr<const char[]> data;
  };

} // namespace server
} // namespace carla
