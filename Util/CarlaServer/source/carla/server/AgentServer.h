// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/server/AsyncServer.h"
#include "carla/server/EncoderServer.h"
#include "carla/server/SensorDataInbox.h"
#include "carla/server/TCPServer.h"

namespace carla {
namespace server {

  class CarlaEncoder;

  class AgentServer : private NonCopyable {
  public:

    explicit AgentServer(
        CarlaEncoder &encoder,
        uint32_t out_port,
        uint32_t in_port,
        const SensorDataInbox::Sensors &sensors,
        time_duration timeout);

    error_code WriteSensorData(const carla_sensor_data &data);

    error_code WriteMeasurements(const carla_measurements &measurements);

    error_code ReadControl(carla_control &control, timeout_t timeout);

  private:

    AsyncServer<EncoderServer<TCPServer>> _out;

    AsyncServer<EncoderServer<TCPServer>> _in;

    SensorDataInbox _sensor_inbox;

    StreamWriteTask<MeasurementsMessage> _measurements;

    StreamReadTask<carla_control> _control;
  };

} // namespace server
} // namespace carla
