#include <iostream>
#include <signal.h>
#include <atomic>

#include "carla/client/Client.h"
#include "CarlaDataAccessLayer.h"
#include "carla/client/ActorList.h"

#include "SharedData.h"
#include "InMemoryMap.h"
#include "SyncQueue.h"
#include "PipelineStage.h"
#include "FeederCallable.h"
#include "LocalizationCallable.h"
#include "MotionPlannerCallable.h"
#include "TrafficLightStateCallable.h"
#include "BatchControlCallable.h"
#include "CollisionCallable.h"
#include "Pipeline.h"

void test_dense_topology(const carla::client::World &);
void test_in_memory_map(carla::SharedPtr<carla::client::Map>);
void test_lane_change(const carla::client::World &world);
void test_pipeline_stages(
    carla::SharedPtr<carla::client::ActorList> actor_list,
    carla::SharedPtr<carla::client::Map> world_map,
    carla::client::Client &client_conn);
void test_pipeline(
    carla::client::World& world,
    carla::client::Client &client_conn,
    int target_traffic_amount
  );

std::atomic<bool> quit(false);    // signal flag
void got_signal(int)
{
    quit.store(true);
}

int main(int argc, char* argv[]) {
  auto client_conn = carla::client::Client("localhost", 2000);
  std::cout << "Connected with client object : " << client_conn.GetClientVersion() << std::endl;
  auto world = client_conn.GetWorld();
  auto world_map = world.GetMap();
  auto actorList = world.GetActors();
  auto vehicle_list = actorList->Filter("vehicle.*");

  // test_dense_topology(world);
  // test_in_memory_map(world_map);
  // test_lane_change(world);
  // test_pipeline_stages(vehicle_list, world_map, client_conn);
  int target_traffic_amount =0;
  // if (argc > 1) {
  //   target_traffic_amount = std::stoi(argv[1]);
  // }
  test_pipeline(world, client_conn, target_traffic_amount);

  return 0;
}

void test_pipeline(
    carla::client::World& world,
    carla::client::Client &client_conn,
    int target_traffic_amount
  ) {

  struct sigaction sa;
  memset( &sa, 0, sizeof(sa) );
  sa.sa_handler = got_signal;
  sigfillset(&sa.sa_mask);
  sigaction(SIGINT,&sa,NULL);

  traffic_manager::SharedData shared_data;
  auto world_map = world.GetMap();
  auto debug_helper = client_conn.GetWorld().MakeDebugHelper();
  auto dao = traffic_manager::CarlaDataAccessLayer(world_map);
  auto topology = dao.getTopology();
  auto local_map = std::make_shared<traffic_manager::InMemoryMap>(topology);
  local_map->setUp(1.0);
  shared_data.local_map = local_map;
  shared_data.client = &client_conn;
  shared_data.debug = &debug_helper;

  auto core_count = traffic_manager::read_core_count();
  std::cout << "Found " << core_count << " CPU cores" << std::endl;
  shared_data.registered_actors = traffic_manager::spawn_traffic(world, core_count, target_traffic_amount);

  traffic_manager::Pipeline pipeline(
      {0.1f, 0.15f, 0.01f},
      {10.0f, 0.01f, 0.1f},
      7.0f,
      std::ceil(core_count/4),
      shared_data
      );
  pipeline.setup();
  pipeline.start();

  std::cout << "Started " << 2 + 4 * std::ceil(core_count/4) << " pipeline threads" << std::endl;

  while (true) {
    sleep(1);
    if(quit.load()) break; 
  }

  pipeline.stop();
  sleep(1);
  shared_data.destroy();

  std::terminate();
}

void test_pipeline_stages(
    carla::SharedPtr<carla::client::ActorList> actor_list,
    carla::SharedPtr<carla::client::Map> world_map,
    carla::client::Client &client_conn) {

  traffic_manager::SyncQueue<traffic_manager::PipelineMessage> feeder_queue(20);
  traffic_manager::SyncQueue<traffic_manager::PipelineMessage> localization_queue(20);
  traffic_manager::SyncQueue<traffic_manager::PipelineMessage> pid_queue(20);
  traffic_manager::SyncQueue<traffic_manager::PipelineMessage> tl_queue(20);
  traffic_manager::SyncQueue<traffic_manager::PipelineMessage> collision_queue(20);
  traffic_manager::SyncQueue<traffic_manager::PipelineMessage> batch_control_queue(20);

  traffic_manager::SharedData shared_data;
  for (auto it = actor_list->begin(); it != actor_list->end(); it++) {
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

  traffic_manager::LocalizationCallable actor_localization_callable(&feeder_queue,
      &localization_queue,
      &shared_data);
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

  traffic_manager::BatchControlCallable batch_control_callable(&pid_queue, &batch_control_queue,
      &shared_data);
  traffic_manager::PipelineStage batch_control_stage(1, batch_control_callable);
  batch_control_stage.start();

  std::cout << "All stage pipeline started !" << std::endl;

  while (true) {
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
  for (auto &swp : dense_topology) {
    if (swp->getNextWaypoint().size() < 1 || swp->getNextWaypoint()[0] == 0) {
      loose_ends_count += 1;
      auto loc = swp->getLocation();
      std::cout << "Loose end at : " << loc.x << " " << loc.y << std::endl;
    }
  }
  std::cout << "Number of loose ends : " << loose_ends_count << std::endl;
}

void test_dense_topology(const carla::client::World &world) {
  auto debug = world.MakeDebugHelper();
  auto dao = traffic_manager::CarlaDataAccessLayer(world.GetMap());
  auto topology = dao.getTopology();
  traffic_manager::InMemoryMap local_map(topology);
  local_map.setUp(1.0);
  for (auto point : local_map.get_dense_topology()) {
    auto location = point->getLocation();
    debug.DrawPoint(location + carla::geom::Location(0,
        0,
        1), 0.2, carla::client::DebugHelper::Color{225U, 0U, 0U}, 30.0F);
  }
}

void test_lane_change(const carla::client::World &world) {
  auto debug = world.MakeDebugHelper();
  auto dao = traffic_manager::CarlaDataAccessLayer(world.GetMap());
  auto topology = dao.getTopology();
  traffic_manager::InMemoryMap local_map(topology);
  local_map.setUp(1.0);
  for (auto point : local_map.get_dense_topology()) {
    auto zoffset = carla::geom::Location(0,0,5);
    if(point->getLeftWaypoint() != nullptr){
      debug.DrawArrow(point->getLocation()+zoffset,(point->getLeftWaypoint())->getLocation()+zoffset);
    }
    world.WaitForTick(carla::time_duration(std::chrono::duration<int, std::milli>(1000)));
    if(point->getRightWaypoint() != nullptr){
      debug.DrawArrow(point->getLocation()+zoffset,(point->getRightWaypoint())->getLocation()+zoffset);
    }
    world.WaitForTick(carla::time_duration(std::chrono::duration<int, std::milli>(1000)));
  }
}
