#include "Sky.h"

#include <util/ue-header-guard-begin.h>
#include "Components/PostProcessComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include <util/ue-header-guard-end.h>

#include "BlueprintLibary/PostProcessJsonUtils.h"
#include "BlueprintLibary/WeatherJsonUtils.h"
#include "Carla/Weather/Weather.h"

#include <util/ue-header-guard-begin.h>
#include "Kismet/GameplayStatics.h"
#include <util/ue-header-guard-end.h>

namespace
{
    AWeather* FindWeatherActor(const UWorld* World)
    {
        return World != nullptr
            ? Cast<AWeather>(UGameplayStatics::GetActorOfClass(World, AWeather::StaticClass()))
            : nullptr;
    }

    // "Current weather" for this sky rig: if a BP_CarlaWeather happens to be
    // placed in the level, its Weather member is authoritative (same value
    // the API/recorder see). Otherwise fall back to this actor's own "Sky
    // Parameters" -- kept in sync by ApplyWeatherTo below -- so none of this
    // requires an AWeather actor to be placed at all.
    FWeatherParameters GetEffectiveCurrentWeather(AActor* SkyActor)
    {
        if (AWeather* Weather = FindWeatherActor(SkyActor->GetWorld()))
            return Weather->GetCurrentWeather();
        if (FWeatherParameters* Params = ASkyBase::FindWeatherParameters(SkyActor))
            return *Params;
        return FWeatherParameters();
    }

    // Applies NewWeather to this sky rig. Prefers routing through a placed
    // AWeather (full pipeline: post-process camera blendables, day/night
    // light broadcast, recorder) when one exists; otherwise pushes directly
    // to the rig via the same per-actor logic AWeather itself uses.
    void ApplyWeatherTo(AActor* SkyActor, const FWeatherParameters& NewWeather)
    {
        if (AWeather* Weather = FindWeatherActor(SkyActor->GetWorld()))
            Weather->ApplyWeather(NewWeather);
        else
            AWeather::ApplyWeatherToSkyActor(SkyActor, NewWeather);
    }
}


FWeatherParameters* ASkyBase::FindWeatherParameters(AActor* SkyActor)
{
	for (TFieldIterator<FProperty> It(SkyActor->GetClass()); It; ++It)
	{
		FStructProperty* StructProperty = CastField<FStructProperty>(*It);
		if (StructProperty != nullptr && StructProperty->Struct == FWeatherParameters::StaticStruct())
			return StructProperty->ContainerPtrToValuePtr<FWeatherParameters>(SkyActor);
	}
	return nullptr;
}

ASkyBase::ASkyBase(
	const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
  PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>("PostProcessComponent");
  RootComponent = PostProcessComponent;

  ExponentialHeightFogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>("ExponentialHeightFogComponent");
  ExponentialHeightFogComponent->SetupAttachment(RootComponent);

  DirectionalLightComponentSun = CreateDefaultSubobject<UDirectionalLightComponent>("DirectionalLightComponentSun");
  DirectionalLightComponentSun->SetupAttachment(RootComponent);
  // Both lights default to ForwardShadingPriority 0, so forward shading/
  // translucency/volumetric fog fall back to picking whichever is brighter
  // right now -- flip-flopping as day turns to night -- and the engine logs
  // "Multiple directional lights are competing..." every time. The Sun
  // should always win outright; it's the one everything (fog, water,
  // volumetrics) is actually meant to be lit by.
  DirectionalLightComponentSun->ForwardShadingPriority = 1;

  DirectionalLightComponentMoon = CreateDefaultSubobject<UDirectionalLightComponent>("DirectionalLightComponentMoon");
  DirectionalLightComponentMoon->SetupAttachment(RootComponent);
  // Below the Sun's priority (1) so it never contends for forward shading.
  // Separately: every DirectionalLightComponent defaults bAtmosphereSunLight
  // true -- a completely different resolution SkyAtmosphere uses to pick its
  // single "sun" reference for scattering, independent of ForwardShadingPriority
  // above. Leaving the Moon eligible there let it win that tie against the
  // real Sun, which reads as a broken/black sky once the sun actually rises
  // (SkyAtmosphere doing all its math against the moon's position instead).
  DirectionalLightComponentMoon->ForwardShadingPriority = -1;
  DirectionalLightComponentMoon->SetAtmosphereSunLight(false);

  SkyLightComponent = CreateDefaultSubobject<USkyLightComponent>("SkyLightComponent");
  SkyLightComponent->SetupAttachment(RootComponent);

  VolumetricCloudComponent = CreateDefaultSubobject<UVolumetricCloudComponent>("VolumetricCloudComponent");
  VolumetricCloudComponent->SetupAttachment(RootComponent);

  SkyAtmosphereComponent = CreateDefaultSubobject<USkyAtmosphereComponent>("SkyAtmosphereComponent");
  SkyAtmosphereComponent->SetupAttachment(RootComponent);

}

