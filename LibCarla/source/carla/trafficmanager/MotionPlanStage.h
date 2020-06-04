
/// This file has functionality for motion planning based on information
/// from localization, collision avoidance and traffic light response.

#pragma once

#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/LocalizationUtils.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/Stage.h"
#include "carla/trafficmanager/TrackTraffic.h"

namespace carla {
namespace traffic_manager {

class MotionPlanStage: Stage {
private:
  const std::vector<ActorId> &vehicle_id_list;
  const SimulationState &simulation_state;
  const Parameters &parameters;
  const BufferMap &buffer_map;
  const TrackTraffic &track_traffic;
  // PID paramenters for various road conditions.
  const std::vector<float> urban_longitudinal_parameters;
  const std::vector<float> highway_longitudinal_parameters;
  const std::vector<float> urban_lateral_parameters;
  const std::vector<float> highway_lateral_parameters;
  const LocalizationFrame &localization_frame;
  const CollisionFrame &collision_frame;
  const TLFrame &tl_frame;
  // Structure holding the controller state for registered vehicles.
  std::unordered_map<ActorId, StateEntry> pid_state_map;
  // Structure to keep track of duration between teleportation
  // in hybrid physics mode.
  std::unordered_map<ActorId, TimeInstance> teleportation_instance;
  ControlFrame &output_array;

  std::pair<bool, float> CollisionHandling(const CollisionHazardData &collision_hazard,
                                           const bool tl_hazard,
                                           const cg::Vector3D ego_velocity,
                                           const cg::Vector3D ego_heading,
                                           const float max_target_velocity);

bool SafeAfterJunction(const LocalizationData &localization,
                       const bool tl_hazard,
                       const bool collision_emergency_stop);

public:
  MotionPlanStage(const std::vector<ActorId> &vehicle_id_list,
                  const SimulationState &simulation_state,
                  const Parameters &parameters,
                  const BufferMap &buffer_map,
                  const TrackTraffic &track_traffic,
                  const std::vector<float> &urban_longitudinal_parameters,
                  const std::vector<float> &highway_longitudinal_parameters,
                  const std::vector<float> &urban_lateral_parameters,
                  const std::vector<float> &highway_lateral_parameters,
                  const LocalizationFrame &localization_frame,
                  const CollisionFrame &collision_frame,
                  const TLFrame &tl_frame,
                  ControlFrame &output_array);

  void Update(const unsigned long index);

  void RemoveActor(const ActorId actor_id);

  void Reset();
};

} // namespace traffic_manager
} // namespace carla
