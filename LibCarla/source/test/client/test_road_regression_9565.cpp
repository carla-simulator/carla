// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Regression tests for carla-simulator/carla#9565
// Digital Twin Tool: crash on metropolitan OSM data, trees on roads,
// and one-way streets excluded from generation.

#include "test.h"

#include <carla/geom/Vector3D.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/Map.h>

#include <cmath>
#include <string>
#include <utility>
#include <vector>


using namespace carla::opendrive;
using namespace carla::geom;

constexpr float TREE_Y_TOLERANCE = 0.01f;

// --- minimal OpenDRIVE helpers -----------------------------------------------

static std::string MakeHeader() {
  return R"(<?xml version="1.0" encoding="UTF-8"?>
<OpenDRIVE>
  <header revMajor="1" revMinor="4" name="" version="1"
          north="100" south="-100" east="100" west="-100"/>
)";
}

// Straight road running along X axis from (x0,0) to (x0+length,0).
// lane_xml should be the full <lanes>...</lanes> block.
static std::string MakeRoad(
    int id, float x0, float length,
    const std::string& lane_xml,
    bool with_speed = true) {
  std::string speed_block = with_speed
      ? R"(<type s="0" type="town"><speed max="50" unit="km/h"/></type>)"
      : "";
  return
    "  <road name=\"R" + std::to_string(id) + "\" length=\"" + std::to_string(length) +
    "\" id=\"" + std::to_string(id) + "\" junction=\"-1\">\n" +
    "    " + speed_block + "\n" +
    "    <planView><geometry s=\"0\" x=\"" + std::to_string(x0) +
    "\" y=\"0\" hdg=\"0\" length=\"" + std::to_string(length) + "\"><line/></geometry></planView>\n" +
    "    <elevationProfile><elevation s=\"0\" a=\"0\" b=\"0\" c=\"0\" d=\"0\"/></elevationProfile>\n" +
    "    <lateralProfile/>\n" +
    lane_xml + "\n" +
    "  </road>\n";
}

// Bidirectional road: lane -1 (right driving) + lane +1 (left driving).
static std::string BidirectionalLanes() {
  return R"(
    <lanes>
      <laneSection s="0">
        <left>
          <lane id="1" type="driving" level="false">
            <width sOffset="0" a="3.5" b="0" c="0" d="0"/>
          </lane>
        </left>
        <center><lane id="0" type="none" level="false"/></center>
        <right>
          <lane id="-1" type="driving" level="false">
            <width sOffset="0" a="3.5" b="0" c="0" d="0"/>
          </lane>
        </right>
      </laneSection>
    </lanes>)";
}

// One-way road with ONLY positive lanes --no lane -1.
// This road was silently excluded by FilterRoadsByPosition before the fix.
static std::string PositiveOnlyLanes() {
  return R"(
    <lanes>
      <laneSection s="0">
        <left>
          <lane id="1" type="driving" level="false">
            <width sOffset="0" a="3.5" b="0" c="0" d="0"/>
          </lane>
          <lane id="2" type="driving" level="false">
            <width sOffset="0" a="3.5" b="0" c="0" d="0"/>
          </lane>
        </left>
        <center><lane id="0" type="none" level="false"/></center>
      </laneSection>
    </lanes>)";
}

// One-way road with ONLY negative lanes (normal right-side traffic).
static std::string NegativeOnlyLanes() {
  return R"(
    <lanes>
      <laneSection s="0">
        <center><lane id="0" type="none" level="false"/></center>
        <right>
          <lane id="-1" type="driving" level="false">
            <width sOffset="0" a="3.5" b="0" c="0" d="0"/>
          </lane>
        </right>
      </laneSection>
    </lanes>)";
}

// --- Bug 3: FilterRoadsByPosition includes one-way positive-lane roads -------
//
// Before fix: only queried lane -1; roads without it were silently excluded.
// After fix:  falls back to the driving lane with smallest abs(id).
// -----------------------------------------------------------------------------
TEST(road, filter_roads_includes_positive_only_lanes_9565) {
  // Three roads side-by-side along X axis, all centred around Y=0.
  //   Road 0 (x=0..50):  bidirectional   -- has lane -1, was always included
  //   Road 1 (x=60..110): positive-only  -- no lane -1, was EXCLUDED before fix
  //   Road 2 (x=120..170): negative-only -- has lane -1, was always included
  std::string xodr = MakeHeader()
      + MakeRoad(0, 0.f,   50.f, BidirectionalLanes())
      + MakeRoad(1, 60.f,  50.f, PositiveOnlyLanes())
      + MakeRoad(2, 120.f, 50.f, NegativeOnlyLanes())
      + "</OpenDRIVE>\n";

  auto map = OpenDriveParser::Load(xodr);
  ASSERT_TRUE(map.has_value());

  // Bounding box covering all three roads.
  // FilterRoadsByPosition checks minpos.y > y > maxpos.y (Y axis is inverted),
  // so minpos.y must be the larger (positive) value.
  const Vector3D minpos(-10.f,  20.f, -10.f);
  const Vector3D maxpos(200.f, -20.f,  10.f);

  auto included = map->GetMap().FilterRoadsByPosition(minpos, maxpos);

  // All three roads must be included after the fix.
  EXPECT_EQ(included.size(), 3u)
      << "FilterRoadsByPosition excluded a road with positive-only lanes";
}

