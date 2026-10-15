// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// The Traffic Manager's InMemoryMap must free its waypoint graph when it is
// destroyed. Its SimpleWaypoints link to each other (next, previous, left and
// right) through shared pointers, so the graph is freed only if the map breaks
// those cycles: before, every Traffic Manager shut down and every map rebuild
// leaked the whole graph.

#include "test.h"

#include <carla/Memory.h>
#include <carla/client/Map.h>
#include <carla/trafficmanager/InMemoryMap.h>
#include <carla/trafficmanager/SimpleWaypoint.h>

#include <memory>
#include <string>
#include <vector>

using carla::traffic_manager::InMemoryMap;
using carla::traffic_manager::SimpleWaypoint;

namespace {

// A straight 200 m road with two driving lanes in the same direction: its
// waypoints have next/previous links and left/right lane change links.
std::string TwoLaneRoad() {
  return R"(<?xml version="1.0" encoding="UTF-8"?>
<OpenDRIVE>
  <header revMajor="1" revMinor="4" name="" version="1"
          north="100" south="-100" east="300" west="-100"/>
  <road name="R0" length="200" id="0" junction="-1">
    <type s="0" type="town"><speed max="50" unit="km/h"/></type>
    <planView>
      <geometry s="0" x="0" y="0" hdg="0" length="200"><line/></geometry>
    </planView>
    <elevationProfile><elevation s="0" a="0" b="0" c="0" d="0"/></elevationProfile>
    <lateralProfile/>
    <lanes>
      <laneSection s="0">
        <center><lane id="0" type="none" level="false"/></center>
        <right>
          <lane id="-1" type="driving" level="false">
            <width sOffset="0" a="3.5" b="0" c="0" d="0"/>
          </lane>
          <lane id="-2" type="driving" level="false">
            <width sOffset="0" a="3.5" b="0" c="0" d="0"/>
          </lane>
        </right>
      </laneSection>
    </lanes>
  </road>
</OpenDRIVE>
)";
}

} // namespace

TEST(InMemoryMap, frees_its_waypoints_when_destroyed) {
  const auto world_map = carla::MakeShared<carla::client::Map>("two_lanes", TwoLaneRoad());

  std::vector<std::weak_ptr<SimpleWaypoint>> waypoints;
  bool linked_along = false;
  bool linked_across = false;
  {
    auto local_map = std::make_shared<InMemoryMap>(world_map);
    local_map->SetUp();
    for (const auto &waypoint : local_map->GetDenseTopology()) {
      linked_along = linked_along || !waypoint->GetNextWaypoint().empty();
      linked_across = linked_across ||
          waypoint->GetLeftWaypoint() != nullptr ||
          waypoint->GetRightWaypoint() != nullptr;
      waypoints.emplace_back(waypoint);
    }
  }

  // The graph had the links that make cycles...
  ASSERT_FALSE(waypoints.empty());
  ASSERT_TRUE(linked_along);
  ASSERT_TRUE(linked_across);
  // ... and none of its waypoints outlived the map.
  size_t alive = 0u;
  for (const auto &waypoint : waypoints) {
    alive += waypoint.expired() ? 0u : 1u;
  }
  EXPECT_EQ(alive, 0u) << alive << " of " << waypoints.size()
                       << " waypoints outlived their InMemoryMap";
}
