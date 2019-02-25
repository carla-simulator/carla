// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Exception.h"
#include "carla/Time.h"

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <condition_variable>
#include <exception>
#include <map>
#include <mutex>

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
    boost::optional<T> WaitFor(time_duration timeout);

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

    struct mapped_type {
      bool should_wait;
      boost::variant<T, SharedException> value;
    };

    std::map<const char *, mapped_type> _map;
  };

  // ===========================================================================
  // -- RecurrentSharedFuture implementation -----------------------------------
  // ===========================================================================

namespace detail {

  static thread_local const char thread_tag{};

  class SharedException : public std::exception {
  public:

    SharedException(const SharedException &) = default;

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
  boost::optional<T> RecurrentSharedFuture<T>::WaitFor(time_duration timeout) {
    std::unique_lock<std::mutex> lock(_mutex);
    auto &r = _map[&detail::thread_tag];
    r.should_wait = true;
    if (!_cv.wait_for(lock, timeout.to_chrono(), [&]() { return !r.should_wait; })) {
      return {};
    }
    if (r.value.which() == 1) {
      throw_exception(boost::get<SharedException>(r.value));
    }
    return boost::get<T>(std::move(r.value));
  }

  template <typename T>
  template <typename T2>
  void RecurrentSharedFuture<T>::SetValue(const T2 &value) {
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto &pair : _map) {
      pair.second.should_wait = false;
      pair.second.value = value;
    }
    _cv.notify_all();
  }

  template <typename T>
  template <typename ExceptionT>
  void RecurrentSharedFuture<T>::SetException(ExceptionT &&e) {
    SetValue(SharedException(std::make_shared<ExceptionT>(std::forward<ExceptionT>(e))));
  }

} // namespace carla
