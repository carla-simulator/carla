// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/tcp/ServerSession.h"

#include "carla/Debug.h"
#include "carla/Logging.h"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <atomic>

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  static std::atomic_size_t SESSION_COUNTER{0u};

  ServerSession::ServerSession(
      boost::asio::io_service &io_service,
      const time_duration timeout)
    : _session_id(SESSION_COUNTER++),
      _socket(io_service),
      _timeout(timeout),
      _deadline(io_service),
      _strand(io_service) {}

  ServerSession::~ServerSession() {
    _deadline.cancel();
  }

  void ServerSession::Open(callback_function_type callback) {
    StartTimer();
    auto self = shared_from_this(); // To keep myself alive.
    _strand.post([=]() {

      auto handle_query = [this, self, cb=std::move(callback)](
          const boost::system::error_code &ec,
          size_t DEBUG_ONLY(bytes_received)) {
        DEBUG_ASSERT_EQ(bytes_received, sizeof(_stream_id));
        if (!ec) {
          log_debug("session", _session_id, "for stream", _stream_id, " started");
          _socket.get_io_service().post([=]() { cb(self); });
        } else {
          log_error("session", _session_id, ": error retrieving stream id :", ec.message());
          Close();
        }
      };

      // Read the stream id.
      _deadline.expires_from_now(_timeout);
      boost::asio::async_read(
          _socket,
          boost::asio::buffer(&_stream_id, sizeof(_stream_id)),
          _strand.wrap(handle_query));
    });
  }

  void ServerSession::Close() {
    _strand.post([this, self = shared_from_this()]() {
      if (_socket.is_open()) {
        _socket.close();
      }
      log_debug("session", _session_id, "closed");
    });
  }

  void ServerSession::Write(std::shared_ptr<const Message> message) {
    auto self = shared_from_this();
    _strand.post([=]() {

      /// @todo has to be a better way of doing this...
      if (_is_writing) {
        // Repost and return;
        Write(std::move(message));
        return;
      }
      _is_writing = true;

      auto handle_sent = [this, self, message](const boost::system::error_code &ec, size_t DEBUG_ONLY(bytes)) {
        _is_writing = false;
        if (ec) {
          log_error("session", _session_id, ": error sending data :", ec.message());
        } else {
          DEBUG_ONLY(log_debug("session", _session_id, ": successfully sent", bytes, "bytes"));
          DEBUG_ASSERT_EQ(bytes, sizeof(message_size_type) + message->size());
        }
      };

      log_debug("session", _session_id, ": sending message of", message->size(), "bytes");

      _deadline.expires_from_now(_timeout);
      boost::asio::async_write(
          _socket,
          message->GetBufferSequence(),
          _strand.wrap(handle_sent));
    });
  }

  void ServerSession::StartTimer() {
    if (_deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
      log_debug("session", _session_id, "timed out");
      Close();
    } else {
      _deadline.async_wait([self = shared_from_this()](boost::system::error_code) {
        self->StartTimer();
      });
    }
  }

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
