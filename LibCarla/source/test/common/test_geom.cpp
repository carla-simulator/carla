// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/geom/Vector3D.h>
#include <carla/geom/Math.h>
#include <carla/geom/BoundingBox.h>
#include <carla/geom/Transform.h>
#include <carla/geom/Quaternion.h>
#include <limits>

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

TEST(geom, quaternion_get_yaw) {
  constexpr double error = 0.001;

  {  Rotation rotation (0.0,90.0,0.0); // y z x
    Quaternion quat(rotation);
    auto const yaw = quat.YawDegree();

    ASSERT_NEAR(yaw, 90.f, error) << "quat: " << quat.x << " " << quat.y << " " << quat.z << " " << quat.w;
  }
  {
    Rotation rotation (11.0,33.0,55.0); // y z x
    Quaternion quat(rotation);
    auto const yaw = quat.YawDegree();

    ASSERT_NEAR(yaw, 33.f, error) << "quat: " << quat.x << " " << quat.y << " " << quat.z << " " << quat.w;
  }
}

TEST(geom, quaternion_inverse) {
  constexpr double error = 0.001;

  Rotation rotation (11.0,33.0,55.0); // y z x
  Quaternion quat(rotation);
  auto const inverse = quat.Inverse();
  auto const unit = quat * inverse;
  ASSERT_NEAR(unit.x, Quaternion().x, error)  << "unit: " << unit.x << " " << unit.y << " " << unit.z << " " << unit.w;
  ASSERT_NEAR(unit.y, Quaternion().y, error)  << "unit: " << unit.x << " " << unit.y << " " << unit.z << " " << unit.w;
  ASSERT_NEAR(unit.z, Quaternion().z, error)  << "unit: " << unit.x << " " << unit.y << " " << unit.z << " " << unit.w;
  ASSERT_NEAR(unit.w, Quaternion().w, error)  << "unit: " << unit.x << " " << unit.y << " " << unit.z << " " << unit.w;
}

Vector3D carla_0_9_15_RotatedVector(Rotation const &rotator, Vector3D const &in_point)  {
  // Rotates Rz(yaw) * Ry(pitch) * Rx(roll) = first x, then y, then z.
  const float cy = std::cos(Math::ToRadians(rotator.yaw));
  const float sy = std::sin(Math::ToRadians(rotator.yaw));
  const float cr = std::cos(Math::ToRadians(rotator.roll));
  const float sr = std::sin(Math::ToRadians(rotator.roll));
  const float cp = std::cos(Math::ToRadians(rotator.pitch));
  const float sp = std::sin(Math::ToRadians(rotator.pitch));

  Vector3D out_point;
  out_point.x =
    in_point.x * (cp * cy) +
    in_point.y * (cy * sp * sr - sy * cr) +
    in_point.z * (-cy * sp * cr - sy * sr);

  out_point.y =
    in_point.x * (cp * sy) +
    in_point.y * (sy * sp * sr + cy * cr) +
    in_point.z * (-sy * sp * cr + cy * sr);

  out_point.z =
    in_point.x * (sp) +
    in_point.y * (-cp * sr) +
    in_point.z * (cp * cr);

  return out_point;
}

