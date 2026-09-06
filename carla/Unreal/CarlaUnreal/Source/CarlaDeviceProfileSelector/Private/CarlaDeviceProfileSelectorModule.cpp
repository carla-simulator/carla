// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaDeviceProfileSelectorModule.h"

#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "Scalability.h"

#include <optional>
#include <string>
#include <string_view>

IMPLEMENT_MODULE(FCarlaDeviceProfileSelectorModule, CarlaDeviceProfileSelector);

namespace {

enum class QualityTier {
  Low,
  Medium,
  High,
  Epic,
};

// Tier selected when `-quality-level=` is absent on the command line.
inline constexpr std::string_view kDefaultQualityLevel{"Epic"};

// Case-sensitive lookup. Valid inputs are exactly the PascalCase strings
// "Low", "Medium", "High", "Epic". Any other input (different casing,
// surrounding whitespace, empty, or unknown tier) returns std::nullopt and
// the caller logs the unrecognised value and applies no tier.
[[nodiscard]] constexpr std::optional<QualityTier> ResolveQualityLevel(
    std::string_view value) {
  if (value == "Low") {
    return QualityTier::Low;
  }
  if (value == "Medium") {
    return QualityTier::Medium;
  }
  if (value == "High") {
    return QualityTier::High;
  }
  if (value == "Epic") {
    return QualityTier::Epic;
  }
  return std::nullopt;
}

QualityTier GResolvedTier{QualityTier::Epic};
FDelegateHandle GPostEngineInitHandle{};

void SetCVarInt(const TCHAR *name, int32 value) {
  if (auto *cv = IConsoleManager::Get().FindConsoleVariable(name); cv != nullptr) {
    cv->Set(value, ECVF_SetByDeviceProfile);
  }
}

void SetCVarFloat(const TCHAR *name, float value) {
  if (auto *cv = IConsoleManager::Get().FindConsoleVariable(name); cv != nullptr) {
    cv->Set(value, ECVF_SetByDeviceProfile);
  }
}

void ApplyLowTier() {
  // Memory budgets. ReflectionCaptureResolution is dropped to 128 for
  // softer cubemap probes (less mirror-like reflections on car paint).
  SetCVarInt(TEXT("r.Streaming.PoolSize"), 3000);
  SetCVarInt(TEXT("r.SkinCache.SceneMemoryLimitInMB"), 256);
  SetCVarInt(TEXT("r.Nanite.Streaming.PoolSize"), 128);
  SetCVarInt(TEXT("r.LumenScene.SurfaceCache.AtlasSize"), 2048);
  SetCVarInt(TEXT("r.Shadow.Virtual.MaxPhysicalPages"), 2048);
  SetCVarInt(TEXT("r.ReflectionCaptureResolution"), 128);

  // Foliage / grass off.
  SetCVarFloat(TEXT("foliage.DensityScale"), 0.0f);
  SetCVarFloat(TEXT("grass.DensityScale"), 0.0f);

  // TemporalAA blend shifted toward sharpness without changing the AA
  // method itself. UE5 TAA defaults bias the temporal blend toward
  // smoothness; these five CVars (verified against TemporalAA.cpp and
  // PostProcessTonemap.cpp in the UE5.5 fork) produce the HD-readable
  // "Low but sharp" look. Motion blur off.
  SetCVarFloat(TEXT("r.TemporalAACurrentFrameWeight"), 0.2f);
  SetCVarFloat(TEXT("r.TemporalAAFilterSize"), 0.5f);
  SetCVarInt(TEXT("r.TemporalAACatmullRom"), 1);
  SetCVarInt(TEXT("r.TemporalAA.HistoryScreenPercentage"), 200);
  SetCVarFloat(TEXT("r.Tonemapper.Sharpen"), 0.5f);
  SetCVarInt(TEXT("r.DefaultFeature.MotionBlur"), 0);
  SetCVarInt(TEXT("r.SpecularQuality"), 0);
}

void ApplyMediumTier() {
  SetCVarInt(TEXT("r.Streaming.PoolSize"), 3000);
  SetCVarInt(TEXT("r.SkinCache.SceneMemoryLimitInMB"), 512);
  SetCVarInt(TEXT("r.Nanite.Streaming.PoolSize"), 256);
  SetCVarInt(TEXT("r.Shadow.Virtual.MaxPhysicalPages"), 4096);
  SetCVarFloat(TEXT("r.Shadow.DistanceScale"), 1.5f);
  SetCVarInt(TEXT("r.ReflectionCaptureResolution"), 256);
  SetCVarFloat(TEXT("foliage.DensityScale"), 1.0f);
  SetCVarFloat(TEXT("grass.DensityScale"), 1.0f);
}

void ApplyHighTier() {
  // High mirrors Epic on every CVar this function sets except the six
  // axes at the end, which intentionally pull view distance, mesh LOD,
  // anisotropy, and Lumen reflection cost below Epic.
  SetCVarInt(TEXT("r.Streaming.PoolSize"), 4000);
  SetCVarInt(TEXT("r.SkinCache.SceneMemoryLimitInMB"), 1024);
  SetCVarInt(TEXT("r.Nanite.Streaming.PoolSize"), 512);
  SetCVarInt(TEXT("r.LumenScene.SurfaceCache.AtlasSize"), 4096);
  SetCVarInt(TEXT("r.Shadow.Virtual.MaxPhysicalPages"), 8192);
  SetCVarFloat(TEXT("r.Shadow.DistanceScale"), 2.0f);
  SetCVarInt(TEXT("r.Shadow.Virtual.UseFarShadowCulling"), 0);
  SetCVarInt(TEXT("r.ReflectionCaptureResolution"), 512);
  // Hand the medium-roughness band off from Lumen to SSR / reflection
  // captures so wet-asphalt-like surfaces (~0.2-0.3 roughness) stop
  // smearing traffic-light radiance via Lumen surface cache.
  SetCVarFloat(TEXT("r.Lumen.Reflections.MaxRoughnessToTrace"), 0.2f);
  SetCVarInt(TEXT("r.Streaming.AmortizeCPUToGPUCopy"), 1);
  SetCVarInt(TEXT("r.CustomDepth"), 3);
  SetCVarInt(TEXT("Foliage.DitheredLOD"), 1);
  SetCVarFloat(TEXT("foliage.DensityScale"), 1.0f);
  SetCVarFloat(TEXT("grass.DensityScale"), 1.0f);
  SetCVarInt(TEXT("r.DefaultFeature.AntiAliasing"), 1);

  // Six axes that distinguish High from Epic. They are bucket members in
  // [GroupName@N] sections of DefaultScalability.ini and would be reset
  // by any later scalability apply, so they go in at ECVF_SetByDeviceProfile
  // (priority 6) which beats the bucket apply at ECVF_SetByScalability
  // (priority 2) and the GUS re-apply that happens at engine init.
  SetCVarFloat(TEXT("r.ViewDistanceScale"), 0.8f);
  SetCVarInt(TEXT("r.SkeletalMeshLODBias"), 1);
  SetCVarInt(TEXT("r.MaxAnisotropy"), 4);
  SetCVarInt(TEXT("r.VT.MaxAnisotropy"), 4);
  SetCVarInt(TEXT("r.Lumen.Reflections.DownsampleFactor"), 2);
  SetCVarInt(TEXT("r.Lumen.Reflections.MaxBounces"), 1);
}

void ApplyEpicTier() {
  SetCVarInt(TEXT("r.Streaming.PoolSize"), 4000);
  SetCVarInt(TEXT("r.SkinCache.SceneMemoryLimitInMB"), 1024);
  SetCVarInt(TEXT("r.Nanite.Streaming.PoolSize"), 512);
  SetCVarInt(TEXT("r.LumenScene.SurfaceCache.AtlasSize"), 4096);
  SetCVarInt(TEXT("r.Shadow.Virtual.MaxPhysicalPages"), 8192);
  SetCVarFloat(TEXT("r.Shadow.DistanceScale"), 2.0f);
  SetCVarInt(TEXT("r.Shadow.Virtual.UseFarShadowCulling"), 0);
  SetCVarInt(TEXT("r.ReflectionCaptureResolution"), 512);
  SetCVarFloat(TEXT("r.Lumen.Reflections.MaxRoughnessToTrace"), 0.2f);
  SetCVarInt(TEXT("r.Streaming.AmortizeCPUToGPUCopy"), 1);
  SetCVarInt(TEXT("r.CustomDepth"), 3);
  SetCVarInt(TEXT("Foliage.DitheredLOD"), 1);
  SetCVarFloat(TEXT("foliage.DensityScale"), 1.0f);
  SetCVarFloat(TEXT("grass.DensityScale"), 1.0f);
  SetCVarInt(TEXT("r.DefaultFeature.AntiAliasing"), 1);
}

// Per-tier sg.*Quality bucket selection. Each tier defines its own mix so
// UGameUserSettings::ApplyNonResolutionSettings cannot leak the previous
// run's persisted scalability into the current tier. OnPostEngineInit
// re-invokes this for the resolved tier after the engine's apply has run,
// at the same ECVF_SetByScalability priority but later in the init
// sequence, so the tier-selected buckets land last.
void ApplyScalabilityForTier(QualityTier tier) {
  Scalability::FQualityLevels q{};
  switch (tier) {
    case QualityTier::Low:
      // HD-readable, flat-lit. Full-res render, full textures / LOD
      // / AA / shading kept high. ShadowQ=0 disables dynamic shadows;
      // the scene renders with no cast shadows. RQ=0 disables Lumen
      // reflections; reflection-capture cubemap probes still provide
      // a fixed specular character on metals. GIQ=2 keeps SW Lumen
      // GI on at engine "Medium" level so diffuse bounce fills the
      // non-sun-facing side of vehicles and the global SDF carries
      // indirect light to distant buildings -- without this,
      // low-albedo materials and background geometry crush to black.
      // PostProcessQ / EffectsQ / FoliageQ at 1 for the perf floor.
      q.ResolutionQuality = 100.0f;
      q.ViewDistanceQuality = 3;
      q.AntiAliasingQuality = 3;
      q.ShadowQuality = 0;
      q.GlobalIlluminationQuality = 2;
      q.ReflectionQuality = 0;
      q.PostProcessQuality = 1;
      q.TextureQuality = 3;
      q.EffectsQuality = 1;
      q.FoliageQuality = 1;
      q.ShadingQuality = 3;
      break;
    case QualityTier::Medium:
      // Engine "Medium" preset across the board.
      q.ResolutionQuality = 100.0f;
      q.ViewDistanceQuality = 2;
      q.AntiAliasingQuality = 2;
      q.ShadowQuality = 2;
      q.GlobalIlluminationQuality = 2;
      q.ReflectionQuality = 2;
      q.PostProcessQuality = 2;
      q.TextureQuality = 2;
      q.EffectsQuality = 2;
      q.FoliageQuality = 2;
      q.ShadingQuality = 2;
      break;
    case QualityTier::High:
    case QualityTier::Epic:
      // Engine "Epic" preset everywhere. High's six distinguishing axes
      // (r.ViewDistanceScale 0.8, r.SkeletalMeshLODBias 1, r.MaxAnisotropy
      // 4, r.VT.MaxAnisotropy 4, r.Lumen.Reflections.DownsampleFactor 2,
      // r.Lumen.Reflections.MaxBounces 1) are set in ApplyHighTier at
      // ECVF_SetByDeviceProfile, which beats this bucket apply and is
      // what makes High visibly cheaper than Epic.
      q.ResolutionQuality = 100.0f;
      q.ViewDistanceQuality = 3;
      q.AntiAliasingQuality = 3;
      q.ShadowQuality = 3;
      q.GlobalIlluminationQuality = 3;
      q.ReflectionQuality = 3;
      q.PostProcessQuality = 3;
      q.TextureQuality = 3;
      q.EffectsQuality = 3;
      q.FoliageQuality = 3;
      q.ShadingQuality = 3;
      break;
  }
  Scalability::SetQualityLevels(q, /*bApply=*/true);
}

void OnPostEngineInit() {
  // UGameUserSettings::ApplyNonResolutionSettings has run and re-applied
  // scalability from GameUserSettings.ini at ECVF_SetByScalability,
  // overwriting Low's bucket members. Re-apply the tier scalability so
  // the persisted state cannot shadow the tier-selected buckets.
  ApplyScalabilityForTier(GResolvedTier);
}

}  // namespace

