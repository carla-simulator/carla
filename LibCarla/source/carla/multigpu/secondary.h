// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/NonCopyable.h"
#include "carla/TypeTraits.h"
#include "carla/profiler/LifetimeProfiled.h"
#include "carla/multigpu/secondaryCommands.h"
#include "carla/streaming/detail/tcp/Message.h"
#include "carla/streaming/detail/Token.h"
#include "carla/streaming/detail/Types.h"
#include "carla/ThreadPool.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

#include <atomic>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>

namespace carla {

  class BufferPool;

namespace multigpu {

  class Secondary
    : public std::enable_shared_from_this<Secondary>,
      private profiler::LifetimeProfiled,
      private NonCopyable {
  public:

    using endpoint = boost::asio::ip::tcp::endpoint;
    using protocol_type = endpoint::protocol_type;

    Secondary(boost::asio::ip::tcp::endpoint ep, SecondaryCommands::callback_type callback);
    Secondary(std::string ip, uint16_t port, SecondaryCommands::callback_type callback);
    ~Secondary();

    void Connect();

    /// Blocks until the io threads have exited. Must not be called from one
    /// of them (i.e. from a command callback).
    void Stop();

    void AsyncRun(size_t worker_threads);

    void Write(std::shared_ptr<const carla::streaming::detail::tcp::Message> message);
    void Write(Buffer buffer);
    void Write(std::string text);

    /// Whether the connection to the primary is up: false before the first
    /// connect, after a read fails (the primary is gone; this session is not
    /// resumed) and after Stop(). Any thread.
    bool IsConnected() const {
      return _connected;
    }

    SecondaryCommands &GetCommander() {
      return _commander;
    }

    template <typename... Buffers>
    static auto MakeMessage(Buffers... buffers) {
      static_assert(
          are_same<SharedBufferView, Buffers...>::value,
          "This function only accepts arguments of type BufferView.");
      return std::make_shared<const carla::streaming::detail::tcp::Message>(buffers...);
    }

  private:

    using message_type = std::shared_ptr<const carla::streaming::detail::tcp::Message>;

    void Reconnect();

    void ReadData();

    /// Starts writing the front of the write queue. Strand only.
    void WriteNext();

    /// Marks the connection lost and closes the socket. Strand only.
    void Disconnect();

    ThreadPool                        _pool;
    boost::asio::ip::tcp::socket      _socket;
    boost::asio::ip::tcp::endpoint    _endpoint;
    boost::asio::io_context::strand   _strand;
    boost::asio::steady_timer         _connection_timer;
    std::shared_ptr<BufferPool>       _buffer_pool;
    std::atomic_bool                  _done {false};
    std::atomic_bool                  _connected {false};
    /// Strand only: messages waiting to be written, oldest first, with a
    /// single write in flight so that two replies never interleave.
    std::deque<message_type>          _write_queue;
    bool                              _is_writing = false;
    /// Strand only: bumped by Disconnect(), so that the completion of a write
    /// it dropped leaves the queue alone.
    uint64_t                          _write_generation = 0u;
    std::once_flag                    _run_flag;
    SecondaryCommands                 _commander;
  };

} // namespace multigpu
} // namespace carla
