#include "Pipeline.h"

namespace traffic_manager {

  namespace PipelineConstants {
    int MINIMUM_CORE_COUNT = 4;
    int MINIMUM_NUMBER_OF_VEHICLES = 100;
  }
  using namespace PipelineConstants;

  // Pick a random element from @a range.
  template <typename RangeT, typename RNG>
  static auto &RandomChoice(const RangeT &range, RNG &&generator) {
    EXPECT_TRUE(range.size() > 0u);
    std::uniform_int_distribution<size_t> dist{0u, range.size() - 1u};
    return range[dist(std::forward<RNG>(generator))];
  }

  int read_core_count() {
    auto core_count = std::thread::hardware_concurrency();
    // Assuming quad core if core count not available
    return core_count > 0 ? core_count : MINIMUM_CORE_COUNT;
  }

  std::vector<carla::SharedPtr<carla::client::Actor>> spawn_traffic(
      carla::client::World &world,
      int core_count,
      int target_amount = 0) {

    std::vector<carla::SharedPtr<carla::client::Actor>> actor_list;
    auto world_map = world.GetMap();
    auto spawn_points = world_map->GetRecommendedSpawnPoints();
    std::random_shuffle(spawn_points.begin(), spawn_points.end());
    auto blueprint_library = world.GetBlueprintLibrary()->Filter("vehicle.*");
    std::mt19937_64 rng((std::random_device())());

    int number_of_vehicles;
    if (target_amount <= 0) {
      number_of_vehicles = MINIMUM_NUMBER_OF_VEHICLES;
    } else {
      number_of_vehicles = target_amount;
    }

    if (number_of_vehicles > spawn_points.size()) {
      carla::log_warning("Number of requested vehicles more than number available spawn points\n");
      carla::log_info("Spawning vehicle at every spawn point\n");
      number_of_vehicles = spawn_points.size();
    }
    
    carla::log_info("Spawning "+ std::to_string(number_of_vehicles) +" vehicles\n");

    for (int i = 0; i < number_of_vehicles; ++i) {
      auto spawn_point = spawn_points[i];
      auto blueprint = RandomChoice(*blueprint_library, rng);
      while (
        blueprint.GetAttribute("number_of_wheels") != 4 ||
        blueprint.GetId().compare("vehicle.carlamotors.carlacola") == 0 ||
        blueprint.GetId().compare("vehicle.bmw.isetta") == 0
        ) {
        blueprint = RandomChoice(*blueprint_library, rng);
      }

      auto actor = world.TrySpawnActor(blueprint, spawn_point);
      if (actor != nullptr) {
        actor_list.push_back(actor);
      }
    }

    return actor_list;
  }

  Pipeline::Pipeline(
      std::vector<float> longitudinal_PID_parameters,
      std::vector<float> longitudinal_highway_PID_parameters,
      std::vector<float> lateral_PID_parameters,
      float urban_target_velocity,
      float highway_target_velocity,
      std::vector<carla::SharedPtr<carla::client::Actor>> &actor_list,
      InMemoryMap &local_map,
      carla::client::Client &client_connection,
      carla::client::World &world,
      carla::client::DebugHelper &debug_helper,
      int pipeline_width)
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
        actor_list.size(), pipeline_width);

    planner_stage = std::make_unique<MotionPlannerStage>(
        localization_planner_messenger,
        collision_planner_messenger,
        traffic_light_planner_messenger,
        planner_control_messenger,
        actor_list.size(), pipeline_width,
        urban_target_velocity,
        highway_target_velocity,
        longitudinal_PID_parameters,
        longitudinal_highway_PID_parameters,
        lateral_PID_parameters);

    control_stage = std::make_unique<BatchControlStage>(
        planner_control_messenger, client_connection,
        actor_list.size(), pipeline_width);

  }

  // To start the pipeline
  void Pipeline::Start() {
    localization_stage->Start();
    collision_stage->Start();
    traffic_light_stage->Start();
    planner_stage->Start();
    control_stage->Start();
  }

  // To stop the pipeline
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
