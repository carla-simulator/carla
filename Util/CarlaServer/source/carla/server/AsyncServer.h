// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/Profiler.h"
#include "carla/server/AsyncService.h"
#include "carla/server/ServerTraits.h"
#include "carla/server/Task.h"

namespace carla {
namespace server {

  // ===========================================================================
  // -- AsyncServer ------------------------------------------------------------
  // ===========================================================================

  /// Asynchronous server. Every "Connect", "Write", and "Read" tasks are
  /// submitted to a queue of asynchronous jobs. These jobs are executed in a
  /// single separate thread in order of submission. The "Disconnect()" function
  /// of the underlying server is assumed to be thread-safe.
  template <typename SERVER>
  class AsyncServer : private NonCopyable {
  public:

    using server_type = SERVER;

    template<typename... Args>
    AsyncServer(Args&&... args) : _server(std::forward<Args>(args)...) {}

    void Disconnect() {
      _server.Disconnect();
    }

    std::future<error_code> Connect(uint32_t port, time_duration timeout);

    void Execute(ConnectTask &task);

    template <typename T>
    void Execute(ReadTask<T> &task);

    template <typename T>
    void Execute(WriteTask<T> &task);

    template <typename T>
    void Execute(StreamReadTask<T> &task);

    template <typename T>
    void Execute(StreamWriteTask<T> &task);

  private:

    server_type _server;

    AsyncService _service;
  };

  // ===========================================================================
  // -- AsyncServer implementation ---------------------------------------------
  // ===========================================================================

  template <typename S>
  std::future<error_code> AsyncServer<S>::Connect(
      const uint32_t port,
      const time_duration timeout) {
    return _service.Post([=](){
      return _server.Connect(port, timeout);
    });
  }

  template <typename S>
  void AsyncServer<S>::Execute(ConnectTask &task) {
    task._result = std::move(Connect(task.port(), task.timeout()));
  }

  template <typename S>
  template <typename T>
  void AsyncServer<S>::Execute(ReadTask<T> &task) {
    auto job = [this, timeout=task.timeout()](){
      CARLA_PROFILE_SCOPE(AsyncServer, Read);
      Reading<T> result;
      result.error_code = _server.Read(result.message, timeout);
      return result;
    };
    task._result = _service.Post(std::move(job));
  }

  template <typename S>
  template <typename T>
  void AsyncServer<S>::Execute(WriteTask<T> &task) {
    // We could make the lambda mutable to avoid creating the shared_ptr, but it
    // doesn't compile with VC++2015. Perhaps we can change it in the future.
    auto message = std::make_shared<std::future<T>>(task.get_future_message());
    auto job = [this, message{std::move(message)}, timeout = task.timeout()]() {
      CARLA_PROFILE_SCOPE(AsyncServer, Write);
      while (!_service.done()) {
        T message_value;
        if (future::wait_and_get(*message, message_value, timeout_t::milliseconds(1))) {
          return _server.Write(message_value, timeout);
        }
      }
      return errc::operation_aborted();
    };
    task._result = _service.Post(std::move(job));
  }

  template <typename S>
  template <typename T>
  void AsyncServer<S>::Execute(StreamReadTask<T> &task) {
    auto job = [this, buffer=task.buffer(), timeout=task.timeout()]() {
      error_code ec;
      do {
        CARLA_PROFILE_SCOPE(AsyncServer, StreamRead);
        if (_service.done()) {
          ec = errc::operation_aborted();
          break;
        }
        auto writer = buffer->MakeWriter();
        ec = _server.Read(*writer, timeout);
      } while (!ec);
      return ec;
    };
    task._result = _service.Post(std::move(job));
  }

  template <typename S>
  template <typename T>
  void AsyncServer<S>::Execute(StreamWriteTask<T> &task) {
    auto job = [this, buffer=task.buffer(), timeout=task.timeout()]() {
      error_code ec;
      do {
        CARLA_PROFILE_SCOPE(AsyncServer, StreamWrite);
        if (_service.done()) {
          ec = errc::operation_aborted();
          break;
        }
        auto reader = buffer->TryMakeReader(timeout);
        if (reader != nullptr) {
          ec = _server.Write(*reader, timeout);
        } else if (buffer->done()) {
          ec = errc::operation_aborted();
        }
      } while (!ec);
      return ec;
    };
    task._result = _service.Post(std::move(job));
  }

} // namespace server
} // namespace carla
