// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
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
      const SensorDataInbox::Sensors &sensors,
      const time_duration timeout)
      : _out(encoder),
        _in(encoder),
        _sensor_inbox(sensors),
        _measurements(timeout),
        _control(timeout) {
    _out.Connect(out_port, timeout);
    _out.Execute(_measurements);
    _in.Connect(in_port, timeout);
    _in.Execute(_control);
  }

  error_code AgentServer::WriteSensorData(const carla_sensor_data &data) {
    _sensor_inbox.Write(data);
    return errc::success();
  }

  error_code AgentServer::WriteMeasurements(const carla_measurements &measurements) {
    error_code ec;
    if (!_measurements.TryGetResult(ec)) {
      auto writer = _measurements.buffer()->MakeWriter();
      writer->Write(measurements, _sensor_inbox);
      ec = errc::success();
    }
    return ec;
  };

  error_code AgentServer::ReadControl(carla_control &control, timeout_t timeout) {
    error_code ec = errc::try_again();
    if (!_control.TryGetResult(ec)) {
      auto reader = _control.buffer()->TryMakeReader(timeout);
      if (reader != nullptr) {
        control = *reader;
        ec = errc::success();
      }
    }
    return ec;
  }

} // namespace server
} // namespace carla
