#pragma once

#include <map>
#include <cmath>
#include <memory>
#include <string>

#include "carla/Memory.h"
#include "carla/client/Waypoint.h"
#include "carla/geom/Location.h"

#include "SimpleWaypoint.h"

namespace traffic_manager {

  typedef std::vector<
      std::pair<
      carla::SharedPtr<carla::client::Waypoint>,
      carla::SharedPtr<carla::client::Waypoint>
      >
      > TopologyList;
  typedef std::vector<std::shared_ptr<SimpleWaypoint>> NodeList;
  
  class InMemoryMap {

    /// This class constructs a descretised local map cache.
    /// Instantiate the class with map toplogy from the simulator
    /// and run setUp() to construct the local map.

  private:

    TopologyList topology;
    std::vector<std::shared_ptr<SimpleWaypoint>> dense_topology;
    NodeList entry_node_list;
    NodeList exit_node_list;
    
    std::map <
      uint32_t,
      std::map<
        uint32_t,
        std::map<
          int32_t,
          std::vector<std::shared_ptr<SimpleWaypoint>>
        >
      >
    > waypoint_structure;

  public:

    InMemoryMap(TopologyList topology);
    ~InMemoryMap();
    
    /// Constructs the local map with a resolution of sampling_resolution.
    void setUp(int sampling_resolution);

    /// Returns the closest waypoint to a given location on the map.
    std::shared_ptr<SimpleWaypoint> getWaypoint(const carla::geom::Location &location) const;

    /// Returns the full list of descrete samples of the map in local cache.
    std::vector<std::shared_ptr<SimpleWaypoint>> get_dense_topology() const;

    void structuredWaypoints(std::shared_ptr<SimpleWaypoint> waypoint);
    void LinkLaneChangePoint(
      std::shared_ptr<SimpleWaypoint> reference_waypoint,
      carla::SharedPtr<carla::client::Waypoint> neighbor_waypoint
    );
    void FindAndLinkLaneChange (std::shared_ptr<SimpleWaypoint> reference_waypoint);
  };

}