
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
        /*
        This is a thread safe generic wrapper around std::deque.
        */

        private:

        std::shared_mutex               q_mutex;
        std::condition_variable_any     empty_condition;
        std::condition_variable_any     full_condition;
        std::deque<T>                   queue;
        int                             buffer_size;

        public:

        SyncQueue(int buffer_size = 20):buffer_size(buffer_size){}

        /* Adds an element to the end of the queue. */
        void push(T value){
            std::unique_lock<std::shared_mutex> lock(this->q_mutex);
            this->full_condition.wait(lock, [=]{ return !(this->queue.size()>=buffer_size); });
            queue.push_back(value);
            this->empty_condition.notify_one();
        }

        /* Returns and removes an element from the front of the queue. */
        T pop(){
            std::unique_lock<std::shared_mutex> lock(this->q_mutex);
            this->empty_condition.wait(lock, [=]{ return !this->queue.empty(); });
            T rc(std::move(this->queue.front()));
            this->queue.pop_front();
            this->full_condition.notify_one();
            return rc;
        }

        /* Returns a reference to the element at the front of the queue. */
        T front(){
            return this->queue.front();
        }

        /* Returns a reference to the element at the back of the queue. */
        T back(){
            return this->queue.back();
        }

        /* Returns the number of elements in the queue. */
        int size() {
            return queue.size();
        }

        /* Returns true if the queue is empty. */
        bool empty() {
            return queue.empty();
        }

        /* Returns true if the queue is full. */
        bool full() {
            return queue.size() >= buffer_size;
        }

        /* Returns a vector of elements from the front of the queue till a given a number of elements. */
        std::vector<T> getContent(int number_of_elements) {
            std::shared_lock<std::shared_mutex> lock(q_mutex);
            this->empty_condition.wait(lock, [=]{ return !this->queue.empty(); });
            if (queue.size() >= number_of_elements)
                return std::vector<T>(queue.begin(), queue.begin()+number_of_elements);
            else
                return std::vector<T>(queue.begin(), queue.end());
        }

        /* Returns the reference to the element at a given index on the queue. */
        T get(int index) {
            std::shared_lock<std::shared_mutex> lock(q_mutex);
            this->empty_condition.wait(lock, [=]{ return !this->queue.empty(); });
            auto queue_size = this->size();
            index = index >= queue_size ? queue_size: index;
            return queue.at(index);
        }
    };
}
