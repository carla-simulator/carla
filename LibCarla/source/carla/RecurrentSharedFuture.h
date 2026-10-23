// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Exception.h"
#include "carla/NonCopyable.h"
#include "carla/Time.h"

#include <optional>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <variant>
#pragma warning(pop)
#else
#include <variant>
#endif

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <vector>

namespace carla {

namespace detail {

  class SharedException;

} // namespace detail

  // ===========================================================================
  // -- RecurrentSharedFuture --------------------------------------------------
  // ===========================================================================

  /// This class is meant to be used similar to a shared future, but the value
  /// can be set any number of times.
  template <typename T>
  class RecurrentSharedFuture {
  public:

    using SharedException = detail::SharedException;

    /// Wait until the next value is set. Any number of threads can be waiting
    /// simultaneously.
    ///
    /// @return empty optional if the timeout is met.
    std::optional<T> WaitFor(time_duration timeout);

    /// Set the value and notify all waiting threads.
    template <typename T2>
    void SetValue(const T2 &value);

    /// Set a exception, this exception will be thrown on all the threads
    /// waiting.
    ///
    /// @note The @a exception will be stored on a SharedException and thrown
    /// as such.
    template <typename ExceptionT>
    void SetException(ExceptionT &&exception);

  private:

    std::mutex _mutex;

    std::condition_variable _cv;

    struct Waiter : private NonCopyable {
      struct Unset {};

      explicit Waiter(std::vector<Waiter *> &waiters)
        : waiters(waiters), index(waiters.size()) {
        waiters.push_back(this);
      }

      ~Waiter() {
        waiters.back()->index = index;
        waiters[index] = waiters.back();
        waiters.pop_back();
      }

      bool should_wait = true;
      std::variant<Unset, SharedException, T> value;

      std::vector<Waiter *> &waiters;
      size_t index;
    };

    // Registration and removal both happen while _mutex is held.
    std::vector<Waiter *> _waiters;
  };

  // ===========================================================================
  // -- RecurrentSharedFuture implementation -----------------------------------
  // ===========================================================================

namespace detail {

  class SharedException : public std::exception {
  public:

    SharedException()
      : _exception(std::make_shared<std::runtime_error>("uninitialized SharedException")) {}

    SharedException(std::shared_ptr<std::exception> e)
      : _exception(std::move(e)) {}

    const char *what() const noexcept override {
      return _exception->what();
    }

    std::shared_ptr<std::exception> GetException() const {
      return _exception;
    }

  private:

    std::shared_ptr<std::exception> _exception;
  };

} // namespace detail

  template <typename T>
  std::optional<T> RecurrentSharedFuture<T>::WaitFor(time_duration timeout) {
    std::unique_lock<std::mutex> lock(_mutex);
    Waiter r(_waiters);
    if (!_cv.wait_for(lock, timeout.to_chrono(), [&]() { return !r.should_wait; })) {
      return {};
    }
    if (std::holds_alternative<SharedException>(r.value)) {
      throw_exception(std::get<SharedException>(r.value));
    }
    return std::get<T>(std::move(r.value));
  }

  template <typename T>
  template <typename T2>
  void RecurrentSharedFuture<T>::SetValue(const T2 &value) {
    std::scoped_lock<std::mutex> lock(_mutex);
    struct NotifyOnExit {
      std::condition_variable &cv;
      ~NotifyOnExit() { cv.notify_all(); }
    } notify{_cv};

    for (auto *waiter : _waiters) {
      waiter->should_wait = true;
      waiter->value = value;
      waiter->should_wait = false;
    }
  }

  template <typename T>
  template <typename ExceptionT>
  void RecurrentSharedFuture<T>::SetException(ExceptionT &&e) {
    SetValue(SharedException(std::make_shared<ExceptionT>(std::forward<ExceptionT>(e))));
  }

} // namespace carla