void FCarlaDeviceProfileSelectorModule::StartupModule() {
  FString quality_level;
  if (!FParse::Value(FCommandLine::Get(), TEXT("-quality-level="), quality_level)) {
    quality_level = FString{ANSI_TO_TCHAR(kDefaultQualityLevel.data())};
  }

  const std::string quality_level_std{TCHAR_TO_UTF8(*quality_level)};
  const auto tier = ResolveQualityLevel(quality_level_std);
  if (!tier.has_value()) {
    UE_LOG(
        LogInit,
        Warning,
        TEXT("CarlaDeviceProfileSelector: unrecognised -quality-level=%s; no tier applied"),
        *quality_level);
    return;
  }

  GResolvedTier = *tier;

  switch (*tier) {
    case QualityTier::Low:
      ApplyLowTier();
      break;
    case QualityTier::Medium:
      ApplyMediumTier();
      break;
    case QualityTier::High:
      ApplyHighTier();
      break;
    case QualityTier::Epic:
      ApplyEpicTier();
      break;
  }
  ApplyScalabilityForTier(*tier);

  GPostEngineInitHandle = FCoreDelegates::OnPostEngineInit.AddStatic(&OnPostEngineInit);
}

void FCarlaDeviceProfileSelectorModule::ShutdownModule() {
  if (GPostEngineInitHandle.IsValid()) {
    FCoreDelegates::OnPostEngineInit.Remove(GPostEngineInitHandle);
    GPostEngineInitHandle.Reset();
  }
}
