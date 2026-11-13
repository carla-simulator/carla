// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/tcp/ServerSession.h"
#include "carla/streaming/detail/tcp/Server.h"

#include "carla/Debug.h"
#include "carla/Logging.h"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/post.hpp>

#include <atomic>
#include <chrono>
#include <thread>

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  static std::atomic_size_t SESSION_COUNTER{0u};

  ServerSession::ServerSession(
      boost::asio::io_context &io_context,
      const time_duration timeout,
      Server &server)
    : LIBCARLA_INITIALIZE_LIFETIME_PROFILER(
          std::string("tcp server session ") + std::to_string(SESSION_COUNTER)),
      _server(server),
      _session_id(SESSION_COUNTER++),
      _socket(io_context),
      _timeout(timeout),
      _deadline(io_context, std::chrono::steady_clock::time_point::max()),
      _strand(io_context) {}

  void ServerSession::Open(
      callback_function_type on_opened,
      callback_function_type on_closed) {
    DEBUG_ASSERT(on_opened && on_closed);
    _on_closed = std::move(on_closed);

    // This forces not using Nagle's algorithm.
    // Improves the sync mode velocity on Linux by a factor of ~3.
    const boost::asio::ip::tcp::no_delay option(true);
    _socket.set_option(option);

    StartTimer();
    auto self = shared_from_this(); // To keep myself alive.
    boost::asio::post(_strand, [=]() {

      auto handle_query = [this, self, callback=std::move(on_opened)](
          const boost::system::error_code &ec,
          size_t DEBUG_ONLY(bytes_received)) {
        if (!ec) {
          DEBUG_ASSERT_EQ(bytes_received, sizeof(_stream_id));
          log_debug("session", _session_id, "for stream", _stream_id, " started");
          boost::asio::post(_strand.context(), [=]() { callback(self); });
        } else {
          log_error("session", _session_id, ": error retrieving stream id :", ec.message());
          CloseNow(ec);
        }
      };

      // Read the stream id.
      _deadline.expires_after(_timeout.to_chrono());
      boost::asio::async_read(
          _socket,
          boost::asio::buffer(&_stream_id, sizeof(_stream_id)),
          boost::asio::bind_executor(_strand, handle_query));
    });
  }

  void ServerSession::Write(std::shared_ptr<const Message> message) {
    DEBUG_ASSERT(message != nullptr);
    DEBUG_ASSERT(!message->empty());
    auto self = shared_from_this();
    if (!_socket.is_open()) {
      return;
    }
    if (_is_writing) {
      if (_server.IsSynchronousMode()) {
        // Wait until the previous message has been sent, but not forever: the
        // caller is the game thread or the render thread, and a client that
        // stops reading this socket (a listen() callback that never returns,
        // a frozen process) would otherwise stall the whole server, every
        // other client included (#10233). Drop that client instead; its
        // streaming client reconnects on its own.
        const auto start = std::chrono::steady_clock::now();
        const auto limit = _timeout.to_chrono();
        bool warned = false;
        while (_is_writing) {
          std::this_thread::sleep_for(std::chrono::microseconds(100));
          const auto waited = std::chrono::steady_clock::now() - start;
          if (!warned && waited > std::chrono::seconds(1)) {
            warned = true;
            log_warning("streaming session", _session_id, "for stream", _stream_id,
                ": client is not reading, synchronous send blocked for 1 s");
          }
          if (waited > limit) {
            log_error("streaming session", _session_id, "for stream", _stream_id,
                ": client did not read for", _timeout.milliseconds(),
                "ms in synchronous mode, closing the session and dropping the message");
            Close();
            return;
          }
        }
      } else {
        // ignore this message
        log_debug("session", _session_id, ": connection too slow: message discarded");
        return;
      }
    }
    _is_writing = true;

    auto handle_sent = [this, self, message](const boost::system::error_code &ec, size_t DEBUG_ONLY(bytes)) {
      _is_writing = false;
      if (ec) {
        log_info("session", _session_id, ": error sending data :", ec.message());
        CloseNow(ec);
      } else {
        DEBUG_ONLY(log_debug("session", _session_id, ": successfully sent", bytes, "bytes"));
        DEBUG_ASSERT_EQ(bytes, sizeof(message_size_type) + message->size());
      }
    };

    log_debug("session", _session_id, ": sending message of", message->size(), "bytes");

    // Start the write on the session's strand: the socket and the timer are
    // not thread-safe, and CloseNow() runs on the strand.
    boost::asio::post(_strand, [this, self, message, handle_sent]() mutable {
      if (_is_closed || !_socket.is_open()) {
        _is_writing = false;
        return;
      }
      _deadline.expires_after(_timeout.to_chrono());
      boost::asio::async_write(_socket, message->GetBufferSequence(),
        boost::asio::bind_executor(_strand, std::move(handle_sent)));
    });
  }

  void ServerSession::Close() {
    boost::asio::post(_strand, [self=shared_from_this()]() { self->CloseNow(); });
  }

  void ServerSession::StartTimer() {
    if (_deadline.expiry() <= std::chrono::steady_clock::now()) {
      log_debug("session", _session_id, "timed out");
      Close();
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

  void ServerSession::CloseNow(boost::system::error_code ec) {
    if (_is_closed.exchange(true)) return;
    _deadline.cancel();
    if (!ec)
    {
      if (_socket.is_open()) {
        boost::system::error_code ec2;
        _socket.shutdown(boost::asio::socket_base::shutdown_both, ec2);
        _socket.close();
      }
    }
    _on_closed(shared_from_this());
    log_debug("session", _session_id, "closed");
  }

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
