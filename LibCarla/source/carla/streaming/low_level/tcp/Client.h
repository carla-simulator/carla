// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Logging.h"
#include "carla/streaming/Message.h"
#include "carla/streaming/low_level/Types.h"
#include "carla/streaming/low_level/tcp/Timeout.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>

#include <functional>
#include <memory>
#include <string>

namespace carla {
namespace streaming {
namespace low_level {
namespace tcp {

  class Encoder {
  public:

    boost::asio::mutable_buffer header() {
      return boost::asio::buffer(reinterpret_cast<unsigned char *>(&_size), sizeof(_size));
    }

    boost::asio::mutable_buffer body() {
      DEBUG_ASSERT(_size > 0u);
      DEBUG_ASSERT(_message == nullptr);
      _message = std::make_shared<Message>(_size);
      return _message->buffer();
    }

    auto size() const {
      return _size;
    }

    auto pop() {
      return std::move(_message);
    }

  private:

    message_size_type _size = 0u;

    std::shared_ptr<Message> _message;
  };

  /// @warning The client should not be destroyed before the @a io_service is
  /// stopped.
  class Client : private boost::noncopyable {
  public:

    using endpoint = boost::asio::ip::tcp::endpoint;

    template <typename Functor>
    Client(
        boost::asio::io_service &io_service,
        endpoint ep,
        stream_id_type stream_id,
        Functor &&callback)
      : _endpoint(std::move(ep)),
        _stream_id(stream_id),
        _callback(std::forward<Functor>(callback)),
        _socket(io_service),
        _strand(io_service),
        _connection_timer(io_service) {
      Connect();
    }

    ~Client() {
      Stop();
    }

    stream_id_type get_id() const {
      return _stream_id;
    }

    bool operator==(const Client &rhs) const {
      return get_id() == rhs.get_id();
    }

    void Stop() {
      _connection_timer.cancel();
      _strand.post([this]() {
        _done = true;
        if (_socket.is_open()) {
          _socket.close();
        }
      });
    }

  private:

    /// @todo Stop inlining and make cpp files.

    inline void Connect();

    inline void Reconnect() {
      _connection_timer.expires_from_now(timeout_type::seconds(1u));
      _connection_timer.async_wait([this](boost::system::error_code ec) {
        if (!ec) {
          Connect();
        }
      });
    }

    inline void ReadData();

    const endpoint _endpoint;

    const stream_id_type _stream_id;

    std::function<void(std::shared_ptr<Message>)> _callback;

    boost::asio::ip::tcp::socket _socket;

    boost::asio::io_service::strand _strand;

    boost::asio::deadline_timer _connection_timer;

    bool _done = false;
  };

  void Client::Connect() {
    _strand.post([this]() {
      if (_done) {
        return;
      }

      using boost::system::error_code;

      if (_socket.is_open()) {
        _socket.close();
      }

      auto handle_connect = [=](error_code ec) {
        if (!ec) {
          // Send the stream id to subscribe to the stream.
          log_debug("streaming client: sending stream id", _stream_id);
          boost::asio::async_write(
              _socket,
              boost::asio::buffer(&_stream_id, sizeof(_stream_id)),
              _strand.wrap([=](error_code ec, size_t DEBUG_ONLY(bytes)) {
            if (!ec) {
              DEBUG_ASSERT_EQ(bytes, sizeof(_stream_id));
              // If succeeded start reading data.
              ReadData();
            } else {
              // Else try again.
              log_debug("streaming client: failed to send stream id:", ec.message());
              Connect();
            }
          }));
        } else {
          log_debug("streaming client: connection failed:", ec.message());
          Reconnect();
        }
      };

      log_debug("streaming client: connecting to", _endpoint);
      _socket.async_connect(_endpoint, _strand.wrap(handle_connect));
    });
  }

  void Client::ReadData() {
    _strand.post([this]() {
      if (_done) {
        return;
      }

      log_debug("streaming client: Client::ReadData");

      auto encoder = std::make_shared<Encoder>();

      auto handle_read_data = [=](boost::system::error_code ec, size_t DEBUG_ONLY(bytes)) {
        DEBUG_ONLY(log_debug("streaming client: Client::ReadData.handle_read_data", bytes, "bytes"));
        if (!ec) {
          DEBUG_ASSERT_EQ(bytes, encoder->size());
          DEBUG_ASSERT_NE(bytes, 0u);
          // Move the buffer to the callback function and start reading the next
          // piece of data.
          log_debug("streaming client: success reading data, calling the callback");
          _socket.get_io_service().post([this, encoder]() { _callback(encoder->pop()); });
          ReadData();
        } else {
          // As usual, if anything fails start over from the very top.
          log_debug("streaming client: failed to read data:", ec.message());
          Connect();
        }
      };

      auto handle_read_header = [=](boost::system::error_code ec, size_t DEBUG_ONLY(bytes)) {
        DEBUG_ONLY(log_debug("streaming client: Client::ReadData.handle_read_header", bytes, "bytes"));
        if (!ec && (encoder->size() > 0u)) {
          DEBUG_ASSERT_EQ(bytes, sizeof(message_size_type));
          // Now that we know the size of the coming buffer, we can allocate
          // our buffer and start putting data into it.
          boost::asio::async_read(
              _socket,
              encoder->body(),
              _strand.wrap(handle_read_data));
        } else {
          log_debug("streaming client: failed to read header:", ec.message());
          DEBUG_ONLY(log_debug("size  = ", encoder->size()));
          DEBUG_ONLY(log_debug("bytes = ", bytes));
          Connect();
        }
      };

      // Read the size of the buffer that is coming.
      boost::asio::async_read(
          _socket,
          encoder->header(),
          _strand.wrap(handle_read_header));
    });
  }

} // namespace tcp
} // namespace low_level
} // namespace streaming
} // namespace carla

namespace std {

  // Injecting a hash function for our clients into std namespace so we can
  // directly insert them into std::unordered_set.
  template <>
  struct hash<carla::streaming::low_level::tcp::Client> {
    using argument_type = carla::streaming::low_level::tcp::Client;
    using result_type = std::size_t;
    result_type operator()(const argument_type &client) const noexcept {
      return std::hash<carla::streaming::low_level::stream_id_type>()(client.get_id());
    }
  };

} // namespace std
