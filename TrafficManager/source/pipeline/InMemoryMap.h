#pragma once

#include <unordered_map>
#include <cmath>
#include <memory>
#include <string>
#include <limits>

#include "carla/Memory.h"
#include "carla/client/Waypoint.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"

#include "SimpleWaypoint.h"

namespace traffic_manager {

  typedef std::vector<
      std::pair<
      carla::SharedPtr<carla::client::Waypoint>,
      carla::SharedPtr<carla::client::Waypoint>
      >
      > TopologyList;
  typedef std::vector<std::shared_ptr<SimpleWaypoint>> NodeList;
  typedef std::unordered_map<int, std::vector<std::shared_ptr<SimpleWaypoint>>> LaneWaypointMap;
  typedef std::unordered_map<uint, LaneWaypointMap> SectionWaypointMap;
  typedef std::unordered_map<uint, SectionWaypointMap> RoadWaypointMap;

  class InMemoryMap {

    /// This class constructs a descretised local map cache.
    /// Instantiate the class with map toplogy from the simulator
    /// and run setUp() to construct the local map.

  private:

    TopologyList topology;
    std::vector<std::shared_ptr<SimpleWaypoint>> dense_topology;
    RoadWaypointMap road_to_waypoint;

  public:

    InMemoryMap(TopologyList topology);
    ~InMemoryMap();

    /// Constructs the local map with a resolution of sampling_resolution.
    void SetUp(int sampling_resolution);

    /// Returns the closest waypoint to a given location on the map.
    std::shared_ptr<SimpleWaypoint> GetWaypoint(const carla::geom::Location &location) const;

    /// Returns the full list of descrete samples of the map in local cache.
    std::vector<std::shared_ptr<SimpleWaypoint>> GetDenseTopology() const;

    void StructuredWaypoints(std::shared_ptr<SimpleWaypoint> waypoint);

    void LinkLaneChangePoint(
        std::shared_ptr<SimpleWaypoint> reference_waypoint,
        carla::SharedPtr<carla::client::Waypoint> neighbor_waypoint,
        int side);

    void FindAndLinkLaneChange(std::shared_ptr<SimpleWaypoint> reference_waypoint);

  };

}
