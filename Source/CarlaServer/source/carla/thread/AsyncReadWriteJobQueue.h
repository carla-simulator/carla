// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <carla/thread/ThreadSafeQueue.h>
#include <carla/thread/ThreadUniquePointer.h>

#include <atomic>
#include <functional>
#include <iostream>

namespace carla {
namespace thread {

  /// Executes the given job asynchronously. Every item that the job returns is
  /// added to the queue.
  template<typename W, typename R>
  class AsyncReadWriteJobQueue {
  public:

    using WritingJob = std::function<W()>;
    using ReadingJob = std::function<void(R)>;
    using ConnectJob = std::function<void()>;
    using ReconnectJob = std::function<void()>;

    explicit AsyncReadWriteJobQueue(
        WritingJob && writingJob,
        ReadingJob && readingJob,
        ConnectJob && connectJob,
        ReconnectJob && reconnectJob) :
      _done(false),
      _restart(true),
      _writeJob(std::move(writingJob)),
      _readJob(std::move(readingJob)),
      _connectJob(std::move(connectJob)),
      _reconnectJob(std::move(reconnectJob)),
      _writeQueue(),
      _thread(new std::thread(&AsyncReadWriteJobQueue::workerThread, this)) {}

    ~AsyncReadWriteJobQueue() {
      _done = true;
    }

    bool tryPop(W &value) {
      return _writeQueue.try_pop(value);
    }

    void push(R item) {
      _readQueue.push(item);
    }

    void reconnect(){
      _reconnectJob();
    }

    void restart(){
      _restart = true;
      _readQueue.canWait(false);
      _writeQueue.canWait(false);
      //_thread->detach();
      //_thread = ThreadUniquePointer(new std::thread(&AsyncReadWriteJobQueue::workerThread, this));
    }

    bool getRestart(){
      return _restart;
    }

  private:

    void workerThread() {
      while(!_done){
        _connectJob();
        _restart = false; 
        _readQueue.canWait(true);
        while (!_restart && !_done) {
          R value;
          if (_readQueue.wait_and_pop(value)) {
            _readJob(value);
          }
          if (!_restart){
            _writeQueue.push(_writeJob());
          }

        }
      }
    }

    std::atomic_bool _done;
    std::atomic_bool _restart;

    WritingJob _writeJob;
    ReadingJob _readJob;
    ConnectJob _connectJob;
    ReconnectJob _reconnectJob;

    ThreadSafeQueue<W> _writeQueue;
    ThreadSafeQueue<R> _readQueue;

    ThreadUniquePointer _thread;
  };

} // namespace thread
} // namespace carla
