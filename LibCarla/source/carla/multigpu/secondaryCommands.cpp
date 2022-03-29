// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// #include "carla/Logging.h"
#include "carla/multigpu/secondaryCommands.h"
#include "carla/multigpu/commands.h"

namespace carla {
namespace multigpu {

using callback_response = std::function<void(carla::Buffer)>;

void SecondaryCommands::set_secondary(std::shared_ptr<Secondary> secondary) {
  _secondary = secondary;  
}

void SecondaryCommands::on_command(Buffer buffer) {
  log_info("Secondary got a command to process");
}


} // namespace multigpu
} // namespace carla
