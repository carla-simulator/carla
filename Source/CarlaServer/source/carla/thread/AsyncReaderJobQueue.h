// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <carla/thread/ThreadSafeQueue.h>
#include <carla/thread/ThreadUniquePointer.h>

#include <atomic>
#include <functional>

namespace carla {
namespace thread {

  /// Executes the given job asynchronously for each item added to the queue.
  ///
  /// The job gets called each time an item is added to the queue, the item is
  /// passed as argument.
  template <typename T>
  class AsyncReaderJobQueue {
  public:

    using Job = std::function<void(T)>;
	using ConnectJob = std::function<void()>;
  using ReconnectJob = std::function<void()>;

	explicit AsyncReaderJobQueue(Job &&job, ConnectJob &&connectionJob, ReconnectJob &&reconnectJob) :
		_done(false),
    _restart(true),
		_job(std::move(job)),
		_connectionJob(std::move(connectionJob)),
    _reconnectJob(std::move(reconnectJob)),
        _queue(),
        _thread(new std::thread(&AsyncReaderJobQueue::workerThread, this)) {}

    ~AsyncReaderJobQueue() {
      _done = true;
    }

    void push(T item) {
      // Empty the queue before push a new item
      T temp;
      while (_queue.try_pop(temp));
      _queue.push(item);
    }

    void restart(){
      _restart = true;
      _queue.canWait(false);
    }

    bool getRestart(){
      return _restart;
    }

    void reconnect(){
      _reconnectJob();
    }
    
  private:
    void workerThread() {
      while (!_done){
    		_connectionJob();
        _restart = false;
        T temp;
        while(_queue.try_pop(temp));
        _queue.canWait(true);
        while (!_restart && !_done) {
          T value;
          if (_queue.wait_and_pop(value)) _job(value);
    		  //Sleep(10);
        }
      }
    }

    std::atomic_bool _done;
    std::atomic_bool _restart;

    Job _job;
    ConnectJob _connectionJob;
    ReconnectJob _reconnectJob;


    ThreadSafeQueue<T> _queue;

    ThreadUniquePointer _thread;
  };

} // namespace thread
} // namespace carla
