// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
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

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <atomic>
#include <functional>
#include <memory>

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

    void Stop();

    void AsyncRun(size_t worker_threads);

    void Write(std::shared_ptr<const carla::streaming::detail::tcp::Message> message);
    void Write(Buffer buffer);
    void Write(std::string text);

    SecondaryCommands &GetCommander() {
      return _commander;
    }

    template <typename... Buffers>
    static auto MakeMessage(Buffers &&... buffers) {
      static_assert(
          are_same<Buffer, Buffers...>::value,
          "This function only accepts arguments of type Buffer.");
      return std::make_shared<const carla::streaming::detail::tcp::Message>(std::move(buffers)...);
    }

  private:

    void Reconnect();

    void ReadData();

    ThreadPool                        _pool;
    boost::asio::ip::tcp::socket      _socket;
    boost::asio::ip::tcp::endpoint    _endpoint;
    boost::asio::io_context::strand   _strand;
    boost::asio::deadline_timer       _connection_timer;
    std::shared_ptr<BufferPool>       _buffer_pool;
    std::atomic_bool                  _done {false};
    SecondaryCommands                 _commander;
  };

} // namespace multigpu
} // namespace carla
