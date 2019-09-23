#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

using namespace std::chrono_literals;

namespace traffic_manager {

  template <typename Data>
  struct DataPacket {
    int id;
    Data data;
  };

  /// This class is the template for messaging functionality between
  /// different stage classes to send and receive data.
  /// One object of this type can only facilitate receiving data from
  /// a sender stage and passing the data onto a receiver stage.
  /// The class maintains state internally and blocks send or receive
  /// requests until data is available/successfully passed on.
  template <typename Data>
  class Messenger {

  private:

    /// Flag used to wake up and join any waiting function calls on this object
    std::atomic<bool> stop_messenger;
    /// State variable that will progress upon every successful communication
    /// between sender and receiver
    std::atomic<int> state_counter;
    /// Member used to hold data sent by sender
    Data data;
    /// Mutex used to manage contention between sender and receiver
    std::mutex data_modification_mutex;
    /// Variable to conditionally block sender incase waiting for receiver
    std::condition_variable send_condition;
    /// Variable to conditionally block receiver incase waiting for sender
    std::condition_variable receive_condition;

  public:

    Messenger() {
      state_counter = 0;
      stop_messenger.store(false);
    }
    ~Messenger() {}

    /// This method receives data from a sender, stores in a member and
    /// increments state
    int SendData(DataPacket<Data> packet) {

      std::unique_lock<std::mutex> lock(data_modification_mutex);
      while (state_counter.load() == packet.id && !stop_messenger.load()) {
        send_condition.wait_for(lock, 1ms, [=] {return state_counter.load() != packet.id;});
      }
      data = packet.data;
      state_counter.store(state_counter.load() + 1);
      int present_state = state_counter.load();
      receive_condition.notify_one();

      return present_state;
    }

    /// This method presents stored data to receiver and increments state
    DataPacket<Data> ReceiveData(int old_state) {

      std::unique_lock<std::mutex> lock(data_modification_mutex);
      while (state_counter.load() == old_state && !stop_messenger.load()) {
        receive_condition.wait_for(lock, 1ms, [=] {return state_counter.load() != old_state;});
      }
      state_counter.store(state_counter.load() + 1);
      DataPacket<Data> packet = {state_counter.load(), data};
      send_condition.notify_one();

      return packet;
    }

    /// Returns state counter
    int GetState() {
      return state_counter.load();
    }

    /// Unblocks any waiting calls on this object
    void Stop() {
      stop_messenger.store(true);
    }

  };

}
