// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/NonCopyable.h"
#include "carla/profiler/LifetimeProfiled.h"
#include "carla/streaming/detail/tcp/Message.h"
#include "carla/streaming/detail/Token.h"
#include "carla/streaming/detail/Types.h"

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

  /// A client that connects to a single stream.
  ///
  /// @warning This client should be stopped before releasing the shared pointer
  /// or won't be destroyed.
  class Secondary
    : public std::enable_shared_from_this<Secondary>,
      private profiler::LifetimeProfiled,
      private NonCopyable {
  public:

    using endpoint = boost::asio::ip::tcp::endpoint;
    using protocol_type = endpoint::protocol_type;
    using callback_function_type = std::function<void (Buffer)>;

    Secondary(
        boost::asio::io_context &io_context,
        boost::asio::ip::tcp::endpoint ep,
        callback_function_type callback);

    ~Secondary();

    void Connect();

    void Stop();

    void Write(std::shared_ptr<const carla::streaming::detail::tcp::Message> message);
    
    void Write(std::string text);

  private:

    void Reconnect();

    void ReadData();

    callback_function_type _callback;

    boost::asio::ip::tcp::socket _socket;

    boost::asio::ip::tcp::endpoint _endpoint;

    boost::asio::io_context::strand _strand;

    boost::asio::deadline_timer _connection_timer;
    
    std::shared_ptr<BufferPool> _buffer_pool;

    std::atomic_bool _done{false};
  };

} // namespace multigpu
} // namespace carla
