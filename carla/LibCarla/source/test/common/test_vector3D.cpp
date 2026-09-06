// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/geom/Vector3D.h>
#include <carla/geom/Math.h>

using namespace carla::geom;

TEST(vector3D, make_unit_vec) {
  ASSERT_EQ(Vector3D(10,0,0).MakeUnitVector(), Vector3D(1,0,0));
  ASSERT_NE(Vector3D(10,0,0).MakeUnitVector(), Vector3D(0,1,0));
  ASSERT_EQ(Vector3D(0,10,0).MakeUnitVector(), Vector3D(0,1,0));
  ASSERT_EQ(Vector3D(0,0,512).MakeUnitVector(), Vector3D(0,0,1));
  ASSERT_NE(Vector3D(0,1,512).MakeUnitVector(), Vector3D(0,0,1));

  // The following test is to check that the MakeUnitVector doesn't fail on zero-length vectors
  ASSERT_EQ(Vector3D().MakeUnitVector(), Vector3D(0,0,0));
}
