// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/geom/Mesh.h"

using carla::geom::Mesh;

TEST(MeshBulkAppend, EmptyBatchesPreserveExistingData) {
  Mesh mesh;
  mesh.AddVertices({});
  mesh.AddUVs({});
  EXPECT_EQ(mesh.GetVerticesNum(), 0u);
  EXPECT_TRUE(mesh.GetUVs().empty());
  mesh.AddVertex({1.25f, -2.5f, 3.75f});
  mesh.AddUV({0.25f, 0.75f});
  mesh.AddVertices({});
  mesh.AddUVs({});
  ASSERT_EQ(mesh.GetVerticesNum(), 1u);
  ASSERT_EQ(mesh.GetUVs().size(), 1u);
  EXPECT_EQ(mesh.GetVertices().front(), (Mesh::vertex_type{1.25f, -2.5f, 3.75f}));
  EXPECT_EQ(mesh.GetUVs().front(), (Mesh::uv_type{0.25f, 0.75f}));
}

TEST(MeshBulkAppend, LargeBatchesPreserveEveryCoordinateAndInput) {
  std::vector<Mesh::vertex_type> vertices;
  std::vector<Mesh::uv_type> uvs;
  for (size_t i = 0; i < 1000000; ++i) {
    vertices.emplace_back(static_cast<float>(i) * 0.25f,
        static_cast<float>(i % 89) * -0.5f, -3.5f);
    uvs.emplace_back(static_cast<float>(i % 2), static_cast<float>((i / 2) % 2));
  }
  const auto original_vertices = vertices;
  const auto original_uvs = uvs;
  Mesh mesh;
  mesh.AddVertices(vertices);
  mesh.AddUVs(uvs);
  EXPECT_TRUE(mesh.GetVertices() == original_vertices);
  EXPECT_TRUE(mesh.GetUVs() == original_uvs);
  EXPECT_TRUE(vertices == original_vertices);
  EXPECT_TRUE(uvs == original_uvs);
  mesh.AddVertices({{1.0f, 2.0f, 3.0f}});
  mesh.AddUVs({{0.1f, 0.2f}});
  EXPECT_EQ(mesh.GetVerticesNum(), original_vertices.size() + 1);
  EXPECT_EQ(mesh.GetUVs().size(), original_uvs.size() + 1);
  EXPECT_EQ(mesh.GetVertices().back(), (Mesh::vertex_type{1.0f, 2.0f, 3.0f}));
}

TEST(MeshBulkAppend, RepeatedSmallBatchesPreservePrefixAndOrdering) {
  Mesh mesh;
  std::vector<Mesh::vertex_type> expected_vertices;
  std::vector<Mesh::uv_type> expected_uvs;
  for (size_t batch = 0; batch < 100; ++batch) {
    std::vector<Mesh::vertex_type> vertices;
    std::vector<Mesh::uv_type> uvs;
    for (size_t i = 0; i < batch % 17; ++i) {
      const Mesh::vertex_type vertex{static_cast<float>(batch), static_cast<float>(i), -0.5f};
      const Mesh::uv_type uv{static_cast<float>(i), 0.5f};
      vertices.push_back(vertex);
      uvs.push_back(uv);
      expected_vertices.push_back(vertex);
      expected_uvs.push_back(uv);
    }
    mesh.AddVertices(vertices);
    mesh.AddUVs(uvs);
    EXPECT_TRUE(mesh.GetVertices() == expected_vertices);
    EXPECT_TRUE(mesh.GetUVs() == expected_uvs);
  }
}

TEST(MeshBulkAppend, ExistingVertexReferencesRemainValidWithoutReallocation) {
  Mesh mesh;
  mesh.GetVertices().reserve(32);
  mesh.AddVertex({4.0f, 5.0f, 6.0f});
  const auto *first = &mesh.GetVertices().front();
  mesh.AddVertices({{7.0f, 8.0f, 9.0f}, {10.0f, 11.0f, 12.0f}});
  ASSERT_EQ(first, &mesh.GetVertices().front());
  EXPECT_EQ(*first, (Mesh::vertex_type{4.0f, 5.0f, 6.0f}));
}

TEST(MeshBulkAppend, TriangleStripsKeepTheirWindingAndVertexOffsets) {
  Mesh mesh;
  mesh.AddVertex({-1.0f, -1.0f, -1.0f});
  mesh.AddNormal({0.0f, 0.0f, 1.0f});
  const std::vector<Mesh::vertex_type> vertices{
      {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
      {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}};
  mesh.AddTriangleStrip(vertices);
  EXPECT_EQ(mesh.GetVerticesNum(), 5u);
  EXPECT_EQ(mesh.GetIndexes(), (std::vector<Mesh::index_type>{2, 3, 4, 5, 4, 3}));
  ASSERT_EQ(mesh.GetNormals().size(), 1u);
  EXPECT_EQ(mesh.GetNormals().front(), (Mesh::normal_type{0.0f, 0.0f, 1.0f}));
  EXPECT_TRUE(std::equal(vertices.begin(), vertices.end(), mesh.GetVertices().begin() + 1));
}

TEST(MeshBulkAppend, TriangleFansKeepTheirWindingAndVertexOffsets) {
  Mesh mesh;
  mesh.AddVertex({-1.0f, -1.0f, -1.0f});
  const std::vector<Mesh::vertex_type> vertices{
      {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
      {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}};
  mesh.AddTriangleFan(vertices);
  EXPECT_EQ(mesh.GetVerticesNum(), 5u);
  EXPECT_EQ(mesh.GetIndexes(), (std::vector<Mesh::index_type>{2, 3, 4, 2, 4, 5}));
  EXPECT_TRUE(std::equal(vertices.begin(), vertices.end(), mesh.GetVertices().begin() + 1));
}
