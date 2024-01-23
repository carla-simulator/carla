// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "carla/Logging.h"
#include "carla/Buffer.h"
#include "carla/multigpu/commands.h"
#include <functional>

namespace carla {
namespace multigpu {

class Secondary;

class SecondaryCommands {
  public:

  using callback_type = std::function<void(MultiGPUCommand, carla::Buffer)>;
  
  void set_secondary(std::shared_ptr<Secondary> secondary);
  void set_callback(callback_type callback);
  void process_command(Buffer buffer);

  private:
  std::shared_ptr<Secondary>  _secondary;
  callback_type               _callback;
};

} // namespace multigpu
} // namespace carla