void ASkyBase::SaveProfile()
{
  UPostProcessJsonUtils::SaveAllPostProcessComponentToJson(PostProcessComponent, ProfileName);
}

void ASkyBase::LoadProfile()
{
  UPostProcessJsonUtils::LoadAllPostProcessFromJsonToPostProcessComponent(PostProcessComponent, ProfileName);

  // Profiles that don't claim AutoExposureMinBrightness/MaxBrightness (i.e.
  // rely on Weather's fallback clamp, see ApplyWeatherToSkyActor) store those
  // overrides as false -- loading them resets the component straight back to
  // the engine's own AutoExposure defaults until something else happens to
  // re-trigger the fallback. Re-run it immediately so a profile switch alone
  // (no other property edit in between) doesn't leave exposure unclamped.
  RefreshWeather();

  // CameraParameters is a curated view onto the same Settings this profile
  // just overwrote wholesale -- resync it so the shortcuts reflect the
  // loaded profile instead of whatever was left over from before.
  SyncCameraParametersFromPostProcess();
}

void ASkyBase::CreateProfile()
{
  if (NewProfileName.IsEmpty())
    return;
  ProfileName = NewProfileName;
  SaveProfile();
}

TArray<FString> ASkyBase::GetAvailablePostProcessProfileNamesForPicker()
{
  return UPostProcessJsonUtils::GetAvailablePostProcessProfileNames();
}

void ASkyBase::ApplyTimeAndConditionPresets()
{
  FWeatherTimePreset TimePreset;
  FWeatherConditionPreset ConditionPreset;
  const bool bTimeOk = UWeatherJsonUtils::LoadTimePreset(TimePreset, TimePresetName);
  const bool bConditionOk = UWeatherJsonUtils::LoadConditionPreset(ConditionPreset, ConditionPresetName);
  if (!bTimeOk && !bConditionOk)
    return;

  // Only overwrite the half that actually loaded, so a missing condition
  // preset (say) doesn't reset the sun position to defaults.
  FWeatherParameters Base = GetEffectiveCurrentWeather(this);
  if (!bTimeOk)
  {
    TimePreset.SunAzimuthAngle = Base.SunAzimuthAngle;
    TimePreset.SunAltitudeAngle = Base.SunAltitudeAngle;
  }
  if (!bConditionOk)
  {
    ConditionPreset.Cloudiness = Base.Cloudiness;
    ConditionPreset.Precipitation = Base.Precipitation;
    ConditionPreset.PrecipitationDeposits = Base.PrecipitationDeposits;
    ConditionPreset.WindIntensity = Base.WindIntensity;
    ConditionPreset.FogDensity = Base.FogDensity;
    ConditionPreset.FogDistance = Base.FogDistance;
    ConditionPreset.FogFalloff = Base.FogFalloff;
    ConditionPreset.Wetness = Base.Wetness;
    ConditionPreset.ScatteringIntensity = Base.ScatteringIntensity;
    ConditionPreset.MieScatteringScale = Base.MieScatteringScale;
    ConditionPreset.RayleighScatteringScale = Base.RayleighScatteringScale;
  }

  ApplyWeatherTo(this, UWeatherJsonUtils::MergeWeatherPresets(Base, TimePreset, ConditionPreset));

  // A hand-edit of a WeatherParameters field always gets a FULL editor
  // reconstruction cycle for free (property edit -> OnConstruction -> the
  // Blueprint's own construction graph runs, THEN this class's C++ override
  // re-pushes). This function updates the struct and pushes natively, same
  // as OnConstruction's own push, but skips the Blueprint graph's own pass
  // entirely -- if that pass does any one-time setup the raw push doesn't
  // replicate (respawning/re-registering the sky sphere, stars material,
  // etc.), the result can look wrong until some later, unrelated edit
  // finally triggers a real reconstruction. Force that same full cycle here
  // instead of leaving it to chance. Editor-only API -- irrelevant at
  // runtime anyway, OnConstruction only ever runs once there, at spawn.
#if WITH_EDITOR
  RerunConstructionScripts();
#endif
}

