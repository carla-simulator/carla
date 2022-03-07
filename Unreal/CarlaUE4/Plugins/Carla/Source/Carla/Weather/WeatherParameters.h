// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "WeatherParameters.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FWeatherParameters
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
  float Cloudiness = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
  float Precipitation = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
  float PrecipitationDeposits = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
  float WindIntensity = 0.35f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "360.0", UIMin = "0.0", UIMax = "360.0"))
  float SunAzimuthAngle = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "-90.0", ClampMax = "90.0", UIMin = "-90.0", UIMax = "90.0"))
  float SunAltitudeAngle = 75.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
  float FogDensity = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", UIMin = "0.0", UIMax = "1000.0"))
  float FogDistance = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", UIMin = "0.0", UIMax = "10.0"))
  float FogFalloff = 0.2f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
  float Wetness = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
  float ScatteringIntensity = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "5.0", UIMin = "0.0", UIMax = "5.0"))
  float MieScatteringScale = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
  float RayleighScatteringScale = 0.0331f;

  bool operator==(const FWeatherParameters &Other) const
  {
    return (
      Cloudiness == Other.Cloudiness &&
      Precipitation == Other.Precipitation &&
      PrecipitationDeposits == Other.PrecipitationDeposits &&
      WindIntensity == Other.WindIntensity &&
      SunAzimuthAngle == Other.SunAzimuthAngle &&
      SunAltitudeAngle == Other.SunAltitudeAngle &&
      FogDensity == Other.FogDensity &&
      FogDistance == Other.FogDistance &&
      FogFalloff == Other.FogFalloff &&
      Wetness == Other.Wetness &&
      ScatteringIntensity == Other.ScatteringIntensity &&
      MieScatteringScale == Other.MieScatteringScale &&
      RayleighScatteringScale == Other.RayleighScatteringScale
    );
  }

  bool operator!=(const FWeatherParameters &Other) const
  {
    return !(*this == Other);
  }

  FString ToString() const
  {
    FString Print;
    Print += FString::Printf(TEXT("Cloudiness: %.2f, "), Cloudiness);
    Print += FString::Printf(TEXT("Precipitation: %.2f, "), Precipitation);
    Print += FString::Printf(TEXT("PrecipitationDeposits: %.2f, "), PrecipitationDeposits);
    Print += FString::Printf(TEXT("WindIntensity: %.2f, "), WindIntensity);
    Print += FString::Printf(TEXT("SunAzimuthAngle: %.2f, "), SunAzimuthAngle);
    Print += FString::Printf(TEXT("SunAltitudeAngle: %.2f, "), SunAltitudeAngle);
    Print += FString::Printf(TEXT("FogDensity: %.2f, "), FogDensity);
    Print += FString::Printf(TEXT("FogDistance: %.2f, "), FogDistance);
    Print += FString::Printf(TEXT("FogFalloff: %.2f, "), FogFalloff);
    Print += FString::Printf(TEXT("Wetness: %.2f, "), Wetness);
    Print += FString::Printf(TEXT("ScatteringIntensity: %.2f, "), ScatteringIntensity);
    Print += FString::Printf(TEXT("MieScatteringScale: %.2f, "), MieScatteringScale);
    Print += FString::Printf(TEXT("RayleighScatteringScale: %.2f, "), RayleighScatteringScale);
    return Print;
  }
};
