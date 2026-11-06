// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

namespace carla {
namespace multigpu {

  /// Primary actor id -> id of the local actor a secondary replayed it onto,
  /// for the current episode only.
  using MirroredActorMap = std::unordered_map<uint32_t, uint32_t>;

  struct ReplayedActorPlan {
    /// Local actor to reuse; if empty, spawn a new one.
    std::optional<uint32_t> reuse;
    /// Id to request for the new actor, 0 for a fresh one.
    uint32_t spawn_id = 0u;
  };

  /// Where a secondary replays the add event of primary actor @a primary_id.
  /// Local ids follow the secondary's own counter, so a local actor whose id
  /// equals @a primary_id may be another actor, and is only reused if it is
  /// a map actor that mirrors no other primary actor. @a local provides:
  ///   bool IsSameKind(id): the local actor exists with the replayed blueprint.
  ///   std::optional<uint32_t> FindMapActor(): the map-registered local actor
  ///     that is the replayed one (the spectator, a prop at its place), if any.
  ///   bool IsMapActorLike(id): a map-registered local actor matching the
  ///     replayed description.
  ///   bool Contains(id): the id is taken, dormant actors included.
  template <typename LocalActors>
  ReplayedActorPlan PlanReplayedActor(
      const MirroredActorMap &mirrors,
      uint32_t primary_id,
      const LocalActors &local) {
    const auto mirror = mirrors.find(primary_id);
    if ((mirror != mirrors.end()) && local.IsSameKind(mirror->second)) {
      return {mirror->second, 0u};
    }

    const auto is_free_map_actor = [&](uint32_t local_id) {
      for (const auto &item : mirrors) {
        if (item.second == local_id) {
          return false;
        }
      }
      return local.IsMapActorLike(local_id);
    };

    const std::optional<uint32_t> map_actor = local.FindMapActor();
    if (map_actor && is_free_map_actor(*map_actor)) {
      return {map_actor, 0u};
    }
    if (is_free_map_actor(primary_id)) {
      return {primary_id, 0u};
    }
    return {std::nullopt, local.Contains(primary_id) ? 0u : primary_id};
  }

} // namespace multigpu
} // namespace carla
