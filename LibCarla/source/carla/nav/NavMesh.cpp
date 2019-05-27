// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Logging.h"
#include "carla/nav/NavMesh.h"

#include <iostream>
#include <iterator>
#include <fstream>

namespace carla {
namespace nav {

  static const int MAX_POLYS = 256;

  NavMesh::NavMesh() {
    // const FString FilePath = FPaths::ProjectContentDir() + TEXT("Carla/Maps/Nav/") + MapName + TEXT(".bin");
    // Load(FilePath);
    // carla::geom::Location a, b;

    // a.x = -141.788834;
    // a.y = 0.139954;
    // a.z = -153.839020;
    // b.x = -29.750393;
    // b.y = 0.270432;
    // b.z = -26.876369;
    // if (GetPath(a, b, nullptr, Path)) {
    //   logging::log("NAV: Path %d (%f,%f,%f)", Path.size, Path[0].x, Path[0].y, Path[0].z);
    // }
    // allocate object

    // dtNavMesh *mesh = dtAllocNavMesh();
    // DEBUG_ASSERT(mesh != nullptr);
    // // DEBUG_ASSERT(false);
    // int i = mesh->getMaxTiles();
    // logging::log("NAV: %d", i);
    // i += 1;
    // logging::log("NAV: %d", i);
    // dtFreeNavMesh(mesh);
  }

  // load navigation data
  bool NavMesh::Load(const std::string filename) {
    std::ifstream f;
    std::istream_iterator<uint8_t> start(f), end;

    // read the whole file
    f.open(filename, std::ios::binary);
    if (!f.is_open())
      return false;
    std::vector<uint8_t> Content(start, end);
    f.close();

    // parse the content
    return Load(Content);
  }

  // load navigation data from memory
  bool NavMesh::Load(const std::vector<uint8_t> Content) {
    const int NAVMESHSET_MAGIC = 'M'<<24 | 'S'<<16 | 'E'<<8 | 'T'; //'MSET';
    const int NAVMESHSET_VERSION = 1;
    #pragma pack(push, 1)
    struct NavMeshSetHeader {
      int magic;
      int version;
      int numTiles;
      dtNavMeshParams params;
    } header;
    struct NavMeshTileHeader {
      dtTileRef tileRef;
      int dataSize;
    };
    #pragma pack(pop)

    // read the file header
    // f.read(reinterpret_cast<char *>(&header), sizeof(header));
    unsigned long Pos = 0;
    memcpy(&header, &Content[Pos], sizeof(header));
    Pos += sizeof(header);

    // check file magic and version
    if (header.magic != NAVMESHSET_MAGIC || header.version != NAVMESHSET_VERSION) {
      return false;
    }

    // allocate object
    dtNavMesh* mesh = dtAllocNavMesh();
    // set number of tiles and origin
    dtStatus status = mesh->init(&header.params);
    if (dtStatusFailed(status)) {
      return false;
    }

    // read the tiles data
    for (int i = 0; i < header.numTiles; ++i) {
      NavMeshTileHeader tileHeader;

      // read the tile header
      // f.read(reinterpret_cast<char *>(&tileHeader), sizeof(tileHeader));
      memcpy(&tileHeader, &Content[Pos], sizeof(tileHeader));
      Pos += sizeof(tileHeader);
      if (Pos >= Content.size()) {
        dtFreeNavMesh(mesh);
        return false;
      }

      // check for valid tile
      if (!tileHeader.tileRef || !tileHeader.dataSize)
        break;

      // allocate the buffer
      char* data = static_cast<char*>(dtAlloc(static_cast<size_t>(tileHeader.dataSize), DT_ALLOC_PERM));
      if (!data) break;

      // read the tile
      memset(data, 0, static_cast<size_t>(tileHeader.dataSize));
      // f.read(data, tileHeader.dataSize);
      memcpy(data, &Content[Pos], static_cast<size_t>(tileHeader.dataSize));
      Pos += static_cast<unsigned long>(tileHeader.dataSize);
      if (Pos >= Content.size()) {
        dtFree(data);
        dtFreeNavMesh(mesh);
        return false;
      }

      // add the tile data
      mesh->addTile(reinterpret_cast<unsigned char*>(data), tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
    }

    // exchange
    dtFreeNavMesh(m_navMesh);
    m_navMesh = mesh;

    // prepare the query object
    dtFreeNavMeshQuery(m_navQuery);
    m_navQuery = dtAllocNavMeshQuery();
    m_navQuery->init(m_navMesh, 2048);

    return true;
  }

  // return the path points to go from one position to another
  bool NavMesh::GetPath(const carla::geom::Location from, const carla::geom::Location to, dtQueryFilter* filter, std::vector<carla::geom::Location> path) {
    // path found
    float m_straightPath[MAX_POLYS*3];
    unsigned char m_straightPathFlags[MAX_POLYS];
    dtPolyRef m_straightPathPolys[MAX_POLYS];
    int m_nstraightPath;
    int m_straightPathOptions = 0;
    // polys in path
    dtPolyRef m_polys[MAX_POLYS];
    int m_npolys;

    // point extension
    float m_polyPickExt[3];
    m_polyPickExt[0] = 2;
    m_polyPickExt[1] = 4;
    m_polyPickExt[2] = 2;

    // filter
    dtQueryFilter m_filter;
    if (filter == nullptr) {
      m_filter.setIncludeFlags(SAMPLE_POLYFLAGS_ALL ^ SAMPLE_POLYFLAGS_DISABLED);
      m_filter.setExcludeFlags(0);
      filter = &m_filter;
    }

  	// set the points
    dtPolyRef m_startRef = 0;
    dtPolyRef m_endRef = 0;
    float m_spos[3] = { from.x, from.z, from.y };
	  float m_epos[3] = { to.x, to.z, to.y };
    m_navQuery->findNearestPoly(m_spos, m_polyPickExt, filter, &m_startRef, 0);
    m_navQuery->findNearestPoly(m_epos, m_polyPickExt, filter, &m_endRef, 0);
    if (!m_startRef || !m_endRef) {
        return false;
    }

    // get the path of nodes
	  m_navQuery->findPath(m_startRef, m_endRef, m_spos, m_epos, filter, m_polys, &m_npolys, MAX_POLYS);

    // get the path of points
    m_nstraightPath = 0;
    if (m_npolys == 0) {
      return false;
    }

    // in case of partial path, make sure the end point is clamped to the last polygon
    float epos[3];
    dtVcopy(epos, m_epos);
    if (m_polys[m_npolys-1] != m_endRef)
      m_navQuery->closestPointOnPoly(m_polys[m_npolys-1], m_epos, epos, 0);

    // get the points
    m_navQuery->findStraightPath(m_spos, epos, m_polys, m_npolys,
                                 m_straightPath, m_straightPathFlags,
                                 m_straightPathPolys, &m_nstraightPath, MAX_POLYS, m_straightPathOptions);

    // copy the path to the output buffer
    path.clear();
    path.reserve(static_cast<unsigned long>(m_nstraightPath));
    for (int i=0; i<m_nstraightPath*3; i+=3) {
      // export for Unreal axis (x, z, y)
      // path.push_back(carla::geom::Location(m_straightPath[i], m_straightPath[i+2], m_straightPath[i+1]));
      path.emplace_back(m_straightPath[i], m_straightPath[i+2], m_straightPath[i+1]);
      // std::cout << "(" << m_straightPath[i] << ", " << m_straightPath[i+2] << ", " << m_straightPath[i+1] << "), " << std::endl;
    }

    return true;
  }

} // namespace nav
} // namespace carla
