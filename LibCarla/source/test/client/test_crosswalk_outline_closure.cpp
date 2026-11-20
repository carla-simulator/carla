// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/Exception.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/Map.h>
#include <carla/geom/Mesh.h>
#include <gtest/gtest.h>

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

// The legacy direct outline form includes height on every local corner.
// File-level validation against an original OpenDRIVE 1.4 XSD is separate.
carla::road::Map MakeMap(const std::vector<bool> &closed, bool placed = false) {
  std::ostringstream xml;
  xml << R"(<?xml version="1.0" encoding="UTF-8"?>
<OpenDRIVE><header revMajor="1" revMinor="4" name="CrosswalkClosure" version="1.00"
 north="100" south="-100" east="100" west="-100"/>
<road name="StraightRoad" length="60" id="1" junction="-1"><link/>
<type s="0" type="town"><speed max="30" unit="km/h"/></type>
<planView><geometry s="0" x="10" y="-5" hdg=")"
      << (placed ? "0.3" : "0") << R"(" length="60"><line/></geometry></planView>
<elevationProfile><elevation s="0" a="0" b=")" << (placed ? "0.02" : "0")
      << R"(" c="0" d="0"/></elevationProfile><lateralProfile/>
<lanes><laneOffset s="0" a="0" b="0" c="0" d="0"/>
<laneSection s="0"><center><lane id="0" type="none" level="false">
<roadMark sOffset="0" type="none"/></lane></center>
<right><lane id="-1" type="driving" level="false"><link/>
<width sOffset="0" a="8" b="0" c="0" d="0"/>
<roadMark sOffset="0" type="none"/></lane></right></laneSection></lanes><objects>)";
  for (size_t i = 0; i < closed.size(); ++i) {
    xml << "<object id=\"" << i + 1 << "\" name=\"Crosswalk\" type=\"crosswalk\" s=\""
        << 15 + 25 * i << "\" t=\"-4\" zOffset=\"0\" hdg=\"" << (placed ? "0.4" : "0")
        << R"(" pitch="0" roll="0" orientation="+" width="8" length="12" height="0">
<outline><cornerLocal u="-6" v="-4" z="0" height="0"/>
<cornerLocal u="6" v="-4" z="0" height="0"/>
<cornerLocal u="6" v="4" z="0" height="0"/>
<cornerLocal u="-6" v="4" z="0" height="0"/>)";
    if (closed[i]) {
      xml << R"(<cornerLocal u="-6" v="-4" z="0" height="0"/>)";
    }
    xml << "</outline></object>";
  }
  xml << "</objects></road></OpenDRIVE>";
  auto map = carla::opendrive::OpenDriveParser::Load(xml.str());
  if (!map) {
    carla::throw_exception(std::runtime_error("ordinary crosswalk fixture did not parse"));
  }
  return std::move(*map);
}

TEST(CrosswalkOutlineClosure, FourDistinctCornersReturnClosedPolygon) {
  auto map = MakeMap({false});
  const auto points = map.GetAllCrosswalkZones();
  ASSERT_EQ(points.size(), 5u);
  EXPECT_EQ(points.front(), points.back());
  EXPECT_NE(points[0], points[1]);
  EXPECT_NE(points[0], points[3]);
}

TEST(CrosswalkOutlineClosure, FourDistinctCornersProduceNativeMesh) {
  auto map = MakeMap({false});
  const auto mesh = map.GetAllCrosswalkMesh();
  ASSERT_TRUE(mesh.IsValid());
  EXPECT_EQ(mesh.GetVerticesNum(), 4u);
  EXPECT_EQ(mesh.GetIndexesNum(), 6u);
}

TEST(CrosswalkOutlineClosure, RepeatedEndpointKeepsCoordinatesAndMesh) {
  auto map = MakeMap({true});
  const auto points = map.GetAllCrosswalkZones();
  const std::vector<carla::geom::Location> expected = {
      {18, 5, 0}, {32, 5, 0}, {32, 13, 0}, {18, 13, 0}, {18, 5, 0}};
  ASSERT_EQ(points.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i) EXPECT_EQ(points[i], expected[i]);
  const auto mesh = map.GetAllCrosswalkMesh();
  ASSERT_TRUE(mesh.IsValid());
  EXPECT_EQ(mesh.GetVerticesNum(), 4u);
  EXPECT_EQ(mesh.GetIndexesNum(), 6u);
}

TEST(CrosswalkOutlineClosure, EachOfTwoPolygonsIsClosedSeparately) {
  auto map = MakeMap({false, false});
  const auto points = map.GetAllCrosswalkZones();
  ASSERT_EQ(points.size(), 10u);
  EXPECT_EQ(points[0], points[4]);
  EXPECT_EQ(points[5], points[9]);
  EXPECT_NE(points[0], points[5]);
}

TEST(CrosswalkOutlineClosure, MultipleMeshesHaveNoBridgeTriangles) {
  auto map = MakeMap({false, false});
  const auto mesh = map.GetAllCrosswalkMesh();
  ASSERT_TRUE(mesh.IsValid());
  ASSERT_EQ(mesh.GetVerticesNum(), 8u);
  const auto &indices = mesh.GetIndexes();
  ASSERT_EQ(indices.size(), 12u);
  for (size_t i = 0; i < indices.size(); i += 3) {
    const bool first_polygon = indices[i] <= 4;
    for (size_t j = i; j < i + 3; ++j) {
      EXPECT_GE(indices[j], 1u);
      EXPECT_LE(indices[j], 8u);
      EXPECT_EQ(indices[j] <= 4, first_polygon);
    }
  }
}

TEST(CrosswalkOutlineClosure, PlacementMatchesAlreadyClosedNativeReference) {
  auto open = MakeMap({false}, true);
  auto closed = MakeMap({true}, true);
  const auto expected = closed.GetAllCrosswalkZones();
  const auto actual = open.GetAllCrosswalkZones();
  ASSERT_EQ(expected.size(), 5u);
  ASSERT_EQ(actual.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i) EXPECT_EQ(actual[i], expected[i]);
  EXPECT_NE(expected[0].z, expected[1].z);
  const auto actual_mesh = open.GetAllCrosswalkMesh();
  const auto expected_mesh = closed.GetAllCrosswalkMesh();
  ASSERT_EQ(actual_mesh.GetVerticesNum(), 4u);
  EXPECT_EQ(actual_mesh.GetVertices(), expected_mesh.GetVertices());
  EXPECT_EQ(actual_mesh.GetIndexes(), expected_mesh.GetIndexes());
}

} // namespace
