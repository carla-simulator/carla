
#include <mutex>
#include <condition_variable>

namespace traffic_manager {

    template<typename Data>
    struct MarkedPacket {
        int id;
        Data data;
    };

    template<typename Data>
    class Messenger {

    private:
        int state_counter =0;
        Data data;

        std::mutex data_modification_mutex;
        std::condition_variable send_condition;
        std::condition_variable recieve_condition;

    public:

        Messenger() {}
        ~Messenger() {}

        int SendData(MarkedPacket<Data> packet) {

            std::lock_guard<std::mutex> lock(data_modification_mutex);
            send_condition.wait(lock, [=] {return state_counter != packet.id;});
            data = packet.data;
            state_counter++;
            int present_state = state_counter;
            recieve_condition.notify_one();

            return present_state;
        }

        MarkedPacket<Data> RecieveData(int old_state) {

            std::lock_guard<std::mutex> lock(data_modification_mutex);
            recieve_condition.wait(lock, [=] {return state_counter != old_state;});
            state_counter++;
            MarkedPacket<Data> packet = {state_counter, data};
            send_condition.notify_one();

            return packet;
        }

        int GetState() {
            return state_counter;
        }

    };
}
