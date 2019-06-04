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
  static const float AGENT_HEIGHT_HALF = AGENT_HEIGHT / 2.0f;

  // return a random float
  float frand() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  }

  Navigation::Navigation() {
  }

  Navigation::~Navigation() {
    dtFreeCrowd(_crowd);
    dtFreeNavMeshQuery(_navQuery);
    dtFreeNavMesh(_navMesh);
  }

  // load navigation data
  bool Navigation::Load(const std::string filename) {
    std::ifstream f;
    std::istream_iterator<uint8_t> start(f), end;

    // read the whole file
    f.open(filename, std::ios::binary);
    if (!f.is_open())
      return false;
    std::vector<uint8_t> content(start, end);
    f.close();

    // parse the content
    return Load(content);
  }

  // load navigation data from memory
  bool Navigation::Load(const std::vector<uint8_t> content) {
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
    unsigned long pos = 0;
    memcpy(&header, &content[pos], sizeof(header));
    pos += sizeof(header);

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
      memcpy(&tileHeader, &content[pos], sizeof(tileHeader));
      pos += sizeof(tileHeader);
      if (pos >= content.size()) {
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
      memcpy(data, &content[pos], static_cast<size_t>(tileHeader.dataSize));
      pos += static_cast<unsigned long>(tileHeader.dataSize);
      if (pos > content.size()) {
        dtFree(data);
        dtFreeNavMesh(mesh);
        return false;
      }

      // add the tile data
      mesh->addTile(reinterpret_cast<unsigned char*>(data), tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
    }

    // exchange
    dtFreeNavMesh(_navMesh);
    _navMesh = mesh;

    // prepare the query object
    dtFreeNavMeshQuery(_navQuery);
    _navQuery = dtAllocNavMeshQuery();
    _navQuery->init(_navMesh, 2048);

    // create and init the crowd manager
    CreateCrowd();

    // copy
    _binaryMesh = content;

    return true;
  }

  void Navigation::CreateCrowd(void) {

    if (_crowd != nullptr)
      return;

    // create and init
    _crowd = dtAllocCrowd();
    _crowd->init(MAX_AGENTS, AGENT_RADIUS, _navMesh);

    // make polygons with 'disabled' flag invalid
    _crowd->getEditableFilter(0)->setExcludeFlags(SAMPLE_POLYFLAGS_DISABLED);

    // Setup local avoidance params to different qualities.
    dtObstacleAvoidanceParams params;
    // Use mostly default settings, copy from dtCrowd.
    memcpy(&params, _crowd->getObstacleAvoidanceParams(0), sizeof(dtObstacleAvoidanceParams));

    // Low (11)
    params.velBias = 0.5f;
    params.adaptiveDivs = 5;
    params.adaptiveRings = 2;
    params.adaptiveDepth = 1;
    _crowd->setObstacleAvoidanceParams(0, &params);

    // Medium (22)
    params.velBias = 0.5f;
    params.adaptiveDivs = 5;
    params.adaptiveRings = 2;
    params.adaptiveDepth = 2;
    _crowd->setObstacleAvoidanceParams(1, &params);

    // Good (45)
    params.velBias = 0.5f;
    params.adaptiveDivs = 7;
    params.adaptiveRings = 2;
    params.adaptiveDepth = 3;
    _crowd->setObstacleAvoidanceParams(2, &params);

    // High (66)
    params.velBias = 0.5f;
    params.adaptiveDivs = 7;
    params.adaptiveRings = 3;
    params.adaptiveDepth = 3;

    _crowd->setObstacleAvoidanceParams(3, &params);
  }

  // return the path points to go from one position to another
  bool Navigation::GetPath(const carla::geom::Location from, const carla::geom::Location to, dtQueryFilter* filter, std::vector<carla::geom::Location> &path) {
    // path found
    float m_straightPath[MAX_POLYS*3];
    unsigned char m_straightPathFlags[MAX_POLYS];
    dtPolyRef m_straightPathPolys[MAX_POLYS];
    int m_nstraightPath;
    int m_straightPathOptions = 0;
    // polys in path
    dtPolyRef m_polys[MAX_POLYS];
    int m_npolys;

    // check to load the binary _navMesh from server
    if (_binaryMesh.size() == 0) {
      return false;
    }

    // point extension
    float m_polyPickExt[3];
    m_polyPickExt[0] = 2;
    m_polyPickExt[1] = 4;
    m_polyPickExt[2] = 2;

    // filter
    dtQueryFilter filter2;
    if (filter == nullptr) {
      filter2.setIncludeFlags(SAMPLE_POLYFLAGS_ALL ^ SAMPLE_POLYFLAGS_DISABLED);
      filter2.setExcludeFlags(0);
      filter = &filter2;
    }

  	// set the points
    dtPolyRef m_startRef = 0;
    dtPolyRef m_endRef = 0;
    float m_spos[3] = { from.x, from.z, from.y };
	  float m_epos[3] = { to.x, to.z, to.y };
    _navQuery->findNearestPoly(m_spos, m_polyPickExt, filter, &m_startRef, 0);
    _navQuery->findNearestPoly(m_epos, m_polyPickExt, filter, &m_endRef, 0);
    if (!m_startRef || !m_endRef) {
      return false;
    }

    // get the path of nodes
	  _navQuery->findPath(m_startRef, m_endRef, m_spos, m_epos, filter, m_polys, &m_npolys, MAX_POLYS);

    // get the path of points
    m_nstraightPath = 0;
    if (m_npolys == 0) {
      return false;
    }

    // in case of partial path, make sure the end point is clamped to the last polygon
    float epos[3];
    dtVcopy(epos, m_epos);
    if (m_polys[m_npolys-1] != m_endRef)
      _navQuery->closestPointOnPoly(m_polys[m_npolys-1], m_epos, epos, 0);

    // get the points
    _navQuery->findStraightPath(m_spos, epos, m_polys, m_npolys,
                                 m_straightPath, m_straightPathFlags,
                                 m_straightPathPolys, &m_nstraightPath, MAX_POLYS, m_straightPathOptions);

    // copy the path to the output buffer
    path.clear();
    path.reserve(static_cast<unsigned long>(m_nstraightPath));
    for (int i=0; i<m_nstraightPath*3; i+=3) {
      // export for Unreal axis (x, z, y)
      path.emplace_back(m_straightPath[i], m_straightPath[i+2], m_straightPath[i+1]);
    }

    return true;
  }

  // create a new walker but not yet add in crowd
  void Navigation::AddWalker(ActorId id) {
    // add to the queue to be added later and next tick where we can access the transform
    _walkersQueueToAdd.push_back(id);
  }

  // create a new walker in crowd
  bool Navigation::AddWalkerInCrowd(ActorId id, carla::geom::Location from) {
    dtCrowdAgentParams params;

    if (_crowd == nullptr) {
      return false;
    }

    // random location
    // GetRandomLocation(from);
    // logging::log("Nav: from ", from.x, from.y, from.z);

    // set from Unreal coordinates (and adjust center of walker, from middle to bottom)
    float y = from.y;
    from.y = from.z;
    from.z = y - AGENT_HEIGHT_HALF;

    // set parameters
    memset(&params, 0, sizeof(params));
    params.radius = AGENT_RADIUS;
    params.height = AGENT_HEIGHT;
    params.maxAcceleration = 8.0f;
    params.maxSpeed = 2.5f;
    params.collisionQueryRange = params.radius * 12.0f;
    params.pathOptimizationRange = params.radius * 30.0f;

    // flags
    params.updateFlags = 0;
    params.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
    params.updateFlags |= DT_CROWD_OPTIMIZE_VIS;
    params.updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
    params.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
    params.updateFlags |= DT_CROWD_SEPARATION;
    params.obstacleAvoidanceType = 3;
    params.separationWeight = 0.1f;

    // add walker
    float PointFrom[3] = { from.x, from.y, from.z };
    int index = _crowd->addAgent(PointFrom, &params);
    if (index == -1) {
      return false;
    }

    // save the id
    _mappedId[id] = index;

    return true;
  }

  // set a new target point to go
  bool Navigation::SetWalkerTarget(ActorId id, carla::geom::Location to) {
    // get the internal index
    auto it = _mappedId.find(id);
    if (it == _mappedId.end())
      return false;

    return SetWalkerTargetIndex(it->second, to);
  }

  // set a new target point to go
  bool Navigation::SetWalkerTargetIndex(int index, carla::geom::Location to) {
    if (index == -1)
      return false;

    // set target position
    float pointTo[3] = { to.x, to.z, to.y };
    float nearest[3];
    const dtQueryFilter *filter = _crowd->getFilter(0);
    dtPolyRef targetRef;
    _navQuery->findNearestPoly(pointTo, _crowd->getQueryHalfExtents(), filter, &targetRef, nearest);
    if (!targetRef)
      return false;

    return _crowd->requestMoveTarget(index, targetRef, pointTo);
  }

  // update all walkers in crowd
  void Navigation::UpdateCrowd(const client::detail::EpisodeState &state) {

    if (!_navMesh || !_crowd) {
      return;
    }

    // force single thread running this
    std::lock_guard<std::mutex> lock(_mutex);

    // check if we have more walkers in the queue to add
    while (!_walkersQueueToAdd.empty()) {
      // add it
      ActorId id = _walkersQueueToAdd.back();
      carla::geom::Transform trans = state.GetActorState(id).transform;
      if (!AddWalkerInCrowd(id, trans.location)) {
        break;
      }
      // remove it
      _walkersQueueToAdd.pop_back();
    }

    // update all
    double deltaTime = state.GetTimestamp().delta_seconds;
    _crowd->update(static_cast<float>(deltaTime), nullptr);

    // check if walker has finished
    for (int i = 0; i<_crowd->getAgentCount(); ++i)
    {
      const dtCrowdAgent* ag = _crowd->getAgent(i);
      if (!ag->active)
        continue;

      // check distance to the target point
      const float *end = &ag->cornerVerts[(ag->ncorners-1)*3];
      carla::geom::Vector3D dist(end[0] - ag->npos[0], end[1] - ag->npos[1], end[2] - ag->npos[2]);
      if (dist.SquaredLength() <= 2) {
        // set a new random target
        carla::geom::Location location;
        GetRandomLocation(location);
        SetWalkerTargetIndex(i, location);
      }
    }
  }

  // get the walker current transform
  bool Navigation::GetWalkerTransform(ActorId id, carla::geom::Transform &trans) {
    // get the internal index
    auto it = _mappedId.find(id);
    if (it == _mappedId.end())
      return false;

    // get the index found
    int index = it->second;
    if (index == -1)
      return false;

    // get the walker
    const dtCrowdAgent *agent = _crowd->getAgent(index);

    if (!agent->active) {
      return false;
    }

    // set its position in Unreal coordinates
    trans.location.x = agent->npos[0];
    trans.location.y = agent->npos[2];
    trans.location.z = agent->npos[1] + AGENT_HEIGHT_HALF;
    // set its rotation
    trans.rotation.pitch = 0;
    trans.rotation.yaw = atan2f(agent->vel[2] , agent->vel[0]) * (180.0f / 3.14159265f);
    trans.rotation.roll = 0;

    return true;
  }

  float Navigation::GetWalkerSpeed(ActorId id) {
    // get the internal index
    auto it = _mappedId.find(id);
    if (it == _mappedId.end()) {
      return false;
    }

    // get the index found
    int index = it->second;
    if (index == -1) {
      return false;
    }

    // get the walker
    const dtCrowdAgent *agent = _crowd->getAgent(index);
    return sqrt(agent->vel[0] * agent->vel[0] + agent->vel[1] * agent->vel[1] + agent->vel[2] *
        agent->vel[2]);
  }

  // get a random location for navigation
  bool Navigation::GetRandomLocation(carla::geom::Location &location, dtQueryFilter *filter) {
    DEBUG_ASSERT(_navQuery != nullptr);

    // filter
    dtQueryFilter filter2;
    if (filter == nullptr) {
      filter2.setIncludeFlags(SAMPLE_POLYFLAGS_ALL ^ SAMPLE_POLYFLAGS_DISABLED);
      filter2.setExcludeFlags(0);
      filter = &filter2;
    }

    // search
    dtPolyRef randomRef { 0 };
    float point[3] { 0.0f, 0.0f, 0.0f };

    dtStatus status = _navQuery->findRandomPoint(filter, &frand, &randomRef, point);

    if (status == DT_SUCCESS) {
      // set the location in Unreal coords
      location.x = point[0];
      location.y = point[2];
      location.z = point[1];
      return true;
    }

    return false;
  }

} // namespace nav
} // namespace carla
