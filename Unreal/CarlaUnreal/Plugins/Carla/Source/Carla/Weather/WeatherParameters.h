// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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

  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
  float DustStorm = 0.0f;
};
