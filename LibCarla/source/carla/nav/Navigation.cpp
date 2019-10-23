// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#define _USE_MATH_DEFINES // to avoid undefined error of M_PI (bug in Visual
                          // Studio 2015 and 2017)
#include <cmath>

#include "carla/Logging.h"
#include "carla/nav/Navigation.h"
#include "carla/nav/WalkerManager.h"
#include "carla/geom/Math.h"

#include <iterator>
#include <fstream>
#include <mutex>

namespace carla {
namespace nav {

  enum SamplePolyFlags {
    SAMPLE_POLYFLAGS_WALK       = 0x01,   // Ability to walk (ground, grass,
                                          // road)
    SAMPLE_POLYFLAGS_SWIM       = 0x02,   // Ability to swim (water).
    SAMPLE_POLYFLAGS_DOOR       = 0x04,   // Ability to move through doors.
    SAMPLE_POLYFLAGS_JUMP       = 0x08,   // Ability to jump.
    SAMPLE_POLYFLAGS_DISABLED   = 0x10,   // Disabled polygon
  	SAMPLE_POLYFLAGS_CROSS      = 0x20,		// Ability to cross through road
    SAMPLE_POLYFLAGS_ALL        = 0xffff  // All abilities.
  };

  enum UpdateFlags {
    DT_CROWD_ANTICIPATE_TURNS   = 1,
    DT_CROWD_OBSTACLE_AVOIDANCE = 2,
    DT_CROWD_SEPARATION         = 4,
    DT_CROWD_OPTIMIZE_VIS       = 8, ///< Use
                                     ///< #dtPathCorridor::optimizePathVisibility()
                                     ///< to optimize the agent path.
    DT_CROWD_OPTIMIZE_TOPO      = 16, ///< Use
                                      ///< dtPathCorridor::optimizePathTopology()
                                      ///< to optimize the agent path.
  };

  static const int MAX_POLYS = 256;
  static const int MAX_AGENTS = 500;
  static const int MAX_QUERY_SEARCH_NODES = 2048;
  static const float AGENT_HEIGHT = 1.8f;
  static const float AGENT_RADIUS = 0.4f;

  static const float AGENT_UNBLOCK_DISTANCE = 0.5f;
  static const float AGENT_UNBLOCK_DISTANCE_SQUARED = AGENT_UNBLOCK_DISTANCE * AGENT_UNBLOCK_DISTANCE;
  static const float AGENT_UNBLOCK_TIME = 4.0f;

  static const float AREA_ROAD_COST = 10.0f;

  // return a random float
  static float frand() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  }

  Navigation::Navigation() {
    // assign walker manager
    _walkerManager.SetNav(this);
  }

  Navigation::~Navigation() {
    _ready = false;
    _timeToUnblock = 0.0f;
    _mappedWalkersId.clear();
    _mappedVehiclesId.clear();
    _mappedByIndex.clear();
    _walkersBlockedPosition.clear();
    _yaw_walkers.clear();
    _binaryMesh.clear();
    dtFreeCrowd(_crowd);
    dtFreeNavMeshQuery(_navQuery);
    dtFreeNavMesh(_navMesh);
  }

  // load navigation data
  bool Navigation::Load(const std::string &filename) {
    std::ifstream f;
    std::istream_iterator<uint8_t> start(f), end;

    // read the whole file
    f.open(filename, std::ios::binary);
    if (!f.is_open()) {
      return false;
    }
    std::vector<uint8_t> content(start, end);
    f.close();

    // parse the content
    return Load(std::move(content));
  }

