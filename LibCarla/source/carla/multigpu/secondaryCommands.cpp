// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// #include "carla/Logging.h"
#include "carla/multigpu/secondaryCommands.h"
// #include "carla/streaming/detail/tcp/Message.h"

namespace carla {
namespace multigpu {

void SecondaryCommands::set_secondary(std::shared_ptr<Secondary> secondary) {
  _secondary = secondary;  
}

void SecondaryCommands::set_callback(callback_type callback) {
  _callback = callback;
}

void SecondaryCommands::process_command(Buffer buffer) {
  // get the header
  CommandHeader *header;
  header = reinterpret_cast<CommandHeader *>(buffer.data());
  
  // send only data to the callback
  Buffer data(buffer.data() + sizeof(CommandHeader), header->size);
  _callback(header->id, std::move(data));

  // log_info("Secondary got a command to process");
}


} // namespace multigpu
} // namespace carla
