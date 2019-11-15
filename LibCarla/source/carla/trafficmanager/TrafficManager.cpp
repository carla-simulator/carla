#include "TrafficManager.h"

namespace traffic_manager {

  TrafficManager::TrafficManager(
      std::vector<float> longitudinal_PID_parameters,
      std::vector<float> longitudinal_highway_PID_parameters,
      std::vector<float> lateral_PID_parameters,
      float perc_decrease_from_limit,
      cc::Client &client_connection)
    : longitudinal_PID_parameters(longitudinal_PID_parameters),
      longitudinal_highway_PID_parameters(longitudinal_highway_PID_parameters),
      lateral_PID_parameters(lateral_PID_parameters),
      client_connection(client_connection),
      world(client_connection.GetWorld()),
      debug_helper(client_connection.GetWorld().MakeDebugHelper()) {

    using WorldMap = carla::SharedPtr<cc::Map>;
    WorldMap world_map = world.GetMap();
    auto dao = CarlaDataAccessLayer(world_map);
    using Topology = std::vector<std::pair<WaypointPtr, WaypointPtr>>;
    Topology topology = dao.GetTopology();
    local_map = std::make_shared<traffic_manager::InMemoryMap>(topology);
    local_map->SetUp(0.1f);

    parameters.SetGlobalPercentageBelowLimit(perc_decrease_from_limit);

    localization_collision_messenger = std::make_shared<LocalizationToCollisionMessenger>();
    localization_traffic_light_messenger = std::make_shared<LocalizationToTrafficLightMessenger>();
    collision_planner_messenger = std::make_shared<CollisionToPlannerMessenger>();
    localization_planner_messenger = std::make_shared<LocalizationToPlannerMessenger>();
    traffic_light_planner_messenger = std::make_shared<TrafficLightToPlannerMessenger>();
    planner_control_messenger = std::make_shared<PlannerToControlMessenger>();

    localization_stage = std::make_unique<LocalizationStage>(
        localization_planner_messenger, localization_collision_messenger,
        localization_traffic_light_messenger,
        registered_actors, *local_map.get(),
        parameters, debug_helper);

    collision_stage = std::make_unique<CollisionStage>(
        localization_collision_messenger, collision_planner_messenger,
        world, parameters, debug_helper);

    traffic_light_stage = std::make_unique<TrafficLightStage>(
        localization_traffic_light_messenger, traffic_light_planner_messenger);

    planner_stage = std::make_unique<MotionPlannerStage>(
        localization_planner_messenger,
        collision_planner_messenger,
        traffic_light_planner_messenger,
        planner_control_messenger,
        parameters,
        longitudinal_PID_parameters,
        longitudinal_highway_PID_parameters,
        lateral_PID_parameters);

    control_stage = std::make_unique<BatchControlStage>(
        planner_control_messenger, client_connection);

    Start();
  }

  TrafficManager::~TrafficManager() {
    Stop();
  }

  std::unique_ptr<TrafficManager> TrafficManager::singleton_pointer = nullptr;

  TrafficManager& TrafficManager::GetInstance(cc::Client &client_connection) {
    // std::lock_guard<std::mutex> lock(singleton_mutex);

    if (singleton_pointer == nullptr) {

      std::vector<float> longitudinal_param = {0.1f, 0.15f, 0.01f};
      std::vector<float> longitudinal_highway_param = {5.0f, 0.09f, 0.01f};
      std::vector<float> lateral_param = {10.0f, 0.0f, 0.1f};
      float perc_decrease_from_limit = 20.0f;

      TrafficManager* tm_ptr = new TrafficManager(
        longitudinal_param, longitudinal_highway_param, lateral_param,
        perc_decrease_from_limit, client_connection
      );

      singleton_pointer = std::unique_ptr<TrafficManager>(tm_ptr);
    }

    return *singleton_pointer.get();
  }

  void TrafficManager::RegisterVehicles(const std::vector<ActorPtr> &actor_list) {
    registered_actors.Insert(actor_list);
  }

  void TrafficManager::UnregisterVehicles(const std::vector<ActorPtr> &actor_list) {
    registered_actors.Remove(actor_list);
  }

  void TrafficManager::Start() {

    localization_collision_messenger->Start();
    localization_traffic_light_messenger->Start();
    localization_planner_messenger->Start();
    collision_planner_messenger->Start();
    traffic_light_planner_messenger->Start();
    planner_control_messenger->Start();

    localization_stage->Start();
    collision_stage->Start();
    traffic_light_stage->Start();
    planner_stage->Start();
    control_stage->Start();
  }

  void TrafficManager::Stop() {

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

  void TrafficManager::SetPercentageSpeedBelowLimit(const ActorPtr &actor, const float percentage) {

    parameters.SetPercentageSpeedBelowLimit(actor, percentage);
  }

  void TrafficManager::SetCollisionDetection(
      const ActorPtr &reference_actor,
      const ActorPtr &other_actor,
      const bool detect_collision) {

    parameters.SetCollisionDetection(reference_actor, other_actor, detect_collision);
  }

  void TrafficManager::SetForceLaneChange(const ActorPtr &actor, const bool direction) {

    parameters.SetForceLaneChange(actor, direction);
  }

  void TrafficManager::SetAutoLaneChange(const ActorPtr &actor, const bool enable) {

    parameters.SetAutoLaneChange(actor, enable);
  }

  void TrafficManager::SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {

    parameters.SetDistanceToLeadingVehicle(actor, distance);
  }
}
