// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/geom/Mesh.h>

#include "test.h"

namespace {
carla::geom::Mesh OriginalMesh() {
  return carla::geom::Mesh({{0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {1.f, 1.f, 0.f}}, {},
                           {1u, 2u, 3u, 2u, 4u, 3u},
                           {{0.f, 0.f}, {1.f, 0.f}, {0.f, 1.f}, {1.f, 1.f}});
}

TEST(mesh_bulk_alias, vertices_match_independent_input) {
  auto actual = OriginalMesh();
  auto expected = actual;
  const auto snapshot = actual.GetVertices();
  expected.AddVertices(snapshot);
  actual.AddVertices(actual.GetVertices());
  EXPECT_EQ(actual.GetVertices(), expected.GetVertices());
}

TEST(mesh_bulk_alias, uvs_match_independent_input) {
  auto actual = OriginalMesh();
  auto expected = actual;
  const auto snapshot = actual.GetUVs();
  expected.AddUVs(snapshot);
  actual.AddUVs(actual.GetUVs());
  EXPECT_EQ(actual.GetUVs(), expected.GetUVs());
}

TEST(mesh_bulk_alias, triangle_strip_retains_aliased_vertices) {
  auto actual = OriginalMesh();
  auto expected = actual;
  const auto snapshot = actual.GetVertices();
  expected.AddTriangleStrip(snapshot);
  actual.AddTriangleStrip(actual.GetVertices());
  EXPECT_EQ(actual.GetVertices(), expected.GetVertices());
  EXPECT_EQ(actual.GetIndexes(), expected.GetIndexes());
  EXPECT_EQ(actual.GetVerticesNum(), 8u);
  EXPECT_EQ(actual.GetIndexesNum(), 12u);
  EXPECT_EQ(actual.GetIndexes(),
            (std::vector<size_t>{1u, 2u, 3u, 2u, 4u, 3u, 5u, 6u, 7u, 8u, 7u, 6u}));
  for (const auto index : actual.GetIndexes()) {
    EXPECT_GE(index, 1u);
    EXPECT_LE(index, actual.GetVerticesNum());
  }
}

TEST(mesh_bulk_alias, triangle_fan_retains_aliased_vertices) {
  carla::geom::Mesh actual({{0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 1.f, 0.f}, {0.f, 1.f, 0.f}}, {},
                           {1u, 2u, 3u, 1u, 3u, 4u});
  auto expected = actual;
  const auto snapshot = actual.GetVertices();
  expected.AddTriangleFan(snapshot);
  actual.AddTriangleFan(actual.GetVertices());
  EXPECT_EQ(actual.GetVertices(), expected.GetVertices());
  EXPECT_EQ(actual.GetIndexes(), expected.GetIndexes());
  EXPECT_EQ(actual.GetVerticesNum(), 8u);
  EXPECT_EQ(actual.GetIndexesNum(), 12u);
  EXPECT_EQ(actual.GetIndexes(),
            (std::vector<size_t>{1u, 2u, 3u, 1u, 3u, 4u, 5u, 6u, 7u, 5u, 7u, 8u}));
  for (const auto index : actual.GetIndexes()) {
    EXPECT_GE(index, 1u);
    EXPECT_LE(index, actual.GetVerticesNum());
  }
}

TEST(mesh_bulk_alias, external_and_empty_input_control) {
  auto actual = OriginalMesh();
  actual.AddVertices({{2.f, 2.f, 0.f}});
  actual.AddVertices({});
  actual.AddUVs({});
  EXPECT_EQ(actual.GetVerticesNum(), 5u);
  EXPECT_EQ(actual.GetUVs().size(), 4u);
}
}  // namespace
