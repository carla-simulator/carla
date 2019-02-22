// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/road/MapBuilder.h>
#include <carla/geom/Location.h>
#include <carla/geom/Math.h>
#include <carla/road/element/RoadInfoVisitor.h>

using namespace carla::road;
using namespace carla::road::element;
using namespace carla::geom;

TEST(road, add_geometry) {
  MapBuilder builder;
  RoadSegmentDefinition def(1);

  def.MakeGeometry<GeometryLine>(1.0, 2.0, 3.0, carla::geom::Location());
  def.MakeGeometry<GeometrySpiral>(1.0, 2.0, 3.0, carla::geom::Location(), 1.0, 2.0);
  def.MakeGeometry<GeometryArc>(1.0, 2.0, 3.0, carla::geom::Location(), 1.0);

  builder.AddRoadSegmentDefinition(def);
  builder.Build();
}

TEST(road, add_information) {
  MapBuilder builder;
  RoadSegmentDefinition def(0);

  def.MakeGeometry<GeometryLine>(0, 10, 0, carla::geom::Location());
  def.MakeInfo<element::RoadInfoVelocity>(0, 50);
  def.MakeInfo<element::RoadInfoVelocity>(2, 90);
  def.MakeInfo<element::RoadInfoVelocity>(3, 100);
  def.MakeInfo<element::RoadInfoVelocity>(5, 90);
  def.MakeInfo<element::RoadInfoLane>();

  builder.AddRoadSegmentDefinition(def);
  auto map_ptr = builder.Build();
  Map &m = *map_ptr;

  auto r = m.GetData().GetRoad(0)->GetInfo<RoadInfoVelocity>(0.0);
  ASSERT_EQ(r->velocity, 50.0);
  r = m.GetData().GetRoad(0)->GetInfo<RoadInfoVelocity>(2);
  ASSERT_EQ(r->velocity, 90.0);
  r = m.GetData().GetRoad(0)->GetInfo<RoadInfoVelocity>(3);
  ASSERT_EQ(r->velocity, 100.0);
  r = m.GetData().GetRoad(0)->GetInfo<RoadInfoVelocity>(3.5);
  ASSERT_EQ(r->velocity, 100.0);
  r = m.GetData().GetRoad(0)->GetInfo<RoadInfoVelocity>(6);
  ASSERT_EQ(r->velocity, 90.0);
  r = m.GetData().GetRoad(0)->GetInfoReverse<RoadInfoVelocity>(4);
  ASSERT_EQ(r->velocity, 90.0);
  r = m.GetData().GetRoad(0)->GetInfoReverse<RoadInfoVelocity>(2.5);
  ASSERT_EQ(r->velocity, 100.0);
  r = m.GetData().GetRoad(0)->GetInfoReverse<RoadInfoVelocity>(2);
  ASSERT_EQ(r->velocity, 90.0);
}

TEST(road, set_and_get_connections_for) {
  MapBuilder builder;
  for (int i = 0; i < 10; ++i) {
    RoadSegmentDefinition def(i);
    for (int j = 0; j < 10; ++j) {
      def.AddPredecessorID(j);
    }
    for (int j = 0; j < 10; ++j) {
      def.AddSuccessorID(j);
    }
    builder.AddRoadSegmentDefinition(def);
  }
  auto map_ptr = builder.Build();
  Map &m = *map_ptr;
  for (auto &&r : m.GetData().GetAllIds()) {
    for (size_t i = 0; i < 10; ++i) {
      ASSERT_EQ(m.GetData().GetRoad(r)->GetPredecessorsIds().at(i), i);
    }
  }
}

void AssertNear(
    const element::DirectedPoint &d0,
    const element::DirectedPoint &d1) {
  constexpr double error = .01;
  ASSERT_NEAR(d0.location.x, d1.location.x, error);
  ASSERT_NEAR(d0.location.y, d1.location.y, error);
  ASSERT_NEAR(d0.location.z, d1.location.z, error);
  ASSERT_NEAR(d0.tangent, d1.tangent, error);
}