  // load navigation data from memory
  bool Navigation::Load(std::vector<uint8_t> content) {
    const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; // 'MSET';
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

    // check size for header
    if (content.size() < sizeof(header)) {
      logging::log("Nav: failed loading binary");
      return false;
    }

    // read the file header
    unsigned long pos = 0;
    memcpy(&header, &content[pos], sizeof(header));
    pos += sizeof(header);

    // check file magic and version
    if (header.magic != NAVMESHSET_MAGIC || header.version != NAVMESHSET_VERSION) {
      return false;
    }

    // allocate object
    dtNavMesh *mesh = dtAllocNavMesh();
    if (!mesh) {
      return false;
    }

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
      if (!tileHeader.tileRef || !tileHeader.dataSize) {
        break;
      }

      // allocate the buffer
      char *data = static_cast<char *>(dtAlloc(static_cast<size_t>(tileHeader.dataSize), DT_ALLOC_PERM));
      if (!data) {
        break;
      }

      // read the tile
      memcpy(data, &content[pos], static_cast<size_t>(tileHeader.dataSize));
      pos += static_cast<unsigned long>(tileHeader.dataSize);
      if (pos > content.size()) {
        dtFree(data);
        dtFreeNavMesh(mesh);
        return false;
      }

      // add the tile data
      mesh->addTile(reinterpret_cast<unsigned char *>(data), tileHeader.dataSize, DT_TILE_FREE_DATA,
      tileHeader.tileRef, 0);
    }

    // exchange
    dtFreeNavMesh(_navMesh);
    _navMesh = mesh;

    // prepare the query object
    dtFreeNavMeshQuery(_navQuery);
    _navQuery = dtAllocNavMeshQuery();
    _navQuery->init(_navMesh, MAX_QUERY_SEARCH_NODES);

    // copy
    _binaryMesh = std::move(content);
    _ready = true;

    // create and init the crowd manager
    CreateCrowd();

