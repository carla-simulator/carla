// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/geom/Mesh.h"
#include "carla/road/MeshFactory.h"

using carla::geom::Mesh;
using carla::geom::MeshFactory;

TEST(MeshSmoothing, SmoothsBothVerticesOfTheFirstInteriorRow) {
  const std::vector<Mesh::vertex_type> vertices{
      {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
      {1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f},
      {2.0f, -1.0f, 0.0f}, {2.0f, 1.0f, 0.0f}};
  std::vector<std::unique_ptr<Mesh>> lanes;
  lanes.push_back(std::make_unique<Mesh>());
  lanes.back()->AddTriangleStrip(vertices);
  const auto indices = lanes.back()->GetIndexes();

  const auto merged = MeshFactory{}.MergeAndSmooth(lanes);

  ASSERT_EQ(merged->GetVerticesNum(), vertices.size());
  // With all four fixed end vertices at zero, both interior vertices relax
  // toward the same plane. Skipping index two leaves a one-sided ridge.
  EXPECT_NEAR(merged->GetVertices()[2].z, 0.0f, 1e-5f);
  EXPECT_NEAR(merged->GetVertices()[3].z, 0.0f, 1e-5f);
  for (std::size_t i = 0; i < vertices.size(); ++i) {
    EXPECT_FLOAT_EQ(merged->GetVertices()[i].x, vertices[i].x);
    EXPECT_FLOAT_EQ(merged->GetVertices()[i].y, vertices[i].y);
    if (i < 2 || i >= vertices.size() - 2) {
      EXPECT_FLOAT_EQ(merged->GetVertices()[i].z, vertices[i].z);
    }
  }
  EXPECT_EQ(merged->GetIndexes(), indices);
}

TEST(MeshSmoothing, PreservesEmptyAndBoundaryOnlyMeshes) {
  MeshFactory factory;
  std::vector<std::unique_ptr<Mesh>> lanes;
  EXPECT_EQ(factory.MergeAndSmooth(lanes)->GetVerticesNum(), 0u);

  const std::vector<Mesh::vertex_type> vertices{
      {0.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 2.0f},
      {1.0f, -1.0f, 3.0f}, {1.0f, 1.0f, 4.0f}};
  lanes.push_back(std::make_unique<Mesh>());
  lanes.back()->AddTriangleStrip(vertices);
  const auto merged = factory.MergeAndSmooth(lanes);
  EXPECT_TRUE(merged->GetVertices() == vertices);
  EXPECT_TRUE(lanes.front()->GetVertices() == vertices);
}

TEST(MeshSmoothing, PreservesAnAlreadyFlatInterior) {
  const std::vector<Mesh::vertex_type> vertices{
      {0.0f, -1.0f, 2.0f}, {0.0f, 1.0f, 2.0f},
      {1.0f, -1.0f, 2.0f}, {1.0f, 1.0f, 2.0f},
      {2.0f, -1.0f, 2.0f}, {2.0f, 1.0f, 2.0f}};
  std::vector<std::unique_ptr<Mesh>> lanes;
  lanes.push_back(std::make_unique<Mesh>());
  lanes.back()->AddTriangleStrip(vertices);
  const auto merged = MeshFactory{}.MergeAndSmooth(lanes);
  EXPECT_TRUE(merged->GetVertices() == vertices);
  EXPECT_TRUE(lanes.front()->GetVertices() == vertices);
}
