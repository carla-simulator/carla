// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/geom/Vector3D.h>
#include <carla/geom/Math.h>
#include <carla/geom/Transform.h>
#include <limits>

namespace carla {
namespace geom {

  std::ostream &operator<<(std::ostream &out, const Vector3D &vector3D) {
    out << "{x=" << vector3D.x << ", y=" << vector3D.y << ", z=" << vector3D.z << '}';
    return out;
  }

} // namespace geom
} // namespace carla

using namespace carla::geom;

TEST(geom, single_point_no_transform) {
  constexpr double error = 0.001;

  Location translation (0.0, 0.0, 0.0);
  Rotation rotation(0.0, 0.0, 0.0);
  Transform transform (translation, rotation);

  Location point (1.0,1.0,1.0);
  transform.TransformPoint(point);
  Location result_point(1.0, 1.0, 1.0);

  ASSERT_NEAR(point.x, result_point.x, error);
  ASSERT_NEAR(point.y, result_point.y, error);
  ASSERT_NEAR(point.z, result_point.z, error);

}

TEST(geom, single_point_translation) {
  constexpr double error = 0.001;

  Location translation (2.0,5.0,7.0);
  Rotation rotation (0.0, 0.0, 0.0);
  Transform transform (translation, rotation);

  Location point (0.0, 0.0, 0.0);
  transform.TransformPoint(point);
  Location result_point(2.0, 5.0, 7.0);

  ASSERT_NEAR(point.x, result_point.x, error);
  ASSERT_NEAR(point.y, result_point.y, error);
  ASSERT_NEAR(point.z, result_point.z, error);
}

TEST(geom, single_point_rotation) {
  constexpr double error = 0.001;

  Location translation (0.0,0.0,0.0);
  Rotation rotation (0.0,180.0,0.0); // y z x
  Transform transform (translation, rotation);

  Location point (0.0, 0.0, 1.0);
  transform.TransformPoint(point);
  Location result_point(0.0, 0.0, 1.0);
  ASSERT_NEAR(point.x, result_point.x, error);
  ASSERT_NEAR(point.y, result_point.y, error);
  ASSERT_NEAR(point.z, result_point.z, error);
}

TEST(geom, single_point_translation_and_rotation) {
  constexpr double error = 0.001;

  Location translation (0.0,0.0,-1.0); // x y z
  Rotation rotation (90.0,0.0,0.0); // y z x
  Transform transform (translation, rotation);

  Location point (0.0, 0.0, 2.0);
  transform.TransformPoint(point);
  Location result_point(-2.0, 0.0, -1.0);
  ASSERT_NEAR(point.x, result_point.x, error);
  ASSERT_NEAR(point.y, result_point.y, error);
  ASSERT_NEAR(point.z, result_point.z, error);
}

TEST(geom, distance) {
  constexpr double error = .01;
  ASSERT_NEAR(Math::Distance({0, 0, 0}, {0, 0, 0}), 0.0, error);
  ASSERT_NEAR(Math::Distance({1, 1, 1}, {0, 0, 0}), 1.732051, error);
  ASSERT_NEAR(Math::Distance({0, 0, 0}, {1, 1, 1}), 1.732051, error);
  ASSERT_NEAR(Math::Distance({-1, -1, -1}, {0, 0, 0}), 1.732051, error);
  ASSERT_NEAR(Math::Distance({0, 0, 0}, {-1, -1, -1}), 1.732051, error);
  ASSERT_NEAR(Math::Distance({7, 4, 3}, {17, 6, 2}), 10.246951, error);
  ASSERT_NEAR(Math::Distance({7, -4, 3}, {-17, 6, 2}), 26.019224, error);
  ASSERT_NEAR(Math::Distance({5, 6, 7}, {-6, 3, -4}), 15.84298, error);
  ASSERT_NEAR(Math::Distance({7, 4, 3}, {17, 6, 2}), 10.246951, error);
}

TEST(geom, nearest_point_segment) {
  const float segment[] = {
     0,  0, 10,  0,
     2,  5, 10,  8,
    -6,  8,  8, -2,
     8,  2,-10,  3,
     3,  3, -6, -5,
     3, -3,  2,  5,
     4, -6,  5,  4,
    -1, -4,-10,  8,
    -7, -5,  5,  5,
    -5,  6,  3, -9
  };

  const Vector3D point[] = {
    {  1,  -1, 0},
    { 10,  10, 0},
    {-10,  10, 0},
    { 10, -10, 0},
    {-10, -10, 0},
    {  0,   5, 0},
    {  0,  -5, 0},
    {  1,   4, 0},
    { -1,   1, 0},
    {  3, 2.5, 0}
  };

  const int results[] = {
    0, 1, 7, 9, 8, 2, 9, 2, 8, 3
  };

  for (int i = 0; i < 10; ++i) {
    double min_dist = std::numeric_limits<double>::max();
    int id = -1;
    for (int j = 0; j < 40; j += 4) {
      const double dist = Math::DistSegmentPoint(
          point[i],
          {segment[j + 0], segment[j + 1], 0},
          {segment[j + 2], segment[j + 3], 0}).second;
      if (dist < min_dist) {
        min_dist = dist;
        id = j / 4;
      }
    }
    ASSERT_EQ(id, results[i]) << "Fails point number: " << i;
  }
}

