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
  class AsyncWriterJobQueue {
  public:

    using Job = std::function<T()>;
	using ConnectJob = std::function<void()>;

	explicit AsyncWriterJobQueue(Job &&job, ConnectJob &&connectJob) :
		_done(false),
    _restart(true),
		_job(std::move(job)),
		_connectJob(std::move(connectJob)),
        _queue(),
        _thread(new std::thread(&AsyncWriterJobQueue::workerThread, this))
	{}

    ~AsyncWriterJobQueue() {
      _done = true;
    }

    bool tryPop(T &value) {
      return _queue.try_pop(value);
    }

    void restart(){
      _restart = true;
    }

  private:

    void workerThread() {
      while (!_done){
        _restart = false;
		    _connectJob();
        while (!_restart && !_done) {
          _queue.push(_job());
		      //Sleep(10);
        }
      }
    }

    std::atomic_bool _done;
    std::atomic_bool _restart;

    Job _job;
	ConnectJob _connectJob;

    ThreadSafeQueue<T> _queue;

    const ThreadUniquePointer _thread;

  };

} // namespace thread
} // namespace carla
