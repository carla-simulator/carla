// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <condition_variable>
// #include <memory>
#include <mutex>
#include <queue>

#include <iostream>

namespace carla {
namespace thread {

  /// A thread safe queue.
  ///
  /// From "C++ Concurrency In Action", Anthony Williams, listing 4.5.
  template<typename T>
  class ThreadSafeQueue {
  public:

    ThreadSafeQueue() = default;

    ThreadSafeQueue(const ThreadSafeQueue &other) {
      std::lock_guard<std::mutex> lock(other._mutex);
      _queue = other._queue;
      _canWait = true;
      _empty = true;
    }

    void push(T new_value) {
      std::lock_guard<std::mutex> lock(_mutex);
      //_queue.push(new_value);
      //_condition.notify_one();
      _value = new_value;
      _empty = false;
    }

    void canWait(bool wait){
      std::lock_guard<std::mutex> lock(_mutex);
      _canWait = wait;
      _condition.notify_one();
    }

    bool wait_and_pop(T &value) {
      std::unique_lock<std::mutex> lock(_mutex);
      _condition.wait(lock, [this] {
        //return !_queue.empty() || !_canWait; 
        return !_empty || !_canWait;
      });

      //while(_queue.empty()  && _canWait);

      //if (!_queue.empty() && _canWait) {
      if (!_empty && _canWait){
       // value = _queue.front();
        //_queue.pop();
        value = _value;
        _empty = true;
        return true;
      }

      else return false;
    }

    bool try_pop(T &value) {
      std::lock_guard<std::mutex> lock(_mutex);
      //if (_queue.empty()) {
      if(_empty){
        return false;
      }
  	  else {
  		  //value = _queue.front();
  		  //_queue.pop();
        value = _value;
        empty = true;
  		  return true;
	    }
    }
    
    bool empty() const {
      std::lock_guard<std::mutex> lock(_mutex);
      //return _queue.empty();
      return _empty;
    }


    



  private:

    mutable std::mutex _mutex;

    std::atomic_bool _canWait;

    //std::queue<T> _queue;
    ///////
    bool _empty;
    T _value;
    ///////

    std::condition_variable _condition;

  };

} // namespace thread
} // namespace carla
