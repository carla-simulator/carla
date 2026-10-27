// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

// Parse ordinary LibCarla dependencies before enabling its Unreal conversions.
#include <carla/MsgPack.h>
#include <carla/MsgPackAdaptors.h>

#include <cmath>
#include <iostream>
#include <optional>

// Only reflection boilerplate and the optional-value boundary are inert.
// FEpisodeSettings itself comes from the actual Carla plugin header below;
// both conversion bodies come from the actual RPC header, without extraction.
template <typename T>
class TOptional {
public:
  T Get(T fallback) const { return _value.value_or(fallback); }
  TOptional &operator=(T value) { _value = value; return *this; }
private:
  std::optional<T> _value;
};

#define USTRUCT(...)
#define UPROPERTY(...)
#define GENERATED_BODY(...)
#define CARLA_API
#define LIBCARLA_INCLUDED_FROM_UE4
#include <carla/rpc/EpisodeSettings.h>

int main() {
  int checks = 0;
  int failures = 0;
  const auto check = [&](bool condition, const char *message) {
    ++checks;
    if (!condition) {
      ++failures;
      std::cerr << message << '\n';
    }
  };
  for (const float metres : {0.125f, 1.0f, 125.5f, 4000.0f}) {
    carla::rpc::EpisodeSettings rpc;
    rpc.max_culling_distance = metres;
    const FEpisodeSettings unreal = rpc;
    check(unreal.MaxCullingDistance == metres * 100.0f,
          "RPC draw distance must enter Unreal in centimetres");

    FEpisodeSettings authored;
    authored.MaxCullingDistance = metres * 100.0f;
    const carla::rpc::EpisodeSettings converted(authored);
    check(std::abs(converted.max_culling_distance - metres) < 0.001f,
          "Unreal draw distance must leave as RPC metres");
  }
  const carla::rpc::EpisodeSettings defaults;
  const FEpisodeSettings native_defaults = defaults;
  check(native_defaults.MaxCullingDistance == 0.0f &&
        carla::rpc::EpisodeSettings(native_defaults).max_culling_distance == 0.0f,
        "Zero remains the no-distance-limit setting");

  carla::rpc::EpisodeSettings configured;
  configured.tile_stream_distance = 100.0f;
  configured.actor_active_distance = 50.0f;
  configured.fixed_delta_seconds = 0.05;
  configured.spectator_as_ego = false;
  const FEpisodeSettings native = configured;
  check(native.TileStreamingDistance == 10000.0f && native.ActorActiveDistance == 5000.0f,
        "Existing tile and actor distance conversions remain active");
  const carla::rpc::EpisodeSettings restored(native);
  check(restored == configured, "Other settings survive a complete round trip");
  std::cout << "checks=" << checks << " failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