void ASkyBase::B_SaveTimePreset()
{
  const FWeatherParameters Current = GetEffectiveCurrentWeather(this);
  FWeatherTimePreset Preset;
  Preset.SunAzimuthAngle = Current.SunAzimuthAngle;
  Preset.SunAltitudeAngle = Current.SunAltitudeAngle;
  UWeatherJsonUtils::SaveTimePreset(Preset, TimePresetName);
}

void ASkyBase::C_SaveConditionPreset()
{
  const FWeatherParameters Current = GetEffectiveCurrentWeather(this);
  FWeatherConditionPreset Preset;
  Preset.Cloudiness = Current.Cloudiness;
  Preset.Precipitation = Current.Precipitation;
  Preset.PrecipitationDeposits = Current.PrecipitationDeposits;
  Preset.WindIntensity = Current.WindIntensity;
  Preset.FogDensity = Current.FogDensity;
  Preset.FogDistance = Current.FogDistance;
  Preset.FogFalloff = Current.FogFalloff;
  Preset.Wetness = Current.Wetness;
  Preset.ScatteringIntensity = Current.ScatteringIntensity;
  Preset.MieScatteringScale = Current.MieScatteringScale;
  Preset.RayleighScatteringScale = Current.RayleighScatteringScale;
  UWeatherJsonUtils::SaveConditionPreset(Preset, ConditionPresetName);
}

void ASkyBase::F_CreateTimePreset()
{
  if (NewTimePresetName.IsEmpty())
    return;
  TimePresetName = NewTimePresetName;
  B_SaveTimePreset();
}

void ASkyBase::G_CreateConditionPreset()
{
  if (NewConditionPresetName.IsEmpty())
    return;
  ConditionPresetName = NewConditionPresetName;
  C_SaveConditionPreset();
}

void ASkyBase::RefreshWeather()
{
  if (FWeatherParameters* SkyParameters = ASkyBase::FindWeatherParameters(this))
    ApplyWeatherTo(this, *SkyParameters);
}

void ASkyBase::H_LoadMapDefault()
{
  const UWorld* World = GetWorld();
  if (World == nullptr)
    return;
  FString MapName = World->GetMapName();
  MapName.RemoveFromStart(World->StreamingLevelsPrefix);

  const FWeatherParameters Base = GetEffectiveCurrentWeather(this);
  FWeatherParameters MapDefault;
  if (UWeatherJsonUtils::GetMapDefaultWeather(MapName, Base, MapDefault))
  {
    ApplyWeatherTo(this, MapDefault);
#if WITH_EDITOR
    RerunConstructionScripts();  // see the comment in ApplyTimeAndConditionPresets
#endif
  }
}

