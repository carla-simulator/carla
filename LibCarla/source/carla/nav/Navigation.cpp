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

  enum UpdateFlags {
    DT_CROWD_ANTICIPATE_TURNS   = 1,
    DT_CROWD_OBSTACLE_AVOIDANCE = 2,
    DT_CROWD_SEPARATION         = 4,
    DT_CROWD_OPTIMIZE_VIS       = 8,
    DT_CROWD_OPTIMIZE_TOPO      = 16
  };

  // these settings are the same than in RecastBuilder, so if you change the height of the agent, 
  // you should do the same in RecastBuilder
  static const int   MAX_POLYS = 256;
  static const int   MAX_AGENTS = 500;
  static const int   MAX_QUERY_SEARCH_NODES = 2048;
  static const float AGENT_HEIGHT = 1.8f;
  static const float AGENT_RADIUS = 0.3f;

  static const float AGENT_UNBLOCK_DISTANCE = 0.5f;
  static const float AGENT_UNBLOCK_DISTANCE_SQUARED = AGENT_UNBLOCK_DISTANCE * AGENT_UNBLOCK_DISTANCE;
  static const float AGENT_UNBLOCK_TIME = 4.0f;

  static const float AREA_GRASS_COST =  1.0f;
  static const float AREA_ROAD_COST  = 10.0f;

  // return a random float
  static float frand() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  }

  Navigation::Navigation() {
    // assign walker manager
    _walker_manager.SetNav(this);
  }

  Navigation::~Navigation() {
    _ready = false;
    _time_to_unblock = 0.0f;
    _mapped_walkers_id.clear();
    _mapped_vehicles_id.clear();
    _mapped_by_index.clear();
    _walkers_blocked_position.clear();
    _yaw_walkers.clear();
    _binary_mesh.clear();
    dtFreeCrowd(_crowd);
    dtFreeNavMeshQuery(_nav_query);
    dtFreeNavMesh(_nav_mesh);
  }

  // set the seed to use with random numbers
  void Navigation::SetSeed(unsigned int seed) {
    srand(seed);
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
      int num_tiles;
      dtNavMeshParams params;
    } header;
    struct NavMeshTileHeader {
      dtTileRef tile_ref;
      int data_size;
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
    for (int i = 0; i < header.num_tiles; ++i) {
      NavMeshTileHeader tile_header;

      // read the tile header
      memcpy(&tile_header, &content[pos], sizeof(tile_header));
      pos += sizeof(tile_header);
      if (pos >= content.size()) {
        dtFreeNavMesh(mesh);
        return false;
      }

      // check for valid tile
      if (!tile_header.tile_ref || !tile_header.data_size) {
        break;
      }

      // allocate the buffer
      char *data = static_cast<char *>(dtAlloc(static_cast<size_t>(tile_header.data_size), DT_ALLOC_PERM));
      if (!data) {
        break;
      }

      // read the tile
      memcpy(data, &content[pos], static_cast<size_t>(tile_header.data_size));
      pos += static_cast<unsigned long>(tile_header.data_size);
      if (pos > content.size()) {
        dtFree(data);
        dtFreeNavMesh(mesh);
        return false;
      }

      // add the tile data
      mesh->addTile(reinterpret_cast<unsigned char *>(data), tile_header.data_size, DT_TILE_FREE_DATA,
      tile_header.tile_ref, 0);
    }

    // exchange
    dtFreeNavMesh(_nav_mesh);
    _nav_mesh = mesh;

    // prepare the query object
    dtFreeNavMeshQuery(_nav_query);
    _nav_query = dtAllocNavMeshQuery();
    _nav_query->init(_nav_mesh, MAX_QUERY_SEARCH_NODES);

    // copy
    _binary_mesh = std::move(content);
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

    DEBUG_ASSERT(_crowd == nullptr);

    // create and init
    _crowd = dtAllocCrowd();
    // these radius should be the maximum size of the vehicles (CarlaCola for Carla)
    const float max_agent_radius = AGENT_RADIUS * 20;
    if (!_crowd->init(MAX_AGENTS, max_agent_radius, _nav_mesh)) {
      logging::log("Nav: failed to create crowd");
      return;
    }

    // set different filters
    // filter 0 can not walk on roads
    _crowd->getEditableFilter(0)->setIncludeFlags(CARLA_TYPE_WALKABLE);
    _crowd->getEditableFilter(0)->setExcludeFlags(CARLA_TYPE_ROAD);
    _crowd->getEditableFilter(0)->setAreaCost(CARLA_AREA_ROAD, AREA_ROAD_COST);
    _crowd->getEditableFilter(0)->setAreaCost(CARLA_AREA_GRASS, AREA_GRASS_COST);
    // filter 1 can walk on roads
    _crowd->getEditableFilter(1)->setIncludeFlags(CARLA_TYPE_WALKABLE);
    _crowd->getEditableFilter(1)->setExcludeFlags(CARLA_TYPE_NONE);
    _crowd->getEditableFilter(1)->setAreaCost(CARLA_AREA_ROAD, AREA_ROAD_COST);
    _crowd->getEditableFilter(1)->setAreaCost(CARLA_AREA_GRASS, AREA_GRASS_COST);

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
    float straight_path[MAX_POLYS * 3];
    unsigned char straight_path_flags[MAX_POLYS];
    dtPolyRef straight_path_polys[MAX_POLYS];
    int num_straight_path;
    int straight_path_options = DT_STRAIGHTPATH_AREA_CROSSINGS;

    // polys in path
    dtPolyRef polys[MAX_POLYS];
    int num_polys;

    // check if all is ready
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_nav_query != nullptr);

    // point extension
    float poly_pick_ext[3];
    poly_pick_ext[0] = 2;
    poly_pick_ext[1] = 4;
    poly_pick_ext[2] = 2;

    // filter
    dtQueryFilter filter2;
    if (filter == nullptr) {
      filter2.setAreaCost(CARLA_AREA_ROAD, AREA_ROAD_COST);
      filter2.setAreaCost(CARLA_AREA_GRASS, AREA_GRASS_COST);
      filter2.setIncludeFlags(CARLA_TYPE_WALKABLE);
      filter2.setExcludeFlags(CARLA_TYPE_NONE);
      filter = &filter2;
    }

    // set the points
    dtPolyRef start_ref = 0;
    dtPolyRef end_ref = 0;
    float start_pos[3] = { from.x, from.z, from.y };
    float end_pos[3] = { to.x, to.z, to.y };
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->findNearestPoly(start_pos, poly_pick_ext, filter, &start_ref, 0);
      _nav_query->findNearestPoly(end_pos, poly_pick_ext, filter, &end_ref, 0);
    }
    if (!start_ref || !end_ref) {
      return false;
    }

    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      // get the path of nodes
      _nav_query->findPath(start_ref, end_ref, start_pos, end_pos, filter, polys, &num_polys, MAX_POLYS);
    }

    // get the path of points
    num_straight_path = 0;
    if (num_polys == 0) {
      return false;
    }

    // in case of partial path, make sure the end point is clamped to the last
    // polygon
    float end_pos2[3];
    dtVcopy(end_pos2, end_pos);
    if (polys[num_polys - 1] != end_ref) {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->closestPointOnPoly(polys[num_polys - 1], end_pos, end_pos2, 0);
    }

    // get the points
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->findStraightPath(start_pos, end_pos2, polys, num_polys,
      straight_path, straight_path_flags,
      straight_path_polys, &num_straight_path, MAX_POLYS, straight_path_options);
    }

    // copy the path to the output buffer
    path.clear();
    path.reserve(static_cast<unsigned long>(num_straight_path));
    unsigned char area_type;
    for (int i = 0, j = 0; j < num_straight_path; i += 3, ++j) {
      // save coordinate for Unreal axis (x, z, y)
      path.emplace_back(straight_path[i], straight_path[i + 2], straight_path[i + 1]);
      // save area type
      {
        // critical section, force single thread running this
        std::lock_guard<std::mutex> lock(_mutex);
        _nav_mesh->getPolyArea(straight_path_polys[j], &area_type);
      }
      area.emplace_back(area_type);
    }

    return true;
  }

  bool Navigation::GetAgentRoute(ActorId id, carla::geom::Location from, carla::geom::Location to,
  std::vector<carla::geom::Location> &path, std::vector<unsigned char> &area) {
    // path found
    float straight_path[MAX_POLYS * 3];
    unsigned char straight_path_flags[MAX_POLYS];
    dtPolyRef straight_path_polys[MAX_POLYS];
    int num_straight_path = 0;
    int straight_path_options = DT_STRAIGHTPATH_AREA_CROSSINGS;

    // polys in path
    dtPolyRef polys[MAX_POLYS];
    int num_polys;

    // check if all is ready
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_nav_query != nullptr);

    // point extension
    float poly_pick_ext[3] = {2,4,2};

    // get current filter from agent
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end())
      return false;

    const dtQueryFilter *filter;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      filter = _crowd->getFilter(_crowd->getAgent(it->second)->params.queryFilterType);
    }

    // set the points
    dtPolyRef start_ref = 0;
    dtPolyRef end_ref = 0;
    float start_pos[3] = { from.x, from.z, from.y };
    float end_pos[3] = { to.x, to.z, to.y };
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->findNearestPoly(start_pos, poly_pick_ext, filter, &start_ref, 0);
      _nav_query->findNearestPoly(end_pos, poly_pick_ext, filter, &end_ref, 0);
    }
    if (!start_ref || !end_ref) {
      return false;
    }

    // get the path of nodes
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->findPath(start_ref, end_ref, start_pos, end_pos, filter, polys, &num_polys, MAX_POLYS);
    }

    // get the path of points
    if (num_polys == 0) {
      return false;
    }

    // in case of partial path, make sure the end point is clamped to the last
    // polygon
    float end_pos2[3];
    dtVcopy(end_pos2, end_pos);
    if (polys[num_polys - 1] != end_ref) {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->closestPointOnPoly(polys[num_polys - 1], end_pos, end_pos2, 0);
    }

    // get the points
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->findStraightPath(start_pos, end_pos2, polys, num_polys,
      straight_path, straight_path_flags,
      straight_path_polys, &num_straight_path, MAX_POLYS, straight_path_options);
    }

    // copy the path to the output buffer
    path.clear();
    path.reserve(static_cast<unsigned long>(num_straight_path));
    unsigned char area_type;
    for (int i = 0, j = 0; j < num_straight_path; i += 3, ++j) {
      // save coordinate for Unreal axis (x, z, y)
      path.emplace_back(straight_path[i], straight_path[i + 2], straight_path[i + 1]);
      // save area type
      {
        // critical section, force single thread running this
        std::lock_guard<std::mutex> lock(_mutex);
        _nav_mesh->getPolyArea(straight_path_polys[j], &area_type);
      }
      area.emplace_back(area_type);
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
    params.maxAcceleration = 160.0f;
    params.maxSpeed = 1.47f;
    params.collisionQueryRange = 10;
    params.obstacleAvoidanceType = 3;
    params.separationWeight = 0.5f;
    
    // set if the agent can cross roads or not
    if (frand() <= _probability_crossing) {
      params.queryFilterType = 1;
    } else {
      params.queryFilterType = 0;
    }

    // flags
    params.updateFlags = 0;
    params.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
    params.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
    params.updateFlags |= DT_CROWD_SEPARATION;

    // from Unreal coordinates (subtract half height to move pivot from center
    // (unreal) to bottom (recast))
    float point_from[3] = { from.x, from.z - (AGENT_HEIGHT / 2.0f), from.y };
    // add walker
    int index;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      index = _crowd->addAgent(point_from, &params);
      if (index == -1) {
        return false;
      }
    }

    // save the id
    _mapped_walkers_id[id] = index;
    _mapped_by_index[index] = id;

    // init yaw
    _yaw_walkers[id] = 0.0f;

    // add walker for the route planning
    _walker_manager.AddWalker(id);

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
    float marge = 0.8f;
    float hx = vehicle.bounding.extent.x + marge;
    float hy = vehicle.bounding.extent.y + marge;
    // define the 4 corners of the bounding box
    cg::Vector3D box_corner1 {-hx, -hy, 0};
    cg::Vector3D box_corner2 { hx + 0.2f, -hy, 0};
    cg::Vector3D box_corner3 { hx + 0.2f,  hy, 0};
    cg::Vector3D box_corner4 {-hx,  hy, 0};
    // rotate the points
    float angle = cg::Math::ToRadians(vehicle.transform.rotation.yaw);
    box_corner1 = cg::Math::RotatePointOnOrigin2D(box_corner1, angle);
    box_corner2 = cg::Math::RotatePointOnOrigin2D(box_corner2, angle);
    box_corner3 = cg::Math::RotatePointOnOrigin2D(box_corner3, angle);
    box_corner4 = cg::Math::RotatePointOnOrigin2D(box_corner4, angle);
    // translate to world position
    box_corner1 += vehicle.transform.location;
    box_corner2 += vehicle.transform.location;
    box_corner3 += vehicle.transform.location;
    box_corner4 += vehicle.transform.location;

    // check if this actor exists
    auto it = _mapped_vehicles_id.find(vehicle.id);
    if (it != _mapped_vehicles_id.end()) {
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
          agent->params.obb[0]  = box_corner1.x;
          agent->params.obb[1]  = box_corner1.z;
          agent->params.obb[2]  = box_corner1.y;
          agent->params.obb[3]  = box_corner2.x;
          agent->params.obb[4]  = box_corner2.z;
          agent->params.obb[5]  = box_corner2.y;
          agent->params.obb[6]  = box_corner3.x;
          agent->params.obb[7]  = box_corner3.z;
          agent->params.obb[8]  = box_corner3.y;
          agent->params.obb[9]  = box_corner4.x;
          agent->params.obb[10] = box_corner4.z;
          agent->params.obb[11] = box_corner4.y;
        }
        return true;
      }
    }

    // set parameters
    memset(&params, 0, sizeof(params));
    params.radius = 2;
    params.height = AGENT_HEIGHT;
    params.maxAcceleration = 0.0f;
    params.maxSpeed = 1.47f;
    params.collisionQueryRange = 0;
    params.obstacleAvoidanceType = 0;
    params.separationWeight = 100.0f;

    // flags
    params.updateFlags = 0;
    params.updateFlags |= DT_CROWD_SEPARATION;

    // update its oriented bounding box
    // data: [x][y][z] [x][y][z] [x][y][z] [x][y][z]
    params.useObb = true;
    params.obb[0]  = box_corner1.x;
    params.obb[1]  = box_corner1.z;
    params.obb[2]  = box_corner1.y;
    params.obb[3]  = box_corner2.x;
    params.obb[4]  = box_corner2.z;
    params.obb[5]  = box_corner2.y;
    params.obb[6]  = box_corner3.x;
    params.obb[7]  = box_corner3.z;
    params.obb[8]  = box_corner3.y;
    params.obb[9]  = box_corner4.x;
    params.obb[10] = box_corner4.z;
    params.obb[11] = box_corner4.y;

    // from Unreal coordinates (vertical is Z) to Recast coordinates (vertical is Y)
    float point_from[3] = { vehicle.transform.location.x,
                            vehicle.transform.location.z,
                            vehicle.transform.location.y };

    // add walker
    int index;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      index = _crowd->addAgent(point_from, &params);
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
    _mapped_vehicles_id[vehicle.id] = index;
    _mapped_by_index[index] = vehicle.id;

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
    auto it = _mapped_walkers_id.find(id);
    if (it != _mapped_walkers_id.end()) {
      // remove from crowd
      {
        // critical section, force single thread running this
        std::lock_guard<std::mutex> lock(_mutex);
        _crowd->removeAgent(it->second);
      }
      _walker_manager.RemoveWalker(id);
      // remove from mapping
      _mapped_walkers_id.erase(it);
      _mapped_by_index.erase(it->second);

      return true;
    }

    // get the internal vehicle index
    it = _mapped_vehicles_id.find(id);
    if (it != _mapped_vehicles_id.end()) {
      // remove from crowd
      {
        // critical section, force single thread running this
        std::lock_guard<std::mutex> lock(_mutex);
        _crowd->removeAgent(it->second);
      }
      // remove from mapping
      _mapped_vehicles_id.erase(it);
      _mapped_by_index.erase(it->second);

      return true;
    }

    return false;
  }

  // add/update/delete vehicles in crowd
  bool Navigation::UpdateVehicles(std::vector<VehicleCollisionInfo> vehicles) {
    std::unordered_set<carla::rpc::ActorId> updated;

    // add all current mapped vehicles in the set
    for (auto &&entry : _mapped_vehicles_id) {
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
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
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
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
      return false;
    }

    return _walker_manager.SetWalkerRoute(id, to);
  }

  // set a new target point to go directly without events
  bool Navigation::SetWalkerDirectTarget(ActorId id, carla::geom::Location to) {

    // check if all is ready
    if (!_ready) {
      return false;
    }

    // get the internal index
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
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
    DEBUG_ASSERT(_nav_query != nullptr);

    if (index == -1) {
      return false;
    }

    // set target position
    float point_to[3] = { to.x, to.z, to.y };
    float nearest[3];
    bool res;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      const dtQueryFilter *filter = _crowd->getFilter(0);
      dtPolyRef target_ref;
      _nav_query->findNearestPoly(point_to, _crowd->getQueryHalfExtents(), filter, &target_ref, nearest);
      if (!target_ref) {
        return false;
      }

      res = _crowd->requestMoveTarget(index, target_ref, point_to);
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
    _walker_manager.Update(_delta_seconds);

    // update the time to check for blocked agents
    _time_to_unblock += _delta_seconds;

    // check all active agents
    int total_unblocked = 0;
    int total_agents;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      total_agents = _crowd->getAgentCount();
    }
    const dtCrowdAgent *ag;
    for (int i = 0; i < total_agents; ++i) {
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
        bool reset_target_pos = false;
        bool use_same_filter = false;

        // check for unblocking actors
        if (_time_to_unblock >= AGENT_UNBLOCK_TIME) {
          // get the distance moved by each actor
          carla::geom::Vector3D previous = _walkers_blocked_position[i];
          carla::geom::Vector3D current = carla::geom::Vector3D(ag->npos[0], ag->npos[1], ag->npos[2]);
          carla::geom::Vector3D distance = current - previous;
          float d = distance.SquaredLength();
          if (d < AGENT_UNBLOCK_DISTANCE_SQUARED) {
            ++total_unblocked;
            reset_target_pos = true;
            use_same_filter = true;
          }
          // update with current position
          _walkers_blocked_position[i] = current;

          // check to assign a new target position
          if (reset_target_pos) {
            // set if the agent can cross roads or not
            if (!use_same_filter) {
              if (frand() <= _probability_crossing) {
                SetAgentFilter(i, 1);
              } else {
                SetAgentFilter(i, 0);
              }
            }
            // set a new random target
            carla::geom::Location location;
            GetRandomLocation(location, nullptr);
            _walker_manager.SetWalkerRoute(_mapped_by_index[i], location);
          }
        }
      }
    }

    // check for resetting time
    if (_time_to_unblock >= AGENT_UNBLOCK_TIME) {
      _time_to_unblock = 0.0f;
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
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
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
    float speed = 0.0f;
    float min = 0.1f;
    if (agent->vel[0] < -min || agent->vel[0] > min ||
        agent->vel[2] < -min || agent->vel[2] > min) {
      yaw = atan2f(agent->vel[2], agent->vel[0]) * (180.0f / static_cast<float>(M_PI));
      speed = sqrtf(agent->vel[0] * agent->vel[0] + agent->vel[1] * agent->vel[1] + agent->vel[2] * agent->vel[2]);
    } else {
      yaw = atan2f(agent->dvel[2], agent->dvel[0]) * (180.0f / static_cast<float>(M_PI));
      speed = sqrtf(agent->dvel[0] * agent->dvel[0] + agent->dvel[1] * agent->dvel[1] + agent->dvel[2] * agent->dvel[2]);
    }

    // interpolate current and target angle
    float shortest_angle = fmod(yaw - _yaw_walkers[id] + 540.0f, 360.0f) - 180.0f;
    float per = (speed / 1.5f);
    if (per > 1.0f) per = 1.0f;
    float rotation_speed = per * 6.0f;
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
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
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
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
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
  bool Navigation::GetRandomLocation(carla::geom::Location &location, dtQueryFilter * filter) const {

    // check if all is ready
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_nav_query != nullptr);

    // filter
    dtQueryFilter filter2;
    if (filter == nullptr) {
      filter2.setIncludeFlags(CARLA_TYPE_SIDEWALK);
      filter2.setExcludeFlags(CARLA_TYPE_NONE);
      filter = &filter2;
    }

    // we will try up to 10 rounds, otherwise we failed to find a good location
    dtPolyRef random_ref { 0 };
    float point[3] { 0.0f, 0.0f, 0.0f };
    int rounds = 10;
    {
      dtStatus status;
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      do {
        status = _nav_query->findRandomPoint(filter, frand, &random_ref, point);
        // set the location in Unreal coords
        if (status == DT_SUCCESS) {
          location.x = point[0];
          location.y = point[2];
          location.z = point[1];
        }
        --rounds;
      } while (status != DT_SUCCESS && rounds > 0);
    }

    return (rounds > 0);
  }

  // assign a filter index to an agent
  void Navigation::SetAgentFilter(int agent_index, int filter_index)
  {
    // get the walker
    dtCrowdAgent *agent;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getEditableAgent(agent_index);
    }
    agent->params.queryFilterType = static_cast<unsigned char>(filter_index);
  }

  // set the probability that an agent could cross the roads in its path following
  // percentage of 0.0f means no pedestrian can cross roads
  // percentage of 0.5f means 50% of all pedestrians can cross roads
  // percentage of 1.0f means all pedestrians can cross roads if needed
  void Navigation::SetPedestriansCrossFactor(float percentage)
  {
    _probability_crossing = percentage;
  }

  // set an agent as paused for the crowd
  void Navigation::PauseAgent(ActorId id, bool pause) {
    // check if all is ready
    if (!_ready) {
      return;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // get the internal index
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
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

  bool Navigation::HasVehicleNear(ActorId id, float distance, carla::geom::Location direction) {
    // get the internal index (walker or vehicle)
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
      it = _mapped_vehicles_id.find(id);
      if (it == _mapped_vehicles_id.end()) {
        return false;
      }
    }

    float dir[3] = { direction.x, direction.z, direction.y };
    bool result;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      result = _crowd->hasVehicleNear(it->second, distance * distance, dir, false);
    }
    return result;
  }

  /// make agent look at some location
  bool Navigation::SetWalkerLookAt(ActorId id, carla::geom::Location location) {
    // get the internal index (walker or vehicle)
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
      it = _mapped_vehicles_id.find(id);
      if (it == _mapped_vehicles_id.end()) {
        return false;
      }
    }

    dtCrowdAgent *agent;
    {
      // critical section, force single thread running this
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getEditableAgent(it->second);
    }

    // get the position
    float x = (location.x - agent->npos[0]) * 0.0001f;
    float y = (location.y - agent->npos[2]) * 0.0001f;
    float z = (location.z - agent->npos[1]) * 0.0001f;

    // set its velocity
    agent->vel[0] = x;
    agent->vel[2] = y;
    agent->vel[1] = z;
    agent->nvel[0] = x;
    agent->nvel[2] = y;
    agent->nvel[1] = z;
    agent->dvel[0] = x;
    agent->dvel[2] = y;
    agent->dvel[1] = z;

    return true;
  }

} // namespace nav
} // namespace carla
