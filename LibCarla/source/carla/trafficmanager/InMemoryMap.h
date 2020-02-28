// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

#include "carla/client/Map.h"
#include "carla/client/Waypoint.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/Memory.h"
#include "carla/road/Lane.h"
#include "carla/road/RoadTypes.h"

#include "carla/trafficmanager/SimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

namespace cg = carla::geom;
namespace cc = carla::client;
namespace crd = carla::road;

  using WaypointPtr = carla::SharedPtr<cc::Waypoint>;
  using TopologyList = std::vector<std::pair<WaypointPtr, WaypointPtr>>;
  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
  using NodeList = std::vector<SimpleWaypointPtr>;
  using RawNodeList = std::vector<WaypointPtr>;
  using GeoGridId = crd::JuncId;
  using WorldMap = carla::SharedPtr<cc::Map>;
  using WaypointGrid = std::unordered_map<std::string, std::unordered_set<SimpleWaypointPtr>>;

  using SegmentId = std::tuple<crd::RoadId, crd::LaneId, crd::SectionId>;
  using SegmentTopology = std::map<SegmentId, std::pair<std::vector<SegmentId>, std::vector<SegmentId>>>;
  using SegmentMap = std::map<SegmentId, std::vector<SimpleWaypointPtr>>;

  /// This class builds a discretized local map-cache.
  /// Instantiate the class with the world and run SetUp() to construct the
  /// local map.
  class InMemoryMap {

  private:

    /// Object to hold the world map received by the constructor.
    WorldMap _world_map;
    /// Structure to hold all custom waypoint objects after interpolation of
    /// sparse topology.
    NodeList dense_topology;
    /// Grid localization map for all waypoints in the system.
    WaypointGrid waypoint_grid;
    /// Larger localization map for all waypoints to be used for localizing pedestrians.
    WaypointGrid ped_waypoint_grid;
    /// Geodesic grid topology.
    std::unordered_map<GeoGridId, cg::Location> geodesic_grid_center;

  public:

    InMemoryMap(WorldMap world_map);
    ~InMemoryMap();

    /// This method constructs the local map with a resolution of
    /// sampling_resolution.
    void SetUp();

    /// Computes the segment id of a given waypoint.
    ///
    /// The Id takes into account OpenDrive's road Id, lane Id and Section Id.
    SegmentId GetSegmentId(const WaypointPtr &wp) const;
    SegmentId GetSegmentId(const SimpleWaypointPtr &swp) const;

    /// This method returns the closest waypoint to a given location on the map.
    SimpleWaypointPtr GetWaypoint(const cg::Location &location) const;

    /// This method returns closest waypoint in the vicinity of the given co-ordinates.
    SimpleWaypointPtr GetWaypointInVicinity(cg::Location location);
    SimpleWaypointPtr GetPedWaypoint(cg::Location location);

    /// This method returns the full list of discrete samples of the map in the
    /// local cache.
    NodeList GetDenseTopology() const;

    void MakeGeodesiGridCenters();
    cg::Location GetGeodesicGridCenter(GeoGridId ggid);

  private:

    /// Method to generate the grid ids for given co-ordinates.
    std::pair<int, int> MakeGridId(float x, float y, bool vehicle_or_pedestrian);

    /// Method to generate map key for waypoint_grid.
    std::string MakeGridKey(std::pair<int, int> gird_id);

    /// This method is used to find and place lane change links.
    void FindAndLinkLaneChange(SimpleWaypointPtr reference_waypoint);

    std::vector<SimpleWaypointPtr> GetSuccessors(const SegmentId segment_id,
    const SegmentTopology &segment_topology, const SegmentMap &segment_map);
    std::vector<SimpleWaypointPtr> GetPredecessors(const SegmentId segment_id,
    const SegmentTopology &segment_topology, const SegmentMap &segment_map);
  };

} // namespace traffic_manager
} // namespace carla
