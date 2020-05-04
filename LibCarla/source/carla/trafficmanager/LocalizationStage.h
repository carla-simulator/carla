
/// This file has functionality to maintain a horizon of waypoints ahead
/// of the vehicle for it to follow.
/// The class is also responsible for managing lane change decisions and
/// modify the waypoint trajectory appropriately.

#pragma once

#include <deque>
#include <memory>

#include "carla/rpc/ActorId.h"

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include "carla/trafficmanager/LocalizationUtils.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/SimpleWaypoint.h"
#include "carla/trafficmanager/TrackTraffic.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/Stage.h"

namespace carla
{
namespace traffic_manager
{

using namespace constants::PathBufferUpdate;
using namespace constants::LaneChange;

namespace cc = carla::client;

using LocalMapPtr = std::shared_ptr<InMemoryMap>;
using LaneChangeLocationMap = std::unordered_map<ActorId, cg::Location>;

class LocalizationStage : Stage
{
private:
  const std::vector<ActorId> &vehicle_id_list;
  BufferMapPtr &buffer_map;
  const SimulationState &simulation_state;
  TrackTraffic &track_traffic;
  const LocalMapPtr &local_map;
  Parameters &parameters;
  LaneChangeLocationMap last_lane_change_location;

public:
  LocalizationStage(const std::vector<ActorId> &vehicle_id_list,
                    BufferMapPtr &buffer_map,
                    const SimulationState &simulation_state,
                    TrackTraffic &track_traffic,
                    const LocalMapPtr &local_map,
                    Parameters &parameters);

  void Update(const unsigned long index) override;

  void RemoveActor(const ActorId actor_id) override;

  void Reset() override;

  SimpleWaypointPtr AssignLaneChange(const ActorId actor_id,
                                     const cg::Location vehicle_location,
                                     const float vehicle_speed,
                                     bool force, bool direction);
};

} // namespace traffic_manager
} // namespace carla
