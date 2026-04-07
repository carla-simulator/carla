// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

/// @file primary.h
/// @brief Primary GPU server session for multi-GPU streaming
///
/// Provides TCP server session management for primary GPU in multi-GPU setup.

#ifndef CARLA_MULTIGPU_PRIMARY_H
#define CARLA_MULTIGPU_PRIMARY_H

#include "carla/NonCopyable.h"
#include "carla/Time.h"
#include "carla/TypeTraits.h"
#include "carla/profiler/LifetimeProfiled.h"
#include "carla/streaming/detail/Types.h"
#include "carla/streaming/detail/tcp/Message.h"
#include "carla/multigpu/listener.h"

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <functional>
#include <memory>
#include <string>

namespace carla {
namespace multigpu {

  /// @brief A TCP server session for primary GPU in multi-GPU setup.
  ///
  /// Primary manages a TCP server session that reads stream ID from socket
  /// and passes itself to callback functors. The session automatically closes
  /// after timeout of inactivity.
  ///
  /// This class is non-copyable and uses shared_ptr for lifetime management.
  ///
  /// @example
  /// ```cpp
  /// auto primary = std::make_shared<Primary>(io_context, timeout, listener);
  /// primary->Open(on_opened, on_closed, on_response);
  /// ```
  class Primary
    : public std::enable_shared_from_this<Primary>,
      private profiler::LifetimeProfiled,
      private NonCopyable {
  public:
    /// @brief Socket type for TCP connections.
    using socket_type = boost::asio::ip::tcp::socket;

    /// @brief Construct a new Primary session.
    ///
    /// @param io_context Boost ASIO IO context.
    /// @param timeout Session timeout duration.
    /// @param server Listener that manages this session.
    explicit Primary(
        boost::asio::io_context &io_context,
        time_duration timeout,
        Listener &server);

    /// @brief Destructor - closes session.
    ~Primary();

    /// @brief Start the session and register callbacks.
    ///
    /// @param on_opened Callback invoked after successfully reading stream ID.
    /// @param on_closed Callback invoked when session is closed.
    /// @param on_response Callback for handling responses.
    void Open(
        Listener::callback_function_type on_opened,
        Listener::callback_function_type on_closed,
        Listener::callback_function_type_response on_response);

    /// @brief Create a message from buffer views.
    ///
    /// @tparam Buffers Variadic template of BufferView types.
    /// @param buffers Buffer views to include in message.
    /// @return Shared pointer to TCP message.
    ///
    /// @note All arguments must be of type SharedBufferView.
    template <typename... Buffers>
    static auto MakeMessage(Buffers... buffers) {
      static_assert(
          are_same<SharedBufferView, Buffers...>::value,
          "This function only accepts arguments of type BufferView.");
      return std::make_shared<const carla::streaming::detail::tcp::Message>(buffers...);
    }

    /// @brief Write data to socket.
    ///
    /// @param message Message to write.
    void Write(std::shared_ptr<const carla::streaming::detail::tcp::Message> message);

    /// @brief Write string data to socket.
    ///
    /// @param text String to write.
    void Write(std::string text);

    /// @brief Read data from socket.
    void ReadData();

    /// @brief Write buffers to socket.
    ///
    /// @tparam Buffers Variadic template of BufferView types.
    /// @param buffers Buffer views to write.
    template <typename... Buffers>
    void Write(Buffers... buffers) {
      Write(MakeMessage(buffers...));
    }

    /// @brief Post a job to close the session.
    void Close();

  private:
    /// @brief Start inactivity timer.
    void StartTimer();

    /// @brief Close session immediately.
    ///
    /// @param ec Error code (optional).
    void CloseNow(boost::system::error_code ec = boost::system::error_code());

    friend class Listener;

    Listener &_server;                    ///< Parent listener
    const size_t _session_id;             ///< Unique session identifier
    socket_type _socket;                  ///< TCP socket
    time_duration _timeout;               ///< Inactivity timeout
    boost::asio::deadline_timer _deadline; ///< Inactivity timer
    boost::asio::io_context::strand _strand; ///< ASIO strand for thread safety
    Listener::callback_function_type _on_closed; ///< Close callback
    Listener::callback_function_type_response _on_response; ///< Response callback
    std::shared_ptr<BufferPool> _buffer_pool; ///< Buffer pool for memory management
    bool _is_writing = false;             ///< Writing state flag
  };

} // namespace multigpu
} // namespace carla

#endif // CARLA_MULTIGPU_PRIMARY_H
