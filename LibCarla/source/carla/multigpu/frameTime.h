// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/multigpu/frameDataCodec.h"

#include <cstdint>
#include <istream>
#include <optional>
#include <ostream>

namespace carla {
namespace multigpu {

  /// Frame data packet 25: the primary's clocks and episode settings for one
  /// frame. A secondary plays frame N with exactly this delta and these times,
  /// so whatever it computes is stamped and integrated like on the primary.
  struct FrameTime {

    static constexpr uint8_t kPacketId = 25u;

    /// Size of the version 1 payload; later versions only append fields.
    static constexpr uint32_t kPayloadSizeV1 = 98u;

    uint8_t protocol_version = kMultiGpuProtocolVersion;

    /// Frame counter of frame N; equal to the frame's FrameCounter packet.
    uint64_t frame = 0u;

    /// The raw DeltaSeconds that UWorld::Tick(N) received, before time
    /// dilation and clamping.
    float delta_seconds = 0.0f;

    /// UCarlaEpisode clocks at the end of frame N.
    double elapsed_game_time = 0.0;
    double visual_game_time = 0.0;

    /// UWorld clocks at the start of frame N, before UWorld::Tick adds the
    /// delta to them.
    double world_time_seconds = 0.0;
    double world_unpaused_time_seconds = 0.0;
    double world_real_time_seconds = 0.0;
    double world_audio_time_seconds = 0.0;

    /// The primary's episode settings after frame N (FEpisodeSettings, in UE
    /// units). no_rendering_mode is informational: secondaries keep their own.
    bool synchronous_mode = false;
    bool no_rendering_mode = false;
    std::optional<double> fixed_delta_seconds;
    bool substepping = true;
    double max_substep_delta_time = 0.01;
    int32_t max_substeps = 10;
    float max_culling_distance = 0.0f;
    bool deterministic_ragdolls = true;
    float tile_streaming_distance = 300000.0f;
    float actor_active_distance = 200000.0f;
    bool spectator_as_ego = true;
    bool adjust_signs_height_to_ground = false;

    /// AWorldSettings::GetEffectiveTimeDilation() of frame N.
    float time_dilation = 1.0f;

    /// Set on the frame that rebuilds a secondary from scratch (a new or
    /// reconnected secondary, or the first frame after a map load).
    bool full_resync = false;

    void Write(std::ostream &out) const {
      WriteFrameDataHeader(out, kPacketId, kPayloadSizeV1);
      detail::WritePod(out, protocol_version);
      detail::WritePod(out, PackFlags());
      detail::WritePod(out, frame);
      detail::WritePod(out, delta_seconds);
      detail::WritePod(out, elapsed_game_time);
      detail::WritePod(out, visual_game_time);
      detail::WritePod(out, world_time_seconds);
      detail::WritePod(out, world_unpaused_time_seconds);
      detail::WritePod(out, world_real_time_seconds);
      detail::WritePod(out, world_audio_time_seconds);
      detail::WritePod(out, fixed_delta_seconds.value_or(0.0));
      detail::WritePod(out, max_substep_delta_time);
      detail::WritePod(out, max_substeps);
      detail::WritePod(out, max_culling_distance);
      detail::WritePod(out, tile_streaming_distance);
      detail::WritePod(out, actor_active_distance);
      detail::WritePod(out, time_dilation);
    }

    /// Reads a payload of @a size bytes. Fails (and fails @a in) on a payload
    /// shorter than version 1; bytes beyond the fields known here are left for
    /// the packet loop to skip.
    bool Read(std::istream &in, uint32_t size) {
      if (size < kPayloadSizeV1) {
        in.setstate(std::ios_base::failbit);
        return false;
      }
      BoundedReader reader(in, size);
      uint8_t flags = 0u;
      double fixed_delta = 0.0;
      const bool ok =
          reader.Read(protocol_version) &&
          reader.Read(flags) &&
          reader.Read(frame) &&
          reader.Read(delta_seconds) &&
          reader.Read(elapsed_game_time) &&
          reader.Read(visual_game_time) &&
          reader.Read(world_time_seconds) &&
          reader.Read(world_unpaused_time_seconds) &&
          reader.Read(world_real_time_seconds) &&
          reader.Read(world_audio_time_seconds) &&
          reader.Read(fixed_delta) &&
          reader.Read(max_substep_delta_time) &&
          reader.Read(max_substeps) &&
          reader.Read(max_culling_distance) &&
          reader.Read(tile_streaming_distance) &&
          reader.Read(actor_active_distance) &&
          reader.Read(time_dilation);
      if (!ok) {
        return false;
      }
      UnpackFlags(flags);
      fixed_delta_seconds.reset();
      if ((flags & kFixedDelta) != 0u) {
        fixed_delta_seconds = fixed_delta;
      }
      return true;
    }

  private:

    enum Flag : uint8_t {
      kSynchronousMode = 1u << 0u,
      kFixedDelta = 1u << 1u,
      kSubstepping = 1u << 2u,
      kDeterministicRagdolls = 1u << 3u,
      kSpectatorAsEgo = 1u << 4u,
      kAdjustSignsHeight = 1u << 5u,
      kNoRenderingMode = 1u << 6u,
      kFullResync = 1u << 7u
    };

    uint8_t PackFlags() const {
      uint8_t flags = 0u;
      flags |= synchronous_mode ? kSynchronousMode : 0u;
      flags |= fixed_delta_seconds.has_value() ? kFixedDelta : 0u;
      flags |= substepping ? kSubstepping : 0u;
      flags |= deterministic_ragdolls ? kDeterministicRagdolls : 0u;
      flags |= spectator_as_ego ? kSpectatorAsEgo : 0u;
      flags |= adjust_signs_height_to_ground ? kAdjustSignsHeight : 0u;
      flags |= no_rendering_mode ? kNoRenderingMode : 0u;
      flags |= full_resync ? kFullResync : 0u;
      return flags;
    }

    void UnpackFlags(uint8_t flags) {
      synchronous_mode = (flags & kSynchronousMode) != 0u;
      substepping = (flags & kSubstepping) != 0u;
      deterministic_ragdolls = (flags & kDeterministicRagdolls) != 0u;
      spectator_as_ego = (flags & kSpectatorAsEgo) != 0u;
      adjust_signs_height_to_ground = (flags & kAdjustSignsHeight) != 0u;
      no_rendering_mode = (flags & kNoRenderingMode) != 0u;
      full_resync = (flags & kFullResync) != 0u;
    }
  };

} // namespace multigpu
} // namespace carla
