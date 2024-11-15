// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/listener.h"
#include "carla/multigpu/primary.h"

#include "carla/Logging.h"

#include <boost/asio/post.hpp>

#include <memory>

namespace carla {
namespace multigpu {

  Listener::Listener(boost::asio::io_context &io_context, endpoint ep)
    : _io_context(io_context),
      _acceptor(_io_context, std::move(ep)),
      _timeout(time_duration::seconds(1u)) {
        _acceptor.listen();
      }

  Listener::~Listener() {
    Stop();
  }
  
  void Listener::Stop() {
    _acceptor.cancel();
    _acceptor.close();
    _io_context.stop();
    _io_context.reset();
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
