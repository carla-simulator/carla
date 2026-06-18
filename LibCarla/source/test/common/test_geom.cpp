// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/geom/Acceleration.h>
#include <carla/geom/AngularVelocity.h>
#include <carla/geom/BoundingBox.h>
#include <carla/geom/Math.h>
#include <carla/geom/Quaternion.h>
#include <carla/geom/RightHandedVector3D.h>
#include <carla/geom/Transform.h>
#include <carla/geom/Vector3D.h>
#include <carla/geom/Velocity.h>
#include <cmath>
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

  // The point {0, 0, 2} rotated by pitch=90 lands at {2, 0, 0} under the
  // corrected sign convention (pre-fix this returned {-2, 0, 0}). The
  // subsequent translation by {0, 0, -1} brings the final point to
  // {2, 0, -1}.
  Location translation (0.0,0.0,-1.0); // x y z
  Rotation rotation (90.0,0.0,0.0); // y z x
  Transform transform (translation, rotation);

  Location point (0.0, 0.0, 2.0);
  transform.TransformPoint(point);
  Location result_point(2.0, 0.0, -1.0);
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
  // Regression guard for the pitch/roll fix: pitch=90 now produces
  // {0, 0, -1}, not {0, 0, 1} as on the pre-fix matrix.
  compare({ 90.0f,   0.0f,   0.0f}, {0.0f, 0.0f,-1.0f});
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

TEST(geom, right_handed_vector3d_boundary_negation) {
  // Going Vector3D -> RightHandedVector3D negates Y exactly once.
  const Vector3D lh{1.0f, 2.0f, 3.0f};
  const RightHandedVector3D rh{lh};
  ASSERT_FLOAT_EQ(rh.x, 1.0f);
  ASSERT_FLOAT_EQ(rh.y, -2.0f);
  ASSERT_FLOAT_EQ(rh.z, 3.0f);

  // Round-trip back to Vector3D restores the original.
  const Vector3D round_trip = static_cast<Vector3D>(rh);
  ASSERT_FLOAT_EQ(round_trip.x, 1.0f);
  ASSERT_FLOAT_EQ(round_trip.y, 2.0f);
  ASSERT_FLOAT_EQ(round_trip.z, 3.0f);
}

TEST(geom, quaternion_default_is_identity) {
  const Quaternion q;
  ASSERT_FLOAT_EQ(q.x, 0.0f);
  ASSERT_FLOAT_EQ(q.y, 0.0f);
  ASSERT_FLOAT_EQ(q.z, 0.0f);
  ASSERT_FLOAT_EQ(q.w, 1.0f);

  // Identity quaternion rotates a vector to itself.
  const Vector3D v{1.0f, 2.0f, 3.0f};
  const Vector3D out = static_cast<Vector3D>(q.RotatedVector(RightHandedVector3D{v}));
  ASSERT_NEAR(out.x, v.x, 1e-5f);
  ASSERT_NEAR(out.y, v.y, 1e-5f);
  ASSERT_NEAR(out.z, v.z, 1e-5f);
}

TEST(geom, quaternion_get_forward_matches_rotation_get_forward) {
  // The quaternion built from a Rotation must agree with that Rotation's
  // forward-vector for every axis-aligned angle. This is the canonical
  // regression guard against drift between the two math paths.
  constexpr float eps = 1e-4f;
  const float angles[] = {-180.0f, -90.0f, -45.0f, 0.0f, 30.0f, 90.0f, 123.0f, 180.0f};
  for (const float a : angles) {
    for (const auto axis : {0, 1, 2}) {
      Rotation r{};
      if (axis == 0) r.pitch = a;
      else if (axis == 1) r.yaw = a;
      else r.roll = a;

      const Vector3D r_forward = r.GetForwardVector();
      const Vector3D q_forward = Quaternion(r).GetForwardVector();
      EXPECT_NEAR(q_forward.x, r_forward.x, eps) << "axis=" << axis << " angle=" << a;
      EXPECT_NEAR(q_forward.y, r_forward.y, eps) << "axis=" << axis << " angle=" << a;
      EXPECT_NEAR(q_forward.z, r_forward.z, eps) << "axis=" << axis << " angle=" << a;
    }
  }
}

