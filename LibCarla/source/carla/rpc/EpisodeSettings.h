// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Carla/Settings/EpisodeSettings.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  class EpisodeSettings {
  public:

    // =========================================================================
    // -- Public data members --------------------------------------------------
    // =========================================================================

    bool synchronous_mode = false;

    bool no_rendering_mode = false;

    MSGPACK_DEFINE_ARRAY(synchronous_mode, no_rendering_mode);

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    EpisodeSettings() = default;

    EpisodeSettings(
        bool synchronous_mode,
        bool no_rendering_mode)
      : synchronous_mode(synchronous_mode),
        no_rendering_mode(no_rendering_mode) {}

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const EpisodeSettings &rhs) const {
      return
          (synchronous_mode == rhs.synchronous_mode) &&
          (no_rendering_mode == rhs.no_rendering_mode);
    }

    bool operator!=(const EpisodeSettings &rhs) const {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- Conversions to UE4 types ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    EpisodeSettings(const FEpisodeSettings &Settings)
      : synchronous_mode(Settings.bSynchronousMode),
        no_rendering_mode(Settings.bNoRenderingMode) {}

    operator FEpisodeSettings() const {
      FEpisodeSettings Settings;
      Settings.bSynchronousMode = synchronous_mode;
      Settings.bNoRenderingMode = no_rendering_mode;
      return Settings;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace rpc
} // namespace carla
