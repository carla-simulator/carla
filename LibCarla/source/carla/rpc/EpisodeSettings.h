// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/MsgPackAdaptors.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Carla/Settings/EpisodeSettings.h"
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

    MSGPACK_DEFINE_ARRAY(synchronous_mode, no_rendering_mode, fixed_delta_seconds);

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    EpisodeSettings() = default;

    EpisodeSettings(
        bool synchronous_mode,
        bool no_rendering_mode,
        double fixed_delta_seconds = 0.0)
      : synchronous_mode(synchronous_mode),
        no_rendering_mode(no_rendering_mode),
        fixed_delta_seconds(
            fixed_delta_seconds > 0.0 ? fixed_delta_seconds : boost::optional<double>{}) {}

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const EpisodeSettings &rhs) const {
      return
          (synchronous_mode == rhs.synchronous_mode) &&
          (no_rendering_mode == rhs.no_rendering_mode) &&
          (fixed_delta_seconds == rhs.fixed_delta_seconds);
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
            Settings.FixedDeltaSeconds.Get(0.0)) {}

    operator FEpisodeSettings() const {
      FEpisodeSettings Settings;
      Settings.bSynchronousMode = synchronous_mode;
      Settings.bNoRenderingMode = no_rendering_mode;
      if (fixed_delta_seconds.has_value()) {
        Settings.FixedDeltaSeconds = *fixed_delta_seconds;
      }
      return Settings;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace rpc
} // namespace carla
