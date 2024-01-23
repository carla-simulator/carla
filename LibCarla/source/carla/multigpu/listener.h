// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/Time.h"
#include "carla/Buffer.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>

#include <atomic>

namespace carla {
namespace multigpu {
  
  class Primary;

  /// @warning This server cannot be destructed before its @a io_context is
  /// stopped.
  class Listener : public std::enable_shared_from_this<Listener>, private NonCopyable {
  public:

    using endpoint = boost::asio::ip::tcp::endpoint;
    using protocol_type = endpoint::protocol_type;
    using Session = std::shared_ptr<Primary>;
    using callback_function_type = std::function<void(std::shared_ptr<Primary>)>;
    using callback_function_type_response = std::function<void(std::shared_ptr<Primary>, carla::Buffer)>;

    explicit Listener(boost::asio::io_context &io_context, endpoint ep);
    ~Listener();
    
    endpoint GetLocalEndpoint() const {
      return _acceptor.local_endpoint();
    }

    /// Set session time-out. Applies only to newly created sessions. By default
    /// the time-out is set to 10 seconds.
    void SetTimeout(time_duration timeout) {
      _timeout = timeout;
    }

    /// Start listening for connections. On each new connection, @a
    /// on_session_opened is called, and @a on_session_closed when the session
    /// is closed, also @a on_response is called when an answer is received.
    void Listen(callback_function_type on_session_opened, 
                callback_function_type on_session_closed,
                callback_function_type_response on_response) {
      boost::asio::post(
        _io_context,
        [
          this,
          on_session_opened = std::move(on_session_opened),
          on_session_closed = std::move(on_session_closed),
          on_response = std::move(on_response)
        ]()
        {
          OpenSession(
            _timeout,
            on_session_opened,
            on_session_closed,
            on_response);
      });
    }

    void Stop();

  private:

    void OpenSession(
        time_duration timeout,
        callback_function_type on_session_opened,
        callback_function_type on_session_closed,
        callback_function_type_response on_response);

    boost::asio::io_context         &_io_context;
    boost::asio::ip::tcp::acceptor  _acceptor;
    std::atomic<time_duration>      _timeout;
  };

} // namespace multigpu
} // namespace carla
