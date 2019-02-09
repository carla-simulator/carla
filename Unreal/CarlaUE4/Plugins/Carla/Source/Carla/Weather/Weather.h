// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"

#include "Carla/Weather/WeatherParameters.h"

#include "Weather.generated.h"

UCLASS(Abstract)
class CARLA_API AWeather : public AActor
{
  GENERATED_BODY()

public:

  AWeather(const FObjectInitializer& ObjectInitializer);

  UFUNCTION(BlueprintCallable)
  void ApplyWeather(const FWeatherParameters &WeatherParameters);

  UFUNCTION(BlueprintCallable)
  const FWeatherParameters &GetCurrentWeather() const
  {
    return Weather;
  }

protected:

  UFUNCTION(BlueprintImplementableEvent)
  void RefreshWeather(const FWeatherParameters &WeatherParameters);

private:

  UPROPERTY(VisibleAnywhere)
  FWeatherParameters Weather;
};
