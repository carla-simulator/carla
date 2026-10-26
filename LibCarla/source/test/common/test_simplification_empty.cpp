// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"

#include <carla/geom/Simplification.h>

#include <memory>

TEST(simplification, empty_mesh_does_not_read_nonexistent_indexes) {
  auto mesh = std::make_unique<carla::geom::Mesh>();
  carla::geom::Simplification simplifier(1.0f);
  simplifier.Simplificate(mesh);
  EXPECT_TRUE(mesh->GetVertices().empty());
  EXPECT_TRUE(mesh->GetIndexes().empty());
}

TEST(simplification, triangle_without_reduction_control) {
  auto mesh = std::make_unique<carla::geom::Mesh>();
  mesh->AddVertex({0.0f, 0.0f, 0.0f});
  mesh->AddVertex({1.0f, 0.0f, 0.0f});
  mesh->AddVertex({0.0f, 1.0f, 0.0f});
  mesh->GetIndexes() = {1, 2, 3};
  const auto vertices = mesh->GetVertices();
  const auto indexes = mesh->GetIndexes();
  carla::geom::Simplification simplifier(1.0f);
  simplifier.Simplificate(mesh);
  EXPECT_EQ(mesh->GetVertices(), vertices);
  EXPECT_EQ(mesh->GetIndexes(), indexes);
}
