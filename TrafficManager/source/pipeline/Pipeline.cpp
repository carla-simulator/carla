#include "Pipeline.h"

namespace traffic_manager {

  Pipeline::Pipeline(
      std::vector<float> longitudinal_PID_parameters,
      std::vector<float> longitudinal_highway_PID_parameters,
      std::vector<float> lateral_PID_parameters,
      float urban_target_velocity,
      float highway_target_velocity,
      std::vector<ActorPtr> &actor_list,
      cc::Client &client_connection,
      uint pipeline_width)
    : longitudinal_PID_parameters(longitudinal_PID_parameters),
      longitudinal_highway_PID_parameters(longitudinal_highway_PID_parameters),
      lateral_PID_parameters(lateral_PID_parameters),
      urban_target_velocity(urban_target_velocity),
      actor_list(actor_list),
      client_connection(client_connection),
      pipeline_width(pipeline_width),
      world(client_connection.GetWorld()),
      debug_helper(client_connection.GetWorld().MakeDebugHelper()) {

    using WorldMap = carla::SharedPtr<cc::Map>;
    WorldMap world_map = world.GetMap();
    auto dao = CarlaDataAccessLayer(world_map);
    using Topology = std::vector<std::pair<WaypointPtr, WaypointPtr>>;
    Topology topology = dao.GetTopology();
    local_map = std::make_shared<traffic_manager::InMemoryMap>(topology);
    local_map->SetUp(1.0);


    localization_collision_messenger = std::make_shared<LocalizationToCollisionMessenger>();
    localization_traffic_light_messenger = std::make_shared<LocalizationToTrafficLightMessenger>();
    collision_planner_messenger = std::make_shared<CollisionToPlannerMessenger>();
    localization_planner_messenger = std::make_shared<LocalizationToPlannerMessenger>();
    traffic_light_planner_messenger = std::make_shared<TrafficLightToPlannerMessenger>();
    planner_control_messenger = std::make_shared<PlannerToControlMessenger>();

    localization_stage = std::make_unique<LocalizationStage>(
        localization_planner_messenger, localization_collision_messenger,
        localization_traffic_light_messenger, actor_list.size(), pipeline_width,
        actor_list, *local_map.get(),
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
