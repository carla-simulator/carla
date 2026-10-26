// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"
#include <carla/geom/Mesh.h>
#include <cstdlib>
#include <fstream>
#include <string>

namespace {
using carla::geom::Mesh;

Mesh Quad(bool uv=true, bool normals=true) {
  return Mesh({{0.f,0.f,0.f},{2.f,0.f,0.f},{2.f,3.f,0.f},{0.f,3.f,0.f}},
              normals ? std::vector<Mesh::normal_type>{{0.f,.6f,.8f},{.6f,0.f,.8f},{0.f,-.6f,.8f},{-.6f,0.f,.8f}}
                      : std::vector<Mesh::normal_type>{},
              {1u,2u,3u,1u,3u,4u},
              uv ? std::vector<Mesh::uv_type>{{0.f,0.f},{1.f,0.f},{1.f,1.f},{0.f,1.f}}
                 : std::vector<Mesh::uv_type>{});
}

void EqualMesh(const Mesh &actual,const Mesh &expected) {
  EXPECT_EQ(actual.GetVertices(),expected.GetVertices());
  EXPECT_EQ(actual.GetNormals(),expected.GetNormals());
  EXPECT_EQ(actual.GetUVs(),expected.GetUVs());
  EXPECT_EQ(actual.GetIndexes(),expected.GetIndexes());
  ASSERT_EQ(actual.GetMaterials().size(),expected.GetMaterials().size());
  for (size_t i=0;i<actual.GetMaterials().size();++i) {
    EXPECT_EQ(actual.GetMaterials()[i].name,expected.GetMaterials()[i].name);
    EXPECT_EQ(actual.GetMaterials()[i].index_start,expected.GetMaterials()[i].index_start);
    EXPECT_EQ(actual.GetMaterials()[i].index_end,expected.GetMaterials()[i].index_end);
  }
}

TEST(mesh_contracts, self_append_matches_an_independent_source_copy) {
  auto source=Quad();
  source.AddMaterial("quad");
  source.AddIndex(1u);source.AddIndex(2u);source.AddIndex(3u);source.EndMaterial();
  auto expected=source;
  expected+=source;
  source+=source;
  EqualMesh(source,expected);
}

TEST(mesh_contracts, self_concat_matches_an_independent_source_copy) {
  auto source=Quad();
  auto expected=source;
  expected.ConcatMesh(source,2);
  source.ConcatMesh(source,2);
  EqualMesh(source,expected);
}

TEST(mesh_contracts, independent_append_and_empty_self_controls) {
  auto source=Quad();
  auto appended=source;
  appended+=source;
  EXPECT_EQ(appended.GetVerticesNum(),8u);
  EXPECT_EQ(appended.GetIndexes(),(std::vector<size_t>{1,2,3,1,3,4,5,6,7,5,7,8}));
  Mesh empty;
  empty+=empty;
  EXPECT_TRUE(empty.GetVertices().empty());
  EXPECT_TRUE(empty.GetIndexes().empty());
}

class MeshExports : public ::testing::TestWithParam<int> {};

TEST_P(MeshExports, obj_connects_complete_per_vertex_attributes) {
  const int mode=GetParam();
  const auto mesh=Quad(mode&1,mode&2);
  const auto obj=mesh.GenerateOBJ();
  const std::string face[] = {"f 1 2 3", "f 1/1 2/2 3/3", "f 1//1 2//2 3//3", "f 1/1/1 2/2/2 3/3/3"};
  EXPECT_NE(obj.find(face[mode]),std::string::npos);
  if (const auto *folder=std::getenv("CARLA_MESH_EXPORT_DIR")) {
    std::ofstream(std::string(folder)+"/mesh-"+std::to_string(mode)+".obj")<<obj;
  }
}

TEST_P(MeshExports, ply_contains_a_mesh_document) {
  const int mode=GetParam();
  const auto mesh=Quad(mode&1,mode&2);
  const auto ply=mesh.GeneratePLY();
  EXPECT_EQ(ply.find("ply\nformat ascii 1.0\n"),0u);
  EXPECT_NE(ply.find("element vertex 4\n"),std::string::npos);
  EXPECT_NE(ply.find("element face 2\n"),std::string::npos);
  if (const auto *folder=std::getenv("CARLA_MESH_EXPORT_DIR")) {
    std::ofstream(std::string(folder)+"/mesh-"+std::to_string(mode)+".ply")<<ply;
  }
}

INSTANTIATE_TEST_SUITE_P(attribute_combinations,MeshExports,::testing::Values(0,1,2,3));

TEST(mesh_contracts, partial_attribute_arrays_do_not_create_invalid_face_references_control) {
  auto mesh=Quad(false,false);
  mesh.AddUV({0.f,0.f});
  mesh.AddNormal({0.f,0.f,1.f});
  EXPECT_NE(mesh.GenerateOBJ().find("f 1 2 3"),std::string::npos);
}

TEST(mesh_contracts, ply_exports_a_valid_vertex_only_mesh) {
  Mesh mesh({{1e-20f,-100000.f,.125f}});
  const auto ply=mesh.GeneratePLY();
  EXPECT_EQ(ply.find("ply\nformat ascii 1.0\n"),0u);
  EXPECT_NE(ply.find("element vertex 1\n"),std::string::npos);
  EXPECT_NE(ply.find("element face 0\n"),std::string::npos);
  if (const auto *folder=std::getenv("CARLA_MESH_EXPORT_DIR")) {
    std::ofstream(std::string(folder)+"/point-cloud.ply")<<ply;
  }
}
}
