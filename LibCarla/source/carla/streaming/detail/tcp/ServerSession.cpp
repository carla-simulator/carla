// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/tcp/ServerSession.h"
#include "carla/streaming/detail/tcp/Server.h"

#include "carla/Debug.h"
#include "carla/Logging.h"

#include "carla/Futex.h"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/post.hpp>

#include <atomic>
#include <thread>
#include <mutex>

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  static std::atomic_size_t SESSION_COUNTER { 0 };

  ServerSession::ServerSession(
      boost::asio::io_context &io_context,
      const time_duration timeout,
      Server &server)
    : LIBCARLA_INITIALIZE_LIFETIME_PROFILER(
          std::string("tcp server session ") +
          std::to_string(SESSION_COUNTER.load(std::memory_order_acquire))),
      _server(server),
      _session_id(
        SESSION_COUNTER.fetch_add(1, std::memory_order_release) + 1),
      _socket(io_context),
      _timeout(timeout),
      _deadline(io_context),
      _strand(io_context),
      _is_writing() {
    }

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
      _deadline.expires_from_now(_timeout);
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
      if (_is_writing.load(std::memory_order_acquire)) {
        if (_server.IsSynchronousMode()) {
          // wait until previous message has been sent
          auto wait_start = std::chrono::high_resolution_clock::now();
          if (LOOP_WAIT_MODE == 0)
          {
            carla::futex::wait(_is_writing, false);
          }
          else
          {
            if (LOOP_WAIT_MODE == 1)
            {
              while (_is_writing.load(std::memory_order_acquire) != 0)
                std::this_thread::yield();
            }
            else
            {
              while (_is_writing.load(std::memory_order_acquire) != 0)
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
          }
#ifndef LOOP_WAIT_BENCHMARK
          (void)wait_start;
#else
          auto wait_end = std::chrono::high_resolution_clock::now();
          auto dt = wait_end - wait_start;
          char path[512];
          snprintf(path, sizeof(path), "~/%s-%u.txt", __FUNCTION__, LOOP_WAIT_MODE);
          static std::mutex lock;
          std::lock_guard<std::mutex> guard(lock);
          auto file = fopen(path, "a");
          fprintf(
            file,
            "%lluus\n",
            (unsigned long long)std::chrono::duration_cast<std::chrono::microseconds>(dt).count());
          fclose(file);
#endif
        } else {
          // ignore this message
          log_debug("session", _session_id, ": connection too slow: message discarded");
          return;
        }
      }
      _is_writing.store(true, std::memory_order_relaxed);
      auto handle_sent = [this, self, message](
        const boost::system::error_code &ec,
        size_t DEBUG_ONLY(bytes)) {
        _is_writing.store(false, std::memory_order_release);
        if (LOOP_WAIT_MODE == 0)
          carla::futex::wake(_is_writing);
        if (ec) {
          log_info("session", _session_id, ": error sending data :", ec.message());
          CloseNow(ec);
        } else {
          DEBUG_ONLY(log_debug("session", _session_id, ": successfully sent", bytes, "bytes"));
          DEBUG_ASSERT_EQ(bytes, sizeof(message_size_type) + message->size());
        }
      };

      log_debug("session", _session_id, ": sending message of", message->size(), "bytes");

      _deadline.expires_from_now(_timeout);
      boost::asio::async_write(_socket, message->GetBufferSequence(), 
        boost::asio::bind_executor(_strand, handle_sent));
  }

  void ServerSession::Close() {
    boost::asio::post(_strand, [self=shared_from_this()]() { self->CloseNow(); });
  }

  void ServerSession::StartTimer() {
    if (_deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
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
