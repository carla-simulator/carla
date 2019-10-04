#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

#include "carla/client/Waypoint.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/Memory.h"

#include "SimpleWaypoint.h"

namespace traffic_manager {

namespace cg = carla::geom;
namespace cc = carla::client;

  using WaypointPtr = carla::SharedPtr<cc::Waypoint>;
  using TopologyList = std::vector<std::pair<WaypointPtr, WaypointPtr>>;
  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
  using NodeList = std::vector<SimpleWaypointPtr>;
  using LaneWaypointMap = std::unordered_map<int, NodeList>;
  using SectionWaypointMap = std::unordered_map<uint, LaneWaypointMap>;
  using RoadWaypointMap = std::unordered_map<uint, SectionWaypointMap>;

  /// This class builds a discretized local map-cache.
  /// Instantiate the class with map topology from the simulator
  /// and run SetUp() to construct the local map.
  class InMemoryMap {

  private:

    /// Object to hold sparse topology received by the constructor.
    TopologyList _topology;
    /// Structure to hold all custom waypoint objects after
    /// interpolation of sparse topology.
    NodeList dense_topology;
    /// Structure to segregate waypoints according to their geo ids.
    RoadWaypointMap road_to_waypoint;

    /// This method is used to segregate and place waypoints into RoadWaypointMap.
    void StructuredWaypoints(SimpleWaypointPtr waypoint);

    /// This method is used to place a lane change link between waypoints
    void LinkLaneChangePoint(SimpleWaypointPtr reference_waypoint, WaypointPtr neighbor_waypoint, int side);

    /// This method is used to find and place lane change links.
    void FindAndLinkLaneChange(SimpleWaypointPtr reference_waypoint);

  public:

    InMemoryMap(TopologyList topology);
    ~InMemoryMap();

    /// This method constructs the local map with a resolution of sampling_resolution.
    void SetUp(int sampling_resolution);

    /// This method returns the closest waypoint to a given location on the map.
    SimpleWaypointPtr GetWaypoint(const cg::Location &location) const;

    /// This method returns the full list of discrete samples of the map in the local cache.
    NodeList GetDenseTopology() const;

  };

}
