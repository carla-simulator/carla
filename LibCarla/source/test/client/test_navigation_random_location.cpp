// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/nav/Navigation.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {
using namespace carla::nav;

struct TileSpec {
  unsigned short flags;
  unsigned char area;
};

struct MeshDeleter {
  void operator()(dtNavMesh *mesh) const { dtFreeNavMesh(mesh); }
};
using Mesh = std::unique_ptr<dtNavMesh, MeshDeleter>;

Mesh MakeMesh(const std::vector<TileSpec> &tiles) {
  Mesh mesh(dtAllocNavMesh());
  if (!mesh) return {};
  dtNavMeshParams mesh_params{};
  mesh_params.tileWidth = mesh_params.tileHeight = 10.0f;
  mesh_params.maxTiles = 8;
  mesh_params.maxPolys = 16;
  if (dtStatusFailed(mesh->init(&mesh_params))) return {};
  for (size_t index = 0; index < tiles.size(); ++index) {
    const unsigned short vertices[] = {0, 0, 0, 0, 0, 10, 10, 0, 10, 10, 0, 0};
    const unsigned short polygons[] = {0, 1, 2, 3, 0xffff, 0xffff, 0xffff, 0xffff};
    dtNavMeshCreateParams params{};
    params.verts = vertices;
    params.vertCount = 4;
    params.polys = polygons;
    params.polyCount = 1;
    params.nvp = 4;
    params.polyFlags = &tiles[index].flags;
    params.polyAreas = &tiles[index].area;
    params.walkableHeight = 1.8f;
    params.walkableRadius = 0.3f;
    params.walkableClimb = 0.5f;
    params.tileX = static_cast<int>(index);
    params.bmin[0] = static_cast<float>(index) * 10.0f;
    params.bmax[0] = params.bmin[0] + 10.0f;
    params.bmax[1] = 2.0f;
    params.bmax[2] = 10.0f;
    params.cs = 1.0f;
    params.ch = 0.2f;
    params.buildBvTree = true;
    unsigned char *data = nullptr;
    int size = 0;
    if (!dtCreateNavMeshData(&params, &data, &size)) return {};
    if (dtStatusFailed(mesh->addTile(data, size, DT_TILE_FREE_DATA, 0, nullptr))) {
      dtFree(data);
      return {};
    }
  }
  return mesh;
}

template <typename T>
void Append(std::vector<uint8_t> &bytes, const T &value) {
  const auto *first = reinterpret_cast<const uint8_t *>(&value);
  bytes.insert(bytes.end(), first, first + sizeof(value));
}

std::vector<uint8_t> Serialize(const dtNavMesh &mesh) {
#pragma pack(push, 1)
  struct Header { int magic, version, count; dtNavMeshParams params; };
  struct TileHeader { dtTileRef ref; int size; };
#pragma pack(pop)
  Header header{'M' << 24 | 'S' << 16 | 'E' << 8 | 'T', 1, 0, *mesh.getParams()};
  for (int i = 0; i < mesh.getMaxTiles(); ++i) {
    const auto *tile = mesh.getTile(i);
    if (tile && tile->header) ++header.count;
  }
  std::vector<uint8_t> bytes;
  Append(bytes, header);
  for (int i = 0; i < mesh.getMaxTiles(); ++i) {
    const auto *tile = mesh.getTile(i);
    if (!tile || !tile->header) continue;
    Append(bytes, TileHeader{mesh.getTileRef(tile), tile->dataSize});
    bytes.insert(bytes.end(), tile->data, tile->data + tile->dataSize);
  }
  return bytes;
}

float RandomFloat() { return static_cast<float>(std::rand()) / RAND_MAX; }

unsigned int SeedForTenthSuccess(const dtNavMesh &mesh) {
  dtNavMeshQuery query;
  if (dtStatusFailed(query.init(&mesh, 32))) throw std::runtime_error("query init");
  dtQueryFilter filter;
  filter.setIncludeFlags(CARLA_TYPE_SIDEWALK);
  filter.setExcludeFlags(CARLA_TYPE_NONE);
  for (unsigned int seed = 0; seed < 100000; ++seed) {
    std::srand(seed);
    bool ninth_failure = true;
    for (int attempt = 0; attempt < 10; ++attempt) {
      dtPolyRef ref{};
      float point[3]{};
      const bool success = dtStatusSucceed(query.findRandomPoint(&filter, RandomFloat, &ref, point));
      if (attempt < 9 && success) { ninth_failure = false; break; }
      if (attempt == 9 && ninth_failure && success) return seed;
    }
  }
  throw std::runtime_error("No bounded tenth-success seed found");
}

TEST(NavigationRandomLocation, SidewalkMesh) {
  const auto mesh = MakeMesh({{CARLA_TYPE_SIDEWALK, CARLA_AREA_SIDEWALK}});
  ASSERT_TRUE(mesh);
  Navigation navigation;
  ASSERT_TRUE(navigation.Load(Serialize(*mesh)));
  navigation.SetSeed(123);
  carla::geom::Location location;
  EXPECT_TRUE(navigation.GetRandomLocation(location));
  EXPECT_GE(location.x, 0.0f); EXPECT_LE(location.x, 10.0f);
  EXPECT_GE(location.y, 0.0f); EXPECT_LE(location.y, 10.0f);
}

