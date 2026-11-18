// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Logging.h"
#include "carla/multigpu/secondaryCommands.h"

#include <cstring>

namespace carla {
namespace multigpu {

void SecondaryCommands::set_secondary(std::shared_ptr<Secondary> secondary) {
  _secondary = secondary;  
}

void SecondaryCommands::set_callback(callback_type callback) {
  _callback = callback;
}

void SecondaryCommands::process_command(Buffer buffer) {
  // Each message is framed by the transport's size prefix, so dropping a
  // malformed one never desynchronizes the stream.
  CommandHeader header;
  if (buffer.size() < sizeof(header)) {
    log_error("multigpu: dropping a", buffer.size(), "byte command shorter than its header");
    return;
  }
  std::memcpy(&header, buffer.data(), sizeof(header));
  const std::size_t available = buffer.size() - sizeof(header);
  if (header.size > available) {
    log_error("multigpu: dropping command", static_cast<uint32_t>(header.id), "declaring",
        header.size, "payload bytes in a message holding", available);
    return;
  }
  if (header.size < available) {
    log_warning("multigpu: command", static_cast<uint32_t>(header.id), "has",
        available - header.size, "trailing bytes; ignored");
  }

  // send only data to the callback
  Buffer data(buffer.data() + sizeof(header), header.size);
  _callback(header.id, std::move(data));
}


} // namespace multigpu
} // namespace carla
