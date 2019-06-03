// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/detail/EpisodeState.h"
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"
#include "carla/rpc/ActorId.h"
#include "recast/Recast.h"
#include "recast/DetourCrowd.h"
#include "recast/DetourNavMesh.h"
#include "recast/DetourNavMeshBuilder.h"
#include "recast/DetourNavMeshQuery.h"
#include "recast/DetourCommon.h"

namespace carla {
namespace nav {

  enum SamplePolyAreas {
    SAMPLE_POLYAREA_GROUND,
    SAMPLE_POLYAREA_WATER,
    SAMPLE_POLYAREA_ROAD,
    SAMPLE_POLYAREA_DOOR,
    SAMPLE_POLYAREA_GRASS,
    SAMPLE_POLYAREA_JUMP,
  };

  enum SamplePolyFlags {
    SAMPLE_POLYFLAGS_WALK		    = 0x01,		// Ability to walk (ground, grass, road)
    SAMPLE_POLYFLAGS_SWIM		    = 0x02,		// Ability to swim (water).
    SAMPLE_POLYFLAGS_DOOR		    = 0x04,		// Ability to move through doors.
    SAMPLE_POLYFLAGS_JUMP		    = 0x08,		// Ability to jump.
    SAMPLE_POLYFLAGS_DISABLED	  = 0x10,		// Disabled polygon
    SAMPLE_POLYFLAGS_ALL		    = 0xffff	// All abilities.
  };

  enum UpdateFlags
  {
    DT_CROWD_ANTICIPATE_TURNS = 1,
    DT_CROWD_OBSTACLE_AVOIDANCE = 2,
    DT_CROWD_SEPARATION = 4,
    DT_CROWD_OPTIMIZE_VIS = 8,			///< Use #dtPathCorridor::optimizePathVisibility() to optimize the agent path.
    DT_CROWD_OPTIMIZE_TOPO = 16,		///< Use dtPathCorridor::optimizePathTopology() to optimize the agent path.
  };

  class Navigation {

    public:
    Navigation();
    ~Navigation();
    // load navigation data
    bool Load(const std::string filename);
    // load navigation data from memory
    bool Load(const std::vector<uint8_t> content);
    // return the path points to go from one position to another
    bool GetPath(const carla::geom::Location from, const carla::geom::Location to, dtQueryFilter* filter, std::vector<carla::geom::Location> &path);

    // create the crowd object
    void CreateCrowd(void);
    // create a new walker but not yet add in crowd
    void AddWalker(ActorId id);
    // create a new walker in crowd
    bool AddWalkerInCrowd(ActorId id, carla::geom::Location from);
    // set a new target point to go
    bool SetWalkerTarget(ActorId id, carla::geom::Location to);
    // get the walker current transform
    bool GetWalkerTransform(ActorId id, carla::geom::Transform &trans);
    // update all walkers in crowd
    void UpdateCrowd(const client::detail::EpisodeState &state);

    private:
    std::vector<uint8_t> _binaryMesh;
    std::vector<ActorId> _walkersQueueToAdd;
    // meshes
    dtNavMesh *_navMesh { nullptr };
    dtNavMeshQuery *_navQuery { nullptr };
    // crowd
    dtCrowd *_crowd { nullptr };
    // mapping Id
    std::unordered_map<ActorId, int> _mappedId;
  };

} // namespace nav
} // namespace carla