    return true;
  }

  void Navigation::CreateCrowd(void) {

    // check if all is ready
    if (!_ready) {
      return;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // create and init
    _crowd = dtAllocCrowd();
    if (!_crowd->init(MAX_AGENTS, AGENT_RADIUS * 20, _navMesh)) {
      logging::log("Nav: failed to create crowd");
      return;
    }

    // set different filters
    // filter 0 can not cross roads
    _crowd->getEditableFilter(0)->setIncludeFlags(SAMPLE_POLYFLAGS_ALL ^ SAMPLE_POLYFLAGS_DISABLED);
    _crowd->getEditableFilter(0)->setExcludeFlags(SAMPLE_POLYFLAGS_DISABLED | SAMPLE_POLYFLAGS_CROSS);
    _crowd->getEditableFilter(0)->setAreaCost(SAMPLE_POLYAREA_ROAD, AREA_ROAD_COST);
    // filter 1 can cross roads
    _crowd->getEditableFilter(1)->setIncludeFlags(SAMPLE_POLYFLAGS_ALL ^ SAMPLE_POLYFLAGS_DISABLED);
    _crowd->getEditableFilter(1)->setExcludeFlags(SAMPLE_POLYFLAGS_DISABLED);
    _crowd->getEditableFilter(1)->setAreaCost(SAMPLE_POLYAREA_ROAD, AREA_ROAD_COST);

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
  bool Navigation::GetPath(carla::geom::Location from,
                           carla::geom::Location to,
                           dtQueryFilter * filter,
                           std::vector<carla::geom::Location> &path,
                           std::vector<unsigned char> &area) {
    // path found
    float m_straightPath[MAX_POLYS * 3];
    unsigned char m_straightPathFlags[MAX_POLYS];
    dtPolyRef m_straightPathPolys[MAX_POLYS];
    int m_nstraightPath;
    int m_straightPathOptions = DT_STRAIGHTPATH_AREA_CROSSINGS;

    // polys in path
    dtPolyRef m_polys[MAX_POLYS];
    int m_npolys;

    // check if all is ready
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_navQuery != nullptr);

    // point extension
    float m_polyPickExt[3];
    m_polyPickExt[0] = 2;
    m_polyPickExt[1] = 4;
    m_polyPickExt[2] = 2;

    // filter
    dtQueryFilter filter2;
    if (filter == nullptr) {
      filter2.setAreaCost(SAMPLE_POLYAREA_ROAD, AREA_ROAD_COST);
      filter2.setIncludeFlags(SAMPLE_POLYFLAGS_ALL ^ SAMPLE_POLYFLAGS_DISABLED);
      filter2.setExcludeFlags(SAMPLE_POLYFLAGS_DISABLED);
      filter = &filter2;
    }

    // set the points
    dtPolyRef m_startRef = 0;
    dtPolyRef m_endRef = 0;
    float m_spos[3] = { from.x, from.z, from.y };
    float m_epos[3] = { to.x, to.z, to.y };
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _navQuery->findNearestPoly(m_spos, m_polyPickExt, filter, &m_startRef, 0);
      _navQuery->findNearestPoly(m_epos, m_polyPickExt, filter, &m_endRef, 0);
    }
    if (!m_startRef || !m_endRef) {
      return false;
    }

    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      // get the path of nodes
      _navQuery->findPath(m_startRef, m_endRef, m_spos, m_epos, filter, m_polys, &m_npolys, MAX_POLYS);
    }

    // get the path of points
    m_nstraightPath = 0;
    if (m_npolys == 0) {
      return false;
    }

    // in case of partial path, make sure the end point is clamped to the last
    // polygon
    float epos[3];
    dtVcopy(epos, m_epos);
    if (m_polys[m_npolys - 1] != m_endRef) {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _navQuery->closestPointOnPoly(m_polys[m_npolys - 1], m_epos, epos, 0);
    }

    // get the points
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _navQuery->findStraightPath(m_spos, epos, m_polys, m_npolys,
      m_straightPath, m_straightPathFlags,
      m_straightPathPolys, &m_nstraightPath, MAX_POLYS, m_straightPathOptions);
    }

    // copy the path to the output buffer
    path.clear();
    path.reserve(static_cast<unsigned long>(m_nstraightPath));
    unsigned char areaType;
    for (int i = 0, j = 0; j < m_nstraightPath; i += 3, ++j) {
      // save coordinate for Unreal axis (x, z, y)
      path.emplace_back(m_straightPath[i], m_straightPath[i + 2], m_straightPath[i + 1]);
      // save area type
      {
        // critical section, force single thread running this
        std::lock_guard<std::mutex> lock(_mutex);
        _navMesh->getPolyArea(m_straightPathPolys[j], &areaType);
      }
      area.emplace_back(areaType);
    }

    return true;
  }

  bool Navigation::GetAgentRoute(ActorId id, carla::geom::Location from, carla::geom::Location to,
  std::vector<carla::geom::Location> &path, std::vector<unsigned char> &area) {
    // path found
    float m_straightPath[MAX_POLYS * 3];
    unsigned char m_straightPathFlags[MAX_POLYS];
    dtPolyRef m_straightPathPolys[MAX_POLYS];
    int m_nstraightPath;
    int m_straightPathOptions = DT_STRAIGHTPATH_AREA_CROSSINGS;

    // polys in path
    dtPolyRef m_polys[MAX_POLYS];
    int m_npolys;

    // check if all is ready
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_navQuery != nullptr);

    // point extension
    float m_polyPickExt[3];
    m_polyPickExt[0] = 2;
    m_polyPickExt[1] = 4;
    m_polyPickExt[2] = 2;

    // get current filter from agent
    auto it = _mappedWalkersId.find(id);
    if (it == _mappedWalkersId.end())
      return false;

    const dtQueryFilter *filter;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      filter = _crowd->getFilter(_crowd->getAgent(it->second)->params.queryFilterType);
    }

    // set the points
    dtPolyRef m_startRef = 0;
    dtPolyRef m_endRef = 0;
    float m_spos[3] = { from.x, from.z, from.y };
    float m_epos[3] = { to.x, to.z, to.y };
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _navQuery->findNearestPoly(m_spos, m_polyPickExt, filter, &m_startRef, 0);
      _navQuery->findNearestPoly(m_epos, m_polyPickExt, filter, &m_endRef, 0);
    }
    if (!m_startRef || !m_endRef) {
      return false;
    }

    // get the path of nodes
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _navQuery->findPath(m_startRef, m_endRef, m_spos, m_epos, filter, m_polys, &m_npolys, MAX_POLYS);
    }

    // get the path of points
    m_nstraightPath = 0;
    if (m_npolys == 0) {
      return false;
    }

    // in case of partial path, make sure the end point is clamped to the last
    // polygon
    float epos[3];
    dtVcopy(epos, m_epos);
    if (m_polys[m_npolys - 1] != m_endRef) {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _navQuery->closestPointOnPoly(m_polys[m_npolys - 1], m_epos, epos, 0);
    }

    // get the points
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _navQuery->findStraightPath(m_spos, epos, m_polys, m_npolys,
      m_straightPath, m_straightPathFlags,
      m_straightPathPolys, &m_nstraightPath, MAX_POLYS, m_straightPathOptions);
    }

    // copy the path to the output buffer
    path.clear();
    path.reserve(static_cast<unsigned long>(m_nstraightPath));
    unsigned char areaType;
    for (int i = 0, j = 0; j < m_nstraightPath; i += 3, ++j) {
      // save coordinate for Unreal axis (x, z, y)
      path.emplace_back(m_straightPath[i], m_straightPath[i + 2], m_straightPath[i + 1]);
      // save area type
      {
        // critical section, force single thread running this
        std::lock_guard<std::mutex> lock(_mutex);
        _navMesh->getPolyArea(m_straightPathPolys[j], &areaType);
      }
      area.emplace_back(areaType);
    }

    return true;
  }

  // create a new walker in crowd
  bool Navigation::AddWalker(ActorId id, carla::geom::Location from) {
    dtCrowdAgentParams params;

    // check if all is ready
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // set parameters
    memset(&params, 0, sizeof(params));
    params.radius = AGENT_RADIUS;
    params.height = AGENT_HEIGHT;
    params.maxAcceleration = 80.0f;
    params.maxSpeed = 1.47f;
    params.collisionQueryRange = params.radius * 20.0f;
    params.pathOptimizationRange = params.radius * 10.0f;
    params.obstacleAvoidanceType = 3;
    params.separationWeight = 0.25f + (frand() * 0.25f);

    // set if the agent can cross roads or not
    if (frand() <= _probabilityCrossing) {
      params.queryFilterType = 1;
    } else {
      params.queryFilterType = 0;
    }

    // flags
    params.updateFlags = 0;
    params.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
    params.updateFlags |= DT_CROWD_OPTIMIZE_VIS;
    params.updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
    params.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
    params.updateFlags |= DT_CROWD_SEPARATION;

    // from Unreal coordinates (subtract half height to move pivot from center
    // (unreal) to bottom (recast))
    float PointFrom[3] = { from.x, from.z - (AGENT_HEIGHT / 2.0f), from.y };
    // add walker
    int index;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      index = _crowd->addAgent(PointFrom, &params);
      if (index == -1) {
        return false;
      }
    }

    // save the id
    _mappedWalkersId[id] = index;
    _mappedByIndex[index] = id;

    // init yaw
    _yaw_walkers[id] = 0.0f;

    // add walker for the route planning
    _walkerManager.AddWalker(id);

    return true;
  }

  // create a new vehicle in crowd to be avoided by walkers
  bool Navigation::AddOrUpdateVehicle(VehicleCollisionInfo &vehicle) {
    namespace cg = carla::geom;
    dtCrowdAgentParams params;

    // check if all is ready
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // get the bounding box extension plus some space around
    float hx = vehicle.bounding.extent.x + 1.0f;
    float hy = vehicle.bounding.extent.y + 1.0f;
    // define the 4 corners of the bounding box
    cg::Vector3D boxCorner1 {-hx, -hy, 0};
    cg::Vector3D boxCorner2 { hx, -hy, 0};
    cg::Vector3D boxCorner3 { hx,  hy, 0};
    cg::Vector3D boxCorner4 {-hx,  hy, 0};
    // rotate the points
    float angle = cg::Math::ToRadians(vehicle.transform.rotation.yaw);
    boxCorner1 = cg::Math::RotatePointOnOrigin2D(boxCorner1, angle);
    boxCorner2 = cg::Math::RotatePointOnOrigin2D(boxCorner2, angle);
    boxCorner3 = cg::Math::RotatePointOnOrigin2D(boxCorner3, angle);
    boxCorner4 = cg::Math::RotatePointOnOrigin2D(boxCorner4, angle);
    // translate to world position
    boxCorner1 += vehicle.transform.location;
    boxCorner2 += vehicle.transform.location;
    boxCorner3 += vehicle.transform.location;
    boxCorner4 += vehicle.transform.location;

    // check if this actor exists
    auto it = _mappedVehiclesId.find(vehicle.id);
    if (it != _mappedVehiclesId.end()) {
      // get the index found
      int index = it->second;
      if (index != -1) {
        // get the agent
        dtCrowdAgent *agent;
        {
          // critical section, force single thread running this
          std::lock_guard<std::mutex> lock(_mutex);
          agent = _crowd->getEditableAgent(index);
        }
        if (agent) {
          // update its position
          agent->npos[0] = vehicle.transform.location.x;
          agent->npos[1] = vehicle.transform.location.z;
          agent->npos[2] = vehicle.transform.location.y;
          // update its oriented bounding box
          agent->params.obb[0]  = boxCorner1.x;
          agent->params.obb[1]  = boxCorner1.z;
          agent->params.obb[2]  = boxCorner1.y;
          agent->params.obb[3]  = boxCorner2.x;
          agent->params.obb[4]  = boxCorner2.z;
          agent->params.obb[5]  = boxCorner2.y;
          agent->params.obb[6]  = boxCorner3.x;
          agent->params.obb[7]  = boxCorner3.z;
          agent->params.obb[8]  = boxCorner3.y;
          agent->params.obb[9]  = boxCorner4.x;
          agent->params.obb[10] = boxCorner4.z;
          agent->params.obb[11] = boxCorner4.y;
        }
        return true;
      }
    }

    // set parameters
    memset(&params, 0, sizeof(params));
    params.radius = AGENT_RADIUS * 2;
    params.height = AGENT_HEIGHT;
    params.maxAcceleration = 0.0f;
    params.maxSpeed = 1.47f;
    params.collisionQueryRange = params.radius * 20.0f;
    params.pathOptimizationRange = 0.0f;
    params.obstacleAvoidanceType = 3;
    params.separationWeight = 10.0f;

    // flags
    params.updateFlags = 0;
    // params.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
    // params.updateFlags |= DT_CROWD_OPTIMIZE_VIS;
    // params.updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
    // params.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
    params.updateFlags |= DT_CROWD_SEPARATION;

    // update its oriented bounding box
    // data: [x][y][z] [x][y][z] [x][y][z] [x][y][z]
    params.useObb = true;
    params.obb[0]  = boxCorner1.x;
    params.obb[1]  = boxCorner1.z;
    params.obb[2]  = boxCorner1.y;
    params.obb[3]  = boxCorner2.x;
    params.obb[4]  = boxCorner2.z;
    params.obb[5]  = boxCorner2.y;
    params.obb[6]  = boxCorner3.x;
    params.obb[7]  = boxCorner3.z;
    params.obb[8]  = boxCorner3.y;
    params.obb[9]  = boxCorner4.x;
    params.obb[10] = boxCorner4.z;
    params.obb[11] = boxCorner4.y;

    // from Unreal coordinates (vertical is Z) to Recast coordinates (vertical is Y)
    float PointFrom[3] = { vehicle.transform.location.x,
                           vehicle.transform.location.z,
                           vehicle.transform.location.y };

    // add walker
    int index;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      index = _crowd->addAgent(PointFrom, &params);
      if (index == -1) {
        logging::log("Vehicle agent not added to the crowd by some problem!");
        return false;
      }

      // mark as valid
      dtCrowdAgent *agent = _crowd->getEditableAgent(index);
      if (agent) {
        agent->state = DT_CROWDAGENT_STATE_WALKING;
      }
    }

    // save the id
    _mappedVehiclesId[vehicle.id] = index;
    _mappedByIndex[index] = vehicle.id;

    return true;
  }

  // remove an agent
  bool Navigation::RemoveAgent(ActorId id) {

    // check if all is ready
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // get the internal walker index
    auto it = _mappedWalkersId.find(id);
    if (it != _mappedWalkersId.end()) {
      // remove from crowd
      {
        // critical section, force single thread running this
        std::lock_guard<std::mutex> lock(_mutex);
        _crowd->removeAgent(it->second);
      }
      _walkerManager.RemoveWalker(id);
      logging::log("Nav: removing walker agent", id);
      // remove from mapping
      _mappedWalkersId.erase(it);
      _mappedByIndex.erase(it->second);

      return true;
    }

    // get the internal vehicle index
    it = _mappedVehiclesId.find(id);
    if (it != _mappedVehiclesId.end()) {
      // remove from crowd
      {
        // critical section, force single thread running this
        std::lock_guard<std::mutex> lock(_mutex);
        _crowd->removeAgent(it->second);
      }
      // logging::log("Nav: removing vehicle agent", id);
      // remove from mapping
      _mappedVehiclesId.erase(it);
      _mappedByIndex.erase(it->second);

      return true;
    }

    return false;
  }

  // add/update/delete vehicles in crowd
  bool Navigation::UpdateVehicles(std::vector<VehicleCollisionInfo> vehicles) {
    std::unordered_set<carla::rpc::ActorId> updated;

    // add all current mapped vehicles in the set
    for (auto &&entry : _mappedVehiclesId) {
      updated.insert(entry.first);
    }

    // add all vehicles (if already exists, it gets updated only)
    for (auto &&entry : vehicles) {
      // try to add or update the vehicle
      AddOrUpdateVehicle(entry);
      // mark as updated (to avoid removing it in this frame)
      updated.erase(entry.id);
    }

    // remove all vehicles not updated (they don't exist in this frame)
    for (auto &&entry : updated) {
      // remove agent not updated
      RemoveAgent(entry);
    }

    return true;
  }

  // set new max speed
  bool Navigation::SetWalkerMaxSpeed(ActorId id, float max_speed) {

    // check if all is ready
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // get the internal index
    auto it = _mappedWalkersId.find(id);
    if (it == _mappedWalkersId.end()) {
      return false;
    }

    // get the agent
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      dtCrowdAgent *agent = _crowd->getEditableAgent(it->second);
      if (agent) {
        agent->params.maxSpeed = max_speed;
        return true;
      }
    }

    return false;
  }

  // set a new target point to go
  bool Navigation::SetWalkerTarget(ActorId id, carla::geom::Location to) {

    // check if all is ready
    if (!_ready) {
      return false;
    }

    // get the internal index
    auto it = _mappedWalkersId.find(id);
    if (it == _mappedWalkersId.end()) {
      return false;
    }

    return _walkerManager.SetWalkerRoute(id, to);
  }

  // set a new target point to go directly without events
  bool Navigation::SetWalkerDirectTarget(ActorId id, carla::geom::Location to) {

    // check if all is ready
    if (!_ready) {
      return false;
    }

    // get the internal index
    auto it = _mappedWalkersId.find(id);
    if (it == _mappedWalkersId.end()) {
      return false;
    }

    return SetWalkerDirectTargetIndex(it->second, to);
  }

  // set a new target point to go directly without events
  bool Navigation::SetWalkerDirectTargetIndex(int index, carla::geom::Location to) {

    // check if all is ready
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);
    DEBUG_ASSERT(_navQuery != nullptr);

    if (index == -1) {
      return false;
    }

    // set target position
    float pointTo[3] = { to.x, to.z, to.y };
    float nearest[3];
    bool res;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      const dtQueryFilter *filter = _crowd->getFilter(0);
      dtPolyRef targetRef;
      _navQuery->findNearestPoly(pointTo, _crowd->getQueryHalfExtents(), filter, &targetRef, nearest);
      if (!targetRef) {
        return false;
      }

      res = _crowd->requestMoveTarget(index, targetRef, pointTo);
    }

    return res;
  }

  // update all walkers in crowd
  void Navigation::UpdateCrowd(const client::detail::EpisodeState &state) {

    // check if all is ready
    if (!_ready) {
      return;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // update crowd agents
    _delta_seconds = state.GetTimestamp().delta_seconds;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _crowd->update(static_cast<float>(_delta_seconds), nullptr);
    }

    // update the walkers route
    _walkerManager.Update(_delta_seconds);

    // update the time to check for blocked agents
    _timeToUnblock += _delta_seconds;

    // check all active agents
    int totalUnblocked = 0;
    int totalAgents;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      totalAgents = _crowd->getAgentCount();
    }
    const dtCrowdAgent *ag;
    for (int i = 0; i < totalAgents; ++i) {
      {
        // critical section, force single thread running this
        std::lock_guard<std::mutex> lock(_mutex);
        ag = _crowd->getAgent(i);
      }
      if (!ag->active || ag->paused) {
        continue;
      }

      // check only pedestrians not paused, and no vehicles
      if (!ag->params.useObb && !ag->paused) {
        bool resetTargetPos = false;
        bool useSameFilter = false;

        // check for unblocking actors
        if (_timeToUnblock >= AGENT_UNBLOCK_TIME) {
          // get the distance moved by each actor
          carla::geom::Vector3D previous = _walkersBlockedPosition[i];
          carla::geom::Vector3D current = carla::geom::Vector3D(ag->npos[0], ag->npos[1], ag->npos[2]);
          carla::geom::Vector3D distance = current - previous;
          float d = distance.SquaredLength();
          if (d < AGENT_UNBLOCK_DISTANCE_SQUARED) {
            ++totalUnblocked;
            resetTargetPos = true;
            useSameFilter = true;
          }
          // update with current position
          _walkersBlockedPosition[i] = current;

          // check to assign a new target position
          if (resetTargetPos) {
            // set if the agent can cross roads or not
            if (!useSameFilter) {
              if (frand() <= _probabilityCrossing) {
                SetAgentFilter(i, 1);
              } else {
                SetAgentFilter(i, 0);
              }
            }
            // set a new random target
            carla::geom::Location location;
            GetRandomLocation(location, 1, nullptr);
            _walkerManager.SetWalkerRoute(_mappedByIndex[i], location);
          }
        }
      }
    }

    // check for resetting time
    if (_timeToUnblock >= AGENT_UNBLOCK_TIME) {
      // logging::log("Unblocked agents: ", totalUnblocked);
      _timeToUnblock = 0.0f;
    }
  }

  // get the walker current transform
  bool Navigation::GetWalkerTransform(ActorId id, carla::geom::Transform &trans) {

    // check if all is ready
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // get the internal index
    auto it = _mappedWalkersId.find(id);
    if (it == _mappedWalkersId.end()) {
      return false;
    }

    // get the index found
    int index = it->second;
    if (index == -1) {
      return false;
    }

    // get the walker
    const dtCrowdAgent *agent;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getAgent(index);
    }

    if (!agent->active) {
      return false;
    }

    // set its position in Unreal coordinates
    trans.location.x = agent->npos[0];
    trans.location.y = agent->npos[2];
    trans.location.z = agent->npos[1];

    // set its rotation
    float yaw;
    float min = 0.2f;
    if (agent->vel[0] < -min || agent->vel[0] > min ||
        agent->vel[2] < -min || agent->vel[2] > min) {
      yaw = atan2f(agent->vel[2], agent->vel[0]) * (180.0f / static_cast<float>(M_PI));
    } else {
      yaw = atan2f(agent->dvel[2], agent->dvel[0]) * (180.0f / static_cast<float>(M_PI));
    }

    // interpolate current and target angle
    float shortest_angle = fmod(yaw - _yaw_walkers[id] + 540.0f, 360.0f) - 180.0f;
    float rotation_speed = 6.0f;
    trans.rotation.yaw = _yaw_walkers[id] +
    (shortest_angle * rotation_speed * static_cast<float>(_delta_seconds));
    _yaw_walkers[id] = trans.rotation.yaw;


    return true;
  }

  // get the walker current location
  bool Navigation::GetWalkerPosition(ActorId id, carla::geom::Location &location) {

    // check if all is ready
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // get the internal index
    auto it = _mappedWalkersId.find(id);
    if (it == _mappedWalkersId.end()) {
      return false;
    }

    // get the index found
    int index = it->second;
    if (index == -1) {
      return false;
    }

    // get the walker
    const dtCrowdAgent *agent;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getAgent(index);
    }

    if (!agent->active) {
      return false;
    }

    // set its position in Unreal coordinates
    location.x = agent->npos[0];
    location.y = agent->npos[2];
    location.z = agent->npos[1];

    return true;
  }

  float Navigation::GetWalkerSpeed(ActorId id) {

    // check if all is ready
    if (!_ready) {
      return 0.0f;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // get the internal index
    auto it = _mappedWalkersId.find(id);
    if (it == _mappedWalkersId.end()) {
      return 0.0f;
    }

    // get the index found
    int index = it->second;
    if (index == -1) {
      return 0.0f;
    }

    // get the walker
    const dtCrowdAgent *agent;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getAgent(index);
    }

    return sqrt(agent->vel[0] * agent->vel[0] + agent->vel[1] * agent->vel[1] + agent->vel[2] *
    agent->vel[2]);
  }

  // get a random location for navigation
  bool Navigation::GetRandomLocation(carla::geom::Location &location, float maxHeight, dtQueryFilter * filter) const {

    // check if all is ready
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_navQuery != nullptr);

    // filter
    dtQueryFilter filter2;
    if (filter == nullptr) {
      filter2.setIncludeFlags(SAMPLE_POLYFLAGS_ALL ^ SAMPLE_POLYFLAGS_DISABLED);
      filter2.setExcludeFlags(SAMPLE_POLYFLAGS_DISABLED | SAMPLE_POLYFLAGS_CROSS);
      filter = &filter2;
    }

    // search
    dtPolyRef randomRef { 0 };
    float point[3] { 0.0f, 0.0f, 0.0f };

    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      do {
        dtStatus status = _navQuery->findRandomPoint(filter, frand, &randomRef, point);
        if (status == DT_SUCCESS) {
          // set the location in Unreal coords
          location.x = point[0];
          location.y = point[2];
          location.z = point[1];
          // check for max height (to avoid roofs, it is a workaround until next version)
          if (maxHeight == -1.0f || (maxHeight >= 0.0f && location.z <= maxHeight)) {
            break;
          }
        }
      } while (1);
    }

    return true;
  }

  // assign a filter index to an agent
  void Navigation::SetAgentFilter(int agentIndex, int filterIndex)
  {
    // get the walker
    dtCrowdAgent *agent;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getEditableAgent(agentIndex);
    }
    agent->params.queryFilterType = static_cast<unsigned char>(filterIndex);
  }

  /// set the probability that an agent could cross the roads in its path following
  /// percentage of 0.0f means no pedestrian can cross roads
  /// percentage of 0.5f means 50% of all pedestrians can cross roads
  /// percentage of 1.0f means all pedestrians can cross roads if needed
  void Navigation::SetPedestriansCrossFactor(float percentage)
  {
    _probabilityCrossing = percentage;
  }

  /// set an agent as paused for the crowd
  void Navigation::PauseAgent(ActorId id, bool pause) {
    // check if all is ready
    if (!_ready) {
      return;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // get the internal index
    auto it = _mappedWalkersId.find(id);
    if (it == _mappedWalkersId.end()) {
      return;
    }

    // get the index found
    int index = it->second;
    if (index == -1) {
      return;
    }

    // get the walker
    dtCrowdAgent *agent;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getEditableAgent(index);
    }

    // mark
    agent->paused = pause;
  }
  
  bool Navigation::hasVehicleNear(ActorId id) {
    // get the internal index (walker or vehicle)
    auto it = _mappedWalkersId.find(id);
    if (it == _mappedWalkersId.end()) {
      it = _mappedVehiclesId.find(id);
      if (it == _mappedVehiclesId.end()) {
        return false;
      }
    }

    bool result;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      result = _crowd->hasVehicleNear(it->second);
    }
    return result;
  }

} // namespace nav
} // namespace carla
