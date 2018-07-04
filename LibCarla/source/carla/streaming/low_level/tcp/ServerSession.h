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

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>

#include <array>
#include <memory>

namespace carla {
namespace streaming {
namespace low_level {
namespace tcp {

namespace detail {

  static std::atomic_size_t SESSION_COUNTER{0u};

} // namespace detail

  /// A TCP server session. When a session opens, it reads from the socket a
  /// stream id object and passes itself to the callback functor. The session
  /// closes itself after @a timeout of inactivity is met.
  class ServerSession
    : public std::enable_shared_from_this<ServerSession>,
      private boost::noncopyable {
  public:

    using socket_type = boost::asio::ip::tcp::socket;
    using duration_type = timeout_type;

    explicit ServerSession(boost::asio::io_service &io_service, duration_type timeout)
      : _session_id(detail::SESSION_COUNTER++),
        _socket(io_service),
        _timeout(timeout),
        _deadline(io_service),
        _strand(io_service) {}

    ~ServerSession() {
      _deadline.cancel();
    }

    /// Starts the session and calls @a callback after successfully reading the
    /// stream id.
    ///
    /// @pre Callback function signature:
    /// `void(std::shared_ptr<ServerSession>)`.
    template <typename Functor>
    void Open(Functor callback) {
      StartTimer();
      auto self = shared_from_this(); // To keep myself alive.
      _strand.post([=]() {

        auto handle_query = [this, self, callback](
            const boost::system::error_code &ec,
            size_t DEBUG_ONLY(bytes_received)) {
          DEBUG_ASSERT_EQ(bytes_received, sizeof(_stream_id));
          if (!ec) {
            log_debug("session", _session_id, "for stream", _stream_id, " started");
            _socket.get_io_service().post([=]() { callback(self); });
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

    stream_id_type get_stream_id() const {
      // Note that the stream id isn't synchronized. This function should only be
      // called from the @a callback function, and after that point the stream_id
      // can't change.
      return _stream_id;
    }

    /// Writes some data to the socket.
    void Write(std::shared_ptr<const Message> message) {
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
            message->encode(),
            _strand.wrap(handle_sent));
      });
    }

    void Close() {
      _strand.post([this, self = shared_from_this()]() {
        if (_socket.is_open()) {
          _socket.close();
        }
        log_debug("session", _session_id, "closed");
      });
    }

  private:

    void StartTimer() {
      if (_deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
        log_debug("session", _session_id, "timed out");
        Close();
      } else {
        _deadline.async_wait([self = shared_from_this()](boost::system::error_code) {
          self->StartTimer();
        });
      }
     }

    friend class Server;

    const size_t _session_id;

    stream_id_type _stream_id = 0u;

    socket_type _socket;

    duration_type _timeout;

    boost::asio::deadline_timer _deadline;

    boost::asio::io_service::strand _strand;

    bool _is_writing = false;
  };

} // namespace tcp
} // namespace low_level
} // namespace streaming
} // namespace carla
