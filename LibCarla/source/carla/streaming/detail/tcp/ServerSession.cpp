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

    {
      boost::system::error_code ec;
      const auto peer = _socket.remote_endpoint(ec);
      if (!ec) {
        _remote = peer.address().to_string() + ":" + std::to_string(peer.port());
        _loopback = peer.address().is_loopback();
      }
    }

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

  WriteStatus ServerSession::Reserve() {
    if (IsClosing()) {
      return WriteStatus::Closed;
    }
    if (_is_writing.exchange(true)) {
      return WriteStatus::Busy;
    }
    if (IsClosing()) {
      // Closed between the two checks; nothing is in flight, so give the
      // reservation back.
      _is_writing = false;
      return WriteStatus::Closed;
    }
    return WriteStatus::Accepted;
  }

  WriteStatus ServerSession::WaitAndReserve() {
    // Wait until the previous message has been sent, but not forever: the
    // caller is the game thread or the render thread, and a client that
    // stops reading this socket (a listen() callback that never returns, a
    // frozen process) would otherwise stall the whole server, every other
    // client included (#10233). Drop that client instead; its streaming
    // client reconnects on its own.
    const uint64_t start_ns = StreamCountersRegistry::NowNs();
    const auto start = std::chrono::steady_clock::now();
    const auto limit = _timeout.to_chrono();
    bool warned = false;
    WriteStatus status = WriteStatus::Busy;
    while (status == WriteStatus::Busy) {
      std::this_thread::sleep_for(std::chrono::microseconds(100));
      status = Reserve();
      if (status != WriteStatus::Busy) {
        break;
      }
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
        Count(&StreamCounters::timeouts);
        Close();
        status = WriteStatus::Closed;
      }
    }
    const uint64_t blocked = StreamCountersRegistry::NowNs() - start_ns;
    Count(&StreamCounters::blocked_writes);
    Count(&StreamCounters::blocked_ns, blocked);
    _stream_counters->blocked_histogram.Add(blocked);
    return status;
  }

  WriteStatus ServerSession::Write(std::shared_ptr<const Message> message) {
    DEBUG_ASSERT(message != nullptr);
    DEBUG_ASSERT(!message->empty());
    WriteStatus status = Reserve();
    if ((status == WriteStatus::Busy) && _server.IsSynchronousMode()) {
      status = WaitAndReserve();
    }
    switch (status) {
      case WriteStatus::Accepted:
        StartWrite(std::move(message));
        break;
      case WriteStatus::Busy:
        log_debug("session", _session_id, ": connection too slow: message discarded");
        Count(&StreamCounters::busy_drops);
        break;
      case WriteStatus::Closed:
        Count(&StreamCounters::closed_drops);
        break;
    }
    return status;
  }

  void ServerSession::StartWrite(std::shared_ptr<const Message> message) {
    auto self = shared_from_this();
    const uint64_t accepted_ns = StreamCountersRegistry::NowNs();

    log_debug("session", _session_id, ": sending message of", message->size(), "bytes");

    // Start the write on the session's strand: the socket and the timer are
    // not thread-safe, and CloseNow() runs on the strand.
    boost::asio::post(_strand, [this, self, message, accepted_ns]() mutable {
      const uint64_t start_ns = StreamCountersRegistry::NowNs();
      Count(&StreamCounters::queue_ns, start_ns - accepted_ns);
      _stream_counters->queue_histogram.Add(start_ns - accepted_ns);
      if (_is_closed || !_socket.is_open()) {
        Count(&StreamCounters::strand_drops);
        _is_writing = false;
        return;
      }

      auto handle_sent = [this, self, message, start_ns](
          const boost::system::error_code &ec,
          size_t bytes) {
        _is_writing = false;
        if (ec) {
          log_info("session", _session_id, ": error sending data :", ec.message());
          Count(&StreamCounters::write_errors);
          CloseNow(ec);
        } else {
          DEBUG_ONLY(log_debug("session", _session_id, ": successfully sent", bytes, "bytes"));
          DEBUG_ASSERT_EQ(bytes, sizeof(message_size_type) + message->size());
          const uint64_t send_ns = StreamCountersRegistry::NowNs() - start_ns;
          Count(&StreamCounters::writes);
          Count(&StreamCounters::wire_bytes, bytes);
          Count(&StreamCounters::send_ns, send_ns);
          _stream_counters->send_histogram.Add(send_ns);
        }
      };

      _deadline.expires_after(_timeout.to_chrono());
      boost::asio::async_write(_socket, message->GetBufferSequence(),
        boost::asio::bind_executor(_strand, std::move(handle_sent)));
    });
  }

  void ServerSession::Close() {
    _is_closing = true;
    boost::asio::post(_strand, [self=shared_from_this()]() { self->CloseNow(); });
  }

  SessionCountersRow ServerSession::GetCountersRow() const {
    SessionCountersRow row;
    row.stream_id = _stream_id;
    row.session_id = _session_id;
    row.remote = _remote;
    row.loopback = _loopback;
    row.writes = _counters.writes.load(std::memory_order_relaxed);
    row.wire_bytes = _counters.wire_bytes.load(std::memory_order_relaxed);
    row.busy_drops = _counters.busy_drops.load(std::memory_order_relaxed);
    row.closed_drops = _counters.closed_drops.load(std::memory_order_relaxed);
    row.timeouts = _counters.timeouts.load(std::memory_order_relaxed);
    row.blocked_ns = _counters.blocked_ns.load(std::memory_order_relaxed);
    row.queue_ns = _counters.queue_ns.load(std::memory_order_relaxed);
    row.send_ns = _counters.send_ns.load(std::memory_order_relaxed);
    return row;
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
    _is_closing = true;
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
