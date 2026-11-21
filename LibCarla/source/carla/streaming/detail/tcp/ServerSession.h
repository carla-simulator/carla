// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/Time.h"
#include "carla/TypeTraits.h"
#include "carla/profiler/LifetimeProfiled.h"
#include "carla/streaming/detail/StreamCounters.h"
#include "carla/streaming/detail/Types.h"
#include "carla/streaming/detail/tcp/Message.h"

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wshadow"
#endif
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  class Server;

  /// What happened to a message given to ServerSession::Write.
  enum class WriteStatus : uint8_t {
    /// The message is the next one written to this session's socket. It
    /// reaches the client unless the session closes first.
    Accepted,
    /// Asynchronous mode only: the previous message was still being written,
    /// so this one was skipped. The session stays open.
    Busy,
    /// The session is closing or closed (in synchronous mode, also after the
    /// client did not read for the whole time-out). Nothing more will be
    /// written to it.
    Closed
  };

  /// A TCP server session. When a session opens, it reads from the socket a
  /// stream id object and passes itself to the callback functor. The session
  /// closes itself after @a timeout of inactivity is met.
  class ServerSession
    : public std::enable_shared_from_this<ServerSession>,
      private profiler::LifetimeProfiled,
      private NonCopyable {
  public:

    using socket_type = boost::asio::ip::tcp::socket;
    using callback_function_type = std::function<void(std::shared_ptr<ServerSession>)>;

    explicit ServerSession(
        boost::asio::io_context &io_context,
        time_duration timeout,
        Server &server);

    /// Starts the session and calls @a on_opened after successfully reading the
    /// stream id, and @a on_closed once the session is closed.
    void Open(
        callback_function_type on_opened,
        callback_function_type on_closed);

    /// @warning This function should only be called after the session is
    /// opened. It is safe to call this function from within the @a callback.
    stream_id_type get_stream_id() const {
      return _stream_id;
    }

    template <typename... Buffers>
    static auto MakeMessage(Buffers... buffers) {
      static_assert(
          are_same<SharedBufferView, Buffers...>::value,
          "This function only accepts arguments of type BufferView.");
      return std::make_shared<const Message>(buffers...);
    }

    /// Writes some data to the socket. There is at most one message in flight
    /// per session: in synchronous mode this waits for the previous one (up
    /// to the time-out, then closes the session); in asynchronous mode it
    /// skips the message instead.
    WriteStatus Write(std::shared_ptr<const Message> message);

    /// Writes some data to the socket.
    template <typename... Buffers>
    WriteStatus Write(Buffers... buffers) {
      return Write(MakeMessage(buffers...));
    }

    /// Post a job to close the session. From now on Write returns
    /// WriteStatus::Closed.
    void Close();

    /// True once the session started closing; it never becomes false again.
    bool IsClosing() const {
      return _is_closing.load(std::memory_order_acquire);
    }

    /// Where this session adds its counters besides its own: the counters of
    /// the stream it joined. Set by the stream before it writes to the
    /// session.
    void SetStreamCounters(StreamCounters &counters) {
      _stream_counters = &counters;
    }

    /// This session's own counters.
    SessionCountersRow GetCountersRow() const;

  private:

    void StartTimer();

    void CloseNow(boost::system::error_code ec = boost::system::error_code());

    /// Takes the right to write the next message, if it is free.
    WriteStatus Reserve();

    /// Synchronous mode: waits for the message in flight, then reserves.
    WriteStatus WaitAndReserve();

    /// Posts the write of a reserved message to the strand.
    void StartWrite(std::shared_ptr<const Message> message);

    /// Adds @a value to the counter @a member of this session and of its
    /// stream.
    void Count(StreamCounters::counter StreamCounters::*member, uint64_t value = 1u) {
      (_counters.*member).fetch_add(value, std::memory_order_relaxed);
      (_stream_counters->*member).fetch_add(value, std::memory_order_relaxed);
    }

    friend class Server;

    Server &_server;

    const size_t _session_id;

    stream_id_type _stream_id = 0u;

    socket_type _socket;

    time_duration _timeout;

    boost::asio::steady_timer _deadline;

    boost::asio::io_context::strand _strand;

    callback_function_type _on_closed;

    /// Set by the sending thread, cleared by the write handler on the strand.
    std::atomic_bool _is_writing{false};
    std::atomic_bool _is_closed{false};

    /// Set as soon as the session starts closing (Close(), the synchronous
    /// time-out, CloseNow), so that writers stop at once instead of reading
    /// the socket, which only the strand may touch.
    std::atomic_bool _is_closing{false};

    /// Peer address, set in Open() before the session joins a stream.
    std::string _remote;
    bool _loopback = false;

    StreamCounters _counters;

    StreamCounters *_stream_counters = &StreamCountersRegistry::Other();
  };

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
