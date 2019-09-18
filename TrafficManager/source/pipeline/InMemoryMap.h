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

  using WaypointPtr = carla::SharedPtr<carla::client::Waypoint>;
  using TopologyList = std::vector<std::pair<WaypointPtr, WaypointPtr>>;
  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
  using NodeList = std::vector<SimpleWaypointPtr>;
  using LaneWaypointMap = std::unordered_map<int, std::vector<SimpleWaypointPtr>>;
  using SectionWaypointMap = std::unordered_map<uint, LaneWaypointMap>;
  using RoadWaypointMap = std::unordered_map<uint, SectionWaypointMap>;

  /// This class constructs a descretised local map cache.
  /// Instantiate the class with map toplogy from the simulator
  /// and run setUp() to construct the local map.
  class InMemoryMap {

  private:

    TopologyList _topology;
    std::vector<SimpleWaypointPtr> dense_topology;
    RoadWaypointMap road_to_waypoint;

  public:

    InMemoryMap(TopologyList topology);
    ~InMemoryMap();

    /// Constructs the local map with a resolution of sampling_resolution.
    void SetUp(int sampling_resolution);

    /// Returns the closest waypoint to a given location on the map.
    SimpleWaypointPtr GetWaypoint(const carla::geom::Location &location) const;

    /// Returns the full list of descrete samples of the map in local cache.
    std::vector<SimpleWaypointPtr> GetDenseTopology() const;

    void StructuredWaypoints(SimpleWaypointPtr waypoint);

    void LinkLaneChangePoint(SimpleWaypointPtr reference_waypoint, WaypointPtr neighbor_waypoint, int side);

    void FindAndLinkLaneChange(SimpleWaypointPtr reference_waypoint);

  };

}
