// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/router.h"

#include "carla/multigpu/listener.h"
#include "carla/streaming/EndPoint.h"

#include <algorithm>
#include <utility>

namespace carla {
namespace multigpu {

Router::Router(void) :
  _next(0) { }

Router::~Router() {
  Stop();
}

void Router::Stop() {
  ClearSessions();
  // Joins worker threads before the acceptor closes, so no in-flight accept
  // can race with teardown.
  _pool.Stop();
  // ~Router() never runs in production (PrimaryCommands holds a shared_ptr
  // back to this Router, see set_router()), so Stop() must release the
  // listening socket itself. Listener::Stop() is idempotent, so this is
  // safe even if ~Listener() also runs later.
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

  carla::multigpu::Listener::callback_function_type on_open = [weak](std::shared_ptr<carla::multigpu::Primary> session) {
    auto self = weak.lock();
    if (!self) return;
    self->ConnectSession(std::move(session));
  };

  carla::multigpu::Listener::callback_function_type on_close = [weak](std::shared_ptr<carla::multigpu::Primary> session) {
    auto self = weak.lock();
    if (!self) return;
    self->DisconnectSession(std::move(session));
  };

  carla::multigpu::Listener::callback_function_type_response on_response =
    [weak](std::shared_ptr<carla::multigpu::Primary> session, carla::Buffer buffer) {
      auto self = weak.lock();
      if (!self) return;
      self->HandleResponse(std::move(session), std::move(buffer));
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
  _new_sessions.emplace_back(session);
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
  _loading.erase(session.get());

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
  promise->set_value(SessionInfo{});
}

void Router::HandleResponse(std::shared_ptr<Primary> session, Buffer buffer) {
  std::scoped_lock<std::mutex> lock(_mutex);

  // Checked before the promise lookup: no command reply can equal this
  // exact marker (see commands.h), so it can never be misdelivered as data
  // for whatever happens to be pending on this session.
  const std::string_view payload(
      reinterpret_cast<const char *>(buffer.data()), buffer.size());
  if (const auto ready_load_id = ParseEpisodeReadyMessage(payload)) {
    log_info("Secondary episode ready, re-arming full resync");
    auto loading = _loading.find(session.get());
    if ((loading != _loading.end()) && (loading->second == *ready_load_id)) {
      _loading.erase(loading);
    } else if (loading != _loading.end()) {
      log_info("multigpu router: ignoring episode ready of load ", *ready_load_id,
          ", waiting for load ", loading->second);
    }
    if (_callback) {
      _callback();
    }
    return;
  }

  auto prom = _promises.find(session.get());
  if (prom != _promises.end()) {
    log_info("Got data from secondary (with promise): ", buffer.size());
    prom->second->set_value({std::move(session), std::move(buffer)});
    _promises.erase(prom);
  } else {
    log_warning("multigpu router: ignoring unsolicited data with no pending request: ", buffer.size(), " bytes");
  }
}

void Router::ClearSessions() {
  std::scoped_lock<std::mutex> lock(_mutex);
  _sessions.clear();
  _new_sessions.clear();
  _loading.clear();
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

load_map_id_type Router::WriteLoadMap(std::string_view map) {
  std::scoped_lock<std::mutex> lock(_mutex);
  const load_map_id_type load_id = ++_last_load_id;
  const std::string payload = MakeLoadMapPayload(map, load_id);
  Buffer buffer(reinterpret_cast<const unsigned char *>(payload.data()), payload.size());

  CommandHeader header;
  header.id = MultiGPUCommand::LOAD_MAP;
  header.size = static_cast<uint32_t>(buffer.size());
  Buffer buf_header(reinterpret_cast<uint8_t *>(&header), sizeof(header));

  auto view_header = carla::BufferView::CreateFrom(std::move(buf_header));
  auto view_data = carla::BufferView::CreateFrom(std::move(buffer));
  auto message = Primary::MakeMessage(view_header, view_data);

  for (auto &s : _sessions) {
    if (s != nullptr) {
      s->Write(message);
      _loading[s.get()] = load_id;
    }
  }
  return load_id;
}

bool Router::IsAnySecondaryLoading() {
  std::scoped_lock<std::mutex> lock(_mutex);
  return !_loading.empty();
}

void Router::StopWaitingForSecondaryLoads() {
  std::scoped_lock<std::mutex> lock(_mutex);
  _loading.clear();
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
      // One slot per session (no per-request correlation id in the wire
      // protocol): overwriting it would orphan the earlier promise forever,
      // so refuse the newer request instead.
      RejectPromise(response, "a request is already pending on this secondary session");
    } else {
      _promises[s.get()] = response;
      if (_request_observer) {
        _request_observer(s.get(), id, *view_data);
      }
      s->Write(message);
    }
  } else {
    RejectPromise(response, "no secondary server connected");
  }
  ++_next;
  return response->get_future();
}

bool Router::IsConnected(const std::weak_ptr<Primary> &server) {
  std::scoped_lock<std::mutex> lock(_mutex);
  auto s = server.lock();
  return s && (std::find(_sessions.begin(), _sessions.end(), s) != _sessions.end());
}

std::vector<std::weak_ptr<Primary>> Router::TakeNewSessions() {
  std::scoped_lock<std::mutex> lock(_mutex);
  return std::exchange(_new_sessions, {});
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
  // A session's socket can already be closed while the Primary object is
  // kept alive a little longer elsewhere, so also check _sessions
  // membership; otherwise Write() would silently no-op and this promise
  // would never resolve.
  const bool still_connected =
      s && (std::find(_sessions.begin(), _sessions.end(), s) != _sessions.end());
  if (!still_connected) {
    RejectPromise(response, "secondary session is no longer connected");
  } else if (_promises.contains(s.get())) {
    // Same reasoning as WriteToNext: one slot per session.
    RejectPromise(response, "a request is already pending on this secondary session");
  } else {
    _promises[s.get()] = response;
    if (_request_observer) {
      _request_observer(s.get(), id, *view_data);
    }
    s->Write(message);
  }
  return response->get_future();
}

} // namespace multigpu
} // namespace carla
