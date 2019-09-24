#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "carla/client/Vehicle.h"
#include "carla/rpc/ActorId.h"

#include "MessengerAndDataTypes.h"
#include "SimpleWaypoint.h"

namespace traffic_manager {

  /// Structure used to hold geo ids
  struct GeoIds {
    uint road_id = 0;
    uint section_id = 0;
    int lane_id = 0;

    bool operator==(const GeoIds &other) const {
      return (road_id == other.road_id &&
             section_id == other.section_id &&
             lane_id == other.lane_id);
    }
  };

}

namespace std {

  /// Specialization of std::hash for GeoIds type
  template <>
  struct hash<traffic_manager::GeoIds>{
    std::size_t operator()(const traffic_manager::GeoIds &k) const {
      using std::size_t;
      using std::hash;

      return ((hash<uint>()(k.road_id) ^
             (hash<uint>()(k.section_id) << 1)) >> 1) ^
             (hash<int>()(k.lane_id) << 1);
    }
  };

}

namespace traffic_manager {

namespace cc = carla::client;
namespace cg = carla::geom;
  using ActorId = carla::ActorId;
  using Actor = carla::SharedPtr<cc::Actor>;

  /// Returns the cross product (z component value) between vehicle's heading
  /// vector and the vector along the direction to the next target waypoint in
  /// the horizon.
  float DeviationCrossProduct(Actor actor, const cg::Location &target_location);

  /// Returns the dot product between vehicle's heading vector and
  /// the vector along the direction to the next target waypoint in the horizon.
  float DeviationDotProduct(Actor actor, const cg::Location &target_location);

  /// This class keeps track of vehicle’s positions in road sections, lanes and
  /// provides lane change decisions.
  class TrafficDistributor {

  private:

    /// Mutex used to manage contention for internal resources between various
    /// accessors
    mutable std::shared_timed_mutex distributor_mutex;
    /// Map connecting geo ids to a set of vehicles with those specific geo ids
    std::unordered_map<GeoIds, std::unordered_set<ActorId>> road_to_vehicle_id_map;
    /// Map connecting vehicle id to it's geo ids
    std::unordered_map<ActorId, GeoIds> vehicle_id_to_road_map;

    void SetVehicleId(ActorId actor_id, GeoIds ids);

    void EraseVehicleId(ActorId actor_id, GeoIds ids);

    void SetRoadIds(ActorId actor_id, GeoIds ids);

    std::unordered_set<ActorId> GetVehicleIds(GeoIds ids) const;

    GeoIds GetRoadIds(ActorId vehicle_id) const;

  public:

    TrafficDistributor();
    ~TrafficDistributor();

    void UpdateVehicleRoadPosition(ActorId actor_id, GeoIds road_ids);

    /// Returns the shared pointer of SimpleWaypoint for Lane Change
    /// if Lane Change is required and possible, else returns nullptr
    std::shared_ptr<SimpleWaypoint> AssignLaneChange(
        Actor vehicle,
        std::shared_ptr<SimpleWaypoint> current_waypoint,
        GeoIds current_road_ids,
        std::shared_ptr<BufferList> buffer_list,
        std::unordered_map<ActorId, uint> &vehicle_id_to_index,
        std::vector<Actor> &actor_list,
        cc::DebugHelper &debug_helper);

  };

}
