// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "carla/Logging.h"
#include "carla/streaming/detail/tcp/Message.h"
#include "carla/multigpu/commands.h"
#include "carla/multigpu/secondary.h"

namespace carla {
namespace multigpu {

// using session = std::shared_ptr<Primary>;
using callback_response = std::function<void(carla::Buffer)>;

class SecondaryCommands {
  
  public:
  void set_secondary(std::shared_ptr<Secondary> secondary);
  void on_command(Buffer buffer);

  private:
  std::shared_ptr<Secondary> _secondary;
};

} // namespace multigpu
} // namespace carla
