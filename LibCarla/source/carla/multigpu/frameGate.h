// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cmath>
#include <cstdint>
#include <optional>

namespace carla {
namespace multigpu {

  /// What a secondary does at the start of an engine frame, before the engine
  /// computes the frame's time step.
  enum class BeginFrameAction {
    /// Not connected to a primary: run on this server's own clock.
    OwnClock,
    /// Connected, but this frame must not block (no episode yet, or a map load
    /// or travel is under way, during which the primary waits for this server).
    DontWait,
    /// Wait for the primary's next frame and take its time step.
    Wait
  };

  struct BeginFrameState {
    bool connected = false;
    bool has_episode = false;
    bool travel_pending = false;
    bool load_map_pending = false;
    bool exit_requested = false;
  };

  inline BeginFrameAction DecideBeginFrame(const BeginFrameState &state) {
    if (!state.connected) {
      return BeginFrameAction::OwnClock;
    }
    if (!state.has_episode || state.travel_pending || state.load_map_pending || state.exit_requested) {
      return BeginFrameAction::DontWait;
    }
    return BeginFrameAction::Wait;
  }

  /// What a secondary does when its world starts ticking.
  enum class PreTickAction {
    /// Open the map the primary loaded; nothing else this frame.
    OpenLevel,
    /// Play the primary frame prepared for this engine frame.
    Play,
    /// No primary frame: let the frame pass without simulation time.
    Skip
  };

  inline PreTickAction DecidePreTick(
      bool load_map_pending,
      bool has_episode,
      bool frame_prepared,
      bool episode_world) {
    if (load_map_pending && has_episode) {
      return PreTickAction::OpenLevel;
    }
    if (!frame_prepared || !has_episode || !episode_world) {
      return PreTickAction::Skip;
    }
    return PreTickAction::Play;
  }

  /// Whether @a frame directly follows the last frame this secondary played.
  inline bool IsConsecutive(std::optional<uint64_t> last_played, uint64_t frame) {
    return last_played.has_value() && (frame == *last_played + 1u);
  }

  /// Whether playing @a frame shows that this secondary missed primary state
  /// (a frame went missing) that only a full resync gives back.
  inline bool NeedsResync(std::optional<uint64_t> last_played, uint64_t frame, bool full_resync) {
    return !full_resync && last_played.has_value() && !IsConsecutive(last_played, frame);
  }

  /// Whether a primary's delta can be this engine frame's time step.
  inline bool IsUsableDelta(float delta_seconds) {
    return std::isfinite(delta_seconds) && (delta_seconds >= 0.0f);
  }

} // namespace multigpu
} // namespace carla
