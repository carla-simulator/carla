// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/router.h"

#include "carla/multigpu/listener.h"
#include "carla/streaming/EndPoint.h"

#include <algorithm>
#include <stdexcept>

namespace carla {
namespace multigpu {

Router::Router(void) :
  _next(0) { }

Router::~Router() {
  Stop();
}

void Router::Stop() {
  ClearSessions();
  // _pool.Stop() joins every worker thread that may still be inside
  // _pool.io_context().run() (started by AsyncRun()) before the listener's
  // acceptor is closed, so no in-flight accept can race with the teardown
  // (boost::asio objects are not thread-safe against a concurrent close).
  _pool.Stop();
  // _listener.reset() alone is NOT enough to close the acceptor in
  // production: SetCallbacks() (always called before AsyncRun(), see
  // CarlaServer.cpp) leaves a pending async_accept whose handler captures
  // shared_from_this(), and _commander (PrimaryCommands) holds a
  // shared_ptr<Router> back to this object once set_router() runs -- a
  // reference cycle that means ~Router() itself never runs in production.
  // So ~Listener() would never fire and the listening socket would stay
  // open for the rest of the process if this explicit call were removed.
  // It is safe to call unconditionally: Listener::Stop() is idempotent, so
  // it does not matter whether ~Listener() also happens to run later (as it
  // does in a unit test with no SetCallbacks() call and no reference cycle).
  if (_listener) {
    _listener->Stop();
  }
  _listener.reset();
}

Router::Router(uint16_t port) :
  _next(0) {

  _endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("0.0.0.0"), port);
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

  carla::multigpu::Listener::callback_function_type_response on_response =
    [=](std::shared_ptr<carla::multigpu::Primary> session, carla::Buffer buffer) {
      auto self = weak.lock();
      if (!self) return;
      std::scoped_lock<std::mutex> lock(self->_mutex);
      auto prom =self-> _promises.find(session.get());
      if (prom != self->_promises.end()) {
        log_info("Got data from secondary (with promise): ", buffer.size());
        prom->second->set_value({session, std::move(buffer)});
        self->_promises.erase(prom);
      } else {
        log_info("Got data from secondary (without promise): ", buffer.size());
      }
    };

  _commander.set_router(shared_from_this());