TEST(road, geom_line) {
  MapBuilder builder;
  RoadSegmentDefinition def1(1);

  // Line params:
  // - start_offset [double]
  // - length       [double]
  // - heading      [double]
  // - &start_pos   [const geom::Location]
  def1.MakeGeometry<GeometryLine>(0, 10, 0, Location(0, 0, 0));
  def1.MakeGeometry<GeometryLine>(10, 5, Math::pi_half(), Location(10, 0, 0));
  def1.MakeGeometry<GeometryLine>(15, 5, 0, Location(10, 5, 0));

  builder.AddRoadSegmentDefinition(def1);

  auto map_ptr = builder.Build();
  Map &m = *map_ptr;

  ASSERT_EQ(m.GetData().GetRoad(1)->GetLength(), 20.0);
  AssertNear(m.GetData().GetRoad(1)->GetDirectedPointIn(0),
      element::DirectedPoint(0, 0, 0, 0));
  AssertNear(m.GetData().GetRoad(1)->GetDirectedPointIn(-1.0),
      element::DirectedPoint(0, 0, 0, 0));
  AssertNear(m.GetData().GetRoad(1)->GetDirectedPointIn(1.0),
      element::DirectedPoint(1.0, 0, 0, 0));
  AssertNear(m.GetData().GetRoad(1)->GetDirectedPointIn(3.0),
      element::DirectedPoint(3.0, 0, 0, 0));
  AssertNear(m.GetData().GetRoad(1)->GetDirectedPointIn(10.0),
      element::DirectedPoint(10.0, 0, 0, 0));
  AssertNear(m.GetData().GetRoad(1)->GetDirectedPointIn(11.0),
      element::DirectedPoint(10.0, 1.0, 0, Math::pi_half()));
  AssertNear(m.GetData().GetRoad(1)->GetDirectedPointIn(15.0),
      element::DirectedPoint(10.0, 5.0, 0, Math::pi_half()));
  AssertNear(m.GetData().GetRoad(1)->GetDirectedPointIn(17.0),
      element::DirectedPoint(12.0, 5.0, 0, 0));
  AssertNear(m.GetData().GetRoad(1)->GetDirectedPointIn(20.0),
      element::DirectedPoint(15.0, 5.0, 0, 0));
  AssertNear(m.GetData().GetRoad(1)->GetDirectedPointIn(22.0),
      element::DirectedPoint(15.0, 5.0, 0, 0));
}

TEST(road, geom_arc) {
  MapBuilder builder;
  RoadSegmentDefinition def1(1);

  // Arc params:
  // - start_offset [double]
  // - length       [double]
  // - heading      [double]
  // - &start_pos   [const geom::Location]
  // - curvature    [double]
  def1.MakeGeometry<GeometryArc>(0, 10, 0, Location(0, 0, 0), -0.5);

  builder.AddRoadSegmentDefinition(def1);

  auto map_ptr = builder.Build();
  Map &m = *map_ptr;

  ASSERT_EQ(m.GetData().GetRoad(1)->GetLength(), 10.0);
}

TEST(road, geom_spiral) {
  MapBuilder builder;
  RoadSegmentDefinition def1(1);

  // Arc params:
  // - start_offset    [double]
  // - length          [double]
  // - heading         [double]
  // - &start_pos      [const geom::Location]
  // - curvature_start [double]
  // - curvature_end   [double]
  def1.MakeGeometry<GeometrySpiral>(0, 10, 0, Location(0, 0, 0), 0, -0.5);

  builder.AddRoadSegmentDefinition(def1);

  auto map_ptr = builder.Build();
  Map &m = *map_ptr;

  ASSERT_EQ(m.GetData().GetRoad(1)->GetLength(), 10.0);
  // not implemented yet
  // debug:
  /*const int max = 50;
  for (int i = 0; i < max; ++i) {
    DirectedPoint dp = m.GetData().GetRoad(1)->GetDirectedPointIn((float)i * (10.0f / (float)max));
    printf("(%f,%f)", dp.location.x, dp.location.y);
    if (i != max-1) printf(",");
    else printf("\n");
  }*/
}

TEST(road, get_information) {
  MapBuilder builder;
  RoadSegmentDefinition def(0);

  def.MakeGeometry<GeometryLine>(0, 10, 0, carla::geom::Location());
  def.MakeInfo<element::RoadInfoVelocity>(0, 50);
  def.MakeInfo<element::RoadInfoVelocity>(2, 90);
  def.MakeInfo<element::RoadInfoVelocity>(3, 100);
  def.MakeInfo<element::RoadInfoVelocity>(5, 90);
  def.MakeInfo<element::RoadInfoLane>();

  builder.AddRoadSegmentDefinition(def);

  auto map_ptr = builder.Build();
  Map &m = *map_ptr;

  const auto r = m.GetData().GetRoad(0)->GetInfo<RoadInfoVelocity>(0.0);
  (void)r;
}
