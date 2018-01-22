// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/server/DoubleBuffer.h"
#include "carla/server/Future.h"
#include "carla/server/ServerTraits.h"

namespace carla {
namespace server {

  template<typename T>
  class AsyncServer;

  // ===========================================================================
  // -- Reading ----------------------------------------------------------------
  // ===========================================================================

  /// The result of a reading task.
  template <typename T>
  struct Reading {
    error_code error_code;
    T message;
  };

  // ===========================================================================
  // -- Task -------------------------------------------------------------------
  // ===========================================================================

namespace detail {

  /// Base class for a task with result and timeout.
  template <typename Result>
  class Task {
  protected:

    Task() : _timeout(boost::posix_time::pos_infin) {}

    explicit Task(time_duration timeout) : _timeout(timeout) {}

  public:

    time_duration timeout() const {
      return _timeout;
    }

    void set_timeout(time_duration timeout) {
      _timeout = timeout;
    }

    bool valid() const {
      return _result.valid();
    }

    /// Blocks until the result is ready.
    auto get_result() {
      return _result.get();
    }

    /// Gives up the result. Invalidates this Task.
    std::future<Result> ReleaseResult() {
      return std::move(_result);
    }

    bool TryGetResult(Result &value, timeout_t timeout) {
      return future::wait_and_get(_result, value, timeout);
    }

    bool TryGetResult(Result &value) {
      return future::try_get(_result, value);
    }

    bool IsRunning() const {
      return future::is_running(_result);
    }

    template <typename T>
    bool IsReady(T &&timeout) const {
      return future::is_ready(_result, std::forward<T>(timeout));
    }

    bool IsReady() const {
      return future::is_ready(_result);
    }

  private:

    template<typename T>
    friend class carla::server::AsyncServer;

    time_duration _timeout;

    std::future<Result> _result;
  };

} // namespace detail

  // ===========================================================================
  // -- ConnectTask ------------------------------------------------------------
  // ===========================================================================

  /// Connect a server to a port.
  class ConnectTask : public detail::Task<error_code> {
  public:

    explicit ConnectTask(uint32_t port) : _port(port) {}

    explicit ConnectTask(uint32_t port, time_duration timeout)
        : Task(timeout),
          _port(port) {}

    uint32_t port() const {
      return _port;
    }

  private:

    uint32_t _port;
  };

  // ===========================================================================
  // -- ReadTask ---------------------------------------------------------------
  // ===========================================================================

  /// Single read task.
  template <typename T>
  class ReadTask : public detail::Task<Reading<T>> {
  public:

    ReadTask() = default;

    explicit ReadTask(time_duration timeout)
        : detail::Task<Reading<T>>(timeout) {}
  };

  // ===========================================================================
  // -- WriteTask --------------------------------------------------------------
  // ===========================================================================

  /// Single write task.
  template <typename T>
  class WriteTask : public detail::Task<error_code> {
  public:

    WriteTask() = default;

    explicit WriteTask(time_duration timeout) : Task(timeout) {}

    template <typename M>
    void set_message(M &&message) {
      _message.set_value(std::forward<M>(message));
    }

    std::future<T> get_future_message() {
      return _message.get_future();
    }

  private:

    std::promise<T> _message;
  };

  // ===========================================================================
  // -- StreamTask -------------------------------------------------------------
  // ===========================================================================

namespace detail {

  /// Base class for tasks that continuously read/write from/to a buffer.
  template <typename T>
  class StreamTask : public detail::Task<error_code> {
  public:

    StreamTask() : _buffer(std::make_shared<DoubleBuffer<T>>()) {}

    explicit StreamTask(time_duration timeout)
        : Task(timeout),
          _buffer(std::make_shared<DoubleBuffer<T>>()) {}

    ~StreamTask() {
      _buffer->set_done();
    }

    std::shared_ptr<DoubleBuffer<T>> buffer() {
      return _buffer;
    }

  private:

    const std::shared_ptr<DoubleBuffer<T>> _buffer;
  };

} // namespace detail

  // ===========================================================================
  // -- StreamReadTask ---------------------------------------------------------
  // ===========================================================================

  /// Continuously read from a server and write to the buffer.
  template <typename T>
  class StreamReadTask : public detail::StreamTask<T> {
  public:

    StreamReadTask() = default;

    explicit StreamReadTask(time_duration timeout)
        : detail::StreamTask<T>(timeout) {}
  };

  // ===========================================================================
  // -- StreamWriteTask --------------------------------------------------------
  // ===========================================================================

  /// Continuously read from the buffer and write to a server.
  template <typename T>
  class StreamWriteTask : public detail::StreamTask<T> {
  public:

    StreamWriteTask() = default;

    explicit StreamWriteTask(time_duration timeout)
        : detail::StreamTask<T>(timeout) {}
  };

} // namespace server
} // namespace carla
