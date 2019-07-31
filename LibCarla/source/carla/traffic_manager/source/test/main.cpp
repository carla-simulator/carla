#include <iostream>
#include "carla/client/Client.h"
#include "CarlaDataAccessLayer.hpp"
#include "carla/client/ActorList.h"
#include "InMemoryMap.hpp"
#include "SyncQueue.hpp"
#include "PipelineStage.hpp"
#include "FeederCallable.hpp"
#include "LocalizationCallable.hpp"
#include "MotionPlannerCallable.hpp"
#include "TrafficLightStateCallable.hpp"
#include "BatchControlCallable.hpp"
#include "CollisionCallable.hpp"
#include "Pipeline.hpp"

void test_dense_topology(const carla::client::World&);
void test_in_memory_map(carla::SharedPtr<carla::client::Map>);
void test_pipeline_stages(
    carla::SharedPtr<carla::client::ActorList> actor_list,
    carla::SharedPtr<carla::client::Map> world_map, carla::client::Client& client_conn
);
void test_pipeline(
    carla::SharedPtr<carla::client::ActorList> actor_list,
    carla::SharedPtr<carla::client::Map> world_map, carla::client::Client& client_conn
);

int main()
{   
    auto client_conn = carla::client::Client("localhost", 2000);
    std::cout<<"Connected with client object : "<<client_conn.GetClientVersion()<<std::endl;
    auto world = client_conn.GetWorld();
    auto world_map = world.GetMap();
    auto actorList = world.GetActors();
    auto vehicle_list = actorList->Filter("vehicle.*");

    // test_dense_topology(world);
    // test_in_memory_map(world_map);
    // test_pipeline_stages(vehicle_list, world_map, client_conn);
    test_pipeline(vehicle_list, world_map, client_conn);

    return 0;
}

void test_pipeline(
    carla::SharedPtr<carla::client::ActorList> actor_list,
    carla::SharedPtr<carla::client::Map> world_map, carla::client::Client& client_conn
) {
    std::vector<carla::SharedPtr<carla::client::Actor>> vector_of_actors;
    for(auto it = actor_list->begin(); it != actor_list->end(); it++)
        vector_of_actors.push_back(*it);
    auto debug_helper = client_conn.GetWorld().MakeDebugHelper();

    traffic_manager::Pipeline pipeline(
        vector_of_actors,
        world_map,
        7.0f,
        {0.1f, 0.15f, 0.01f},
        {10.0f, 0.0f, 0.1f},
        8,
        client_conn,
        debug_helper
    );
    pipeline.setup();
    pipeline.start();

    while (true)
        sleep(1);
}

void test_pipeline_stages(
    carla::SharedPtr<carla::client::ActorList> actor_list,
    carla::SharedPtr<carla::client::Map> world_map,
    carla::client::Client& client_conn
) {

    traffic_manager::SyncQueue<traffic_manager::PipelineMessage> feeder_queue(20);
    traffic_manager::SyncQueue<traffic_manager::PipelineMessage> localization_queue(20);
    traffic_manager::SyncQueue<traffic_manager::PipelineMessage> pid_queue(20);
    traffic_manager::SyncQueue<traffic_manager::PipelineMessage> tl_queue(20);
    traffic_manager::SyncQueue<traffic_manager::PipelineMessage> collision_queue(20);
    traffic_manager::SyncQueue<traffic_manager::PipelineMessage> batch_control_queue(20);

    traffic_manager::SharedData shared_data;
    for(auto it = actor_list->begin(); it != actor_list->end(); it++)
    {
        shared_data.registered_actors.push_back(*it);
    }

    auto dao = traffic_manager::CarlaDataAccessLayer(world_map);
    auto topology = dao.getTopology();
    auto local_map = std::make_shared<traffic_manager::InMemoryMap>(topology);
    local_map->setUp(1.0);
    shared_data.local_map = local_map;
    shared_data.client = &client_conn;
    auto debug_helper = client_conn.GetWorld().MakeDebugHelper();
    shared_data.debug = &debug_helper;

    traffic_manager::Feedercallable feeder_callable(NULL, &feeder_queue, &shared_data);
    traffic_manager::PipelineStage feeder_stage(1, feeder_callable);
    feeder_stage.start();

    traffic_manager::LocalizationCallable actor_localization_callable(&feeder_queue, &localization_queue, &shared_data);
    traffic_manager::PipelineStage actor_localization_stage(8, actor_localization_callable);
    actor_localization_stage.start();

    traffic_manager::CollisionCallable collision_callable(&localization_queue, &collision_queue, &shared_data);
    traffic_manager::PipelineStage collision_stage(8, collision_callable);
    collision_stage.start();

    traffic_manager::TrafficLightStateCallable tl_state_callable(&collision_queue, &tl_queue, &shared_data);
    traffic_manager::PipelineStage tl_state_stage(8, tl_state_callable);
    tl_state_stage.start();

    float target_velocity = 7.0;
    traffic_manager::MotionPlannerCallable actor_pid_callable(
        target_velocity, &tl_queue, &pid_queue, &shared_data,
        {0.1f, 0.15f, 0.01f}, {10.0f, 0.0f, 0.1f});
    traffic_manager::PipelineStage actor_pid_stage(8, actor_pid_callable);
    actor_pid_stage.start();

    traffic_manager::BatchControlCallable batch_control_callable(&pid_queue, &batch_control_queue, &shared_data);
    traffic_manager::PipelineStage batch_control_stage(1, batch_control_callable);
    batch_control_stage.start();

    std::cout << "All stage pipeline started !" <<std::endl;

    while(true)
    {
        sleep(1);
    }
}

void test_in_memory_map(carla::SharedPtr<carla::client::Map> world_map) {
    auto dao = traffic_manager::CarlaDataAccessLayer(world_map);
    auto topology = dao.getTopology();
    traffic_manager::InMemoryMap local_map(topology);

    std::cout << "setup starting" << std::endl;
    local_map.setUp(1.0);
    std::cout << "setup complete" << std::endl;
    int loose_ends_count = 0;
    auto dense_topology = local_map.get_dense_topology();
    for (auto& swp : dense_topology) {
        if (swp->getNextWaypoint().size() < 1 || swp->getNextWaypoint()[0] == 0) {
            loose_ends_count += 1;
            auto loc = swp->getLocation();
            std::cout << "Loose end at : " << loc.x << " " << loc.y << std::endl;
        }
    }
    std::cout << "Number of loose ends : " << loose_ends_count << std::endl;
}

void test_dense_topology(const carla::client::World& world) {
    auto debug = world.MakeDebugHelper();
    auto dao = traffic_manager::CarlaDataAccessLayer(world.GetMap());
    auto topology = dao.getTopology();
    traffic_manager::InMemoryMap local_map(topology);
    local_map.setUp(1.0);
    for (auto point : local_map.get_dense_topology()) {
        auto location = point->getLocation();
        debug.DrawPoint(location+carla::geom::Location(0, 0, 1), 0.2, carla::client::DebugHelper::Color{225U, 0U, 0U}, 30.0F);
    }
}
