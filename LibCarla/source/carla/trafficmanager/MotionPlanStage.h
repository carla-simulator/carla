

/// This file has functionality for motion planning based on information
/// from localization, collision avoidance and traffic light response.

#pragma once

#include "carla/client/DebugHelper.h"
#include "carla/rpc/VehicleControl.h"
#include "carla/rpc/Command.h"

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/LocalizationUtils.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/Stage.h"
#include "carla/trafficmanager/TrackTraffic.h"

namespace carla
{
namespace traffic_manager
{

namespace cc = carla::client;

using namespace constants::MotionPlan;
using namespace constants::WaypointSelection;
using namespace constants::SpeedThreshold;

using constants::HybridMode::HYBRID_MODE_DT;

class MotionPlanStage: Stage
{
private:
  const std::vector<ActorId> &vehicle_id_list;
  const SimulationState &simulation_state;
  const Parameters &parameters;
  const BufferMapPtr &buffer_map;
  const TrackTraffic &track_traffic;
  const std::vector<float> urban_longitudinal_parameters;
  const std::vector<float> highway_longitudinal_parameters;
  const std::vector<float> urban_lateral_parameters;
  const std::vector<float> highway_lateral_parameters;
  const LocalizationFramePtr &localization_frame;
  const CollisionFramePtr &collision_frame;
  const TLFramePtr &tl_frame;
  std::unordered_map<ActorId, StateEntry> pid_state_map;
  std::unordered_map<ActorId, TimeInstance> teleportation_instance;
  ControlFramePtr &output_array;
  cc::DebugHelper &debug_helper;

public:
  MotionPlanStage(const std::vector<ActorId> &vehicle_id_list,
                  const SimulationState &simulation_state,
                  const Parameters &parameters,
                  const BufferMapPtr &buffer_map,
                  const TrackTraffic &track_traffic,
                  const std::vector<float> &urban_longitudinal_parameters,
                  const std::vector<float> &highway_longitudinal_parameters,
                  const std::vector<float> &urban_lateral_parameters,
                  const std::vector<float> &highway_lateral_parameters,
                  const LocalizationFramePtr &localization_frame,
                  const CollisionFramePtr &collision_frame,
                  const TLFramePtr &tl_frame,
                  ControlFramePtr &output_array,
                  cc::DebugHelper &debug_helper);

  void Update(const unsigned long index);

  void RemoveActor(const ActorId actor_id);

  void Reset();
};

} // namespace traffic_manager
} // namespace carla
