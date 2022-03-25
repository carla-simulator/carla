// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/router.h"

#include "carla/multigpu/listener.h"
#include "carla/streaming/EndPoint.h"

namespace carla {
namespace multigpu {

Router::Router(void) :
  _next(0) { }

Router::~Router() {
  ClearSessions();
  _listener->Stop();
  _pool.Stop();
}

Router::Router(uint16_t port) :
  _next(0) {

  // prepare server
  carla::multigpu::Listener::callback_function_type on_open = [self = this](std::shared_ptr<carla::multigpu::Primary> session) { self->ConnectSession(session); };
  carla::multigpu::Listener::callback_function_type on_close = [self = this](std::shared_ptr<carla::multigpu::Primary> session) { self->DisconnectSession(session); };
  carla::multigpu::Listener::callback_function_type_response on_response = [](std::shared_ptr<carla::multigpu::Primary> session, carla::Buffer buffer) { 
    log_info("Got data from secondary: ", buffer.size());
    };
    
  _endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("0.0.0.0"), port);
  _listener = std::make_shared<carla::multigpu::Listener>(_pool.io_context(), _endpoint);
  _listener->Listen(on_open, on_close, on_response);
  log_info("Listening at ", _endpoint);
}

void Router::AsyncRun(size_t worker_threads) {
  _pool.AsyncRun(worker_threads);
}

boost::asio::ip::tcp::endpoint Router::GetLocalEndpoint() const {
  return _endpoint;
}

void Router::ConnectSession(std::shared_ptr<Primary> session) {
  DEBUG_ASSERT(session != nullptr);
  std::lock_guard<std::mutex> lock(_mutex);
  _sessions.emplace_back(std::move(session));
  log_debug("Connected secondary servers:", _sessions.size());
}

void Router::DisconnectSession(std::shared_ptr<Primary> session) {
  DEBUG_ASSERT(session != nullptr);
  std::lock_guard<std::mutex> lock(_mutex);
  if (_sessions.size() == 0) return;
  _sessions.erase(
      std::remove(_sessions.begin(), _sessions.end(), session),
      _sessions.end());
  log_debug("Connected secondary servers:", _sessions.size());
}

void Router::ClearSessions() {
  std::lock_guard<std::mutex> lock(_mutex);
  _sessions.clear();
  log_debug("Disconnecting all secondary servers");
}

} // namespace multigpu
} // namespace carla
