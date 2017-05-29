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

  using Job = std::function<std::unique_ptr<T>()>;
	using ConnectJob = std::function<void()>;
  using ReconnectJob = std::function<void()>;

	explicit AsyncWriterJobQueue(Job &&job, ConnectJob &&connectJob, ReconnectJob &&reconnectJob) :
		done(false),
    _restart(true),
		_job(std::move(job)),
		_connectJob(std::move(connectJob)),
    _reconnectJob(std::move(reconnectJob)),
        _queue(),
        _thread(new std::thread(&AsyncWriterJobQueue::workerThread, this))
	{}

    ~AsyncWriterJobQueue() {
      std::cout << "Destroyed thread server"<< std::endl;
      done = true;
    }

    std::unique_ptr<T> tryPop() {
      return std::move(_queue.try_pop());
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

    void clear(){
      _queue.clear();
    }
    

    std::atomic_bool done;

  private:

    void workerThread() {
      while (!done){
		    _connectJob();
        _restart = false;
        _queue.canWait(true);

        while (!_restart && !done) {
          //_queue.wait_and_push(_job);
          _queue.push(std::move(_job())); 
		      //Sleep(10);
        }
      }
    }

    
    std::atomic_bool _restart;

    Job _job;
    ConnectJob _connectJob;
    ReconnectJob _reconnectJob;

    ThreadSafeQueue<T> _queue;

    ThreadUniquePointer _thread;

  };

} // namespace thread
} // namespace carla
