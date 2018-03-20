// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>

#include "carla/Logging.h"
#include "carla/server/ServerTraits.h"

namespace carla {
namespace server {

namespace detail {

  /// Keeps the state of an atomic double-buffer.
  class DoubleBufferState {
  public:

    enum ActiveBuffer : uint32_t {
      FIRST_BUFFER,
      SECOND_BUFFER,
      NUMBER_OF_BUFFERS
    };

    DoubleBufferState();

    /// Never returns None.
    ActiveBuffer StartWriting();

    void EndWriting();

    /// Returns None if there appears to be no more data to read yet.
    ActiveBuffer StartReading();

    void EndReading();

  private:

    std::atomic<uint32_t> _state;

    uint32_t _readState;
  };

} // namespace detail

  /// An atomic thread-safe double buffer for one producer and one consumer.
  template <typename T>
  class DoubleBuffer : private detail::DoubleBufferState {
  public:

    DoubleBuffer() : _done(false) {}

    ~DoubleBuffer() { set_done(); }

    bool done() const {
      return _done;
    }

    void set_done() {
      _done = true;
      _condition.notify_all();
    }

    /// Returns an unique_ptr to the buffer to be read. The given buffer will
    /// be locked for reading until the unique_ptr is destroyed.
    ///
    /// Blocks until there is some data to read in one of the buffer, or the
    /// time-out is met.
    ///
    /// Returns nullptr if the time-out was met, or the DoubleBuffer is marked
    /// as done.
    auto TryMakeReader(timeout_t timeout) {
      const auto deleter = [this](const T *ptr) { if (ptr) EndReading(); };
      ActiveBuffer active = NUMBER_OF_BUFFERS;
      {
        std::unique_lock<std::mutex> lock(_mutex);
        _condition.wait_for(lock, timeout.to_chrono(), [&] {
          active = StartReading();
          return _done || (active != NUMBER_OF_BUFFERS);
        });
      }
      const T *pointer = (active != NUMBER_OF_BUFFERS ? &_buffer[active] : nullptr);
      return std::unique_ptr<const T, decltype(deleter)>(pointer, deleter);
    }

    auto TryMakeReader() {
      return TryMakeReader(timeout_t::milliseconds(0u));
    }

    /// Returns an unique_ptr to the buffer to be written. The given buffer
    /// will be locked for writing until the unique_ptr is destroyed.
    ///
    /// Never returns nullptr.
    auto MakeWriter() {
      const auto deleter = [this](T *) {
        EndWriting();
        _condition.notify_one();
      };
      return std::unique_ptr<T, decltype(deleter)>(&_buffer[StartWriting()], deleter);
    }

  private:

    std::mutex _mutex;

    std::condition_variable _condition;

    std::atomic_bool _done;

    T _buffer[NUMBER_OF_BUFFERS];
  };

} // namespace carla
} // namespace server
