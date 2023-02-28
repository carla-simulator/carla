// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/geom/Vector3D.h>
#include <carla/geom/Math.h>
#include <carla/geom/BoundingBox.h>
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


TEST(geom, single_point_transform_inverse_transform_coherence) {
  constexpr double error = 0.001;

  const Location point(-3.14f, 1.337f, 4.20f);
  const Location translation (1.41f, -4.7f, 9.2f);
  const Rotation rotation (-47.0f, 37.0f, 250.2f);
  const Transform transform (translation, rotation);

  auto transformed_point = point;
  transform.TransformPoint(transformed_point);

  auto point_back_to_normal = transformed_point;
  transform.InverseTransformPoint(point_back_to_normal);

  ASSERT_NEAR(point.x, point_back_to_normal.x, error) << "result.x is " << point_back_to_normal.x << " but expected " << point.x;
  ASSERT_NEAR(point.y, point_back_to_normal.y, error) << "result.y is " << point_back_to_normal.y << " but expected " << point.y;
  ASSERT_NEAR(point.z, point_back_to_normal.z, error) << "result.z is " << point_back_to_normal.z << " but expected " << point.z;
}


TEST(geom, bbox_get_local_vertices_get_world_vertices_coherence) {
  constexpr double error = 0.001;

  const BoundingBox bbox (Location(10.2f, -32.4f, 15.6f), Vector3D(9.2f, 13.5f, 20.3f));

  const Location bbox_location(-3.14f, 1.337f, 4.20f);
  const Rotation bbox_rotation (-59.0f, 17.0f, -650.2f);
  const Transform bbox_transform(bbox_location, bbox_rotation);

  const auto local_vertices = bbox.GetLocalVertices();
  const auto world_vertices = bbox.GetWorldVertices(bbox_transform);
  for (auto i = 0u; i < local_vertices.size(); ++i){
      const auto &local_vertex = local_vertices[i];

      auto transformed_local_vertex = local_vertex;
      bbox_transform.TransformPoint(transformed_local_vertex);

      const auto &world_vertex = world_vertices[i];

      ASSERT_NEAR(transformed_local_vertex.x, world_vertex.x, error) << "result.x is " << transformed_local_vertex.x << " but expected " << world_vertex.x;
      ASSERT_NEAR(transformed_local_vertex.y, world_vertex.y, error) << "result.y is " << transformed_local_vertex.y << " but expected " << world_vertex.y;
      ASSERT_NEAR(transformed_local_vertex.z, world_vertex.z, error) << "result.z is " << transformed_local_vertex.z << " but expected " << world_vertex.z;
  }
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
      const double dist = Math::DistanceSegmentToPoint(
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

TEST(geom, nearest_point_arc) {
  ASSERT_NEAR(Math::DistanceArcToPoint(Vector3D(1,0,0),
      Vector3D(0,0,0), 1.57f, 0, 1).second, 0.414214f, 0.01f);
  ASSERT_NEAR(Math::DistanceArcToPoint(Vector3D(2,1,0),
      Vector3D(0,0,0), 1.57f, 0, 1).second, 1.0f, 0.01f);
  ASSERT_NEAR(Math::DistanceArcToPoint(Vector3D(0,1,0),
      Vector3D(0,0,0), 1.57f, 0, 1).second, 1.0f, 0.01f);
  ASSERT_NEAR(Math::DistanceArcToPoint(Vector3D(1,2,0),
      Vector3D(0,0,0), 1.57f, 0, 1).second, 1.0f, 0.01f);
}