// --- Bug 1 + Bug 2: GetTreesTransform robustness -----------------------------
//
// Bug 1: roadinfo==nullptr (no maxspeed tag) caused SIGSEGV.
// Bug 2: min_lane==0 / degenerate corners placed trees on the road surface.
// -----------------------------------------------------------------------------
TEST(road, get_trees_transform_no_crash_without_speed_9565) {
  // Road without any <speed> element --roadinfo will be nullptr.
  std::string xodr = MakeHeader()
      + MakeRoad(0, 0.f, 100.f, BidirectionalLanes(), /*with_speed=*/false)
      + "</OpenDRIVE>\n";

  auto map = OpenDriveParser::Load(xodr);
  ASSERT_TRUE(map.has_value());

  // FilterRoadsByPosition checks minpos.y > y > maxpos.y (Y axis inverted).
  const Vector3D minpos(-10.f,  20.f, -10.f);
  const Vector3D maxpos(110.f, -20.f,  10.f);

  // Must not crash (a regression to the null dereference will fail by crashing).
  std::vector<std::pair<carla::geom::Transform, std::string>> result =
      map->GetMap().GetTreesTransform(minpos, maxpos, 10.f, 2.f);

  // Must produce at least one tree so the fallback-type loop below is not vacuous.
  ASSERT_GT(result.size(), 0u)
      << "No trees generated for road without maxspeed tag";

  // Fallback type "Town" must be used for all entries.
  for (const auto& entry : result) {
    EXPECT_EQ(entry.second, "Town")
        << "Expected \"Town\" fallback for road without maxspeed tag";
  }
}

TEST(road, get_trees_transform_positive_lane_road_gets_trees_9565) {
  // One-way road with positive-only lanes: before the fix, min_lane stayed 0
  // and the lane section was skipped --no trees were generated at all.
  std::string xodr = MakeHeader()
      + MakeRoad(0, 0.f, 100.f, PositiveOnlyLanes())
      + "</OpenDRIVE>\n";

  auto map = OpenDriveParser::Load(xodr);
  ASSERT_TRUE(map.has_value());

  // FilterRoadsByPosition checks minpos.y > y > maxpos.y (Y axis inverted).
  const Vector3D minpos(-10.f,  20.f, -10.f);
  const Vector3D maxpos(110.f, -20.f,  10.f);

  auto result = map->GetMap().GetTreesTransform(minpos, maxpos, 10.f, 2.f);

  ASSERT_GT(result.size(), 0u)
      << "No trees generated for one-way road with positive-only lanes";

  // PositiveOnlyLanes: lane id=1 (3.5m wide) and lane id=2 (3.5m wide).
  // Outer edge of lane id=2 is at |Y| = 3.5 + 3.5 = 7.0m from road centre.
  // distancefromdrivinglineborder=2.0 => trees placed 2.0m beyond that edge.
  // Trees must satisfy |Y| >= 7.0 + 2.0 = 9.0.
  const float lane1_width      = 3.5f;
  const float lane2_width      = 3.5f;
  const float dist_from_edge   = 2.0f;
  const float outer_road_edge  = lane1_width + lane2_width;
  const float min_expected_dist = outer_road_edge + dist_from_edge;  // 9.0
  for (const auto& entry : result) {
    const float tree_y = std::abs(entry.first.location.y);
    EXPECT_NEAR(tree_y, min_expected_dist, TREE_Y_TOLERANCE)
        << "Tree Y=" << entry.first.location.y
        << " is not within ±" << TREE_Y_TOLERANCE << " of the expected minimum distance (|Y| = " << min_expected_dist << ")";
  }
}

TEST(road, get_trees_transform_trees_outside_road_9565) {
  // Verify trees are not placed inside the driving lane.
  // Road: single right lane (id=-1, width=3.5m) centred at Y=0.
  // Driving surface occupies Y in [-3.5, 0] (right side, post-Y-flip).
  // DistanceFromRoadEdge=2.0 => trees should be at Y < -3.5 (beyond outer edge).
  std::string xodr = MakeHeader()
      + MakeRoad(0, 0.f, 100.f, NegativeOnlyLanes())
      + "</OpenDRIVE>\n";

  auto map = OpenDriveParser::Load(xodr);
  ASSERT_TRUE(map.has_value());

  // FilterRoadsByPosition checks minpos.y > y > maxpos.y (Y axis inverted).
  const Vector3D minpos(-10.f,  20.f, -10.f);
  const Vector3D maxpos(110.f, -20.f,  10.f);

  auto result = map->GetMap().GetTreesTransform(minpos, maxpos, 10.f, 2.f);
  ASSERT_GT(result.size(), 0u) << "No trees generated for negative-only lane road";

  // Outer edge of lane -1 is at t=3.5m from road centre.
  // distancefromdrivinglineborder=2.0 => trees placed 2.0m beyond that edge.
  // After Y-flip, trees must be at |Y| >= 3.5 + 2.0 = 5.5.
  const float lane_outer_edge   = 3.5f;
  const float dist_from_edge    = 2.0f;
  const float min_expected_dist = lane_outer_edge + dist_from_edge;

  for (const auto& entry : result) {
    const float tree_y = std::abs(entry.first.location.y);
    EXPECT_NEAR(tree_y, min_expected_dist, TREE_Y_TOLERANCE)
        << "Tree Y=" << entry.first.location.y
        << " is not within ±" << TREE_Y_TOLERANCE << " of the expected minimum distance (|Y| = " << min_expected_dist << ")";
  }
}