TEST(geom, single_point_rotation_90) {
  auto compare = [](int line, Rotation const &rotator, Vector3D point, Vector3D const &result_point)->void {
    constexpr double error = 0.001;

    Vector3D const in_point = point;

    Location translation (0.0,0.0,0.0);
    Transform transform = Transform(translation, rotator);
    transform.TransformPoint(point);
       EXPECT_NEAR(point.x, result_point.x, error)
      << " LINE " << line << " x: \n"
      << " point: " << point.x << " " << point.y << " " << point.z;
    EXPECT_NEAR(point.y, result_point.y, error)
      << " LINE "<< line << " y: \n"
      << " point: " << point.x << " " << point.y << " " << point.z;
    EXPECT_NEAR(point.z, result_point.z, error)
      << " LINE "<< line << " z: \n"
      << " point: " << point.x << " " << point.y << " " << point.z;
    
    transform.InverseTransformPoint(point);
    EXPECT_NEAR(point.x, in_point.x, error)
      << " LINE " << line << " -x: \n"
      << " point: " << point.x << " " << point.y << " " << point.z;
    EXPECT_NEAR(point.y, in_point.y, error)
      << " LINE "<< line << " -y: \n"
      << " point: " << point.x << " " << point.y << " " << point.z;
    EXPECT_NEAR(point.z, in_point.z, error)
      << " LINE "<< line << " -z: \n"
      << " point: " << point.x << " " << point.y << " " << point.z;

    Quaternion quaternion(rotator);
    Vector3D rotated_vector = quaternion.RotatedVector(in_point);
    EXPECT_NEAR(rotated_vector.x, result_point.x, error) 
      << " LINE " << line << " x: \n"
      << " quat: " << quaternion.x << " " << quaternion.y << " " << quaternion.z << " " << quaternion.w << "\n"
      << " rotated_vector: " << rotated_vector.x << " " << rotated_vector.y << " " << rotated_vector.z;
    EXPECT_NEAR(rotated_vector.y, result_point.y, error)
      << " LINE "<< line << " y: \n"
      << " quat: " << quaternion.x << " " << quaternion.y << " " << quaternion.z << " " << quaternion.w << "\n"
      << " rotated_vector: " << rotated_vector.x << " " << rotated_vector.y << " " << rotated_vector.z;
    EXPECT_NEAR(rotated_vector.z, result_point.z, error)
      << " LINE "<< line << " z: \n"
      << " quat: " << quaternion.x << " " << quaternion.y << " " << quaternion.z << " " << quaternion.w << "\n"
      << " rotated_vector: " << rotated_vector.x << " " << rotated_vector.y << " " << rotated_vector.z;
    rotated_vector = quaternion.InverseRotatedVector(rotated_vector);
    EXPECT_NEAR(rotated_vector.x, in_point.x, error) 
      << " LINE " << line << " -x: \n"
      << " quat: " << quaternion.x << " " << quaternion.y << " " << quaternion.z << " " << quaternion.w << "\n"
      << " rotated_vector: " << rotated_vector.x << " " << rotated_vector.y << " " << rotated_vector.z;
    EXPECT_NEAR(rotated_vector.y, in_point.y, error)
      << " LINE "<< line << " -y: \n"
      << " quat: " << quaternion.x << " " << quaternion.y << " " << quaternion.z << " " << quaternion.w << "\n"
      << " rotated_vector: " << rotated_vector.x << " " << rotated_vector.y << " " << rotated_vector.z;
    EXPECT_NEAR(rotated_vector.z, in_point.z, error)
      << " LINE "<< line << " -z: \n"
      << " quat: " << quaternion.x << " " << quaternion.y << " " << quaternion.z << " " << quaternion.w << "\n"
      << " rotated_vector: " << rotated_vector.x << " " << rotated_vector.y << " " << rotated_vector.z;

    auto const carla_0_9_15_result = carla_0_9_15_RotatedVector(rotator, in_point);
    if ((std::fabs(carla_0_9_15_result.x-result_point.x) > error) ||
        (std::fabs(carla_0_9_15_result.y-result_point.y) > error) ||
        (std::fabs(carla_0_9_15_result.z-result_point.z) > error))  {
      std::cerr << "Information: Rotation carla 0.9.16 test Rotation(pitch=" << rotator.pitch << ", yaw=" << rotator.yaw << ", roll=" << rotator.roll << ")" << std::endl
                << " point: " << point.x << " " << point.y << " " << point.z << std::endl
                << " resulted in point: " << carla_0_9_15_result.x << " " << carla_0_9_15_result.y << " " << carla_0_9_15_result.z << std::endl
                << " but correct result is : " << result_point.x << " " << result_point.y << " " << result_point.z << std::endl;
    }
  };

  // test all 90° rotations of positive unit vectors on axis; Remember:
  // UE uses left-handed coordinate system!
  // Because nearly every writing on this is written in a form which let's room for interpretation
  // Even that one talks on clock-wise rotation: https://forums.unrealengine.com/t/ue4-coordinate-system-not-right-handed/80398/4, 
  // but it is not telling if you are watching into axis positive direction or negative direction; therefore "clockwise" can be interpreted in both ways.
  // Ok, the example given makes it definitely clear then, which is our test 7 below.
  // 
  // Therefore let's take the easiest way to explain: Your left hand!
  // Point thumb upwards (positive z-Axis direction),  index finger forwards (positive x-Axis direction), middle finger rightwards (positive y-Axis direction)
  // Positive rotation can be "visualized" with thumb of the left hand pointing into the respective positive direction of the rotation axis,
  // then the fingers when creating a fist are showing the positive rotation direction.
  // 
  // The same by the way, works for right-handed coordinate systems: just take the right hand instead, resulting in the y-axis beeing flipped and rotation direction switches!
  // 
  //            pitch(y) yaw(z)  roll(x)   px    py   pz     r_x    r_y    r_z
  // pitch hand index finger goes down
  compare( 1, {  90.0f,   0.0f,   0.0f}, {1.0f, 0.f, 0.f}, { 0.0f,  0.0f, -1.0f}); // x-axis downwards
  compare( 2, {  90.0f,   0.0f,   0.0f}, {0.0f, 1.f, 0.f}, { 0.0f,  1.0f,  0.0f}); // y-axis constant
  compare( 3, {  90.0f,   0.0f,   0.0f}, {0.0f, 0.f, 1.f}, { 1.0f,  0.0f,  0.0f}); // z-axis forwards
  // pitch hand index finger goes up
  compare( 4, { -90.0f,   0.0f,   0.0f}, {1.0f, 0.f, 0.f}, { 0.0f,  0.0f,  1.0f}); // x-axis upwards
  compare( 5, { -90.0f,   0.0f,   0.0f}, {0.0f, 1.f, 0.f}, { 0.0f,  1.0f,  0.0f}); // y-axis constant
  compare( 6, { -90.0f,   0.0f,   0.0f}, {0.0f, 0.f, 1.f}, {-1.0f,  0.0f,  0.0f}); // z-axis backwards

  // yaw hand index finger goes to the right
  compare( 7, { 0.0f,  90.0f,   0.0f}, {1.0f, 0.f, 0.f}, { 0.0f,  1.0f,  0.0f}); // x-axis rightwards
  compare( 8, { 0.0f,  90.0f,   0.0f}, {0.0f, 1.f, 0.f}, {-1.0f,  0.0f,  0.0f}); // y-axis backwards
  compare( 9, { 0.0f,  90.0f,   0.0f}, {0.0f, 0.f, 1.f}, { 0.0f,  0.0f,  1.0f}); // z-axis constant
  // yaw hand index finger goes to the left
  compare(10, { 0.0f, -90.0f,   0.0f}, {1.0f, 0.f, 0.f}, { 0.0f, -1.0f,  0.0f}); // x-axis leftwards
  compare(11, { 0.0f, -90.0f,   0.0f}, {0.0f, 1.f, 0.f}, { 1.0f,  0.0f,  0.0f}); // y-axis forwards
  compare(12, { 0.0f, -90.0f,   0.0f}, {0.0f, 0.f, 1.f}, { 0.0f,  0.0f,  1.0f}); // z-axis constant

  // roll hand: thumb points to the left
  compare(13, { 0.0f,   0.0f,  90.0f}, {1.0f, 0.f, 0.f}, { 1.0f,  0.0f,  0.0f}); // x-axis constant
  compare(14, { 0.0f,   0.0f,  90.0f}, {0.0f, 1.f, 0.f}, { 0.0f,  0.0f,  1.0f}); // y-axis upwards
  compare(15, { 0.0f,   0.0f,  90.0f}, {0.0f, 0.f, 1.f}, { 0.0f, -1.0f,  0.0f}); // z-axis leftwards
  // roll hand: thumb points to the right
  compare(16, { 0.0f,   0.0f, -90.0f}, {1.0f, 0.f, 0.f}, { 1.0f,  0.0f,  0.0f}); // x-axis constant
  compare(17, { 0.0f,   0.0f, -90.0f}, {0.0f, 1.f, 0.f}, { 0.0f,  0.0f, -1.0f}); // y-axis downwards
  compare(18, { 0.0f,   0.0f, -90.0f}, {0.0f, 0.f, 1.f}, { 0.0f,  1.0f,  0.0f}); // z-axis rightwards
}

