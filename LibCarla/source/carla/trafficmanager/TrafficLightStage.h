
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

  /// Variables used to handle non signalized junctions

  /// Map containing the vehicles entering a specific junction, ordered by time of arrival.
  std::unordered_map<JunctionID, std::deque<ActorId>> entering_vehicles_map;
  /// Map linking the vehicles with their current junction. Used for easy access to the previous two maps.
  std::unordered_map<ActorId, JunctionID> vehicle_last_junction;
  /// Map containing the timestamp at which the actor first stopped at a stop sign.
  std::unordered_map<ActorId, cc::Timestamp> vehicle_stop_time;
  TLFrame &output_array;
  RandomGenerator &random_device;
  cc::Timestamp current_timestamp;

  /// This controls all vehicle's interactions at non signalized junctions. Priorities are done by order of arrival
  /// and no two vehicle will enter the junction at the same time. Only once it is exiting can the next one enter.
  /// Additionally, all vehicles will always brake at the stop sign for a set amount of time.
  bool HandleNonSignalisedJunction(const ActorId ego_actor_id, const JunctionID junction_id,
                                   cc::Timestamp timestamp);

  /// Initialized the vehicle to the non-signalized junction maps
  void AddActorToNonSignalisedJunction(const ActorId ego_actor_id, const JunctionID junction_id);

  /// Get current affected junction id for the vehicle
  JunctionID GetAffectedJunctionId(const ActorId ego_actor_id);

public:
  TrafficLightStage(const std::vector<ActorId> &vehicle_id_list,
                    const SimulationState &Simulation_state,
                    const BufferMap &buffer_map,
                    const Parameters &parameters,
                    const cc::World &world,
                    TLFrame &output_array,
                    RandomGenerator &random_device);

  void Update(const unsigned long index) override;

  void RemoveActor(const ActorId actor_id) override;

  void Reset() override;
};

} // namespace traffic_manager
} // namespace carla
