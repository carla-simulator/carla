// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/MsgPackAdaptors.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Carla/Settings/EpisodeSettings.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

#include <boost/optional.hpp>

namespace carla {
namespace rpc {

  class EpisodeSettings {
  public:

    // =========================================================================
    // -- Public data members --------------------------------------------------
    // =========================================================================

    bool synchronous_mode = false;

    bool no_rendering_mode = false;

    boost::optional<double> fixed_delta_seconds;

    bool substepping = true;

    double max_substep_delta_time = 0.01;

    int max_substeps = 10;

    float max_culling_distance = 0.0f;

    bool deterministic_ragdolls = true;

    float tile_stream_distance = 3000.f; // 3km

    float actor_active_distance = 2000.f; // 2km

    bool spectator_as_ego = true;

    MSGPACK_DEFINE_ARRAY(synchronous_mode, no_rendering_mode, fixed_delta_seconds, substepping,
        max_substep_delta_time, max_substeps, max_culling_distance, deterministic_ragdolls,
        tile_stream_distance, actor_active_distance, spectator_as_ego);

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    EpisodeSettings() = default;

    EpisodeSettings(
        bool synchronous_mode,
        bool no_rendering_mode,
        double fixed_delta_seconds = 0.0,
        bool substepping = true,
        double max_substep_delta_time = 0.01,
        int max_substeps = 10,
        float max_culling_distance = 0.0f,
        bool deterministic_ragdolls = true,
        float tile_stream_distance = 3000.f,
        float actor_active_distance = 2000.f,
        bool spectator_as_ego = true)
      : synchronous_mode(synchronous_mode),
        no_rendering_mode(no_rendering_mode),
        fixed_delta_seconds(
            fixed_delta_seconds > 0.0 ? fixed_delta_seconds : boost::optional<double>{}),
        substepping(substepping),
        max_substep_delta_time(max_substep_delta_time),
        max_substeps(max_substeps),
        max_culling_distance(max_culling_distance),
        deterministic_ragdolls(deterministic_ragdolls),
        tile_stream_distance(tile_stream_distance),
        actor_active_distance(actor_active_distance),
        spectator_as_ego(spectator_as_ego) {}

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const EpisodeSettings &rhs) const {
      return
          (synchronous_mode == rhs.synchronous_mode) &&
          (no_rendering_mode == rhs.no_rendering_mode) &&
          (substepping == rhs.substepping) &&
          (fixed_delta_seconds == rhs.fixed_delta_seconds) &&
          (max_substep_delta_time == rhs.max_substep_delta_time) &&
          (max_substeps == rhs.max_substeps) &&
          (max_culling_distance == rhs.max_culling_distance) &&
          (deterministic_ragdolls == rhs.deterministic_ragdolls) &&
          (tile_stream_distance == rhs.tile_stream_distance) &&
          (actor_active_distance == rhs.actor_active_distance) &&
          (spectator_as_ego == rhs.spectator_as_ego);
    }

    bool operator!=(const EpisodeSettings &rhs) const {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- Conversions to UE4 types ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    EpisodeSettings(const FEpisodeSettings &Settings)
      : EpisodeSettings(
            Settings.bSynchronousMode,
            Settings.bNoRenderingMode,
            Settings.FixedDeltaSeconds.Get(0.0),
            Settings.bSubstepping,
            Settings.MaxSubstepDeltaTime,
            Settings.MaxSubsteps,
            Settings.MaxCullingDistance,
            Settings.bDeterministicRagdolls,
            Settings.TileStreamingDistance,
            Settings.ActorActiveDistance,
            Settings.SpectatorAsEgo) {
      constexpr float CMTOM = 1.f/100.f;
      tile_stream_distance = CMTOM * Settings.TileStreamingDistance;
      actor_active_distance = CMTOM * Settings.ActorActiveDistance;
    }

    operator FEpisodeSettings() const {
      constexpr float MTOCM = 100.f;
      FEpisodeSettings Settings;
      Settings.bSynchronousMode = synchronous_mode;
      Settings.bNoRenderingMode = no_rendering_mode;
      if (fixed_delta_seconds.has_value()) {
        Settings.FixedDeltaSeconds = *fixed_delta_seconds;
      }
      Settings.bSubstepping = substepping;
      Settings.MaxSubstepDeltaTime = max_substep_delta_time;
      Settings.MaxSubsteps = max_substeps;
      Settings.MaxCullingDistance = max_culling_distance;
      Settings.bDeterministicRagdolls = deterministic_ragdolls;
      Settings.TileStreamingDistance = MTOCM * tile_stream_distance;
      Settings.ActorActiveDistance = MTOCM * actor_active_distance;
      Settings.SpectatorAsEgo = spectator_as_ego;

      return Settings;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace rpc
} // namespace carla
