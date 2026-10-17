// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Regression tests for carla-simulator/carla#9869
// OpenDRIVE parser ignores a link's elementType: a junction whose id is also
// a road id cannot be entered.
//
// RoadParser kept <link><successor>/<predecessor>'s elementId but discarded
// its elementType, so MapBuilder::GetLaneNext could only decide "junction or
// road" by asking whether a ROAD with that id exists. With the (spec-valid)
// id overlap junction k / road k, a link to junction k was resolved as a link
// to road k, the lane got no successor, and every approach into the junction
// became a dead end (waypoint.next() == []).
//
// Minimal network (valid OpenDRIVE for ANY junction id; ids are unique within
// a class):  road 1 -> junction j -> connecting road 2 -> road 3.
// Both files below differ ONLY in j.

#include "test.h"

#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/Map.h>
#include <carla/road/element/Waypoint.h>

#include <string>


using namespace carla::opendrive;
using namespace carla::road;
using namespace carla::road::element;

namespace {

std::string MakeNetwork(const int junction_id) {
  const char *lane_width =
      R"(<width sOffset="0" a="3.5" b="0" c="0" d="0"/>)";
  const int j = junction_id;
  return R"(<?xml version="1.0" encoding="UTF-8"?>
<OpenDRIVE>
  <header revMajor="1" revMinor="4">
    <geoReference>+proj=tmerc +lat_0=0 +lon_0=0</geoReference>
  </header>
  <road id="1" junction="-1" length="20">
    <link><successor elementType="junction" elementId=")" + std::to_string(j) + R"("/></link>
    <planView>
      <geometry s="0" x="0" y="0" hdg="0" length="20"><line/></geometry>
    </planView>
    <lanes>
      <laneSection s="0">
        <center><lane id="0" type="none"/></center>
        <right>
          <lane id="-1" type="driving">)" + lane_width + R"(</lane>
        </right>
      </laneSection>
    </lanes>
  </road>
  <road id="2" junction=")" + std::to_string(j) + R"(" length="10">
    <link>
      <predecessor elementType="road" elementId="1" contactPoint="end"/>
      <successor elementType="road" elementId="3" contactPoint="start"/>
    </link>
    <planView>
      <geometry s="0" x="20" y="0" hdg="0" length="10"><line/></geometry>
    </planView>
    <lanes>
      <laneSection s="0">
        <center><lane id="0" type="none"/></center>
        <right>
          <lane id="-1" type="driving">
            <link><predecessor id="-1"/><successor id="-1"/></link>
            )" + lane_width + R"(
          </lane>
        </right>
      </laneSection>
    </lanes>
  </road>
  <road id="3" junction="-1" length="20">
    <link><predecessor elementType="junction" elementId=")" + std::to_string(j) + R"("/></link>
    <planView>
      <geometry s="0" x="30" y="0" hdg="0" length="20"><line/></geometry>
    </planView>
    <lanes>
      <laneSection s="0">
        <center><lane id="0" type="none"/></center>
        <right>
          <lane id="-1" type="driving">)" + lane_width + R"(</lane>
        </right>
      </laneSection>
    </lanes>
  </road>
  <junction id=")" + std::to_string(j) + R"(" name="j">
    <connection id="0" incomingRoad="1" connectingRoad="2" contactPoint="start">
      <laneLink from="-1" to="-1"/>
    </connection>
  </junction>
</OpenDRIVE>
)";
}

} // namespace

// A junction-typed link whose elementId also names an existing road must still
// be resolved as a junction. Before the fix, road 1's lane -1 got no successor
// and waypoint.next() came back empty at the junction approach.
TEST(road, regression_9869_junction_link_with_colliding_road_id_is_entered) {
  // junction id 3 collides with road id 3 (both exist).
  auto map = OpenDriveParser::Load(MakeNetwork(3));
  ASSERT_TRUE(map.has_value());

  auto wp = map->GetWaypoint(RoadId(1u), LaneId(-1), 19.0f);
  ASSERT_TRUE(wp.has_value()) << "waypoint on road 1 lane -1 near its end";

  auto next = map->GetNext(*wp, 2.0);
  ASSERT_FALSE(next.empty())
      << "junction 3 approach is a dead end: the junction-typed link was "
         "resolved against the road table";

  EXPECT_EQ(next.front().road_id, RoadId(2u))
      << "road 1 must continue into connecting road 2 through junction 3";
}

// Same topology, junction id clear of every road id. This is the control
// case: it must keep working (and did before the fix).
TEST(road, regression_9869_junction_link_without_id_collision_is_entered) {
  auto map = OpenDriveParser::Load(MakeNetwork(100));
  ASSERT_TRUE(map.has_value());

  auto wp = map->GetWaypoint(RoadId(1u), LaneId(-1), 19.0f);
  ASSERT_TRUE(wp.has_value());

  auto next = map->GetNext(*wp, 2.0);
  ASSERT_FALSE(next.empty());
  EXPECT_EQ(next.front().road_id, RoadId(2u));
}

// A road-typed link whose elementId names an existing road must keep resolving
// as a road, also when a junction with a nearby id exists in the same file.
// Guards the elementType-aware resolution against over-triggering the
// junction branch.
TEST(road, regression_9869_road_typed_links_still_resolve_across_junction) {
  auto map = OpenDriveParser::Load(MakeNetwork(3));
  ASSERT_TRUE(map.has_value());

  // Connecting road 2, lane -1, near its end: successor is road 3.
  auto wp = map->GetWaypoint(RoadId(2u), LaneId(-1), 10.0f - 0.5f);
  ASSERT_TRUE(wp.has_value());

  auto next = map->GetNext(*wp, 2.0);
  ASSERT_FALSE(next.empty()) << "road 2 must continue into road 3";
  EXPECT_EQ(next.front().road_id, RoadId(3u));
}
