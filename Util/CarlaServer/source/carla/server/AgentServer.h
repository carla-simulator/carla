// CARLA, Copyright (C) 2017 Computer Vision Center (CVC) Project Settings.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/server/AsyncServer.h"
#include "carla/server/EncoderServer.h"
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
        time_duration timeout);

    error_code WriteMeasurements(
        const carla_measurements &measurements,
        const_array_view<carla_image> images) {
      error_code ec;
      if (!_control.TryGetResult(ec)) {
        auto writer = _measurements.buffer()->MakeWriter();
        writer->Write(measurements, images);
        ec = errc::success();
      }
      return ec;
    };

    error_code ReadControl(carla_control &control, timeout_t timeout) {
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

  private:

    AsyncServer<EncoderServer<TCPServer>> _out;

    AsyncServer<EncoderServer<TCPServer>> _in;

    StreamWriteTask<MeasurementsMessage> _measurements;

    StreamReadTask<carla_control> _control;
  };

} // namespace server
} // namespace carla
