// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <condition_variable>
// #include <memory>
#include <mutex>
#include <queue>

#include <iostream>
#include "../CarlaServer.h"

namespace carla {
namespace thread {

  /// A thread safe queue.
  ///
  /// From "C++ Concurrency In Action", Anthony Williams, listing 4.5.
  template<typename T>
  class ThreadSafeQueue {
  public:

    ThreadSafeQueue(): _canWait(true), _empty(true){}

    ThreadSafeQueue(const ThreadSafeQueue &other) = delete;

    void push(std::unique_ptr<T> new_value) {
      std::lock_guard<std::mutex> lock(_mutex);
      //_queue.push(new_value);
      //_condition.notify_one();
      _value = std::move(new_value);
      _condition.notify_one();
    }

    void canWait(bool wait){
      std::lock_guard<std::mutex> lock(_mutex);
      _canWait = wait;
      _condition.notify_one();
    }

    std::unique_ptr<T> wait_and_pop() {
      std::unique_lock<std::mutex> lock(_mutex);
      _condition.wait(lock, [this]() {
        return _value != nullptr || !_canWait;
      });
      return std::move(_value);
    }

    std::unique_ptr<T> try_pop() {
      std::lock_guard<std::mutex> lock(_mutex);
      //if (_queue.empty()) {
      return std::move(_value);

      /*if(_empty){
        return false;
      }
  	  else {
  		  //value = _queue.front();
  		  //_queue.pop();
        value = _value;
        _empty = true;
  		  return true;
	    }*/
    }
    
    bool empty() const {
      std::lock_guard<std::mutex> lock(_mutex);
      //return _queue.empty();
      return _value == nullptr;
    }


    



  private:

    mutable std::mutex _mutex;

    std::atomic_bool _canWait;

    //std::queue<T> _queue;
    ///////
    bool _empty;
    std::unique_ptr<T> _value;
    ///////

    std::condition_variable _condition;

  };

} // namespace thread
} // namespace carla
