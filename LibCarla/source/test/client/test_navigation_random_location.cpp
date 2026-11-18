// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/nav/Navigation.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
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

Mesh MakeMesh(const std::vector<TileSpec> &tiles, const std::array<float, 3> &origin = {}) {
  Mesh mesh(dtAllocNavMesh());
  if (!mesh) return {};
  dtNavMeshParams mesh_params{};
  std::copy(origin.begin(), origin.end(), mesh_params.orig);
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
    params.bmin[0] = origin[0] + static_cast<float>(index) * 10.0f;
    params.bmin[1] = origin[1];
    params.bmin[2] = origin[2];
    params.bmax[0] = params.bmin[0] + 10.0f;
    params.bmax[1] = origin[1] + 2.0f;
    params.bmax[2] = origin[2] + 10.0f;
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

TEST(NavigationBinaryLoad, ValidFilePreservesWhitespaceBytes) {
  // The geometry origin and tile width naturally encode binary whitespace.
  const auto mesh = MakeMesh({{CARLA_TYPE_SIDEWALK, CARLA_AREA_SIDEWALK}},
                            {8.5625f, 8.625f, 8.8125f});
  ASSERT_TRUE(mesh);
  const auto bytes = Serialize(*mesh);
  for (uint8_t value : {0x09, 0x0a, 0x0d, 0x20}) {
    EXPECT_NE(std::find(bytes.begin(), bytes.end(), value), bytes.end());
  }
  const auto filename = std::filesystem::path(::testing::TempDir()) /
      ("carla navigation " + std::to_string(
          std::chrono::steady_clock::now().time_since_epoch().count()) + ".bin");
  struct FileCleanup {
    std::filesystem::path path;
    ~FileCleanup() {
      std::error_code error;
      std::filesystem::remove(path, error);
    }
  } cleanup{filename};
  {
    std::ofstream output(filename, std::ios::binary);
    ASSERT_TRUE(output.is_open());
    output.write(reinterpret_cast<const char *>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
    ASSERT_TRUE(output.good());
  }
  std::ifstream input(filename, std::ios::binary);
  const std::vector<uint8_t> read_back((std::istreambuf_iterator<char>(input)),
                                      std::istreambuf_iterator<char>());
  ASSERT_EQ(read_back, bytes);
  Navigation memory_navigation;
  ASSERT_TRUE(memory_navigation.Load(bytes));
  memory_navigation.SetSeed(17);
  carla::geom::Location memory_location;
  ASSERT_TRUE(memory_navigation.GetRandomLocation(memory_location));
  Navigation file_navigation;
  ASSERT_TRUE(file_navigation.Load(filename.string()));
  file_navigation.SetSeed(17);
  carla::geom::Location file_location;
  ASSERT_TRUE(file_navigation.GetRandomLocation(file_location));
  EXPECT_FLOAT_EQ(file_location.x, memory_location.x);
  EXPECT_FLOAT_EQ(file_location.y, memory_location.y);
  EXPECT_FLOAT_EQ(file_location.z, memory_location.z);
}

TEST(NavigationRandomLocation, UnusableSidewalkDoesNotSuppressGrassFallback) {
  const auto mesh = MakeMesh({{static_cast<unsigned short>(CARLA_TYPE_SIDEWALK | CARLA_TYPE_ROAD),
                               CARLA_AREA_SIDEWALK},
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
      EXPECT_GE(location.x, 10.0f);
      EXPECT_LE(location.x, 20.0f);
    }
  }
  EXPECT_GT(successes, 0);
}

} // namespace
