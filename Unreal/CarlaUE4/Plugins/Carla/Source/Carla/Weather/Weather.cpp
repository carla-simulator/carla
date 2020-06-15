// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Weather/Weather.h"

AWeather::AWeather(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
  RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void AWeather::ApplyWeather(const FWeatherParameters &InWeather)
{
  SetWeather(InWeather);

#ifdef CARLA_WEATHER_EXTRA_LOG
  UE_LOG(LogCarla, Log, TEXT("Changing weather:"));
  UE_LOG(LogCarla, Log, TEXT("  - Cloudiness = %.2f"), Weather.Cloudiness);
  UE_LOG(LogCarla, Log, TEXT("  - Precipitation = %.2f"), Weather.Precipitation);
  UE_LOG(LogCarla, Log, TEXT("  - PrecipitationDeposits = %.2f"), Weather.PrecipitationDeposits);
  UE_LOG(LogCarla, Log, TEXT("  - WindIntensity = %.2f"), Weather.WindIntensity);
  UE_LOG(LogCarla, Log, TEXT("  - SunAzimuthAngle = %.2f"), Weather.SunAzimuthAngle);
  UE_LOG(LogCarla, Log, TEXT("  - SunAltitudeAngle = %.2f"), Weather.SunAltitudeAngle);
  UE_LOG(LogCarla, Log, TEXT("  - FogDensity = %.2f"), Weather.FogDensity);
  UE_LOG(LogCarla, Log, TEXT("  - FogDistance = %.2f"), Weather.FogDistance);
  UE_LOG(LogCarla, Log, TEXT("  - FogFalloff = %.2f"), Weather.FogFalloff);
  UE_LOG(LogCarla, Log, TEXT("  - Wetness = %.2f"), Weather.Wetness);
#endif // CARLA_WEATHER_EXTRA_LOG

  // Call the blueprint that actually changes the weather.
  RefreshWeather(Weather);
}

void AWeather::NotifyWeather()
{
  // Call the blueprint that actually changes the weather.
  RefreshWeather(Weather);
}

void AWeather::SetWeather(const FWeatherParameters &InWeather)
{
  Weather = InWeather;
}
