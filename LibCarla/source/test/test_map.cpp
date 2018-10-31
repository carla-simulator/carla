// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/road/MapBuilder.h>
#include <carla/geom/Location.h>
#include <carla/opendrive/OpenDrive.h>
#include <iostream>

// #include <carla/geom/Math.h>

using namespace carla;
using namespace carla::road;
using namespace carla::road::element;
using namespace carla::geom;

void print_loc(const Location &l) {
  std::cout << "(" << l.x << ", " << l.y << ", " << l.z << ")" << std::endl;
}

TEST(map, main) {
  SharedPtr<Map> m = carla::opendrive::OpenDrive::Load("/home/marc/carla/Util/tests/test_02.xodr", XmlInputType::FILE);
  if (m == nullptr) {
    std::cout << "map is nullptr" << std::endl;
    return;
  }

  ASSERT_EQ(m->GetClosestWaypointOnRoad(Location(-200.0, -50.0, 0.0)).GetRoadId(), 0U);
  ASSERT_EQ(m->GetClosestWaypointOnRoad(Location(-150.0, 10.0, 0.0)).GetRoadId(), 0U);
  ASSERT_EQ(m->GetClosestWaypointOnRoad(Location(-150.0, -10.0, 0.0)).GetRoadId(), 0U);
  ASSERT_EQ(m->GetClosestWaypointOnRoad(Location(-50.0, 50.0, 0.0)).GetRoadId(), 5U);
  ASSERT_EQ(m->GetClosestWaypointOnRoad(Location(-95.0, -2.0, 0.0)).GetRoadId(), 3U);
  ASSERT_EQ(m->GetClosestWaypointOnRoad(Location(-104.0, -1.55, 0.0)).GetRoadId(), 0U);
  ASSERT_EQ(m->GetClosestWaypointOnRoad(Location(-80.0, 1.0, 0.0)).GetRoadId(), 2U);
  ASSERT_EQ(m->GetClosestWaypointOnRoad(Location(-89.0, 8.5, 0.0)).GetRoadId(), 2U);
  ASSERT_EQ(m->GetClosestWaypointOnRoad(Location(-89.0, 0.85, 0.0)).GetRoadId(), 2U);
  ASSERT_EQ(m->GetClosestWaypointOnRoad(Location(-89.0, -0.85, 0.0)).GetRoadId(), 2U);
  ASSERT_EQ(m->GetClosestWaypointOnRoad(Location(-89.0, -2.85, 0.0)).GetRoadId(), 2U);
}