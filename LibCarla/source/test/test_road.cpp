// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/road/MapBuilder.h>

using namespace carla::road;

TEST(road, add_geometry) {
  MapBuilder builder;
  RoadSegmentDefinition def(1);

  def.MakeGeometry<GeometryLine>(1.0, 2.0, 3.0, carla::geom::Location());
  def.MakeGeometry<GeometrySpiral>(1.0, 2.0, 3.0, carla::geom::Location(), 1.0, 2.0);
  def.MakeGeometry<GeometryArc>(1.0, 2.0, 3.0, carla::geom::Location(), 1.0);

  builder.AddRoadSegmentDefinition(def);
}

TEST(road, add_information) {
  MapBuilder builder;
  RoadSegmentDefinition def(1);

  class A : public RoadInfo {};
  class B : public RoadInfo {};
  class C : public RoadInfo {};

  def.MakeInfo<A>();
  def.MakeInfo<B>();
  def.MakeInfo<C>();

  builder.AddRoadSegmentDefinition(def);
}