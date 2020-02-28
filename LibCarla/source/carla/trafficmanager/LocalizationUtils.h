// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/client/ActorList.h"
#include "carla/client/Vehicle.h"
#include "carla/client/World.h"
#include "carla/geom/Location.h"
#include "carla/road/RoadTypes.h"
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
  using GeoGridId = carla::road::JuncId;

  class TrackTraffic{

  private:
    /// Structure to keep track of overlapping waypoints between vehicles.
    using WaypointOverlap = std::unordered_map<uint64_t, ActorIdSet>;
    WaypointOverlap waypoint_overlap_tracker;
    /// Stored vehicle id set record.
    ActorIdSet actor_id_set_record;
    /// Geodesic grids occupied by actors's paths.
    std::unordered_map<ActorId, std::unordered_set<GeoGridId>> actor_to_grids;
    /// Actors currently passing through grids.
    std::unordered_map<GeoGridId, ActorIdSet> grid_to_actors;

  public:
    TrackTraffic();

    /// Methods to update, remove and retrieve vehicles passing through a waypoint.
    void UpdatePassingVehicle(uint64_t waypoint_id, ActorId actor_id);
    void RemovePassingVehicle(uint64_t waypoint_id, ActorId actor_id);
    ActorIdSet GetPassingVehicles(uint64_t waypoint_id);

    void UpdateGridPosition(const ActorId actor_id, const Buffer& buffer);
    void UpdateUnregisteredGridPosition(const ActorId actor_id, const SimpleWaypointPtr& waypoint);

    ActorIdSet GetOverlappingVehicles(ActorId actor_id);
    /// Method to delete actor data from tracking.
    void DeleteActor(ActorId actor_id);

    std::unordered_set<GeoGridId> GetGridIds(ActorId actor_id);

    std::unordered_map<GeoGridId, ActorIdSet> GetGridActors();
  };

  /// Returns the cross product (z component value) between the vehicle's
  /// heading vector and the vector along the direction to the next
  /// target waypoint on the horizon.
  float DeviationCrossProduct(Actor actor, const cg::Location &vehicle_location, const cg::Location &target_location);
  /// Returns the dot product between the vehicle's heading vector and
  /// the vector along the direction to the next target waypoint on the horizon.
  float DeviationDotProduct(Actor actor, const cg::Location &vehicle_location, const cg::Location &target_location, bool rear_offset=false);

} // namespace traffic_manager
} // namespace carla
