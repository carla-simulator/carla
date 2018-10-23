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
    : LIBCARLA_INITIALIZE_LIFETIME_PROFILER(
          std::string("tcp server session ") + std::to_string(SESSION_COUNTER)),
      _session_id(SESSION_COUNTER++),
      _socket(io_service),
      _timeout(timeout),
      _deadline(io_service),
      _strand(io_service) {}

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
          CloseNow();
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

  void ServerSession::Write(std::shared_ptr<const Message> message) {
    DEBUG_ASSERT(message != nullptr);
    DEBUG_ASSERT(!message->empty());
    auto self = shared_from_this();
    _strand.post([=]() {
      if (!_socket.is_open()) {
        return;
      }
      if (_is_writing) {
        log_debug("session", _session_id, ": connection too slow: message discarded");
        return;
      }
      _is_writing = true;

      auto handle_sent = [this, self, message](const boost::system::error_code &ec, size_t DEBUG_ONLY(bytes)) {
        _is_writing = false;
        if (ec) {
          log_info("session", _session_id, ": error sending data :", ec.message());
          CloseNow();
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
      _strand.post([self=shared_from_this()]() { self->CloseNow(); });
    } else {
      _deadline.async_wait([this, self=shared_from_this()](boost::system::error_code ec) {
        if (!ec) {
          StartTimer();
        } else {
          log_debug("session", _session_id, "timed out error:", ec.message());
        }
      });
    }
  }

  void ServerSession::CloseNow() {
    DEBUG_ASSERT(_strand.running_in_this_thread());
    _deadline.cancel();
    if (_socket.is_open()) {
      _socket.close();
    }
    log_debug("session", _session_id, "closed");
  }

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
