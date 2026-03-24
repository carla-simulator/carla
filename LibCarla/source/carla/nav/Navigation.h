// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicList.h"
#include "carla/client/detail/EpisodeState.h"
#include "carla/geom/BoundingBox.h"
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"
#include "carla/nav/WalkerManager.h"
#include "carla/rpc/ActorId.h"
#include <recast/Recast.h>
#include <recast/DetourCrowd.h>
#include <recast/DetourNavMesh.h>
#include <recast/DetourNavMeshBuilder.h>
#include <recast/DetourNavMeshQuery.h>
#include <recast/DetourCommon.h>

namespace carla {
namespace nav {

  enum NavAreas {
    CARLA_AREA_BLOCK = 0,
    CARLA_AREA_SIDEWALK,
    CARLA_AREA_CROSSWALK,
    CARLA_AREA_ROAD,
    CARLA_AREA_GRASS
  };

  enum SamplePolyFlags
  {
    CARLA_TYPE_NONE       = 0x01,
    CARLA_TYPE_SIDEWALK   = 0x02,
    CARLA_TYPE_CROSSWALK  = 0x04,
    CARLA_TYPE_ROAD       = 0x08,
    CARLA_TYPE_GRASS      = 0x10,
    CARLA_TYPE_ALL        = 0xffff,

    CARLA_TYPE_WALKABLE   = CARLA_TYPE_SIDEWALK | CARLA_TYPE_CROSSWALK | CARLA_TYPE_GRASS | CARLA_TYPE_ROAD,
  };

  /// struct to send info about vehicles to the crowd
  struct VehicleCollisionInfo {
    carla::rpc::ActorId id;
    carla::geom::Transform transform;
    carla::geom::BoundingBox bounding;
  };

  /// Manage the pedestrians navigation, using the Recast & Detour library for low level calculations.
  ///
  /// This class gets the binary content of the map from the server, which is required for the path finding.
  /// Then this class can add or remove pedestrians, and also set target points to walk for each one.
  class Navigation : private NonCopyable {

  public:

    Navigation();
    ~Navigation();

    /// load navigation data
    bool Load(const std::string &filename);
    /// load navigation data from memory
    bool Load(std::vector<uint8_t> content);
    /// return the path points to go from one position to another
    bool GetPath(carla::geom::Location from, carla::geom::Location to, dtQueryFilter * filter,
    std::vector<carla::geom::Location> &path, std::vector<unsigned char> &area);
    bool GetAgentRoute(ActorId id, carla::geom::Location from, carla::geom::Location to,
    std::vector<carla::geom::Location> &path, std::vector<unsigned char> &area);

    /// reference to the simulator to access API functions
    void SetSimulator(std::weak_ptr<carla::client::detail::Simulator> simulator);
    /// set the seed to use with random numbers
    void SetSeed(unsigned int seed);
    /// create the crowd object
    void CreateCrowd(void);
    /// create a new walker
    bool AddWalker(ActorId id, carla::geom::Location from);
    /// create a new vehicle in crowd to be avoided by walkers
    bool AddOrUpdateVehicle(VehicleCollisionInfo &vehicle);
    /// remove an agent
    bool RemoveAgent(ActorId id);
    /// add/update/delete vehicles in crowd
    bool UpdateVehicles(std::vector<VehicleCollisionInfo> vehicles);
    /// set new max speed
    bool SetWalkerMaxSpeed(ActorId id, float max_speed);
    /// set a new target point to go through a route with events
    bool SetWalkerTarget(ActorId id, carla::geom::Location to);
    // set a new target point to go directly without events
    bool SetWalkerDirectTarget(ActorId id, carla::geom::Location to);
    bool SetWalkerDirectTargetIndex(int index, carla::geom::Location to);
    /// get the walker current transform
    bool GetWalkerTransform(ActorId id, carla::geom::Transform &trans);
    /// get the walker current location
    bool GetWalkerPosition(ActorId id, carla::geom::Location &location);
    /// get the walker current transform
    float GetWalkerSpeed(ActorId id);
    /// update all walkers in crowd
    void UpdateCrowd(const client::detail::EpisodeState &state);
    /// get a random location for navigation
    bool GetRandomLocation(carla::geom::Location &location, dtQueryFilter * filter = nullptr) const;
    /// set the probability that an agent could cross the roads in its path following
    void SetPedestriansCrossFactor(float percentage);
    /// set an agent as paused for the crowd
    void PauseAgent(ActorId id, bool pause);
    /// return if the agent has a vehicle near (as neighbour)
    bool HasVehicleNear(ActorId id, float distance, carla::geom::Location direction);
    /// make agent look at some location
    bool SetWalkerLookAt(ActorId id, carla::geom::Location location);
    /// return if the agent has been killed by a vehicle
    bool IsWalkerAlive(ActorId id, bool &alive);

    dtCrowd *GetCrowd() { return _crowd; };

    /// return the last delta seconds
    double GetDeltaSeconds() { return _delta_seconds; };

  private:

    bool _ready { false };
    std::vector<uint8_t> _binary_mesh;
    double _delta_seconds { 0.0 };
    /// meshes
    dtNavMesh *_nav_mesh { nullptr };
    dtNavMeshQuery *_nav_query { nullptr };
    /// crowd
    dtCrowd *_crowd { nullptr };
    /// mapping Id
    std::unordered_map<ActorId, int> _mapped_walkers_id;
    std::unordered_map<ActorId, int> _mapped_vehicles_id;
    // mapping by index also
    std::unordered_map<int, ActorId> _mapped_by_index;
    /// store walkers yaw angle from previous tick
    std::unordered_map<ActorId, float> _yaw_walkers;
    /// saves the position of each actor at intervals and check if any is blocked
    std::unordered_map<int, carla::geom::Vector3D> _walkers_blocked_position;
    double _time_to_unblock { 0.0 };

    /// walker manager for the route planning with events
    WalkerManager _walker_manager;

    std::weak_ptr<carla::client::detail::Simulator> _simulator;
    
    mutable std::mutex _mutex;

    float _probability_crossing { 0.0f };

    /// assign a filter index to an agent
    void SetAgentFilter(int agent_index, int filter_index);
  };

} // namespace nav
} // namespace carla
