
/// This file has functionality for motion planning based on information
/// from localization, collision avoidance and traffic light response.

#pragma once

#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include "carla/trafficmanager/LocalizationUtils.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/RandomGenerator.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/Stage.h"
#include "carla/trafficmanager/TrackTraffic.h"

namespace carla {
namespace traffic_manager {

using LocalMapPtr = std::shared_ptr<InMemoryMap>;
using TLMap = std::unordered_map<std::string, SharedPtr<client::Actor>>;

class MotionPlanStage: Stage {
private:
  const std::vector<ActorId> &vehicle_id_list;
  SimulationState &simulation_state;
  const Parameters &parameters;
  const BufferMap &buffer_map;
  TrackTraffic &track_traffic;
  // PID paramenters for various road conditions.
  const std::vector<float> urban_longitudinal_parameters;
  const std::vector<float> highway_longitudinal_parameters;
  const std::vector<float> urban_lateral_parameters;
  const std::vector<float> highway_lateral_parameters;
  const LocalizationFrame &localization_frame;
  const CollisionFrame &collision_frame;
  const TLFrame &tl_frame;
  const cc::World &world;
  // Structure holding the controller state for registered vehicles.
  std::unordered_map<ActorId, StateEntry> pid_state_map;
  // Structure to keep track of duration between teleportation
  // in hybrid physics mode.
  std::unordered_map<ActorId, cc::Timestamp> teleportation_instance;
  ControlFrame &output_array;
  cc::Timestamp current_timestamp;
  RandomGenerator &random_device;
  const LocalMapPtr &local_map;

  std::pair<bool, float> CollisionHandling(const CollisionHazardData &collision_hazard,
                                           const bool tl_hazard,
                                           const cg::Vector3D ego_velocity,
                                           const cg::Vector3D ego_heading,
                                           const float max_target_velocity);

  bool SafeAfterJunction(const LocalizationData &localization,
                         const bool tl_hazard,
                         const bool collision_emergency_stop);

  float GetLandmarkTargetVelocity(const SimpleWaypoint& waypoint,
                                  const cg::Location vehicle_location,
                                  const ActorId actor_id,
                                  float max_target_velocity);

  float GetTurnTargetVelocity(const Buffer &waypoint_buffer,
                              float max_target_velocity);

  float GetThreePointCircleRadius(cg::Location first_location,
                                  cg::Location middle_location,
                                  cg::Location last_location);

public:
  MotionPlanStage(const std::vector<ActorId> &vehicle_id_list,
                  SimulationState &simulation_state,
                  const Parameters &parameters,
                  const BufferMap &buffer_map,
                  TrackTraffic &track_traffic,
                  const std::vector<float> &urban_longitudinal_parameters,
                  const std::vector<float> &highway_longitudinal_parameters,
                  const std::vector<float> &urban_lateral_parameters,
                  const std::vector<float> &highway_lateral_parameters,
                  const LocalizationFrame &localization_frame,
                  const CollisionFrame &collision_frame,
                  const TLFrame &tl_frame,
                  const cc::World &world,
                  ControlFrame &output_array,
                  RandomGenerator &random_device,
                  const LocalMapPtr &local_map);

  void Update(const unsigned long index);

  void RemoveActor(const ActorId actor_id);

  void Reset();
};

} // namespace traffic_manager
} // namespace carla
