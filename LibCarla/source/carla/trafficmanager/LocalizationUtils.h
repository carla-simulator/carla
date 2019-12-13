// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/client/Vehicle.h"
#include "carla/geom/Location.h"
#include "carla/rpc/ActorId.h"

#include "carla/trafficmanager/SimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

  namespace cc = carla::client;
  namespace cg = carla::geom;
  using Actor = carla::SharedPtr<cc::Actor>;
  using ActorId = carla::ActorId;
  using ActorIdSet = std::unordered_set<ActorId>;
  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
  using Buffer = std::deque<SimpleWaypointPtr>;

  class TrackTraffic{

  private:
    /// Structure to keep track of overlapping waypoints between vehicles.
    using WaypointOverlap = std::unordered_map<uint64_t, ActorIdSet>;
    WaypointOverlap waypoint_overlap_tracker;
    /// Structure to keep track of vehicles with overlapping paths.
    std::unordered_map<ActorId, ActorIdSet> overlapping_vehicles;
    /// Stored vehicle id set record.
    ActorIdSet actor_id_set_record;

  public:
    TrackTraffic();

    /// Methods to update, remove and retrieve vehicles passing through a waypoint.
    void UpdatePassingVehicle(uint64_t waypoint_id, ActorId actor_id);
    void RemovePassingVehicle(uint64_t waypoint_id, ActorId actor_id);
    ActorIdSet GetPassingVehicles(uint64_t waypoint_id);

    /// Methods to update, remove and retrieve vehicles with overlapping vehicles.
    void UpdateOverlappingVehicle(ActorId actor_id, ActorId other_id);
    void RemoveOverlappingVehicle(ActorId actor_id, ActorId other_id);
    ActorIdSet GetOverlappingVehicles(ActorId actor_id);

  };

  /// Returns the cross product (z component value) between the vehicle's
  /// heading vector and the vector along the direction to the next
  /// target waypoint on the horizon.
  float DeviationCrossProduct(Actor actor, const cg::Location &target_location);
  /// Returns the dot product between the vehicle's heading vector and
  /// the vector along the direction to the next target waypoint on the horizon.
  float DeviationDotProduct(Actor actor, const cg::Location &target_location, bool rear_offset=false);

} // namespace traffic_manager
} // namespace carla
