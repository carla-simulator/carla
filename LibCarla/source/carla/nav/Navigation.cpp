// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Logging.h"
#include "carla/nav/Navigation.h"

#include <iostream>
#include <iterator>
#include <fstream>

namespace carla {
namespace nav {

  static const int MAX_POLYS = 256;
  static const int MAX_AGENTS = 128;
  static const float AGENT_RADIUS = 0.6f;
  static const float AGENT_HEIGHT = 2.0f;

  Navigation::Navigation() {
  }

  Navigation::~Navigation() {
    dtFreeCrowd(Crowd);
    dtFreeNavMeshQuery(NavQuery);
    dtFreeNavMesh(NavMesh);
  }

  // load navigation data
  bool Navigation::Load(const std::string filename) {
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
  bool Navigation::Load(const std::vector<uint8_t> Content) {
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
      memcpy(data, &Content[Pos], static_cast<size_t>(tileHeader.dataSize));
      Pos += static_cast<unsigned long>(tileHeader.dataSize);
      if (Pos > Content.size()) {
        dtFree(data);
        dtFreeNavMesh(mesh);
        return false;
      }

      // add the tile data
      mesh->addTile(reinterpret_cast<unsigned char*>(data), tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
    }

    // exchange
    dtFreeNavMesh(NavMesh);
    NavMesh = mesh;

    // prepare the query object
    dtFreeNavMeshQuery(NavQuery);
    NavQuery = dtAllocNavMeshQuery();
    NavQuery->init(NavMesh, 2048);

    // create and init the crowd manager
    CreateCrowd();

    // copy
    BinaryMesh = Content;

    return true;
  }

  void Navigation::CreateCrowd(void) {

    if (Crowd != nullptr)
      return;

    // create and init
    Crowd = dtAllocCrowd();
    Crowd->init(MAX_AGENTS, AGENT_RADIUS, NavMesh);

    // make polygons with 'disabled' flag invalid
    Crowd->getEditableFilter(0)->setExcludeFlags(SAMPLE_POLYFLAGS_DISABLED);

    // Setup local avoidance params to different qualities.
    dtObstacleAvoidanceParams Params;
    // Use mostly default settings, copy from dtCrowd.
    memcpy(&Params, Crowd->getObstacleAvoidanceParams(0), sizeof(dtObstacleAvoidanceParams));

    // Low (11)
    Params.velBias = 0.5f;
    Params.adaptiveDivs = 5;
    Params.adaptiveRings = 2;
    Params.adaptiveDepth = 1;
    Crowd->setObstacleAvoidanceParams(0, &Params);

    // Medium (22)
    Params.velBias = 0.5f;
    Params.adaptiveDivs = 5;
    Params.adaptiveRings = 2;
    Params.adaptiveDepth = 2;
    Crowd->setObstacleAvoidanceParams(1, &Params);

    // Good (45)
    Params.velBias = 0.5f;
    Params.adaptiveDivs = 7;
    Params.adaptiveRings = 2;
    Params.adaptiveDepth = 3;
    Crowd->setObstacleAvoidanceParams(2, &Params);

    // High (66)
    Params.velBias = 0.5f;
    Params.adaptiveDivs = 7;
    Params.adaptiveRings = 3;
    Params.adaptiveDepth = 3;

    Crowd->setObstacleAvoidanceParams(3, &Params);
  }

  // return the path points to go from one position to another
  bool Navigation::GetPath(const carla::geom::Location From, const carla::geom::Location To, dtQueryFilter* Filter, std::vector<carla::geom::Location> &Path) {
    // path found
    float m_straightPath[MAX_POLYS*3];
    unsigned char m_straightPathFlags[MAX_POLYS];
    dtPolyRef m_straightPathPolys[MAX_POLYS];
    int m_nstraightPath;
    int m_straightPathOptions = 0;
    // polys in path
    dtPolyRef m_polys[MAX_POLYS];
    int m_npolys;

    // check to load the binary navmesh from server
    logging::log("Nav: ", BinaryMesh.size());
    if (BinaryMesh.size() == 0) {
      return false;
    }

    // point extension
    float m_polyPickExt[3];
    m_polyPickExt[0] = 2;
    m_polyPickExt[1] = 4;
    m_polyPickExt[2] = 2;

    // filter
    dtQueryFilter m_filter;
    if (Filter == nullptr) {
      m_filter.setIncludeFlags(SAMPLE_POLYFLAGS_ALL ^ SAMPLE_POLYFLAGS_DISABLED);
      m_filter.setExcludeFlags(0);
      Filter = &m_filter;
    }

  	// set the points
    dtPolyRef m_startRef = 0;
    dtPolyRef m_endRef = 0;
    float m_spos[3] = { From.x, From.z, From.y };
	  float m_epos[3] = { To.x, To.z, To.y };
    NavQuery->findNearestPoly(m_spos, m_polyPickExt, Filter, &m_startRef, 0);
    NavQuery->findNearestPoly(m_epos, m_polyPickExt, Filter, &m_endRef, 0);
    if (!m_startRef || !m_endRef) {
      return false;
    }

    // get the path of nodes
	  NavQuery->findPath(m_startRef, m_endRef, m_spos, m_epos, Filter, m_polys, &m_npolys, MAX_POLYS);

    // get the path of points
    m_nstraightPath = 0;
    if (m_npolys == 0) {
      return false;
    }

    // in case of partial path, make sure the end point is clamped to the last polygon
    float epos[3];
    dtVcopy(epos, m_epos);
    if (m_polys[m_npolys-1] != m_endRef)
      NavQuery->closestPointOnPoly(m_polys[m_npolys-1], m_epos, epos, 0);

    // get the points
    NavQuery->findStraightPath(m_spos, epos, m_polys, m_npolys,
                                 m_straightPath, m_straightPathFlags,
                                 m_straightPathPolys, &m_nstraightPath, MAX_POLYS, m_straightPathOptions);

    // copy the path to the output buffer
    Path.clear();
    Path.reserve(static_cast<unsigned long>(m_nstraightPath));
    for (int i=0; i<m_nstraightPath*3; i+=3) {
      // export for Unreal axis (x, z, y)
      Path.emplace_back(m_straightPath[i], m_straightPath[i+2], m_straightPath[i+1]);
    }

    return true;
  }

  // create a new walker
  bool Navigation::AddWalker(ActorId Id, carla::geom::Location From) {
    dtCrowdAgentParams Params;

    if (Crowd == nullptr) {
      return false;
    }

    // set parameters
    memset(&Params, 0, sizeof(Params));
    Params.radius = AGENT_RADIUS;
    Params.height = AGENT_HEIGHT;
    Params.maxAcceleration = 8.0f;
    Params.maxSpeed = 3.5f;
    Params.collisionQueryRange = Params.radius * 12.0f;
    Params.pathOptimizationRange = Params.radius * 30.0f;

    // flags
    Params.updateFlags = 0;
    Params.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
    Params.updateFlags |= DT_CROWD_OPTIMIZE_VIS;
    Params.updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
    Params.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
    Params.updateFlags |= DT_CROWD_SEPARATION;
    Params.obstacleAvoidanceType = 3;
    Params.separationWeight = 2.0;

    // add walker
    float PointFrom[3] = { From.x, From.y, From.z };
    int Index = Crowd->addAgent(PointFrom, &Params);
    if (Index != -1) {
      // save the id
      MappedId[Id] = Index;
    }

    return true;
  }

  // set a new target point to go
  bool Navigation::SetWalkerTarget(ActorId Id, carla::geom::Location To) {
    // get the internal index
    auto It = MappedId.find(Id);
    if (It == MappedId.end())
      return false;

    // get the index found
    int Index = It->second;
    if (Index != -1) {

      // set target position
      float PointTo[3] = { To.x, To.y, To.z };
      float Nearest[3];
      const dtQueryFilter *Filter = Crowd->getFilter(0);
      dtPolyRef TargetRef;
      NavQuery->findNearestPoly(PointTo, Crowd->getQueryHalfExtents(), Filter, &TargetRef, Nearest);
      if (TargetRef)
        Crowd->requestMoveTarget(Index, TargetRef, PointTo);
    }

    return true;
  }

  // update all walkers in crowd
  void Navigation::UpdateCrowd(float DeltaTime) {

    if (!NavMesh || !Crowd) {
      return;
    }

    // update all
    Crowd->update(DeltaTime, nullptr);

    // get all walker positions
    for (int i = 0; i < Crowd->getAgentCount(); ++i)
    {
      const dtCrowdAgent* ag = Crowd->getAgent(i);
      if (!ag->active)
        continue;

      // Update agent movement trail.
      // AgentTrail* trail = &m_trails[i];
      // trail->htrail = (trail->htrail + 1) % AGENT_MAX_TRAIL;
      // dtVcopy(&trail->trail[trail->htrail*3], ag->npos);
    }
  }

} // namespace nav
} // namespace carla
