// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include <util/ue-header-guard-end.h>

#include "FGeoLocation.generated.h"

/// Geographic reference point (WGS84) carried by the MGRS data asset.
/// Ported from tier4/autoware-support.
USTRUCT(BlueprintType)
struct FGeoLocation
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Geolocation")
  double Latitude = 0.0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Geolocation")
  double Longitude = 0.0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Geolocation")
  double Altitude = 0.0;

  FGeoLocation() {}

  FGeoLocation(double InLat, double InLon, double InAlt)
    : Latitude(InLat), Longitude(InLon), Altitude(InAlt) {}
};
