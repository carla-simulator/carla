
/// This file has functionality for responding to traffic lights
/// and managing entry into non-signalized junctions.

#pragma once

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/LocalizationUtils.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/Stage.h"

namespace carla
{
namespace traffic_manager
{

using constants::TrafficLight::NO_SIGNAL_PASSTHROUGH_INTERVAL;
using constants::WaypointSelection::JUNCTION_LOOK_AHEAD;

class TrafficLightStage: Stage
{
private:
  const std::vector<ActorId> &vehicle_id_list;
  const SimulationState &simulation_state;
  const BufferMapPtr &buffer_map;
  const Parameters &parameters;
  std::unordered_map<ActorId, TimeInstance> vehicle_last_ticket;
  std::unordered_map<JunctionID, TimeInstance> junction_last_ticket;
  std::unordered_map<ActorId, JunctionID> vehicle_last_junction;
  TLFramePtr &output_array;

public:
  TrafficLightStage(const std::vector<ActorId> &vehicle_id_list,
                    const SimulationState &Simulation_state,
                    const BufferMapPtr &buffer_map,
                    const Parameters &parameters,
                    TLFramePtr &output_array);

  void Update(const unsigned long index) override;

  void RemoveActor(const ActorId actor_id) override;

  void Reset() override;
};

} // namespace traffic_manager
} // namespace carla
