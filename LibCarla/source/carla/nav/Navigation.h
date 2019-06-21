// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicList.h"
#include "carla/client/detail/EpisodeState.h"
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"
#include "carla/rpc/ActorId.h"
#include <recast/Recast.h>
#include <recast/DetourCrowd.h>
#include <recast/DetourNavMesh.h>
#include <recast/DetourNavMeshBuilder.h>
#include <recast/DetourNavMeshQuery.h>
#include <recast/DetourCommon.h>

namespace carla {
namespace nav {

  class Navigation {

  public:

    Navigation() = default;
    ~Navigation();

    // load navigation data
    bool Load(const std::string filename);
    // load navigation data from memory
    bool Load(const std::vector<uint8_t> content);
    // return the path points to go from one position to another
    bool GetPath(const carla::geom::Location from, const carla::geom::Location to, dtQueryFilter * filter,
    std::vector<carla::geom::Location> &path);

    // create the crowd object
    void CreateCrowd(void);
    // create a new walker
    bool AddWalker(ActorId id, carla::geom::Location from, float base_offset);
    // remove a walker
    bool RemoveWalker(ActorId id);
    // set new max speed
    bool SetWalkerMaxSpeed(ActorId id, float max_speed);
    // set a new target point to go
    bool SetWalkerTarget(ActorId id, carla::geom::Location to);
    bool SetWalkerTargetIndex(int index, carla::geom::Location to, bool use_lock = true);
    // get the walker current transform
    bool GetWalkerTransform(ActorId id, carla::geom::Transform &trans);
    // get the walker current transform
    float GetWalkerSpeed(ActorId id);
    // update all walkers in crowd
    void UpdateCrowd(const client::detail::EpisodeState &state);
    // get a random location for navigation
    bool GetRandomLocation(carla::geom::Location &location, float maxHeight = -1.0f,
    dtQueryFilter * filter = nullptr, bool use_lock = true);

  private:

    bool _ready { false };
    std::vector<uint8_t> _binaryMesh;
    double _delta_seconds;
    // meshes
    dtNavMesh *_navMesh { nullptr };
    dtNavMeshQuery *_navQuery { nullptr };
    // crowd
    dtCrowd *_crowd { nullptr };
    // mapping Id
    std::unordered_map<ActorId, int> _mappedId;
    // base height of each walker
    std::unordered_map<ActorId, float> _baseHeight;
    // Store walkers yaw angle from previous tick
    std::unordered_map<ActorId, float> _yaw_walkers;

    std::mutex _mutex;

  };

} // namespace nav
} // namespace carla