TEST(geom, quaternion_get_yaw_round_trip) {
  // Yaw component of Rotation -> Quaternion -> Rotation must round-trip.
  // Pitch and roll left at zero so the Rotator extraction is unambiguous.
  constexpr float eps = 1e-3f;
  for (float yaw_deg = -150.0f; yaw_deg <= 150.0f; yaw_deg += 30.0f) {
    const Quaternion q{Rotation{0.0f, yaw_deg, 0.0f}};
    const Rotation r = q.Rotator();
    EXPECT_NEAR(r.yaw, yaw_deg, eps) << "yaw=" << yaw_deg;
    EXPECT_NEAR(r.pitch, 0.0f, eps);
    EXPECT_NEAR(r.roll, 0.0f, eps);
  }
}

TEST(geom, quaternion_inverse_is_left_inverse) {
  // q * q^-1 must be the identity for sampled axis-aligned and combined
  // rotations. Tolerances are generous to accommodate float accumulation.
  constexpr float eps = 1e-4f;
  const Rotation samples[] = {
    {0.0f, 0.0f, 0.0f},
    {30.0f, 0.0f, 0.0f},
    {0.0f, 45.0f, 0.0f},
    {0.0f, 0.0f, 60.0f},
    {15.0f, 25.0f, 10.0f},
    {-45.0f, 90.0f, -30.0f},
  };
  for (const auto &r : samples) {
    const Quaternion q{r};
    const Quaternion identity = q * q.Inverse();
    EXPECT_NEAR(identity.x, 0.0f, eps) << r.pitch << "/" << r.yaw << "/" << r.roll;
    EXPECT_NEAR(identity.y, 0.0f, eps);
    EXPECT_NEAR(identity.z, 0.0f, eps);
    EXPECT_NEAR(std::abs(identity.w), 1.0f, eps);
  }
}

TEST(geom, quaternion_pitch_90_forward_is_negative_z) {
  // Regression guard for the pitch/roll fix from the ROS 2 port.
  // Pre-fix the forward vector of pitch=90 was {0, 0, +1}; the corrected
  // sign convention now yields {0, 0, -1}.
  const Quaternion q{Rotation{90.0f, 0.0f, 0.0f}};
  const Vector3D forward = q.GetForwardVector();
  EXPECT_NEAR(forward.x, 0.0f, 1e-5f);
  EXPECT_NEAR(forward.y, 0.0f, 1e-5f);
  EXPECT_NEAR(forward.z, -1.0f, 1e-5f);
}

TEST(geom, velocity_inherits_vector3d_storage) {
  // Velocity is a Vector3D-inheriting POD that carries m/s unit semantics
  // without adding any data members. Wire-compatibility with Vector3D is
  // preserved (msgpack adapter inherited).
  const Velocity v{1.5f, -2.0f, 0.25f};
  EXPECT_FLOAT_EQ(v.x, 1.5f);
  EXPECT_FLOAT_EQ(v.y, -2.0f);
  EXPECT_FLOAT_EQ(v.z, 0.25f);

  // Arithmetic operations inherit from Vector3D.
  const Velocity v2 = Velocity{v + Vector3D{0.5f, 1.0f, 0.0f}};
  EXPECT_FLOAT_EQ(v2.x, 2.0f);
  EXPECT_FLOAT_EQ(v2.y, -1.0f);
  EXPECT_FLOAT_EQ(v2.z, 0.25f);
}

TEST(geom, angular_velocity_and_acceleration_inherit_vector3d_storage) {
  const AngularVelocity w{45.0f, 0.0f, 0.0f};
  EXPECT_FLOAT_EQ(w.x, 45.0f);
  EXPECT_FLOAT_EQ(w.y, 0.0f);

  const Acceleration a{0.0f, 9.81f, 0.0f};
  EXPECT_FLOAT_EQ(a.y, 9.81f);

  // The new types are layout-compatible with Vector3D so the
  // serialization wire format stays unchanged.
  static_assert(sizeof(Velocity) == sizeof(Vector3D),
                "Velocity must stay layout-compatible with Vector3D");
  static_assert(sizeof(AngularVelocity) == sizeof(Vector3D),
                "AngularVelocity must stay layout-compatible with Vector3D");
  static_assert(sizeof(Acceleration) == sizeof(Vector3D),
                "Acceleration must stay layout-compatible with Vector3D");
}

