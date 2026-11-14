// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/agents/navigation/ObstacleGeometry.h>
#include <carla/geom/BoundingBox.h>
#include <carla/geom/Transform.h>

namespace geometry = carla::agents::navigation::detail;
namespace bg = boost::geometry;
namespace cg = carla::geom;

TEST(agent_obstacle_geometry, corridor_contains_both_sides_between_route_samples) {
  const auto corridor = geometry::BuildRoutePolygon({
      {{0, 1, 0}}, {{0, -1, 0}}, {{5, 1, 0}}, {{5, -1, 0}},
      {{10, 1, 0}}, {{10, -1, 0}}, {{30, 1, 0}}, {{30, -1, 0}}});
  EXPECT_TRUE(bg::is_valid(corridor));
  EXPECT_DOUBLE_EQ(bg::area(corridor), 60.0);
  for (double lateral : {-0.8, 0.8}) {
    EXPECT_TRUE(bg::within(geometry::ObstaclePoint(7.5, lateral), corridor));
  }
  EXPECT_FALSE(bg::intersects(geometry::ObstaclePoint(7.5, 2.0), corridor));
}

TEST(agent_obstacle_geometry, projected_box_contains_interior) {
  const cg::BoundingBox box(cg::Location(7.5f, 0.5f, 0.0f), cg::Vector3D(3.0f, 1.0f, 1.0f));
  const auto footprint = geometry::ProjectBoundingBox(box.GetWorldVertices(cg::Transform{}));
  EXPECT_TRUE(bg::is_valid(footprint));
  EXPECT_DOUBLE_EQ(bg::area(footprint), 12.0);
  EXPECT_TRUE(bg::within(geometry::ObstaclePoint(7.5, 0.0), footprint));
  EXPECT_FALSE(bg::intersects(geometry::ObstaclePoint(12.0, 0.0), footprint));
}

TEST(agent_obstacle_geometry, rotated_projected_box_contains_actor_center) {
  const cg::BoundingBox box(cg::Location{}, cg::Vector3D(3.0f, 1.0f, 1.0f));
  for (const cg::Rotation &rotation : {
      cg::Rotation(0, 45, 0), cg::Rotation(20, 90, 0), cg::Rotation(10, 15, 25)}) {
    const cg::Transform transform(cg::Location(7.5f, 0.5f, 0.0f), rotation);
    const auto footprint = geometry::ProjectBoundingBox(box.GetWorldVertices(transform));
    EXPECT_TRUE(bg::is_valid(footprint));
    EXPECT_TRUE(bg::within(geometry::ObstaclePoint(7.5, 0.5), footprint));
  }
}
