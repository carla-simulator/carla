// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <carla/thread/ThreadSafeQueue.h>
#include <carla/thread/ThreadUniquePointer.h>

#include <atomic>
#include <functional>

namespace carla {
namespace thread {

  /// Executes the given job asynchronously. Every item that the job returns is
  /// added to the queue.
  template <typename T>
  class CARLA_API AsyncWriterJobQueue {
  public:

    using Job = std::function<T()>;

    explicit AsyncWriterJobQueue(Job &&job) :
        _done(false),
        _job(std::move(job)),
        _queue(),
        _thread(new std::thread(&AsyncWriterJobQueue::workerThread, this))
	{}

    ~AsyncWriterJobQueue() {
      _done = true;
    }

    bool tryPop(T &value) {
      return _queue.try_pop(value);
    }

  private:

    void workerThread() {
      while (!_done) {
        _queue.push(_job());
		Sleep(10);
      }
    }

    std::atomic_bool _done;

    Job _job;

    ThreadSafeQueue<T> _queue;

    const ThreadUniquePointer _thread;

  };

} // namespace thread
} // namespace carla