TEST(NavigationRandomLocation, CrosswalkWithoutSidewalk) {
  const auto mesh = MakeMesh({{CARLA_TYPE_CROSSWALK, CARLA_AREA_CROSSWALK}});
  ASSERT_TRUE(mesh);
  Navigation navigation;
  ASSERT_TRUE(navigation.Load(Serialize(*mesh)));
  carla::geom::Location location;
  EXPECT_TRUE(navigation.GetRandomLocation(location));
}

TEST(NavigationRandomLocation, GrassWithoutSidewalk) {
  const auto mesh = MakeMesh({{CARLA_TYPE_GRASS, CARLA_AREA_GRASS}});
  ASSERT_TRUE(mesh);
  Navigation navigation;
  ASSERT_TRUE(navigation.Load(Serialize(*mesh)));
  carla::geom::Location location;
  EXPECT_TRUE(navigation.GetRandomLocation(location));
}

TEST(NavigationRandomLocation, RoadOnlyRejected) {
  const auto mesh = MakeMesh({{CARLA_TYPE_ROAD, CARLA_AREA_ROAD}});
  ASSERT_TRUE(mesh);
  Navigation navigation;
  ASSERT_TRUE(navigation.Load(Serialize(*mesh)));
  carla::geom::Location location;
  EXPECT_FALSE(navigation.GetRandomLocation(location));
}

TEST(NavigationRandomLocation, ExplicitFilterRemainsAuthoritative) {
  const auto mesh = MakeMesh({{CARLA_TYPE_ROAD, CARLA_AREA_ROAD}});
  ASSERT_TRUE(mesh);
  Navigation navigation;
  ASSERT_TRUE(navigation.Load(Serialize(*mesh)));
  dtQueryFilter filter;
  filter.setIncludeFlags(CARLA_TYPE_ROAD);
  filter.setExcludeFlags(CARLA_TYPE_NONE);
  carla::geom::Location location;
  EXPECT_TRUE(navigation.GetRandomLocation(location, &filter));
}

TEST(NavigationRandomLocation, TenthSuccessfulAttemptIsSuccess) {
  const auto mesh = MakeMesh({{CARLA_TYPE_SIDEWALK, CARLA_AREA_SIDEWALK},
                              {CARLA_TYPE_ROAD, CARLA_AREA_ROAD}});
  ASSERT_TRUE(mesh);
  const auto seed = SeedForTenthSuccess(*mesh);
  Navigation navigation;
  ASSERT_TRUE(navigation.Load(Serialize(*mesh)));
  navigation.SetSeed(seed);
  carla::geom::Location location(-1000.0f, -1000.0f, -1000.0f);
  EXPECT_TRUE(navigation.GetRandomLocation(location)) << "seed=" << seed;
  EXPECT_GE(location.x, 0.0f); EXPECT_LE(location.x, 10.0f);
}

TEST(NavigationRandomLocation, SidewalkPreferredToOtherWalkableAreas) {
  const auto mesh = MakeMesh({{CARLA_TYPE_SIDEWALK, CARLA_AREA_SIDEWALK},
                              {CARLA_TYPE_GRASS, CARLA_AREA_GRASS}});
  ASSERT_TRUE(mesh);
  Navigation navigation;
  ASSERT_TRUE(navigation.Load(Serialize(*mesh)));
  navigation.SetSeed(42);
  int successes = 0;
  for (int i = 0; i < 30; ++i) {
    carla::geom::Location location;
    if (navigation.GetRandomLocation(location)) {
      ++successes;
      EXPECT_GE(location.x, 0.0f);
      EXPECT_LE(location.x, 10.0f);
    }
  }
  EXPECT_GT(successes, 0);
}

TEST(NavigationRandomLocation, DefaultFilterRejectsRoadAndBlockedFlags) {
  for (unsigned short flags : {static_cast<unsigned short>(CARLA_TYPE_SIDEWALK | CARLA_TYPE_ROAD),
                               static_cast<unsigned short>(CARLA_TYPE_GRASS | CARLA_TYPE_NONE)}) {
    const auto mesh = MakeMesh({{flags, CARLA_AREA_GRASS}});
    ASSERT_TRUE(mesh);
    Navigation navigation;
    ASSERT_TRUE(navigation.Load(Serialize(*mesh)));
    carla::geom::Location location;
    EXPECT_FALSE(navigation.GetRandomLocation(location));
  }
}

TEST(NavigationRandomLocation, UnloadedNavigationDoesNotChangeOutput) {
  Navigation navigation;
  carla::geom::Location location(1.0f, 2.0f, 3.0f);
  EXPECT_FALSE(navigation.GetRandomLocation(location));
  EXPECT_EQ(location.x, 1.0f);
  EXPECT_EQ(location.y, 2.0f);
  EXPECT_EQ(location.z, 3.0f);
}
} // namespace
