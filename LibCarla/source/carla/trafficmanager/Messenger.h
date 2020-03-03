// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>
#include <chrono>
#include <deque>
#include <condition_variable>
#include <mutex>

namespace carla {
namespace traffic_manager {

  using namespace std::chrono_literals;

  template <typename Data>
  class Messenger {

  private:

    /// Flag used to wake up and join any waiting function calls on this object.
    std::atomic<bool> stop_messenger;
    /// Member used to hold data sent by the sender.
    std::deque<Data> d_queue;
    /// Mutex used to manage contention between the sender and receiver.
    std::mutex data_modification_mutex;
    /// Variable to conditionally block sender in case waiting for the receiver.
    std::condition_variable send_condition;
    /// Variable to conditionally block receiver in case waiting for the sender.
    std::condition_variable receive_condition;

  public:

    Messenger() {
      stop_messenger.store(false);
    }
    ~Messenger() {}

    void Push(Data data) {

      std::unique_lock<std::mutex> lock(data_modification_mutex);
      while (!d_queue.empty() && !stop_messenger.load()) {
        send_condition.wait_for(lock, 1ms, [=] {
          return (d_queue.empty() && stop_messenger.load());
        });
      }
      if(!stop_messenger.load()){
        d_queue.push_front(data);
        receive_condition.notify_one();
      }
    }

    Data Peek() {

      std::unique_lock<std::mutex> lock(data_modification_mutex);
      while (d_queue.empty() && !stop_messenger.load()) {
        receive_condition.wait_for(lock, 1ms, [=] {
          return (!d_queue.empty() && stop_messenger.load());
        });
      }

      if(!stop_messenger.load()) {
        Data data = d_queue.back();
        return data;
      }
      return Data();
    }

    void Pop() {

      std::unique_lock<std::mutex> lock(data_modification_mutex);
      if (!(d_queue.empty() && stop_messenger.load())) {
        d_queue.pop_back();
        send_condition.notify_one();
      }
    }

    void Start() {
      stop_messenger.store(false);
    }

    void Stop() {
      stop_messenger.store(true);
      d_queue.clear();
      send_condition.notify_one();
      receive_condition.notify_one();
    }

  };

} // namespace traffic_manager
} // namespace carla
