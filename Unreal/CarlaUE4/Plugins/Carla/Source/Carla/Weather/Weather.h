// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Carla/Weather/WeatherParameters.h"

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

protected:

  UFUNCTION(BlueprintImplementableEvent)
  void RefreshWeather(const FWeatherParameters &WeatherParameters);

private:

  void CheckWeatherPostProcessEffects();

  UPROPERTY(VisibleAnywhere)
  FWeatherParameters Weather;

  UMaterial* PrecipitationPostProcessMaterial;

  UMaterial* DustStormPostProcessMaterial;

  TMap<UMaterial*, float> ActiveBlendables;

  UPROPERTY(VisibleAnywhere)
  TArray<ASceneCaptureCamera*> Sensors;

  UPROPERTY(EditAnywhere, Category = "Weather")
  bool DayNightCycle = true;
};
