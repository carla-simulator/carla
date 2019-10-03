#include "Pipeline.h"

namespace traffic_manager {

namespace PipelineConstants {
  uint MINIMUM_CORE_COUNT = 4u;
  uint MINIMUM_NUMBER_OF_VEHICLES = 100u;
}
  using namespace PipelineConstants;

  uint read_core_count() {

    // Assuming quad-core if core count not available.
    uint core_count = std::thread::hardware_concurrency();
    return core_count > 0 ? core_count : MINIMUM_CORE_COUNT;
  }

  std::vector<ActorPtr> spawn_traffic(
      cc::Client &client,
      cc::World &world,
      uint core_count,
      uint target_amount = 0) {

    std::vector<ActorPtr> actor_list;
    carla::SharedPtr<cc::Map> world_map = world.GetMap();

    auto max_random = [] (uint limit) {return rand()%limit;};

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

    uint number_of_vehicles;
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
      uint blueprint_size = safe_blueprint_library.size();
      cc::ActorBlueprint blueprint = safe_blueprint_library.at(i % blueprint_size);

      blueprint.SetAttribute("role_name", "traffic_manager");

      using spawn = cr::Command::SpawnActor;
      batch_spawn_commands.push_back(spawn(blueprint.MakeActorDescription(), spawn_point));
    }

    client.ApplyBatch(std::move(batch_spawn_commands));
    // We need to wait till the simulator spawns all vehicles,
    // tried to use World::WaitForTick but it also wasn't sufficient.
    // We need to find a better way to do this.
    std::this_thread::sleep_for(500ms);

    // Gathering actors spawned by the traffic manager.
    carla::SharedPtr<cc::ActorList> world_actors = world.GetActors();
    for (auto iter = world_actors->begin(); iter != world_actors->end(); ++iter) {
      ActorPtr world_actor = *iter;
      auto world_vehicle = boost::static_pointer_cast<cc::Vehicle>(world_actor);
      std::vector<cc::ActorAttributeValue> actor_attributes = world_vehicle->GetAttributes();
      bool found_traffic_manager_vehicle = false;
      for (
        auto iter = actor_attributes.begin();
        (iter != actor_attributes.end()) && !found_traffic_manager_vehicle;
        ++iter
        ) {
        cc::ActorAttributeValue attribute = *iter;
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

  void destroy_traffic(std::vector<ActorPtr> &actor_list, cc::Client &client) {

    std::vector<cr::Command> batch_spawn_commands;
    for (auto &actor: actor_list) {
      batch_spawn_commands.push_back(cr::Command::DestroyActor(actor->GetId()));
    }
    client.ApplyBatch(std::move(batch_spawn_commands));
  }

  Pipeline::Pipeline(
      std::vector<float> longitudinal_PID_parameters,
      std::vector<float> longitudinal_highway_PID_parameters,
      std::vector<float> lateral_PID_parameters,
      float urban_target_velocity,
      float highway_target_velocity,
      std::vector<ActorPtr> &actor_list,
      InMemoryMap &local_map,
      cc::Client &client_connection,
      cc::World &world,
      cc::DebugHelper &debug_helper,
      uint pipeline_width)
    : longitudinal_PID_parameters(longitudinal_PID_parameters),
      longitudinal_highway_PID_parameters(longitudinal_highway_PID_parameters),
      lateral_PID_parameters(lateral_PID_parameters),
      urban_target_velocity(urban_target_velocity),
      actor_list(actor_list),
      local_map(local_map),
      client_connection(client_connection),
      world(world),
      debug_helper(debug_helper),
      pipeline_width(pipeline_width) {

    localization_collision_messenger = std::make_shared<LocalizationToCollisionMessenger>();
    localization_traffic_light_messenger = std::make_shared<LocalizationToTrafficLightMessenger>();
    collision_planner_messenger = std::make_shared<CollisionToPlannerMessenger>();
    localization_planner_messenger = std::make_shared<LocalizationToPlannerMessenger>();
    traffic_light_planner_messenger = std::make_shared<TrafficLightToPlannerMessenger>();
    planner_control_messenger = std::make_shared<PlannerToControlMessenger>();

    localization_stage = std::make_unique<LocalizationStage>(
        localization_planner_messenger, localization_collision_messenger,
        localization_traffic_light_messenger, actor_list.size(), pipeline_width,
        actor_list, local_map,
        debug_helper);

    collision_stage = std::make_unique<CollisionStage>(
        localization_collision_messenger, collision_planner_messenger,
        actor_list.size(), pipeline_width,
        world, debug_helper);

    traffic_light_stage = std::make_unique<TrafficLightStage>(
        localization_traffic_light_messenger, traffic_light_planner_messenger,
        actor_list.size(), pipeline_width, debug_helper);

    planner_stage = std::make_unique<MotionPlannerStage>(
        localization_planner_messenger,
        collision_planner_messenger,
        traffic_light_planner_messenger,
        planner_control_messenger,
        actor_list.size(),
        debug_helper,
        pipeline_width,
        urban_target_velocity,
        highway_target_velocity,
        longitudinal_PID_parameters,
        longitudinal_highway_PID_parameters,
        lateral_PID_parameters);

    control_stage = std::make_unique<BatchControlStage>(
        planner_control_messenger, client_connection,
        actor_list.size(), pipeline_width);
  }

  void Pipeline::Start() {

    localization_stage->Start();
    collision_stage->Start();
    traffic_light_stage->Start();
    planner_stage->Start();
    control_stage->Start();
  }

  void Pipeline::Stop() {

    localization_collision_messenger->Stop();
    localization_traffic_light_messenger->Stop();
    localization_planner_messenger->Stop();
    collision_planner_messenger->Stop();
    traffic_light_planner_messenger->Stop();
    planner_control_messenger->Stop();

    localization_stage->Stop();
    collision_stage->Stop();
    traffic_light_stage->Stop();
    planner_stage->Stop();
    control_stage->Stop();
  }

}
