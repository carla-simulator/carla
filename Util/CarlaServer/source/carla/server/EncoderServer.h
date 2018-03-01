// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Logging.h"
#include "carla/NonCopyable.h"
#include "carla/server/CarlaEncoder.h"
#include "carla/server/MeasurementsMessage.h"
#include "carla/server/SensorDataInbox.h"
#include "carla/server/ServerTraits.h"

namespace carla {
namespace server {

  /// Wrapper around a server for encoding and decoding the messages with a
  /// CarlaEncoder.
  template <typename SERVER>
  class EncoderServer : private NonCopyable {
  public:

    using server_type = SERVER;
    using encoder_type = CarlaEncoder;

    template<typename... Args>
    explicit EncoderServer(encoder_type &encoder, Args&&... args)
      : _server(std::forward<Args>(args)...),
        _encoder(encoder) {}

    error_code Connect(uint32_t port, time_duration timeout) {
      return _server.Connect(port, timeout);
    }

    void Disconnect() {
      _server.Disconnect();
    }

    /// @warning Since every received message consists of two Reads, the timeout
    /// applies to each individual Read. Effectively, it may wait twice the
    /// timeout.
    template <typename T>
    error_code Read(T &values, time_duration timeout) {
      std::string string;
      auto ec = ReadString(string, timeout);
      if (!ec && !_encoder.Decode(string, values)) {
        ec.assign(
            boost::system::errc::illegal_byte_sequence,
            boost::system::system_category());
      }
      return ec;
    }

    template <typename T>
    error_code Write(const T &values, time_duration timeout) {
      const auto string = _encoder.Encode(values);
      return _server.Write(boost::asio::buffer(string), timeout);
    }

    /// @warning This operation consists of several Writes, the timeout applies
    /// to each individual Write. Effectively, it may wait the timeout for each
    /// sensor.
    error_code Write(const MeasurementsMessage &values, time_duration timeout) {
      const auto string = _encoder.Encode(values.measurements());
      auto ec = _server.Write(boost::asio::buffer(string), timeout);
      if (!ec) {
        ec = Write(values.sensor_inbox(), timeout);
      }
      return ec;
    }

  private:

    error_code Write(SensorDataInbox &inbox, time_duration timeout) {
      error_code ec;
      for (auto &sensor_buffer : inbox) {
        auto reader = sensor_buffer.TryMakeReader();
        if (reader != nullptr) {
          auto ec = _server.Write(reader->buffer(), timeout);
          if (ec)
            return ec;
        }
      }
      const uint32_t end_message = 0u;
      return _server.Write(boost::asio::buffer(&end_message, sizeof(end_message)), timeout);;
    }

    error_code ReadString(std::string &string, time_duration timeout) {
       // Get the message's size.
      uint32_t message_size;
      auto ec = _server.Read(boost::asio::buffer(&message_size, sizeof(uint32_t)), timeout);
      if (ec) {
        return ec;
      }
      // Knowing the size now we can Read the message.
      auto buffer = std::make_unique<char[]>(message_size);
      ec = _server.Read(boost::asio::buffer(buffer.get(), message_size), timeout);
      if (!ec) {
        string.assign(buffer.get(), message_size);
      }
      return ec;
    }

    server_type _server;

    encoder_type &_encoder;
  };

} // namespace server
} // namespace carla