void ASkyBase::PushCameraParameters()
{
  FPostProcessSettings& Settings = PostProcessComponent->Settings;

  // NOTE: ShutterSpeed/ISO/Aperture only actually affect the image when
  // ExposureMode is AEM_Manual -- under AEM_Histogram (the default, what
  // Default/Test.json use) UE's auto exposure ignores them entirely, they're
  // visually inert. Flip ExposureMode below to Manual to make them matter
  // (that's what GoPro.json does). A separate attempt at making them work
  // under Histogram too, by forcing AutoExposureApplyPhysicalCameraExposure
  // on, was reverted: it broke night-time exposure (fights the auto-exposure
  // system's own day/night adaptation).

  // ExposureMinEV/MaxEV are intentionally NOT pushed here (read-only,
  // display-only in FCameraParameters -- see SyncCameraParametersFromPost
  // Process below). Default/Test.json deliberately leave these two fields
  // unclaimed (bOverride_AutoExposureMinBrightness/MaxBrightness = false) so
  // AWeather::ApplyWeatherToSkyActor's own fallback can fill them in per
  // push; forcing them here on every edit claimed them permanently, and once
  // that got captured by a SaveProfile click it corrupted Default.json into
  // a fixed EV range that badly over-exposed night scenes (the moon at night
  // rendering as a blown-out blue sky). Fixed by reverting that override in
  // Default.json; don't reintroduce the same push here.
  Settings.bOverride_LocalExposureHighlightContrastScale = true;
  Settings.LocalExposureHighlightContrastScale = CameraParameters.HighlightContrast;
  Settings.bOverride_LocalExposureShadowContrastScale = true;
  Settings.LocalExposureShadowContrastScale = CameraParameters.ShadowContrast;

  Settings.bOverride_CameraShutterSpeed = true;
  Settings.CameraShutterSpeed = CameraParameters.ShutterSpeed;
  Settings.bOverride_AutoExposureMethod = true;
  Settings.AutoExposureMethod = CameraParameters.ExposureMode;
  Settings.bOverride_CameraISO = true;
  Settings.CameraISO = CameraParameters.ISO;
  Settings.bOverride_DepthOfFieldFstop = true;
  Settings.DepthOfFieldFstop = CameraParameters.Aperture;

  Settings.bOverride_DepthOfFieldSensorWidth = true;
  Settings.DepthOfFieldSensorWidth = CameraParameters.SensorWidth;
  Settings.bOverride_DepthOfFieldFocalDistance = true;
  Settings.DepthOfFieldFocalDistance = CameraParameters.FocalDistance;

  Settings.bOverride_WhiteTemp = true;
  Settings.WhiteTemp = CameraParameters.Temperature;
  Settings.bOverride_WhiteTint = true;
  Settings.WhiteTint = CameraParameters.Tint;
  Settings.bOverride_SceneColorTint = true;
  Settings.SceneColorTint = CameraParameters.SceneColorTint;
  Settings.bOverride_ColorSaturation = true;
  Settings.ColorSaturation = FVector4(CameraParameters.GlobalSaturation, CameraParameters.GlobalSaturation,
      CameraParameters.GlobalSaturation, Settings.ColorSaturation.W);
  Settings.bOverride_ColorContrast = true;
  Settings.ColorContrast = FVector4(CameraParameters.GlobalContrast, CameraParameters.GlobalContrast,
      CameraParameters.GlobalContrast, Settings.ColorContrast.W);
  Settings.bOverride_ColorGamma = true;
  Settings.ColorGamma = FVector4(CameraParameters.GlobalGamma, CameraParameters.GlobalGamma,
      CameraParameters.GlobalGamma, Settings.ColorGamma.W);
  Settings.bOverride_VignetteIntensity = true;
  Settings.VignetteIntensity = CameraParameters.VignetteIntensity;
}

