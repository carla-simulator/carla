// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Weather/Weather.h"
#include "Carla/Sensor/SceneCaptureCamera.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"
#include "ConstructorHelpers.h"

AWeather::AWeather(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrecipitationPostProcessMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(
        TEXT("Material'/Game/Carla/Static/GenericMaterials/00_MastersOpt/Screen_posProcess/M_screenDrops.M_screenDrops'")).Object;

    DustStormPostProcessMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(
        TEXT("Material'/Game/Carla/Static/GenericMaterials/00_MastersOpt/Screen_posProcess/M_screenDust_wind.M_screenDust_wind'")).Object;

    PrimaryActorTick.bCanEverTick = false;
    RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void AWeather::CheckWeatherPostProcessEffects()
{
    if (Weather.Precipitation > 0.0f)
        ActiveBlendables.Add(MakeTuple(PrecipitationPostProcessMaterial, Weather.Precipitation / 100.0f));
    else
        ActiveBlendables.Remove(PrecipitationPostProcessMaterial);

    if (Weather.DustStorm > 0.0f)
        ActiveBlendables.Add(MakeTuple(DustStormPostProcessMaterial, Weather.DustStorm / 100.0f));
    else
        ActiveBlendables.Remove(DustStormPostProcessMaterial);

    for (int32 i = 0; i < Sensors.Num(); i++)
    {
        auto& Sensor = Sensors[i];

        if (!IsValid(Sensor))
        {
            Sensors.RemoveAtSwap(i);
            if (i == Sensors.Num())
                break;
        }

        for (auto& ActiveBlendable : ActiveBlendables)
            Sensor->GetCaptureComponent2D()->PostProcessSettings.AddBlendable(ActiveBlendable.Key, ActiveBlendable.Value);
    }
}

void AWeather::ApplyWeather(const FWeatherParameters& InWeather)
{
    SetWeather(InWeather);
    CheckWeatherPostProcessEffects();

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
    UE_LOG(LogCarla, Log, TEXT("  - ScatteringIntensity = %.2f"), Weather.ScatteringIntensity);
    UE_LOG(LogCarla, Log, TEXT("  - MieScatteringScale = %.2f"), Weather.MieScatteringScale);
    UE_LOG(LogCarla, Log, TEXT("  - RayleighScatteringScale = %.2f"), Weather.RayleighScatteringScale);
    UE_LOG(LogCarla, Log, TEXT("  - DustStorm = %.2f"), Weather.DustStorm);
#endif // CARLA_WEATHER_EXTRA_LOG

    // Call the blueprint that actually changes the weather.
    RefreshWeather(Weather);
}

void AWeather::NotifyWeather(ASensor* Sensor)
{
    auto AsSceneCaptureCamera = Cast<ASceneCaptureCamera>(Sensor);
    if (AsSceneCaptureCamera != nullptr)
        Sensors.Add(AsSceneCaptureCamera);

    CheckWeatherPostProcessEffects();

    // Call the blueprint that actually changes the weather.
    RefreshWeather(Weather);
}

void AWeather::SetWeather(const FWeatherParameters& InWeather)
{
    Weather = InWeather;
}

void AWeather::SetDayNightCycle(const bool& active)
{
    DayNightCycle = active;
}
