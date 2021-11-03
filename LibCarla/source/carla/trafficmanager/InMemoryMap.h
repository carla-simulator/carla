// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "boost/geometry.hpp"
#include "boost/geometry/geometries/point.hpp"
#include "boost/geometry/index/rtree.hpp"

#include "carla/client/Map.h"
#include "carla/client/Waypoint.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/Memory.h"
#include "carla/road/RoadTypes.h"

#include "carla/trafficmanager/RandomGenerator.h"
#include "carla/trafficmanager/SimpleWaypoint.h"
#include "carla/trafficmanager/CachedSimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

namespace cg = carla::geom;
namespace cc = carla::client;
namespace crd = carla::road;
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

  using WaypointPtr = carla::SharedPtr<cc::Waypoint>;
  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
  using NodeList = std::vector<SimpleWaypointPtr>;
  using GeoGridId = crd::JuncId;
  using WorldMap = carla::SharedPtr<const cc::Map>;

  using Point3D = bg::model::point<float, 3, bg::cs::cartesian>;
  using Box = bg::model::box<Point3D>;
  using SpatialTreeEntry = std::pair<Point3D, SimpleWaypointPtr>;

  using SegmentId = std::tuple<crd::RoadId, crd::LaneId, crd::SectionId>;
  using SegmentTopology = std::map<SegmentId, std::pair<std::vector<SegmentId>, std::vector<SegmentId>>>;
  using SegmentMap = std::map<SegmentId, std::vector<SimpleWaypointPtr>>;
  using Rtree = bgi::rtree<SpatialTreeEntry, bgi::rstar<16>>;

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
    /// Spatial quadratic R-tree for indexing and querying waypoints.
    Rtree rtree;

  public:

    InMemoryMap(WorldMap world_map);
    ~InMemoryMap();

    static void Cook(WorldMap world_map, const std::string& path);

    //bool Load(const std::string& filename);
    bool Load(const std::vector<uint8_t>& content);

    /// This method constructs the local map with a resolution of sampling_resolution.
    void SetUp();

    /// This method returns the closest waypoint to a given location on the map.
    SimpleWaypointPtr GetWaypoint(const cg::Location loc) const;

    /// This method returns n waypoints in an delta area with a certain distance from the ego vehicle.
    NodeList GetWaypointsInDelta(const cg::Location loc, const uint16_t n_points, const float random_sample) const;

    /// This method returns the full list of discrete samples of the map in the local cache.
    NodeList GetDenseTopology() const;

    std::string GetMapName();

    const cc::Map& GetMap() const;

  private:
    void Save(const std::string& path);

    void SetUpDenseTopology();
    void SetUpSpatialTree();
    void SetUpRoadOption();

    /// This method is used to find and place lane change links.
    void FindAndLinkLaneChange(SimpleWaypointPtr reference_waypoint);

    NodeList GetSuccessors(const SegmentId segment_id,
                          const SegmentTopology &segment_topology,
                          const SegmentMap &segment_map);
    NodeList GetPredecessors(const SegmentId segment_id,
                            const SegmentTopology &segment_topology,
                            const SegmentMap &segment_map);

    /// Computes the segment id of a given waypoint.
    /// The Id takes into account OpenDrive's road Id, lane Id and Section Id.
    SegmentId GetSegmentId(const WaypointPtr &wp) const;
    SegmentId GetSegmentId(const SimpleWaypointPtr &swp) const;
  };

} // namespace traffic_manager
} // namespace carla
