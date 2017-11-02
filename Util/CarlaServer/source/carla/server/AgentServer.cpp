// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB), and the INTEL Visual Computing Lab.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

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
