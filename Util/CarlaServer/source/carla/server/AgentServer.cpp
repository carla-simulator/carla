// CARLA, Copyright (C) 2017 Computer Vision Center (CVC) Project Settings.

#include "carla/server/AgentServer.h"

namespace carla {
namespace server {

  AgentServer::AgentServer(
      CarlaEncoder &encoder,
      const uint32_t out_port,
      const uint32_t in_port,
      const time_duration timeout)
      : _out(encoder),
        _in(encoder),
        _measurements(timeout),
        _control(timeout) {
    _out.Connect(out_port, timeout);
    _out.Execute(_measurements);
    _in.Connect(in_port, timeout);
    _in.Execute(_control);
  }

} // namespace server
} // namespace carla
