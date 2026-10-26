// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/geom/BoundingBox.h>
#include <carla/geom/CubicPolynomial.h>
#include <carla/geom/GeoLocation.h>
#include <carla/geom/GeoProjection.h>
#include <carla/geom/Mesh.h>
#include <carla/geom/Quaternion.h>
#include <carla/geom/Vector3DInt.h>

#include <array>
#include <cmath>
#include <limits>
#include <new>

using namespace carla::geom;

TEST(geometry_regressions, scalar_vector_division) {
  const auto v2 = 8.0f / Vector2D(2.0f, 4.0f);
  EXPECT_FLOAT_EQ(v2.x, 4.0f);
  EXPECT_FLOAT_EQ(v2.y, 2.0f);
  const auto v3 = 8.0f / Vector3D(2.0f, 4.0f, 8.0f);
  EXPECT_EQ(v3, Vector3D(4.0f, 2.0f, 1.0f));
  const auto vi = 8 / Vector3DInt(2, 4, 8);
  EXPECT_EQ(vi, Vector3DInt(4, 2, 1));
  EXPECT_EQ(Vector3D(2.0f, 4.0f, 8.0f) / 2.0f, Vector3D(1.0f, 2.0f, 4.0f));
}

TEST(geometry_regressions, rotated_bounding_box_contains) {
  const BoundingBox box(Location(1.0f, 2.0f, 3.0f), Vector3D(2.0f, 1.0f, 0.5f), Rotation(0.0f, 90.0f, 0.0f));
  const Transform parent(Location(10.0f, -20.0f, 4.0f), Rotation(15.0f, 35.0f, -10.0f));
  auto inside = box.location + Location(0.0f, 1.5f, 0.0f);
  auto outside = box.location + Location(1.5f, 0.0f, 0.0f);
  parent.TransformPoint(inside);
  parent.TransformPoint(outside);
  EXPECT_TRUE(box.Contains(inside, parent));
  EXPECT_FALSE(box.Contains(outside, parent));
}

TEST(geometry_regressions, unrelated_rotations_are_not_equal) {
  const Rotation a(10.0f, 20.0f, 30.0f);
  const Rotation b(170.0f, 160.0f, 150.0f);
  ASSERT_GT((a.GetForwardVector() - b.GetForwardVector()).Length(), 0.1f);
  EXPECT_NE(a, b);
  EXPECT_EQ(a, Rotation(10.0f, 20.0f, 30.0f));
  EXPECT_NE(Transform(Location(), a), Transform(Location(), b));
}

TEST(geometry_regressions, integer_vector_squared_length_is_wide) {
  const Vector3DInt vector(50000, -60000, 70000);
  EXPECT_EQ(vector.SquaredLength(), 11000000000LL);
  EXPECT_NEAR(vector.Length(), std::sqrt(11000000000.0), 1e-6);
  const auto minimum = std::numeric_limits<int32_t>::min();
  const Vector3DInt extreme(minimum, minimum, minimum);
  EXPECT_EQ(extreme.SquaredLength(), 13835058055282163712ULL);
  EXPECT_NEAR(extreme.Length(), std::sqrt(13835058055282163712.0), 1e-6);
}

TEST(geometry_regressions, default_polynomial_has_zero_origin) {
  alignas(CubicPolynomial) std::array<unsigned char, sizeof(CubicPolynomial)> storage;
  storage.fill(0x42);
  auto *polynomial = new (storage.data()) CubicPolynomial;
  EXPECT_DOUBLE_EQ(polynomial->GetS(), 0.0);
  EXPECT_DOUBLE_EQ(polynomial->Evaluate(25.0), 0.0);
  polynomial->~CubicPolynomial();
}

TEST(geometry_regressions, generate_range_integer_extrema) {
  const auto high = std::numeric_limits<int>::max();
  const auto low = std::numeric_limits<int>::min();
  EXPECT_EQ(Math::GenerateRange(high - 1, high), (std::vector<int>{high - 1, high}));
  EXPECT_EQ(Math::GenerateRange(low + 1, low), (std::vector<int>{low + 1, low}));
  EXPECT_EQ(Math::GenerateRange(high, high), (std::vector<int>{high}));
  EXPECT_EQ(Math::GenerateRange(low, low), (std::vector<int>{low}));
}

TEST(geometry_regressions, lambert_equal_standard_parallels) {
  for (const double latitude : {33.0, -33.0}) {
    const GeoProjection projection = GeoProjection::Make(LambertConformalConicParams(
        latitude, latitude, latitude, 20.0, 100.0, 200.0, Ellipsoid(6378137.0, 298.257223563)));
    const Location origin = projection.GeoLocationToTransform(GeoLocation(latitude, 20.0, 12.0));
    ASSERT_TRUE(std::isfinite(origin.x));
    ASSERT_TRUE(std::isfinite(origin.y));
    EXPECT_NEAR(origin.x, 100.0f, 1e-3f);
    EXPECT_NEAR(origin.y, 200.0f, 1e-3f);
    const GeoLocation input(latitude + 0.25, 20.5, 12.0);
    const auto back = projection.TransformToGeoLocation(projection.GeoLocationToTransform(input));
    EXPECT_NEAR(back.latitude, input.latitude, 1e-6);
    EXPECT_NEAR(back.longitude, input.longitude, 1e-6);
    EXPECT_DOUBLE_EQ(back.altitude, input.altitude);
  }
}