  _listener->Listen(on_open, on_close, on_response);
  log_info("Listening at ", _endpoint);
}

void Router::SetNewConnectionCallback(std::function<void(void)> func)
{
  _callback = func;
}

void Router::AsyncRun(size_t worker_threads) {
  _pool.AsyncRun(worker_threads);
}

boost::asio::ip::tcp::endpoint Router::GetLocalEndpoint() const {
  return _endpoint;
}

void Router::ConnectSession(std::shared_ptr<Primary> session) {
  DEBUG_ASSERT(session != nullptr);
  std::scoped_lock<std::mutex> lock(_mutex);
  _sessions.emplace_back(std::move(session));
  log_info("Connected secondary servers:", _sessions.size());
  // run external callback for new connections
  if (_callback)
    _callback();
}

void Router::DisconnectSession(std::shared_ptr<Primary> session) {
  DEBUG_ASSERT(session != nullptr);
  std::scoped_lock<std::mutex> lock(_mutex);
  if (_sessions.size() > 0) {
    _sessions.erase(
        std::remove(_sessions.begin(), _sessions.end(), session),
        _sessions.end());
    log_info("Connected secondary servers:", _sessions.size());
  }

  // A request may still be waiting on this session's response; without this,
  // the caller blocked in std::future::get() would hang forever.
  auto pending = _promises.find(session.get());
  if (pending != _promises.end()) {
    RejectPromise(pending->second, "secondary server disconnected before responding");
    _promises.erase(pending);
  }
}

void Router::RejectAllPending(std::string_view reason) {
  for (auto &entry : _promises) {
    RejectPromise(entry.second, reason);
  }
  _promises.clear();
}

void Router::RejectPromise(
    std::shared_ptr<std::promise<SessionInfo>> promise,
    std::string_view reason) {
  log_error("multigpu router: rejecting pending request: ", reason);
  promise->set_exception(std::make_exception_ptr(std::runtime_error(std::string(reason))));
}

void Router::ClearSessions() {
  std::scoped_lock<std::mutex> lock(_mutex);
  _sessions.clear();
  // Symmetric with DisconnectSession: any request still waiting on a
  // response from a session that is about to disappear must not be left to
  // hang forever.
  RejectAllPending("router is shutting down");
  log_info("Disconnecting all secondary servers");
}

void Router::Write(MultiGPUCommand id, Buffer &&buffer) {
  // define the command header
  CommandHeader header;
  header.id = id;
  header.size = buffer.size();
  Buffer buf_header(reinterpret_cast<uint8_t *>(&header), sizeof(header));

  auto view_header = carla::BufferView::CreateFrom(std::move(buf_header));
  auto view_data = carla::BufferView::CreateFrom(std::move(buffer));
  auto message = Primary::MakeMessage(view_header, view_data);

  // write to multiple servers
  std::scoped_lock<std::mutex> lock(_mutex);
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
  Buffer buf_header(reinterpret_cast<uint8_t *>(&header), sizeof(header));

  auto view_header = carla::BufferView::CreateFrom(std::move(buf_header));
  auto view_data = carla::BufferView::CreateFrom(std::move(buffer));
  auto message = Primary::MakeMessage(view_header, view_data);

  // create the promise for the posible answer
  auto response = std::make_shared<std::promise<SessionInfo>>();

  // write to the next server only
  std::scoped_lock<std::mutex> lock(_mutex);
  if (_next >= _sessions.size()) {
    _next = 0;
  }
  if (_next < _sessions.size()) {
    auto s = _sessions[_next];
    if (s == nullptr) {
      RejectPromise(response, "no secondary session available to route the command");
    } else if (_promises.contains(s.get())) {
      // _promises holds at most one slot per session (there is no per-request
      // correlation id in the wire protocol to demultiplex more than one).
      // Silently overwriting the earlier entry here would orphan it forever
      // instead of just failing fast, so refuse the newer request instead.
      // Every real caller (PrimaryCommands) already serializes its own
      // requests one at a time; this only ever fires for a caller that
      // bypasses that discipline.
      RejectPromise(response, "a request is already pending on this secondary session");
    } else {
      _promises[s.get()] = response;
      s->Write(message);
    }
  } else {
    RejectPromise(response, "no secondary server connected");
  }
  ++_next;
  return response->get_future();
}

std::future<SessionInfo> Router::WriteToOne(std::weak_ptr<Primary> server, MultiGPUCommand id, Buffer &&buffer) {
  // define the command header
  CommandHeader header;
  header.id = id;
  header.size = buffer.size();
  Buffer buf_header(reinterpret_cast<uint8_t *>(&header), sizeof(header));

  auto view_header = carla::BufferView::CreateFrom(std::move(buf_header));
  auto view_data = carla::BufferView::CreateFrom(std::move(buffer));
  auto message = Primary::MakeMessage(view_header, view_data);

  // create the promise for the posible answer
  auto response = std::make_shared<std::promise<SessionInfo>>();

  // write to the specific server only
  std::scoped_lock<std::mutex> lock(_mutex);
  auto s = server.lock();
  // A session's socket can already be closed (disconnect in progress) while
  // the Primary object itself is kept alive a little longer by another
  // reference, so locking the weak_ptr alone is not enough: check it is
  // still a member of _sessions, which DisconnectSession removes it from
  // synchronously before anything else. Otherwise Write() would silently
  // no-op on the closed socket and this promise would never resolve.
  const bool still_connected =
      s && (std::find(_sessions.begin(), _sessions.end(), s) != _sessions.end());
  if (!still_connected) {
    RejectPromise(response, "secondary session is no longer connected");
  } else if (_promises.contains(s.get())) {
    // See the matching comment in WriteToNext: one slot per session, so an
    // overlapping request must be refused rather than silently orphaning
    // whichever one was already pending.
    RejectPromise(response, "a request is already pending on this secondary session");
  } else {
    _promises[s.get()] = response;
    s->Write(message);
  }
  return response->get_future();
}

} // namespace multigpu
} // namespace carla
