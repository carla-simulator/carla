#include <atomic>
#include <cstdlib>
#include <ctime>
#include <execinfo.h>
#include <iostream>
#include <signal.h>
#include <stdexcept>
#include <random>

#include "boost/stacktrace.hpp"
#include "carla/client/Client.h"
#include "carla/client/TimeoutException.h"
#include "carla/Logging.h"
#include "carla/Memory.h"

#include "CarlaDataAccessLayer.h"
#include "InMemoryMap.h"
#include "Pipeline.h"

namespace cc = carla::client;
using Actor = carla::SharedPtr<cc::Actor>;

void run_pipeline(cc::World &world, cc::Client &client_conn,
                  uint target_traffic_amount, uint randomization_seed);

std::atomic<bool> quit(false);
void got_signal(int) {
  quit.store(true);
}

std::vector<Actor> *global_actor_list;
void handler() {

  if (!quit.load()) {

    carla::log_error("\nTrafficManager encountered a problem!\n");
    carla::log_info("Destroying all spawned actors\n");
    for (auto &actor: *global_actor_list) {
      if (actor != nullptr && actor->IsAlive()) {
        actor->Destroy();
      }
    }

    // Uncomment the below line if compiling with debug options (in CMakeLists.txt)
    // std::cout << boost::stacktrace::stacktrace() << std::endl;
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  std::set_terminate(handler);

  cc::Client client_conn = cc::Client("localhost", 2000);
  cc::World world = client_conn.GetWorld();

  if (argc == 2 && std::string(argv[1]) == "-h") {
    std::cout << "\nAvailable options\n";
    std::cout << "[-n] \t\t Number of vehicles to be spawned\n";
    std::cout << "[-s] \t\t System randomization seed integer\n";
  } else {

    uint target_traffic_amount = 0u;
    if (argc >= 3 && std::string(argv[1]) == "-n") {
      try {
        target_traffic_amount = std::stoi(argv[2]);
      } catch (const std::exception &e) {
        carla::log_warning("Failed to parse argument, choosing defaults\n");
      }
    }

    int randomization_seed = -1;
    if (argc == 5 && std::string(argv[3]) == "-s") {
      try {
        randomization_seed = std::stoi(argv[4]);
      } catch (const std::exception &e) {
        carla::log_warning("Failed to parse argument, choosing defaults\n");
      }
    }

    if (randomization_seed < 0) {
      std::srand(std::time(0));
    } else {
      std::srand(randomization_seed);
    }

    run_pipeline(world, client_conn, target_traffic_amount, randomization_seed);

  }

  return 0;
}

void run_pipeline(cc::World &world, cc::Client &client_conn,
                  uint target_traffic_amount, uint randomization_seed) {

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = got_signal;
  sigfillset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);

  using WorldMap = carla::SharedPtr<cc::Map>;
  WorldMap world_map = world.GetMap();
  cc::DebugHelper debug_helper = client_conn.GetWorld().MakeDebugHelper();
  auto dao = traffic_manager::CarlaDataAccessLayer(world_map);
  using Topology = std::vector<std::pair<traffic_manager::WaypointPtr, traffic_manager::WaypointPtr>>;
  Topology topology = dao.GetTopology();
  auto local_map = std::make_shared<traffic_manager::InMemoryMap>(topology);
  local_map->SetUp(1.0);

  uint core_count = traffic_manager::read_core_count();
  std::vector<Actor> registered_actors = traffic_manager::spawn_traffic(
    client_conn, world, core_count, target_traffic_amount);
  global_actor_list = &registered_actors;

  client_conn.SetTimeout(2s);
  
  traffic_manager::Pipeline pipeline(
      {0.1f, 0.15f, 0.01f},
      {5.0f, 0.0f, 0.1f},
      {10.0f, 0.01f, 0.1f},
      25 / 3.6,
      50 / 3.6,
      registered_actors,
      *local_map.get(),
      client_conn,
      world,
      debug_helper,
      1
      );

  try
  {
    pipeline.Start();
    carla::log_info("TrafficManager started\n");

    while (!quit.load()) {
      sleep(1);
      // Periodically polling if Carla is still running
      world.GetSettings();
    }
  }
  catch(const cc::TimeoutException& e)
  {
    carla::log_error("Carla has stopped running, stopping TrafficManager\n");
  }

  pipeline.Stop();

  traffic_manager::destroy_traffic(registered_actors, client_conn);

  carla::log_info("\nTrafficManager stopped by user\n");
}
