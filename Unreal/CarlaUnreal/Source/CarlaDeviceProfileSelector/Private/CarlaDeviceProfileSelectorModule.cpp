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
#include <span>
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

// Per-tier policy for hardware ray tracing on ordinary scene-capture cameras:
// -1 follows each sensor's bUseRayTracing, 0 forces it off, 1 forces it on.
// Sensors that cannot render without ray tracing ignore a forced off. Read by
// the Carla plugin by name; registered here because this module loads first.
TAutoConsoleVariable<int32> CVarCameraUseRayTracing(
    TEXT("carla.Camera.UseRayTracing"),
    -1,
    TEXT("Per-tier hardware ray tracing policy for scene-capture cameras.\n")
    TEXT("  -1: Respect the per-sensor bUseRayTracing attribute.\n")
    TEXT("   0: Force ray tracing OFF on every camera that does not require it.\n")
    TEXT("   1: Force ray tracing ON on every camera."),
    ECVF_Default);

QualityTier GResolvedTier{QualityTier::Epic};
FDelegateHandle GPostEngineInitHandle{};

struct CVarOverride {
  const TCHAR *name;
  const TCHAR *value;
};

// Epic is the project default: DefaultEngine.ini carries the Epic values and
// DefaultScalability.ini carries one bucket per tier. Only the CVars that a
// project-setting or system-settings pin keeps above scalability priority can
// be lowered here, at ECVF_SetByDeviceProfile.
constexpr CVarOverride kLowOverrides[] = {
    {TEXT("r.DynamicGlobalIlluminationMethod"), TEXT("0")},
    {TEXT("r.ReflectionMethod"), TEXT("2")},
    {TEXT("r.Streaming.PoolSize"), TEXT("2000")},
    {TEXT("r.Nanite.Streaming.StreamingPoolSize"), TEXT("768")},
    {TEXT("r.ReflectionCaptureResolution"), TEXT("128")},
    {TEXT("r.CustomDepth"), TEXT("1")},
    {TEXT("r.DefaultFeature.MotionBlur"), TEXT("0")},
    {TEXT("r.AntiAliasingMethod"), TEXT("2")},
    {TEXT("r.Shadow.DistanceScale"), TEXT("1")},
    {TEXT("r.Shadow.Virtual.UseFarShadowCulling"), TEXT("1")},
    {TEXT("carla.Camera.UseRayTracing"), TEXT("0")},
};

constexpr CVarOverride kMediumOverrides[] = {
    {TEXT("r.DynamicGlobalIlluminationMethod"), TEXT("0")},
    {TEXT("r.ReflectionMethod"), TEXT("2")},
    {TEXT("r.Streaming.PoolSize"), TEXT("3000")},
    {TEXT("r.ReflectionCaptureResolution"), TEXT("256")},
    {TEXT("r.AntiAliasingMethod"), TEXT("2")},
    {TEXT("r.Shadow.DistanceScale"), TEXT("1.5")},
    {TEXT("r.Shadow.Virtual.UseFarShadowCulling"), TEXT("1")},
    {TEXT("carla.Camera.UseRayTracing"), TEXT("0")},
};

constexpr CVarOverride kHighOverrides[] = {
    {TEXT("carla.Camera.UseRayTracing"), TEXT("0")},
};

std::span<const CVarOverride> OverridesForTier(QualityTier tier) {
  switch (tier) {
    case QualityTier::Low:
      return kLowOverrides;
    case QualityTier::Medium:
      return kMediumOverrides;
    case QualityTier::High:
      return kHighOverrides;
    case QualityTier::Epic:
      break;
  }
  return {};
}

void ApplyOverrides(std::span<const CVarOverride> overrides) {
  for (const auto &entry : overrides) {
    if (auto *cv = IConsoleManager::Get().FindConsoleVariable(entry.name); cv != nullptr) {
      cv->Set(entry.value, ECVF_SetByDeviceProfile);
    }
  }
}

// Each tier selects the matching bucket (0..3) of every scalability group
// defined in DefaultScalability.ini. Applied at startup and again on
// OnPostEngineInit so the persisted GameUserSettings.ini scalability cannot
// shadow the selected tier.
void ApplyScalabilityForTier(QualityTier tier) {
  const int32 bucket = static_cast<int32>(tier);
  Scalability::FQualityLevels q{};
  q.ResolutionQuality = 100.0f;
  q.ViewDistanceQuality = bucket;
  q.AntiAliasingQuality = bucket;
  q.ShadowQuality = bucket;
  q.GlobalIlluminationQuality = bucket;
  q.ReflectionQuality = bucket;
  q.PostProcessQuality = bucket;
  q.TextureQuality = bucket;
  q.EffectsQuality = bucket;
  q.FoliageQuality = bucket;
  q.ShadingQuality = bucket;
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

  ApplyOverrides(OverridesForTier(*tier));
  ApplyScalabilityForTier(*tier);

  GPostEngineInitHandle = FCoreDelegates::GetOnPostEngineInit().AddStatic(&OnPostEngineInit);
}

void FCarlaDeviceProfileSelectorModule::ShutdownModule() {
  if (GPostEngineInitHandle.IsValid()) {
    FCoreDelegates::GetOnPostEngineInit().Remove(GPostEngineInitHandle);
    GPostEngineInitHandle.Reset();
  }
}
