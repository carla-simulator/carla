// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

/// @file listener.h
/// @brief Multi-GPU session listener for TCP connections
///
/// Provides TCP server functionality for managing multi-GPU sessions.

#ifndef CARLA_MULTIGPU_LISTENER_H
#define CARLA_MULTIGPU_LISTENER_H

#include "carla/NonCopyable.h"
#include "carla/Time.h"
#include "carla/Buffer.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>

#include <atomic>
#include <functional>
#include <memory>

namespace carla {
namespace multigpu {

  class Primary;

  /// @brief TCP server listener for multi-GPU sessions.
  ///
  /// Listener manages TCP server sessions for multi-GPU streaming. It accepts
  /// incoming connections and creates Primary session objects for each client.
  ///
  /// @warning This server cannot be destructed before its io_context is stopped.
  ///
  /// @example
  /// ```cpp
  /// boost::asio::io_context io_context;
  /// auto listener = std::make_shared<Listener>(io_context, endpoint);
  ///
  /// listener->Listen(
  ///   [](std::shared_ptr<Primary> session) {
  ///     // Session opened
  ///   },
  ///   [](std::shared_ptr<Primary> session) {
  ///     // Session closed
  ///   },
  ///   [](std::shared_ptr<Primary> session, Buffer response) {
  ///     // Response received
  ///   }
  /// );
  ///
  /// io_context.run();
  /// ```
  class Listener : public std::enable_shared_from_this<Listener>, private NonCopyable {
  public:
    /// @brief TCP endpoint type.
    using endpoint = boost::asio::ip::tcp::endpoint;
    
    /// @brief TCP protocol type.
    using protocol_type = endpoint::protocol_type;
    
    /// @brief Session type (shared pointer to Primary).
    using Session = std::shared_ptr<Primary>;
    
    /// @brief Callback function type for session open/close events.
    using callback_function_type = std::function<void(std::shared_ptr<Primary>)>;
    
    /// @brief Callback function type for response events.
    using callback_function_type_response = std::function<void(std::shared_ptr<Primary>, carla::Buffer)>;

    /// @brief Construct a new Listener.
    ///
    /// @param io_context Boost ASIO IO context.
    /// @param ep Endpoint to listen on.
    explicit Listener(boost::asio::io_context &io_context, endpoint ep);
    
    /// @brief Destructor - stops listening.
    ~Listener();

    /// @brief Get local endpoint.
    ///
    /// @return Local TCP endpoint.
    endpoint GetLocalEndpoint() const {
      return _acceptor.local_endpoint();
    }

    /// @brief Set session timeout.
    ///
    /// @param timeout New timeout duration.
    ///
    /// @note Applies only to newly created sessions. Default is 10 seconds.
    void SetTimeout(time_duration timeout) {
      _timeout = timeout;
    }

    /// @brief Start listening for connections.
    ///
    /// @param on_session_opened Callback invoked on new connection.
    /// @param on_session_closed Callback invoked when session closes.
    /// @param on_response Callback invoked when response is received.
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

    /// @brief Stop listening for connections.
    void Stop();

  private:
    /// @brief Open a new session.
    ///
    /// @param timeout Session timeout.
    /// @param on_session_opened Open callback.
    /// @param on_session_closed Close callback.
    /// @param on_response Response callback.
    void OpenSession(
        time_duration timeout,
        callback_function_type on_session_opened,
        callback_function_type on_session_closed,
        callback_function_type_response on_response);

    boost::asio::io_context &_io_context;  ///< IO context reference
    boost::asio::ip::tcp::acceptor _acceptor; ///< TCP acceptor
    time_duration _timeout;  ///< Session timeout duration
    std::atomic<size_t> _session_counter;  ///< Session counter for unique IDs
  };

} // namespace multigpu
} // namespace carla

#endif // CARLA_MULTIGPU_LISTENER_H
