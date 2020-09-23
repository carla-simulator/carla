
#pragma once

#include <memory>

#include "carla/client/DebugHelper.h"

#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include "carla/trafficmanager/LocalizationUtils.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/RandomGenerator.h"
#include "carla/trafficmanager/TrackTraffic.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/Stage.h"

namespace carla {
namespace traffic_manager {

namespace cc = carla::client;

using LocalMapPtr = std::shared_ptr<InMemoryMap>;
using LaneChangeLocationMap = std::unordered_map<ActorId, cg::Location>;

/// This class has functionality to maintain a horizon of waypoints ahead
/// of the vehicle for it to follow.
/// The class is also responsible for managing lane change decisions and
/// modify the waypoint trajectory appropriately.
class LocalizationStage : Stage {
private:
  const std::vector<ActorId> &vehicle_id_list;
  BufferMap &buffer_map;
  const SimulationState &simulation_state;
  TrackTraffic &track_traffic;
  const LocalMapPtr &local_map;
  Parameters &parameters;
  // Array of vehicles marked by stages for removal.
  std::vector<ActorId>& marked_for_removal;
  LocalizationFrame &output_array;
  cc::DebugHelper &debug_helper;
  LaneChangeLocationMap last_lane_change_location;
  ActorIdSet vehicles_at_junction;
  using SimpleWaypointPair = std::pair<SimpleWaypointPtr, SimpleWaypointPtr>;
  std::unordered_map<ActorId, SimpleWaypointPair> vehicles_at_junction_entrance;
  RandomGeneratorMap &random_devices;

  SimpleWaypointPtr AssignLaneChange(const ActorId actor_id,
                                     const cg::Location vehicle_location,
                                     const float vehicle_speed,
                                     bool force, bool direction);

  void DrawBuffer(Buffer &buffer);

  void ExtendAndFindSafeSpace(const ActorId actor_id,
                              const bool is_at_junction_entrance,
                              Buffer &waypoint_buffer);

public:
  LocalizationStage(const std::vector<ActorId> &vehicle_id_list,
                    BufferMap &buffer_map,
                    const SimulationState &simulation_state,
                    TrackTraffic &track_traffic,
                    const LocalMapPtr &local_map,
                    Parameters &parameters,
                    std::vector<ActorId>& marked_for_removal,
                    LocalizationFrame &output_array,
                    cc::DebugHelper &debug_helper,
                    RandomGeneratorMap &random_devices);

  void Update(const unsigned long index) override;

  void RemoveActor(const ActorId actor_id) override;

  void Reset() override;
};

} // namespace traffic_manager
} // namespace carla
