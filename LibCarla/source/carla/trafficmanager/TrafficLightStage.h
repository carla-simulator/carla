
#pragma once

#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/RandomGenerator.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/Stage.h"

namespace carla {
namespace traffic_manager {

/// This class has functionality for responding to traffic lights
/// and managing entry into non-signalized junctions.
class TrafficLightStage: Stage {
private:
  const std::vector<ActorId> &vehicle_id_list;
  const SimulationState &simulation_state;
  const BufferMap &buffer_map;
  const Parameters &parameters;
  const cc::World &world;
  /// Map containing the time ticket issued for vehicles.
  std::unordered_map<ActorId, cc::Timestamp> vehicle_last_ticket;
  /// Map containing the previous time ticket issued for junctions.
  std::unordered_map<JunctionID, cc::Timestamp> junction_last_ticket;
  /// Map containing the previous junction visited by a vehicle.
  std::unordered_map<ActorId, JunctionID> vehicle_last_junction;
  TLFrame &output_array;
  RandomGeneratorMap &random_devices;
  cc::Timestamp current_timestamp;

  bool HandleNonSignalisedJunction(const ActorId ego_actor_id, const JunctionID junction_id,
                                   cc::Timestamp timestamp);

public:
  TrafficLightStage(const std::vector<ActorId> &vehicle_id_list,
                    const SimulationState &Simulation_state,
                    const BufferMap &buffer_map,
                    const Parameters &parameters,
                    const cc::World &world,
                    TLFrame &output_array,
                    RandomGeneratorMap &random_devices);

  void Update(const unsigned long index) override;

  void RemoveActor(const ActorId actor_id) override;

  void Reset() override;
};

} // namespace traffic_manager
} // namespace carla