TEST(geom, quaternion_basis_vectors_for_identity) {
  // Identity quaternion's three basis vectors must be the canonical
  // CARLA left-handed axes.
  constexpr float eps = 1e-5f;
  const Quaternion q;

  const Vector3D forward = q.GetForwardVector();
  EXPECT_NEAR(forward.x, 1.0f, eps);
  EXPECT_NEAR(forward.y, 0.0f, eps);
  EXPECT_NEAR(forward.z, 0.0f, eps);

  const Vector3D right = q.GetRightVector();
  EXPECT_NEAR(right.x, 0.0f, eps);
  EXPECT_NEAR(right.y, 1.0f, eps);
  EXPECT_NEAR(right.z, 0.0f, eps);

  const Vector3D up = q.GetUpVector();
  EXPECT_NEAR(up.x, 0.0f, eps);
  EXPECT_NEAR(up.y, 0.0f, eps);
  EXPECT_NEAR(up.z, 1.0f, eps);
}

TEST(geom, quaternion_basis_vectors_for_yaw_90) {
  // Yaw=90 deg in CARLA's left-handed convention sends:
  //   forward (+X) -> (+Y), right (+Y) -> (-X), up (+Z) -> (+Z).
  constexpr float eps = 1e-4f;
  const Quaternion q{Rotation{0.0f, 90.0f, 0.0f}};

  const Vector3D forward = q.GetForwardVector();
  EXPECT_NEAR(forward.x, 0.0f, eps);
  EXPECT_NEAR(forward.y, 1.0f, eps);
  EXPECT_NEAR(forward.z, 0.0f, eps);

  const Vector3D right = q.GetRightVector();
  EXPECT_NEAR(right.x, -1.0f, eps);
  EXPECT_NEAR(right.y, 0.0f, eps);
  EXPECT_NEAR(right.z, 0.0f, eps);

  const Vector3D up = q.GetUpVector();
  EXPECT_NEAR(up.x, 0.0f, eps);
  EXPECT_NEAR(up.y, 0.0f, eps);
  EXPECT_NEAR(up.z, 1.0f, eps);
}

TEST(geom, quaternion_hamilton_product_yaw_compose) {
  // Two consecutive yaw=45 rotations compose to a yaw=90 rotation:
  // applying the product to the forward axis sends (1,0,0) -> (0,1,0).
  constexpr float eps = 1e-4f;
  const Quaternion q45{Rotation{0.0f, 45.0f, 0.0f}};
  const Quaternion q90 = q45 * q45;

  const Vector3D forward = q90.GetForwardVector();
  EXPECT_NEAR(forward.x, 0.0f, eps);
  EXPECT_NEAR(forward.y, 1.0f, eps);
  EXPECT_NEAR(forward.z, 0.0f, eps);
}

TEST(geom, quaternion_conjugate_equals_inverse_for_unit_quaternion) {
  // For a unit quaternion built from any Rotation, conjugate and inverse
  // are mathematically identical. Sample a few axis-aligned and combined
  // rotations.
  constexpr float eps = 1e-5f;
  const Rotation samples[] = {
    {0.0f, 0.0f, 0.0f},
    {30.0f, 0.0f, 0.0f},
    {0.0f, 45.0f, 0.0f},
    {0.0f, 0.0f, 60.0f},
    {15.0f, 25.0f, 10.0f},
  };
  for (const auto &r : samples) {
    const Quaternion q{r};
    const Quaternion qc = q.Conjugate();
    const Quaternion qi = q.Inverse();
    EXPECT_NEAR(qc.x, qi.x, eps) << r.pitch << "/" << r.yaw << "/" << r.roll;
    EXPECT_NEAR(qc.y, qi.y, eps);
    EXPECT_NEAR(qc.z, qi.z, eps);
    EXPECT_NEAR(qc.w, qi.w, eps);
  }
}
