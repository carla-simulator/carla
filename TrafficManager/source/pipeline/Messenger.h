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

    std::atomic<bool> stop_messenger;
    std::atomic<int> state_counter;
    Data data;

    std::mutex data_modification_mutex;
    std::condition_variable send_condition;
    std::condition_variable receive_condition;

  public:

    Messenger() {
      state_counter = 0;
      stop_messenger.store(false);
    }
    ~Messenger() {}

    int SendData(DataPacket<Data> packet) {

      std::unique_lock<std::mutex> lock(data_modification_mutex);
      while (state_counter.load() == packet.id) {
        send_condition.wait_for(lock, 1ms, [=] {return state_counter.load() != packet.id;});
        if (stop_messenger.load()) {
          break;
        }
      }
      data = packet.data;
      state_counter.store(state_counter.load() + 1);
      int present_state = state_counter.load();
      receive_condition.notify_one();

      return present_state;
    }

    DataPacket<Data> ReceiveData(int old_state) {

      std::unique_lock<std::mutex> lock(data_modification_mutex);
      while (state_counter.load() == old_state) {
        receive_condition.wait_for(lock, 1ms, [=] {return state_counter.load() != old_state;});
        if (stop_messenger.load()) {
          break;
        }
      }
      state_counter.store(state_counter.load() + 1);
      DataPacket<Data> packet = {state_counter.load(), data};
      send_condition.notify_one();

      return packet;
    }

    int GetState() {
      return state_counter.load();
    }

    void Stop() {
      stop_messenger.store(true);
    }

  };

}
