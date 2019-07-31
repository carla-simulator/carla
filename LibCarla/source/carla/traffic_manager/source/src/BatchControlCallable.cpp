// Definition of class memebers

#include "BatchControlCallable.hpp"

namespace traffic_manager{

    BatchControlCallable::BatchControlCallable(
        SyncQueue<PipelineMessage>* input_queue,
        SyncQueue<PipelineMessage>* output_queue,
        SharedData* shared_data):
        input_queue(input_queue),
        PipelineCallable(input_queue, output_queue, shared_data){}

    BatchControlCallable::~BatchControlCallable(){}

    PipelineMessage BatchControlCallable::action(PipelineMessage message) {

        long count = 0;
        auto last_time = std::chrono::system_clock::now();

        while(true)
        {
            std::vector<carla::rpc::Command> commands;

            for (auto _ : shared_data->registered_actors) {

                carla::rpc::VehicleControl vehicle_control;

                auto element = input_queue->pop();
                auto actor = element.getActor();
                vehicle_control.throttle = element.getAttribute("throttle");
                vehicle_control.brake = element.getAttribute("brake");
                vehicle_control.steer = element.getAttribute("steer");
                carla::rpc::Command::ApplyVehicleControl control_command(element.getActorID(), vehicle_control);
                commands.push_back(control_command);
                count++;
            }

            shared_data->client->ApplyBatch(commands);

            auto current_time = std::chrono::system_clock::now();
            std::chrono::duration<double> diff = current_time - last_time;

            if(diff.count() > 1.0) {    // Log once every second
                    last_time = current_time;
                    std::cout << "Updates processed per second " << count << std::endl;
                    count = 0;
            }
        }
        PipelineMessage empty_message;
        return empty_message;
    }
}