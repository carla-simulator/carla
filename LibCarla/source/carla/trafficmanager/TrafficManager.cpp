#include "TrafficManager.h"

namespace traffic_manager {

  TrafficManager::TrafficManager(
      std::vector<float> longitudinal_PID_parameters,
      std::vector<float> longitudinal_highway_PID_parameters,
      std::vector<float> lateral_PID_parameters,
      float urban_target_velocity,
      float highway_target_velocity,
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
    local_map->SetUp(1.0);


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
        force_lane_change, debug_helper);

    collision_stage = std::make_unique<CollisionStage>(
        localization_collision_messenger, collision_planner_messenger,
        world, ignore_collision, debug_helper);

    traffic_light_stage = std::make_unique<TrafficLightStage>(
        localization_traffic_light_messenger, traffic_light_planner_messenger);

    planner_stage = std::make_unique<MotionPlannerStage>(
        localization_planner_messenger,
        collision_planner_messenger,
        traffic_light_planner_messenger,
        planner_control_messenger,
        vehicle_target_velocity,
        urban_target_velocity,
        highway_target_velocity,
        longitudinal_PID_parameters,
        longitudinal_highway_PID_parameters,
        lateral_PID_parameters);

    control_stage = std::make_unique<BatchControlStage>(
        planner_control_messenger, client_connection);
  }

  void TrafficManager::RegisterVehicles(std::vector<ActorPtr> actor_list) {
    registered_actors.Insert(actor_list);
  }

  void TrafficManager::UnregisterVehicles(std::vector<ActorId> actor_ids) {
    registered_actors.Remove(actor_ids);
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

  void TrafficManager::SetVehicleTargetVelocity(ActorId actor_id, float velocity) {

    vehicle_target_velocity.AddEntry({actor_id, velocity});
  }

  void TrafficManager::SetCollisionDetection(ActorPtr reference_actor, ActorPtr other_actor, bool detect_collision) {

    ActorId reference_id = reference_actor->GetId();
    ActorId other_id = other_actor->GetId();

    if (detect_collision) {

      if (ignore_collision.Contains(reference_id)) {
        std::shared_ptr<AtomicActorSet> actor_set = ignore_collision.GetValue(reference_id);
        if (actor_set->Contains(other_id)) {
          actor_set->Remove({other_id});
        }
      }
    } else {

      if (ignore_collision.Contains(reference_id)) {
        std::shared_ptr<AtomicActorSet> actor_set = ignore_collision.GetValue(reference_id);
        if (!actor_set->Contains(other_id)) {
          actor_set->Insert({other_actor});
        }
      } else {
        std::shared_ptr<AtomicActorSet> actor_set = std::make_shared<AtomicActorSet>();
        actor_set->Insert({other_actor});
        auto entry = std::make_pair(reference_id, actor_set);
        ignore_collision.AddEntry(entry);
      }
    }
  }

  void TrafficManager::ForceLaneChange(ActorPtr actor, bool direction) {

    auto entry = std::make_pair(actor->GetId(), direction);
    force_lane_change.AddEntry(entry);
  }
}
