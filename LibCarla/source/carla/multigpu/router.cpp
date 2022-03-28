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

  _endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("0.0.0.0"), port);
  _listener = std::make_shared<carla::multigpu::Listener>(_pool.io_context(), _endpoint);
}

void Router::SetCallbacks() {
  // prepare server
  std::weak_ptr<Router> weak = shared_from_this();

  carla::multigpu::Listener::callback_function_type on_open = [=](std::shared_ptr<carla::multigpu::Primary> session) { 
    auto self = weak.lock();
    if (!self) return;
    self->ConnectSession(session);
  };

  carla::multigpu::Listener::callback_function_type on_close = [=](std::shared_ptr<carla::multigpu::Primary> session) { 
    auto self = weak.lock();
    if (!self) return;
    self->DisconnectSession(session); 
  };

  carla::multigpu::Listener::callback_function_type_response on_response = [=](std::shared_ptr<carla::multigpu::Primary> session, carla::Buffer buffer) { 
    auto self = weak.lock();
    if (!self) return;
    std::lock_guard<std::mutex> lock(self->_mutex);
    auto prom =self-> _promises.find(session.get());
    if (prom != self->_promises.end()) {
      log_info("Got data from secondary (with promise): ", buffer.size());
      prom->second->set_value({session, std::move(buffer)});
      self->_promises.erase(prom);
    } else {
      log_info("Got data from secondary (without promise): ", buffer.size());
    }
  };

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
  log_info("Connected secondary servers:", _sessions.size());
}

void Router::DisconnectSession(std::shared_ptr<Primary> session) {
  DEBUG_ASSERT(session != nullptr);
  std::lock_guard<std::mutex> lock(_mutex);
  if (_sessions.size() == 0) return;
  _sessions.erase(
      std::remove(_sessions.begin(), _sessions.end(), session),
      _sessions.end());
  log_info("Connected secondary servers:", _sessions.size());
}

void Router::ClearSessions() {
  std::lock_guard<std::mutex> lock(_mutex);
  _sessions.clear();
  log_info("Disconnecting all secondary servers");
}

void Router::Write(MultiGPUCommand id, Buffer &&buffer) {
  // define the command header
  CommandHeader header;
  header.id = id;
  header.size = buffer.size();
  Buffer buf_header((uint8_t *) &header, sizeof(header));
  
  auto message = Primary::MakeMessage(std::move(buf_header), std::move(buffer));
  
  // write to multiple servers
  std::lock_guard<std::mutex> lock(_mutex);
  for (auto &s : _sessions) {
    if (s != nullptr) {
      s->Write(message);
    }
  }
}

std::future<SessionInfo> Router::WriteToNext(MultiGPUCommand id, Buffer &&buffer) {
  // define the command header
  CommandHeader header;
  header.id = id;
  header.size = buffer.size();
  Buffer buf_header((uint8_t *) &header, sizeof(header));

  auto message = Primary::MakeMessage(std::move(buf_header), std::move(buffer));

  // create the promise for the posible answer
  auto response = std::make_shared<std::promise<SessionInfo>>();

  // write to the next server only
  std::lock_guard<std::mutex> lock(_mutex);
  if (_next >= _sessions.size()) {
    _next = 0;
  }
  if (_next < _sessions.size()) {
    // std::cout << "Sending to session " << _next << std::endl;
    auto s = _sessions[_next];
    if (s != nullptr) {
      _promises[s.get()] = response;
      std::cout << "Updated promise into map: " << _promises.size() << std::endl;
      s->Write(message);
    }
  }
  ++_next;
  return response->get_future();
}

} // namespace multigpu
} // namespace carla