void ASkyBase::SyncCameraParametersFromPostProcess()
{
  const FPostProcessSettings& Settings = PostProcessComponent->Settings;

  CameraParameters.ExposureMinEV = Settings.AutoExposureMinBrightness;
  CameraParameters.ExposureMaxEV = Settings.AutoExposureMaxBrightness;
  CameraParameters.HighlightContrast = Settings.LocalExposureHighlightContrastScale;
  CameraParameters.ShadowContrast = Settings.LocalExposureShadowContrastScale;

  CameraParameters.ShutterSpeed = Settings.CameraShutterSpeed;
  CameraParameters.ExposureMode = Settings.AutoExposureMethod;
  CameraParameters.ISO = Settings.CameraISO;
  CameraParameters.Aperture = Settings.DepthOfFieldFstop;

  CameraParameters.SensorWidth = Settings.DepthOfFieldSensorWidth;
  CameraParameters.FocalDistance = Settings.DepthOfFieldFocalDistance;

  CameraParameters.Temperature = Settings.WhiteTemp;
  CameraParameters.Tint = Settings.WhiteTint;
  CameraParameters.SceneColorTint = Settings.SceneColorTint;
  CameraParameters.GlobalSaturation = Settings.ColorSaturation.X;
  CameraParameters.GlobalContrast = Settings.ColorContrast.X;
  CameraParameters.GlobalGamma = Settings.ColorGamma.X;
  CameraParameters.VignetteIntensity = Settings.VignetteIntensity;
}

#if WITH_EDITOR
void ASkyBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);

  const FName PropertyName = PropertyChangedEvent.GetPropertyName();
  if (PropertyName == GET_MEMBER_NAME_CHECKED(ASkyBase, TimePresetName)
      || PropertyName == GET_MEMBER_NAME_CHECKED(ASkyBase, ConditionPresetName))
  {
    ApplyTimeAndConditionPresets();
  }
}
#endif

void ASkyBase::OnConstruction(const FTransform& Transform)
{
  Super::OnConstruction(Transform);

  // OnConstruction also fires once at actor spawn in a game world (PIE or
  // packaged), not just on editor property edits. Pushing here unconditionally
  // meant BOTH this AND ACarlaGameModeBase::BeginPlay's own weather push ran
  // at Play start, and SetSkySphere's respawn (destroy-old, spawn-new) isn't
  // safe against two independent, near-simultaneous callers -- it produced
  // duplicate BP_Sky_Sphere/DirectionalLight actors and the "multiple
  // directional lights competing" warning. Editor-only: let BeginPlay's push
  // be the sole driver once actually playing.
  if (GetWorld() != nullptr && GetWorld()->IsGameWorld())
    return;

  // Direct push, not ApplyWeatherTo: this must never depend on finding an
  // AWeather actor (irrelevant here) or go through its full pipeline (camera
  // blendables, recorder) -- just put this rig's own components back in sync
  // with whatever "Sky Parameters" already holds.
  if (FWeatherParameters* SkyParameters = ASkyBase::FindWeatherParameters(this))
    AWeather::ApplyWeatherToSkyActor(this, *SkyParameters);

  // Same self-healing story as Sky Parameters above: any edit reruns
  // Construction and would otherwise leave CameraParameters' last pushed
  // values sitting unapplied against a Settings struct that just got reset.
  PushCameraParameters();
}

void ASkyBase::E_SetAsMapDefault()
{
  const UWorld* World = GetWorld();
  if (World == nullptr)
    return;
  FString MapName = World->GetMapName();
  MapName.RemoveFromStart(World->StreamingLevelsPrefix);
  UWeatherJsonUtils::SetMapDefaultPresetNames(MapName, TimePresetName, ConditionPresetName);
}

void ASkyBase::D_SaveCurrentWeatherAsMapDefault()
{
  const UWorld* World = GetWorld();
  if (World == nullptr)
    return;
  FString MapName = World->GetMapName();
  MapName.RemoveFromStart(World->StreamingLevelsPrefix);
  if (FWeatherParameters* SkyParameters = ASkyBase::FindWeatherParameters(this))
    UWeatherJsonUtils::SetMapDefaultWeather(MapName, *SkyParameters);
}

TArray<FString> ASkyBase::GetAvailableTimePresetNamesForPicker()
{
  return UWeatherJsonUtils::GetAvailableTimePresetNames();
}

TArray<FString> ASkyBase::GetAvailableConditionPresetNamesForPicker()
{
  return UWeatherJsonUtils::GetAvailableConditionPresetNames();
}
