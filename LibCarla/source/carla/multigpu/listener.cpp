// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/listener.h"
#include "carla/multigpu/primary.h"

#include "carla/Logging.h"
#include "carla/Sockets.h"

#include <boost/asio/post.hpp>

#include <memory>

namespace carla {
namespace multigpu {

  Listener::Listener(boost::asio::io_context &io_context, endpoint ep)
    : _io_context(io_context),
      _acceptor(_io_context, std::move(ep)),
      _timeout(time_duration::seconds(1u)) {
        _acceptor.listen();
        // Same fd-leak-into-forked-children concern as
        // carla::streaming::detail::tcp::Server; see carla/Sockets.h.
        carla::SetSocketCloseOnExec(_acceptor.native_handle());
      }

  Listener::~Listener() {
    Stop();
  }
  
  void Listener::Stop() {
    // Non-throwing overloads: Stop() can otherwise be invoked more than once
    // for the same Listener (explicitly, then again from ~Listener() if that
    // destructor runs while some other reference to this object is still
    // live) and the throwing overloads raise on the second call, since the
    // acceptor is already closed by then.
    boost::system::error_code ec;
    _acceptor.cancel(ec);
    _acceptor.close(ec);
    // The io_context is owned and torn down by whoever constructed it
    // (Router's ThreadPool); stopping/restarting it here as a side effect of
    // closing one acceptor is not this class's responsibility.
  }
  
  void Listener::OpenSession(
      time_duration timeout,
      callback_function_type on_opened,
      callback_function_type on_closed,
      callback_function_type_response on_response) {

    using boost::system::error_code;

    auto session = std::make_shared<Primary>(_io_context, timeout, *this);
    auto self = shared_from_this();
    
    auto handle_query = [on_opened, on_closed, on_response, session, self](const error_code &ec)
    {
      if (!ec) {
        session->Open(std::move(on_opened), std::move(on_closed), std::move(on_response));
      } else {
        log_error("Secondary server:", ec.message());
      }
    };

    _acceptor.async_accept(
      session->_socket,
      [this, handle_query, timeout, on_opened, on_closed, on_response](error_code ec)
      {
        boost::asio::post(
          _io_context,
          [ec, handle_query]()
          {
            handle_query(ec);
          });

        OpenSession(timeout, on_opened, on_closed, on_response);
      });
  }

} // namespace multigpu
} // namespace carla
