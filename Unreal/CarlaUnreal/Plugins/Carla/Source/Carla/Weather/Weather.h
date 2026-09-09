// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Weather/WeatherParameters.h"

#include <util/ue-header-guard-begin.h>
#include "GameFramework/Actor.h"
#include <util/ue-header-guard-end.h>

#include "Weather.generated.h"

class ASensor;
class ASceneCaptureCamera;

UCLASS(Abstract)
class CARLA_API AWeather : public AActor
{
  GENERATED_BODY()

public:

  AWeather(const FObjectInitializer& ObjectInitializer);

  /// Update the weather parameters and notifies it to the blueprint's event
  UFUNCTION(BlueprintCallable)
  void ApplyWeather(const FWeatherParameters &WeatherParameters);

  /// Notifing the weather to the blueprint's event
  void NotifyWeather(ASensor* Sensor = nullptr);

  /// Update the weather parameters without notifing it to the blueprint's event
  UFUNCTION(BlueprintCallable)
  void SetWeather(const FWeatherParameters &WeatherParameters);

  /// Returns the current WeatherParameters
  UFUNCTION(BlueprintCallable)
  const FWeatherParameters &GetCurrentWeather() const
  {
    return Weather;
  }

  /// Returns whether the day night cycle is active (automatic on/off switch when changin to night mode)
  UFUNCTION(BlueprintCallable)
  const bool &GetDayNightCycle() const
  {
    return DayNightCycle;
  }

  /// Update the day night cycle
  void SetDayNightCycle(const bool &active);

  /// Pushes Weather into a single ASkyBase rig (sun/fog/clouds/atmosphere/
  /// exposure) and runs its blueprint refresh functions -- everything
  /// PushWeatherToSky does per-actor, factored out so it works on any sky
  /// actor directly, without requiring an AWeather instance to exist in the
  /// world. Used by ASkyBase::RefreshWeather/LoadPreset for editor-time
  /// preview when no BP_CarlaWeather is placed in the level, and by
  /// PushWeatherToSky itself for the full runtime path.
  UFUNCTION(BlueprintCallable, Category = "Weather")
  static void ApplyWeatherToSkyActor(AActor* SkyActor, const FWeatherParameters& Weather);

  /// Environment-map override for the sky light (set_sky_light_map RPC):
  /// every ASkyBase rig in World switches its sky light to the given cubemap
  /// (SLS_SpecifiedCubemap, real-time capture off) at the given intensity,
  /// and ApplyWeatherToSkyActor keeps it that way across weather pushes
  /// until ClearSkyLightMap. Scoped to World: it is dropped when that world
  /// is cleaned up (load_world / reload_world), so the next level starts on
  /// its own real-time capture. The cubemap is rooted for as long as it is
  /// set. Returns false if no sky rig was found.
  static bool SetSkyLightMap(UWorld* World, class UTextureCube* Cubemap, float Intensity);

  /// Drops the override and restores the real-time atmosphere capture with
  /// the curve-driven intensity.
  static void ClearSkyLightMap(UWorld* World);

  static bool HasSkyLightMap();

protected:

#if WITH_EDITOR
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

  UFUNCTION(BlueprintImplementableEvent)
  void RefreshWeather(const FWeatherParameters &WeatherParameters);

private:

  void CheckWeatherPostProcessEffects();

  /// Push the current weather into the ASkyBase rig (BP_Carla_Sky) and run its
  /// blueprint Update. The UE5 sky rig only refreshes itself from its
  /// construction script, so without this call the sun/fog/atmosphere stay
  /// frozen at the state saved in the map regardless of the weather API.
  void PushWeatherToSky();

  /// Broadcast the current day/night state (derived from SunAltitudeAngle) to
  /// UCarlaLightSubsystem so registered CarlaLight components (street lamps)
  /// react. BP_CarlaWeather's graph does reference the subsystem and
  /// DayTimeChangeEvent, but measured behavior is that lights never receive
  /// it (same blueprint has other confirmed-dead exec chains that
  /// PushWeatherToSky already routes around) -- drive it from C++ instead so
  /// it does not depend on that graph being reachable.
  void UpdateStreetLightsForDayNight();

  UPROPERTY(EditAnywhere, Category="Parameters")
  FWeatherParameters Weather;

  UMaterial* PrecipitationPostProcessMaterial;

  UMaterial* DustStormPostProcessMaterial;

  TMap<UMaterial*, float> ActiveBlendables;

  UPROPERTY(EditAnywhere, Category = "Weather")
  bool DayNightCycle = true;
};
