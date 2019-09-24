#pragma once

#include <cmath>
#include <memory>
#include <limits>
#include <string>
#include <unordered_map>

#include "carla/client/Waypoint.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/Memory.h"

#include "SimpleWaypoint.h"

namespace traffic_manager {

namespace cg = carla::geom;

  using WaypointPtr = carla::SharedPtr<carla::client::Waypoint>;
  using TopologyList = std::vector<std::pair<WaypointPtr, WaypointPtr>>;
  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
  using NodeList = std::vector<SimpleWaypointPtr>;
  using LaneWaypointMap = std::unordered_map<int, NodeList>;
  using SectionWaypointMap = std::unordered_map<uint, LaneWaypointMap>;
  using RoadWaypointMap = std::unordered_map<uint, SectionWaypointMap>;

  /// This class constructs a discretised local map cache.
  /// Instantiate the class with map topology from the simulator
  /// and run setUp() to construct the local map.
  class InMemoryMap {

  private:

    /// Object to hold sparse topology received in the constructor
    TopologyList _topology;
    /// Structure to hold all custom waypoint objects after
    /// Interpolation of sparse topology
    NodeList dense_topology;
    /// Structure to segregate waypoints according to their geo ids
    RoadWaypointMap road_to_waypoint;

    /// The method used to segregate and place waypoints into RoadWaypointMap
    void StructuredWaypoints(SimpleWaypointPtr waypoint);

    /// the method used to place lane change link between waypoints
    void LinkLaneChangePoint(SimpleWaypointPtr reference_waypoint, WaypointPtr neighbor_waypoint, int side);

    /// The method used to find and place lane change links
    void FindAndLinkLaneChange(SimpleWaypointPtr reference_waypoint);

  public:

    InMemoryMap(TopologyList topology);
    ~InMemoryMap();

    /// Constructs the local map with a resolution of sampling_resolution.
    void SetUp(int sampling_resolution);

    /// Returns the closest waypoint to a given location on the map.
    SimpleWaypointPtr GetWaypoint(const cg::Location &location) const;

    /// Returns the full list of discrete samples of the map in the local cache.
    NodeList GetDenseTopology() const;

  };

}
