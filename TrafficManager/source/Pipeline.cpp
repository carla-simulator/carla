#include "Pipeline.h"

namespace traffic_manager {

  int MINIMUM_CORE_COUNT = 4;

  /// Pick a random element from @a range.
  template <typename RangeT, typename RNG>
  static auto &RandomChoice(const RangeT &range, RNG &&generator) {
    EXPECT_TRUE(range.size() > 0u);
    std::uniform_int_distribution<size_t> dist{0u, range.size() - 1u};
    return range[dist(std::forward<RNG>(generator))];
  }

  int read_core_count() {
    auto core_count = std::thread::hardware_concurrency();
    // Assuming quad core if core count not available
    return core_count > 0? core_count: MINIMUM_CORE_COUNT;
  }

  std::vector<carla::SharedPtr<carla::client::Actor>> spawn_traffic (
    carla::client::World& world,
    int core_count,
    int target_amount = 0
  ) {
    std::vector<carla::SharedPtr<carla::client::Actor>> actor_list;
    auto world_map = world.GetMap();
    auto spawn_points = world_map->GetRecommendedSpawnPoints();
    auto blueprint_library = world.GetBlueprintLibrary()->Filter("vehicle.*");
    std::mt19937_64 rng((std::random_device())());

    int number_of_vehicles;
    if (target_amount == 0) {
      if (core_count <= 4)
        number_of_vehicles = 100;
      else if (core_count <= 8)
        number_of_vehicles = 150;
      else if (core_count <= 16)
        number_of_vehicles = 200;
      else
        number_of_vehicles = 250;
    } else {
      number_of_vehicles = target_amount;
    }

    if (number_of_vehicles > spawn_points.size())
      number_of_vehicles = spawn_points.size();

    std::cout << "Spawning " << number_of_vehicles << " vehicles" << std::endl;

    for (int i=0; i<number_of_vehicles; i++) {
      auto spawn_point = spawn_points[i];
      auto blueprint = RandomChoice(*blueprint_library, rng);
      while(
        blueprint.GetAttribute("number_of_wheels") != 4
        or blueprint.GetId().compare("vehicle.carlamotors.carlacola") == 0
        or blueprint.GetId().compare("vehicle.bmw.isetta") == 0
      ) {
        blueprint = RandomChoice(*blueprint_library, rng);
      }
      auto actor = world.TrySpawnActor(blueprint, spawn_point);
      actor_list.push_back(actor);
    }

    return actor_list;
  }

  Pipeline::Pipeline(
      std::vector<float> longitudinal_PID_parameters,
      std::vector<float> lateral_PID_parameters,
      float target_velocity,
      int pipeline_width,
      SharedData& shared_data)
    : longitudinal_PID_parameters(longitudinal_PID_parameters),
      lateral_PID_parameters(lateral_PID_parameters),
      target_velocity(target_velocity),
      pipeline_width(pipeline_width),
      shared_data(shared_data)
    {}

  void Pipeline::setup() {

    for (int i = 0; i < NUMBER_OF_STAGES; i++) {
      message_queues.push_back(std::make_shared<SyncQueue<PipelineMessage>>(20));
    }

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
    for (auto stage: stages) {
      stage->start();
    }
  }

  void Pipeline::stop() {
    int i =0;
    for (auto stage: stages) {
      stage->stop();
      i++;
    }
  }

}
