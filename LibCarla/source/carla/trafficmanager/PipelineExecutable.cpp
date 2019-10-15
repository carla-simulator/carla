#include <atomic>
#include <chrono>
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
#include "carla/rpc/Command.h"

#include "Pipeline.h"

static uint MINIMUM_NUMBER_OF_VEHICLES = 100u;

namespace cc = carla::client;
namespace cg = carla::geom;
namespace cr = carla::rpc;
using Actor = carla::SharedPtr<cc::Actor>;

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

std::vector<Actor> spawn_traffic(
    cc::Client &client,
    cc::World &world,
    ulong target_amount = 0u) {

  std::vector<Actor> actor_list;
  carla::SharedPtr<cc::Map> world_map = world.GetMap();

  auto max_random = [] (int limit) {return rand()%limit;};

  // Get a random selection of spawn points from the map.
  std::vector<cg::Transform> spawn_points = world_map->GetRecommendedSpawnPoints();
  std::random_shuffle(spawn_points.begin(), spawn_points.end(), max_random);

  // Blueprint library containing all vehicle types.
  using BlueprintLibraryPtr = carla::SharedPtr<cc::BlueprintLibrary>;
  BlueprintLibraryPtr blueprint_library = world.GetBlueprintLibrary()->Filter("vehicle.*");

  // Removing unsafe vehicles from the blueprint library.
  std::vector<cc::ActorBlueprint> safe_blueprint_library;
  for (auto &blueprint: *blueprint_library.get()) {

    if (blueprint.GetAttribute("number_of_wheels") == 4 &&
        blueprint.GetId() != "vehicle.carlamotors.carlacola" &&
        blueprint.GetId() != "vehicle.bmw.isetta") {

      safe_blueprint_library.push_back(blueprint);
    }
  }

  // Randomizing the order of the list.
  std::random_shuffle(safe_blueprint_library.begin(), safe_blueprint_library.end(), max_random);

  unsigned long number_of_vehicles;
  if (target_amount == 0u) {
    number_of_vehicles = MINIMUM_NUMBER_OF_VEHICLES;
  } else {
    number_of_vehicles = target_amount;
  }

  if (number_of_vehicles > spawn_points.size()) {
    carla::log_warning("Number of requested vehicles more than number available spawn points\n");
    carla::log_info("Spawning vehicle at every spawn point\n");
    number_of_vehicles = spawn_points.size();
  }

  carla::log_info("Spawning " + std::to_string(number_of_vehicles) + " vehicles\n");

  // Creating spawn batch command.
  std::vector<cr::Command> batch_spawn_commands;
  for (uint i = 0u; i < number_of_vehicles; ++i) {

    cg::Transform spawn_point = spawn_points.at(i);
    size_t blueprint_size = safe_blueprint_library.size();
    cc::ActorBlueprint blueprint = safe_blueprint_library.at(i % blueprint_size);

    blueprint.SetAttribute("role_name", "traffic_manager");

    using spawn = cr::Command::SpawnActor;
    batch_spawn_commands.push_back(spawn(blueprint.MakeActorDescription(), spawn_point));
  }

  client.ApplyBatch(std::move(batch_spawn_commands));
  // We need to wait till the simulator spawns all vehicles,
  // tried to use World::WaitForTick but it also wasn't sufficient.
  // We need to find a better way to do this.
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // Gathering actors spawned by the traffic manager.
  carla::SharedPtr<cc::ActorList> world_actors = world.GetActors();
  for (auto iter = world_actors->begin(); iter != world_actors->end(); ++iter) {
    Actor world_actor = *iter;
    auto world_vehicle = boost::static_pointer_cast<cc::Vehicle>(world_actor);
    std::vector<cc::ActorAttributeValue> actor_attributes = world_vehicle->GetAttributes();
    bool found_traffic_manager_vehicle = false;
    for (auto attribute_iter = actor_attributes.begin();
         (attribute_iter != actor_attributes.end()) && !found_traffic_manager_vehicle;
         ++attribute_iter
        ) {
      cc::ActorAttributeValue attribute = *attribute_iter;
      if (attribute.GetValue() == "traffic_manager") {
        found_traffic_manager_vehicle = true;
      }
    }
    if (found_traffic_manager_vehicle) {
      actor_list.push_back(world_actor);
    }
  }

  return actor_list;
}


void destroy_traffic(std::vector<Actor> &actor_list, cc::Client &client) {

  std::vector<cr::Command> batch_spawn_commands;
  for (auto &actor: actor_list) {
    batch_spawn_commands.push_back(cr::Command::DestroyActor(actor->GetId()));
  }
  client.ApplyBatch(std::move(batch_spawn_commands));
}


void run_pipeline(cc::World &world, cc::Client &client_conn, ulong target_traffic_amount) {

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = got_signal;
  sigfillset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);

  std::vector<Actor> registered_actors = spawn_traffic(
    client_conn, world, target_traffic_amount);
  global_actor_list = &registered_actors;

  client_conn.SetTimeout(std::chrono::seconds(2));

  traffic_manager::Pipeline pipeline(
      {0.1f, 0.15f, 0.01f},
      {5.0f, 0.0f, 0.1f},
      {10.0f, 0.01f, 0.1f},
      25 / 3.6f,
      50 / 3.6f,
      client_conn
      );

  try {

    pipeline.Start();

    // Delayed vehicles' registration for demonstration.
    sleep(1);
    pipeline.RegisterVehicles(registered_actors);

    carla::log_info("TrafficManager started\n");

    while (!quit.load()) {
      sleep(1);
      // Periodically polling to check if Carla is still running.
      world.GetSettings();
    }
  } catch(const cc::TimeoutException& e) {

    carla::log_error("Carla has stopped running, stopping TrafficManager\n");
  }

  pipeline.Stop();

  destroy_traffic(registered_actors, client_conn);

  carla::log_info("\nTrafficManager stopped by user\n");
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

    ulong target_traffic_amount = 0u;
    if (argc >= 3 && std::string(argv[1]) == "-n") {
      try {
        target_traffic_amount = std::stoul(argv[2]);
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
      std::srand(static_cast<uint>(std::time(0)));
    } else {
      std::srand(static_cast<uint>(abs(randomization_seed)));
    }

    run_pipeline(world, client_conn, target_traffic_amount);
  }

  return 0;
}
