// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

/// This file implements adaptive lateral control for traffic-manager
/// vehicles: sensing the free part of the lane and shifting within it, and
/// bypassing stopped vehicles instead of stopping behind them.
///
/// The maneuver logic is organised as a behaviour graph of semantically
/// distinct nodes (see ManeuverState in DataStructures.h). This stage owns the
/// per-vehicle state and, each cycle, publishes an AvoidanceCommand consumed by
/// the motion-planning stage (lateral offset + speed factor) and, in later
/// iterations, by the localization stage (lane-borrow intent).
///
/// The feature is opt-in per vehicle and OFF by default: when disabled the
/// stage emits a neutral command so downstream control is byte-identical to a
/// build without this stage.

#pragma once

#include <unordered_map>

#include "carla/trafficmanager/AvoidanceContext.h"
#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/Stage.h"
#include "carla/trafficmanager/TrackTraffic.h"
#include "carla/trafficmanager/UniformPRNG.h"

namespace carla {
namespace traffic_manager {

using LocalMapPtr = std::shared_ptr<InMemoryMap>;

/// Stage that produces the per-vehicle lateral-avoidance command driving the
/// adaptive lateral control behaviour graph.
class LateralAvoidanceStage: Stage {
private:
  const std::vector<ActorId> &vehicle_id_list;
  const SimulationState &simulation_state;
  const BufferMap &buffer_map;
  const TrackTraffic &track_traffic;
  const Parameters &parameters;
  const LocalMapPtr &local_map;
  const cc::World &world;
  /// Collision-stage output (hazard + margin), used to detect a blocker ahead.
  const CollisionFrame &collision_frame;
  /// This stage's output, index-aligned with vehicle_id_list.
  AvoidanceFrame &output_array;
  UniformPRNG &random_device;
  /// Persistent per-vehicle behaviour-graph state.
  std::unordered_map<ActorId, AvoidContext> avoidance_context;

  /// Gather this cycle's perception for a vehicle: the free lateral space
  /// around it and any in-lane blocker ahead. Takes the vehicle's context to
  /// read/refresh the cached static-obstacle raycast hits.
  AvoidPerception Perceive(const ActorId actor_id, AvoidContext &ctx) const;

public:
  LateralAvoidanceStage(const std::vector<ActorId> &vehicle_id_list,
                        const SimulationState &simulation_state,
                        const BufferMap &buffer_map,
                        const TrackTraffic &track_traffic,
                        const Parameters &parameters,
                        const LocalMapPtr &local_map,
                        const cc::World &world,
                        const CollisionFrame &collision_frame,
                        AvoidanceFrame &output_array,
                        UniformPRNG &random_device);

  void Update(const unsigned long index) override;

  void RemoveActor(const ActorId actor_id) override;

  void Reset() override;
};

} // namespace traffic_manager
} // namespace carla
