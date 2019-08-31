#pragma once

#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <deque>
#include <algorithm>
#include <vector>

namespace traffic_manager {

  template <typename T>
  class SyncQueue {

    /// This is a thread safe generic wrapper around std::deque.

  private:

    std::mutex q_mutex;
    std::condition_variable empty_condition;
    std::condition_variable full_condition;
    std::deque<T> queue;
    int buffer_size;

  public:

    SyncQueue(int buffer_size = 20) : buffer_size(buffer_size) {}

    /// Adds an element to the end of the queue.
    void push(T value) {
      std::unique_lock<std::mutex> lock(q_mutex);
      full_condition.wait(lock, [=] {return !(queue.size() >= buffer_size); });
      queue.push_back(value);
      empty_condition.notify_one();
    }

    /// Returns and removes an element from the front of the queue.
    T pop() {
      std::unique_lock<std::mutex> lock(q_mutex);
      empty_condition.wait(lock, [=] { return !queue.empty(); });
      T rc(std::move(queue.front()));
      queue.pop_front();
      full_condition.notify_one();
      return rc;
    }

    /// Returns a reference to the element at the front of the queue.
    T front() {
      return queue.front();
    }

    /// Returns a reference to the element at the back of the queue.
    T back() {
      return queue.back();
    }

    /// Returns the number of elements in the queue.
    int size() {
      return queue.size();
    }

    /// Returns true if the queue is empty.
    bool empty() {
      return queue.empty();
    }

    /// Returns true if the queue is full.
    bool full() {
      return queue.size() >= buffer_size;
    }

    /// Returns a vector of elements from the front of the queue till a given a
    /// number of elements.
    std::vector<T> getContent(int number_of_elements) {
      std::unique_lock<std::mutex> lock(q_mutex);
      empty_condition.wait(lock, [=] { return !queue.empty(); });
      if (queue.size() >= number_of_elements) {
        return std::vector<T>(queue.begin(), queue.begin() + number_of_elements - 1);
      } else {
        return std::vector<T>(queue.begin(), queue.end());
      }
    }

    /// Returns the reference to the element at a given index on the queue.
    T get(int index) {
      std::unique_lock<std::mutex> lock(q_mutex);
      empty_condition.wait(lock, [=] { return !queue.empty(); });
      auto queue_size = size();
      index = index >= queue_size ? queue_size - 1 : index;
      return queue.at(index);
    }
  };

}
