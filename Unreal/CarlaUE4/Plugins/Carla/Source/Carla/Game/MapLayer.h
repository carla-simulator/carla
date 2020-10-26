// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "MapLayer.generated.h"

UENUM(BlueprintType)
enum class EMapLayer : uint8
{
  None            =  0          UMETA(DisplayName = "None"),
  Buildings       =  0x1        UMETA(DisplayName = "Buildings"),
  Decals          =  0x1 << 1   UMETA(DisplayName = "Decals"),
  Foliage         =  0x1 << 2   UMETA(DisplayName = "Folliage"),
  Ground          =  0x1 << 3   UMETA(DisplayName = "Ground"),
  Parked_Vehicles =  0x1 << 4   UMETA(DisplayName = "Parked_Vehicles"),
  Props           =  0x1 << 5   UMETA(DisplayName = "Props"),
  StreetLights    =  0x1 << 6   UMETA(DisplayName = "StreetLights"),
  Walls           =  0x1 << 7   UMETA(DisplayName = "Walls"),
  All             =  0xFF       UMETA(DisplayName = "All")
};
