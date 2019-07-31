
#include "Pipeline.hpp"

namespace traffic_manager {

    const float SAMPLING_RESOLUTION = 1.0;

    Pipeline::Pipeline(
        std::vector<carla::SharedPtr<carla::client::Actor>> actor_list,
        carla::SharedPtr<carla::client::Map> world_map,
        float target_velocity,
        std::vector<float> longitudinal_PID_parameters,
        std::vector<float> lateral_PID_parameters,
        int pipeline_width,
        carla::client::Client& client,
        carla::client::DebugHelper debug_helper
    ):
    actor_list(actor_list),
    world_map(world_map),
    target_velocity(target_velocity),
    longitudinal_PID_parameters(longitudinal_PID_parameters),
    lateral_PID_parameters(lateral_PID_parameters),
    pipeline_width(pipeline_width),
    client(client),
    debug_helper(debug_helper)
    {}

    void Pipeline::setup() {

        for (int i=0; i<NUMBER_OF_STAGES; i++)
            message_queues.push_back(std::make_shared<SyncQueue<PipelineMessage>>(20));

        for(auto actor: actor_list)
            shared_data.registered_actors.push_back(actor);

        auto dao = traffic_manager::CarlaDataAccessLayer(world_map);
        auto topology = dao.getTopology();
        auto local_map = std::make_shared<traffic_manager::InMemoryMap>(topology);
        local_map->setUp(SAMPLING_RESOLUTION);
        shared_data.local_map = local_map;
        shared_data.client = &client;
        shared_data.debug = &debug_helper;

        auto feeder_callable = new Feedercallable(NULL, message_queues.at(0).get(), &shared_data);
        auto feeder_stage = new PipelineStage(1, *feeder_callable);
        callables.push_back(std::shared_ptr<PipelineCallable>(feeder_callable));
        stages.push_back(std::shared_ptr<PipelineStage>(feeder_stage));

        auto localization_callable = new LocalizationCallable(
            message_queues.at(0).get(), message_queues.at(1).get(), &shared_data);
        auto localization_stage = new PipelineStage(pipeline_width, *localization_callable);
        callables.push_back(std::shared_ptr<PipelineCallable>(localization_callable));
        stages.push_back(std::shared_ptr<PipelineStage>(localization_stage));

        auto collision_callable = new CollisionCallable(
            message_queues.at(1).get(), message_queues.at(2).get(), &shared_data);
        auto collision_stage = new PipelineStage(pipeline_width, *collision_callable);
        callables.push_back(std::shared_ptr<PipelineCallable>(collision_callable));
        stages.push_back(std::shared_ptr<PipelineStage>(collision_stage));

        auto traffic_light_callable = new TrafficLightStateCallable(
            message_queues.at(2).get(), message_queues.at(3).get(), &shared_data);
        auto traffic_light_stage = new PipelineStage(pipeline_width, *traffic_light_callable);
        callables.push_back(std::shared_ptr<PipelineCallable>(traffic_light_callable));
        stages.push_back(std::shared_ptr<PipelineStage>(traffic_light_stage));

        auto controller_callable = new MotionPlannerCallable(
            target_velocity, message_queues.at(3).get(), message_queues.at(4).get(), &shared_data,
            longitudinal_PID_parameters, lateral_PID_parameters);
        auto controller_stage = new PipelineStage(pipeline_width, *controller_callable);
        callables.push_back(std::shared_ptr<PipelineCallable>(controller_callable));
        stages.push_back(std::shared_ptr<PipelineStage>(controller_stage));

        auto batch_control_callable = new BatchControlCallable(
            message_queues.at(4).get(), message_queues.at(5).get(), &shared_data);
        auto batch_control_stage = new PipelineStage(1, *batch_control_callable);
        callables.push_back(std::shared_ptr<PipelineCallable>(batch_control_callable));
        stages.push_back(std::shared_ptr<PipelineStage>(batch_control_stage));
    }

    void Pipeline::start() {
        for(auto stage: stages)
            stage->start();
    }
}
