#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <atomic>
#include <execinfo.h>
#include <stdexcept>

#include <boost/stacktrace.hpp>
#include "carla/client/Client.h"
#include "CarlaDataAccessLayer.h"

#include "InMemoryMap.h"
#include "Pipeline.h"

void run_pipeline(
    carla::client::World &world,
    carla::client::Client &client_conn,
    int target_traffic_amount);

std::atomic<bool> quit(false);
void got_signal(int) {
  quit.store(true);
}

std::vector<carla::SharedPtr<carla::client::Actor>>* global_actor_list;
void handler() {

  if (!quit.load()) {
    std::cout << "\nTrafficManager encountered a problem!" << std::endl;
    std::cout << "Destorying all actors spawned" << std::endl;
    for (auto actor: *global_actor_list) {
      if (actor != nullptr and actor->IsAlive()) {
        actor->Destroy();
      }
    }
    std::cout << boost::stacktrace::stacktrace() << std::endl;
    exit(1);
  }
} 

int main(int argc, char *argv[]) {
  std::set_terminate(handler);

  auto client_conn = carla::client::Client("localhost", 2000);
  std::cout << "Connected with client object : " << client_conn.GetClientVersion() << std::endl;
  auto world = client_conn.GetWorld();

  int target_traffic_amount = 0;
  if (argc == 3 and std::string(argv[1]) == "-n") {
    try {
      target_traffic_amount = std::stoi(argv[2]);
    } catch (const std::exception &e) {
      std::cout << "Failed to parse argument, choosing defaults" << std::endl;
    }
  }

  run_pipeline(world, client_conn, target_traffic_amount);

  return 0;
}

void run_pipeline(
    carla::client::World &world,
    carla::client::Client &client_conn,
    int target_traffic_amount) {

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = got_signal;
  sigfillset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);

  auto world_map = world.GetMap();
  auto debug_helper = client_conn.GetWorld().MakeDebugHelper();
  auto dao = traffic_manager::CarlaDataAccessLayer(world_map);
  auto topology = dao.getTopology();
  auto local_map = std::make_shared<traffic_manager::InMemoryMap>(topology);
  local_map->setUp(1.0);

  auto core_count = traffic_manager::read_core_count();
  std::cout << "Found " << core_count << " CPU cores" << std::endl;
  auto registered_actors = traffic_manager::spawn_traffic(world, core_count, target_traffic_amount);
  global_actor_list = &registered_actors;

  traffic_manager::Pipeline pipeline(
    {0.1f, 0.15f, 0.01f},
    {5.0f, 0.0f, 0.1f},
    {10.0f, 0.01f, 0.1f},
    25/3.6,
    50/3.6,
    registered_actors,
    *local_map.get(),
    client_conn,
    debug_helper,
    std::ceil(core_count / 5)
  );
  pipeline.start();

  std::cout << "Started " << 2 + 4 * std::ceil(core_count / 4) << " pipeline threads" << std::endl;

  while (true) {
    sleep(1);
    if (quit.load()) {
      break;
    }
  }

  pipeline.stop();

  for (auto actor: registered_actors) {
    if (actor != nullptr and actor->IsAlive()) {
      actor->Destroy();
    }
  }

  std::cout << "\nTrafficManager stopped by user" << std::endl;
}
