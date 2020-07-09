// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/tcp/Client.h"

#include "carla/BufferPool.h"
#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/Logging.h"
#include "carla/Time.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <exception>

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  // ===========================================================================
  // -- IncomingMessage --------------------------------------------------------
  // ===========================================================================

  /// Helper for reading incoming TCP messages. Allocates the whole message in
  /// a single buffer.
  class IncomingMessage {
  public:

    explicit IncomingMessage(Buffer &&buffer) : _message(std::move(buffer)) {}

    boost::asio::mutable_buffer size_as_buffer() {
      return boost::asio::buffer(&_size, sizeof(_size));
    }

    boost::asio::mutable_buffer buffer() {
      DEBUG_ASSERT(_size > 0u);
      _message.reset(_size);
      return _message.buffer();
    }

    auto size() const {
      return _size;
    }

    auto pop() {
      return std::move(_message);
    }

  private:

    message_size_type _size = 0u;

    Buffer _message;
  };

  // ===========================================================================
  // -- Client -----------------------------------------------------------------
  // ===========================================================================

  Client::Client(
      boost::asio::io_context &io_context,
      const token_type &token,
      callback_function_type callback)
    : LIBCARLA_INITIALIZE_LIFETIME_PROFILER(
          std::string("tcp client ") + std::to_string(token.get_stream_id())),
      _token(token),
      _callback(std::move(callback)),
      _socket(io_context),
      _strand(io_context),
      _connection_timer(io_context),
      _buffer_pool(std::make_shared<BufferPool>()) {
    if (!_token.protocol_is_tcp()) {
      throw_exception(std::invalid_argument("invalid token, only TCP tokens supported"));
    }
  }

  Client::~Client() = default;

  void Client::Connect() {
    auto self = shared_from_this();
    _strand.post([this, self]() {
      if (_done) {
        return;
      }

      using boost::system::error_code;

      if (_socket.is_open()) {
        _socket.close();
      }

      DEBUG_ASSERT(_token.is_valid());
      DEBUG_ASSERT(_token.protocol_is_tcp());
      const auto ep = _token.to_tcp_endpoint();

      auto handle_connect = [this, self, ep](error_code ec) {
        if (!ec) {
          if (_done) {
            return;
          }
          log_debug("streaming client: connected to", ep);
          // Send the stream id to subscribe to the stream.
          const auto &stream_id = _token.get_stream_id();
          log_debug("streaming client: sending stream id", stream_id);
          boost::asio::async_write(
              _socket,
              boost::asio::buffer(&stream_id, sizeof(stream_id)),
              _strand.wrap([=](error_code ec, size_t DEBUG_ONLY(bytes)) {
                if (!ec) {
                  DEBUG_ASSERT_EQ(bytes, sizeof(stream_id));
                  // If succeeded start reading data.
                  ReadData();
                } else {
                  // Else try again.
                  log_info("streaming client: failed to send stream id:", ec.message());
                  Connect();
                }
              }));
        } else {
          log_info("streaming client: connection failed:", ec.message());
          Reconnect();
        }
      };

      log_debug("streaming client: connecting to", ep);
      _socket.async_connect(ep, _strand.wrap(handle_connect));
    });
  }

  void Client::Stop() {
    _connection_timer.cancel();
    auto self = shared_from_this();
    _strand.post([this, self]() {
      _done = true;
      if (_socket.is_open()) {
        _socket.close();
      }
    });
  }

  void Client::Reconnect() {
    auto self = shared_from_this();
    _connection_timer.expires_from_now(time_duration::seconds(1u));
    _connection_timer.async_wait([this, self](boost::system::error_code ec) {
      if (!ec) {
        Connect();
      }
    });
  }

  void Client::ReadData() {
    auto self = shared_from_this();
    _strand.post([this, self]() {
      if (_done) {
        return;
      }

      // log_debug("streaming client: Client::ReadData");

      auto message = std::make_shared<IncomingMessage>(_buffer_pool->Pop());

      auto handle_read_data = [this, self, message](boost::system::error_code ec, size_t DEBUG_ONLY(bytes)) {
        DEBUG_ONLY(log_debug("streaming client: Client::ReadData.handle_read_data", bytes, "bytes"));
        if (!ec) {
          DEBUG_ASSERT_EQ(bytes, message->size());
          DEBUG_ASSERT_NE(bytes, 0u);
          // Move the buffer to the callback function and start reading the next
          // piece of data.
          // log_debug("streaming client: success reading data, calling the callback");
          _strand.context().post([self, message]() { self->_callback(message->pop()); });
          ReadData();
        } else {
          // As usual, if anything fails start over from the very top.
          log_info("streaming client: failed to read data:", ec.message());
          Connect();
        }
      };

      auto handle_read_header = [this, self, message, handle_read_data](
          boost::system::error_code ec,
          size_t DEBUG_ONLY(bytes)) {
        DEBUG_ONLY(log_debug("streaming client: Client::ReadData.handle_read_header", bytes, "bytes"));
        if (!ec && (message->size() > 0u)) {
          DEBUG_ASSERT_EQ(bytes, sizeof(message_size_type));
          if (_done) {
            return;
          }
          // Now that we know the size of the coming buffer, we can allocate our
          // buffer and start putting data into it.
          boost::asio::async_read(
              _socket,
              message->buffer(),
              _strand.wrap(handle_read_data));
        } else {
          log_info("streaming client: failed to read header:", ec.message());
          DEBUG_ONLY(log_debug("size  = ", message->size()));
          DEBUG_ONLY(log_debug("bytes = ", bytes));
          Connect();
        }
      };

      // Read the size of the buffer that is coming.
      boost::asio::async_read(
          _socket,
          message->size_as_buffer(),
          _strand.wrap(handle_read_header));
    });
  }

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