TEST(geom, single_point_translation_and_rotation) {
  constexpr double error = 0.001;

  Location translation (0.0,0.0,-1.0); // x y z
  Rotation rotation (90.0,0.0,0.0); // y z x
  Transform transform (translation, rotation);

  Location point (0.0, 0.0, 2.0);
  transform.TransformPoint(point);
  Location result_point(2.0, 0.0, -1.0);   //!!! This line was wrong in CARLA version <= 0.9.16 due to invalid pitch AND roll rotations, (most relevant) yaw rotations were not affected

  ASSERT_NEAR(point.x, result_point.x, error) <<  point.x << " " << point.y << " " << point.z;
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
        << "expected = " << expected << '\n'
        << " rotation: " << rotation.roll << " " << rotation.pitch << " " << rotation.yaw;
  };
  //        pitch     yaw    roll       x     y     z
  compare({  0.0f,   0.0f,   0.0f}, {1.0f, 0.0f, 0.0f});
  compare({  0.0f,   0.0f, 123.0f}, {1.0f, 0.0f, 0.0f});
  compare({360.0f, 360.0f,   0.0f}, {1.0f, 0.0f, 0.0f});
  compare({  0.0f,  90.0f,   0.0f}, {0.0f, 1.0f, 0.0f});
  compare({  0.0f, -90.0f,   0.0f}, {0.0f,-1.0f, 0.0f});
  compare({ 90.0f,   0.0f,   0.0f}, {0.0f, 0.0f, -1.0f});    //!!! This line was wrong in CARLA version <= 0.9.16 due to invalid pitch AND roll rotations, (most relevant) yaw rotations were not affected
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