TEST(geometry_regressions, zero_curvature_arc_is_a_segment) {
  const auto result = Math::DistanceArcToPoint(Vector3D(3.0f, 4.0f, 0.0f), Vector3D(), 10.0f, 0.0f, 0.0f);
  EXPECT_NEAR(result.first, 3.0f, 1e-5f);
  EXPECT_NEAR(result.second, 4.0f, 1e-5f);
  const auto turned = Math::DistanceArcToPoint(Vector3D(4.0f, 3.0f, 0.0f), Vector3D(2.0f, 1.0f, 0.0f), 10.0f, Math::Pi<float>() / 2.0f, 0.0f);
  EXPECT_NEAR(turned.first, 2.0f, 1e-5f);
  EXPECT_NEAR(turned.second, 2.0f, 1e-5f);
}

TEST(geometry_regressions, mesh_export_after_final_material) {
  Mesh mesh;
  mesh.AddVertices({{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}});
  mesh.AddMaterial("first");
  for (const auto index : {1u, 2u, 3u}) mesh.AddIndex(index);
  mesh.EndMaterial();
  for (const auto index : {1u, 3u, 2u}) mesh.AddIndex(index);
  EXPECT_NE(mesh.GenerateOBJ().find("f 1 3 2"), std::string::npos);
  EXPECT_NE(mesh.GenerateOBJForRecast().find("f 1 2 3"), std::string::npos);
}

TEST(geometry_regressions, mesh_concat_material_follows_bridge) {
  Mesh left({{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, {}, {1u, 2u, 3u});
  Mesh right;
  right.AddVertices({{0.0f, 2.0f, 0.0f}, {1.0f, 2.0f, 0.0f}, {0.0f, 3.0f, 0.0f}});
  right.AddMaterial("second");
  for (const auto index : {1u, 2u, 3u}) right.AddIndex(index);
  right.EndMaterial();
  left.ConcatMesh(right, 2);
  ASSERT_EQ(left.GetMaterials().size(), 1u);
  EXPECT_EQ(left.GetMaterials().front().index_start, 9u);
  EXPECT_EQ(left.GetMaterials().front().index_end, 12u);
  EXPECT_EQ(left.GetIndexesNum(), 12u);
}

TEST(geometry_regressions, normalize_large_rotation_angles) {
  for (const float angle : {1e10f, -1e10f, 1e20f, -1e20f, 1e30f, -1e30f}) {
    const auto result = Rotation(angle, angle, angle).Normalize();
    auto expected = std::fmod(angle, 360.0f);
    if (expected < -180.0f) expected += 360.0f;
    if (expected >= 180.0f) expected -= 360.0f;
    EXPECT_FLOAT_EQ(result.pitch, expected);
    EXPECT_FLOAT_EQ(result.yaw, expected);
    EXPECT_FLOAT_EQ(result.roll, expected);
  }
}

TEST(geometry_regressions, quaternion_gimbal_lock_preserves_orientation) {
  for (const float pitch : {-90.0f, -89.99f, 89.99f, 90.0f}) {
    for (const Rotation rotation : {Rotation(pitch, 30.0f, 20.0f), Rotation(pitch, 120.0f, -10.0f)}) {
      const Quaternion quaternion(rotation);
      for (const Quaternion back : {Quaternion(quaternion.Rotator()), Quaternion(quaternion.ToRightHandedRotation())}) {
        EXPECT_NEAR((back.GetForwardVector() - quaternion.GetForwardVector()).Length(), 0.0f, 1e-5f);
        EXPECT_NEAR((back.GetRightVector() - quaternion.GetRightVector()).Length(), 0.0f, 1e-5f);
        EXPECT_NEAR((back.GetUpVector() - quaternion.GetUpVector()).Length(), 0.0f, 1e-5f);
      }
    }
  }
}

TEST(geometry_regressions, vector_angle_roundoff_remains_finite) {
  for (const Vector3D vector : {Vector3D(-93.561897f, 66.097801f, -98.564323f), Vector3D(1e30f, 2e30f, -3e30f)}) {
    EXPECT_NEAR(Math::GetVectorAngle(vector, vector), 0.0, 1e-6);
    EXPECT_NEAR(Math::GetVectorAngle(vector, -1.0f * vector), Math::Pi<double>(), 1e-6);
  }
  EXPECT_NEAR(Math::GetVectorAngle(Vector3D(1.0f, 0.0f, 0.0f), Vector3D(0.0f, 1.0f, 0.0f)), Math::Pi<double>() / 2.0, 1e-12);
}

TEST(geometry_regressions, vector_normalization_extreme_finite_values) {
  for (const float component : {1e30f, 1e-30f}) {
    const auto v2 = Vector2D(component, component).MakeUnitVector(0.0f);
    EXPECT_NEAR(v2.Length(), 1.0f, 1e-6f);
    EXPECT_NEAR(v2.x, std::sqrt(0.5f), 1e-6f);
    const auto v3 = Vector3D(component, component, component).MakeUnitVector(0.0f);
    EXPECT_NEAR(v3.Length(), 1.0f, 1e-6f);
    EXPECT_NEAR(v3.x, std::sqrt(1.0f / 3.0f), 1e-6f);
    EXPECT_GT(Vector3D(component, component, 0.0f).Length2D(), component);
  }
  EXPECT_EQ(Vector3D().MakeUnitVector(0.0f), Vector3D());
}
