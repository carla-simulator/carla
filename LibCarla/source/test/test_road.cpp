// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/road/Map.h"
#include "carla/road/Types.h"
#include "carla/road/RoadSegment.h"
#include "carla/road/MapBuilder.h"

using namespace carla::road;

TEST(road, compilation_test) {
  Map m;
  RoadSegmentDefinition def(1);

  Geometry geom[3] = {
    GeometryLine(),
    GeometrySpiral(1.0, 2.0),
    GeometryArc(1.0)
  };

  def.AddGeometry(geom[0]);
  def.AddGeometry(geom[1]);
  def.AddGeometry(geom[2]);

  m.AddRoadSegment(def);
}