TEST(geom, forward_vector) {
  auto compare = [](Rotation rotation, Vector3D expected) {
    constexpr float eps = 2.0f * std::numeric_limits<float>::epsilon();
    auto result = rotation.GetForwardVector();
    EXPECT_TRUE(
            (std::abs(expected.x - result.x) < eps) &&
            (std::abs(expected.y - result.y) < eps) &&
            (std::abs(expected.z - result.z) < eps))
        << "result   = " << result << '\n'
        << "expected = " << expected;
  };
  //        pitch     yaw    roll       x     y     z
  compare({  0.0f,   0.0f,   0.0f}, {1.0f, 0.0f, 0.0f});
  compare({  0.0f,   0.0f, 123.0f}, {1.0f, 0.0f, 0.0f});
  compare({360.0f, 360.0f,   0.0f}, {1.0f, 0.0f, 0.0f});
  compare({  0.0f,  90.0f,   0.0f}, {0.0f, 1.0f, 0.0f});
  compare({  0.0f, -90.0f,   0.0f}, {0.0f,-1.0f, 0.0f});
  compare({ 90.0f,   0.0f,   0.0f}, {0.0f, 0.0f, 1.0f});
  compare({180.0f, -90.0f,   0.0f}, {0.0f, 1.0f, 0.0f});
}

TEST(geom, point_in_rectangle) {
  ASSERT_TRUE(Math::PointInRectangle(
      Vector3D(0, 0, 0), Vector3D(1, 1, 0), 0, Vector3D(0, 0, 0)));
  ASSERT_TRUE(Math::PointInRectangle(
      Vector3D(0, 0, 0), Vector3D(1, 1, 0), 0, Vector3D(1, 1, 0)));
  ASSERT_TRUE(Math::PointInRectangle(
      Vector3D(0, 0, 0), Vector3D(1, 1, 0), 0, Vector3D(-1, 1, 0)));
  ASSERT_TRUE(Math::PointInRectangle(
      Vector3D(0, 0, 0), Vector3D(1, 1, 0), 0, Vector3D(1, -1, 0)));
  ASSERT_TRUE(Math::PointInRectangle(
      Vector3D(0, 0, 0), Vector3D(1, 1, 0), 0, Vector3D(-1, -1, 0)));
  ASSERT_FALSE(Math::PointInRectangle(
      Vector3D(0, 0, 0), Vector3D(1, 1, 0), 0, Vector3D(-1.01, -1.01, 0)));
  ASSERT_FALSE(Math::PointInRectangle(
      Vector3D(0, 0, 0), Vector3D(1, 1, 0), 0, Vector3D(1.01, 1.01, 0)));
  ASSERT_FALSE(Math::PointInRectangle(
      Vector3D(1.5, 1.5, 0), Vector3D(1, 1, 0), 0, Vector3D(0, 0, 0)));
  ASSERT_TRUE(Math::PointInRectangle(
      Vector3D(1.5, 1.5, 0), Vector3D(1, 1, 0), 0, Vector3D(1, 1, 0)));
  ASSERT_TRUE(Math::PointInRectangle(
      Vector3D(1.5, 1.5, 0), Vector3D(1, 1, 0), 0, Vector3D(2, 1, 0)));
  ASSERT_TRUE(Math::PointInRectangle(
      Vector3D(1.5, 1.5, 0), Vector3D(1, 1, 0), 0, Vector3D(2, 1, 0)));
  ASSERT_TRUE(Math::PointInRectangle(
      Vector3D(1.5, 1.5, 0), Vector3D(1, 1, 0), 0, Vector3D(2, 2, 0)));
  ASSERT_TRUE(Math::PointInRectangle(
      Vector3D(1.5, 1.5, 0), Vector3D(1, 1, 0), 0, Vector3D(1, 1, 0)));
  ASSERT_FALSE(Math::PointInRectangle(
      Vector3D(0, 0, 0), Vector3D(1, 1, 0), Math::pi_half() * 0.5, Vector3D(1, 1, 0)));
  ASSERT_TRUE(Math::PointInRectangle(
      Vector3D(0, 0, 0), Vector3D(1, 1, 0), Math::pi_half() * 0.5, Vector3D(1, 0, 0)));
  ASSERT_TRUE(Math::PointInRectangle(
      Vector3D(0, 2, 0), Vector3D(0.5, 2, 0), Math::pi_half(), Vector3D(2, 2, 0)));
  ASSERT_FALSE(Math::PointInRectangle(
      Vector3D(0, 2, 0), Vector3D(0.5, 2, 0), Math::pi_half(), Vector3D(2.1, 2, 0)));
  ASSERT_FALSE(Math::PointInRectangle(
      Vector3D(0, 2, 0), Vector3D(0.5, 2, 0), Math::pi_half(), Vector3D(2, 2.6, 0)));
}

TEST(geom, nearest_point_arc) {
  ASSERT_NEAR(Math::DistArcPoint(Vector3D(1,0,0),
      Vector3D(0,0,0), 1.57, 0, 1).second, 0.414214, 0.01);
  ASSERT_NEAR(Math::DistArcPoint(Vector3D(2,-1,0),
      Vector3D(0,0,0), 1.57, 0, 1).second, 1.0, 0.01);
  ASSERT_NEAR(Math::DistArcPoint(Vector3D(0,-1,0),
      Vector3D(0,0,0), 1.57, 0, 1).second, 1.0, 0.01);
  ASSERT_NEAR(Math::DistArcPoint(Vector3D(1,-2,0),
      Vector3D(0,0,0), 1.57, 0, 1).second, 1.0, 0.01);
}
