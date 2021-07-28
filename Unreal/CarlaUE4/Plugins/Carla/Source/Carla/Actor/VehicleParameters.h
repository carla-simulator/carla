// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Vehicle/CarlaWheeledVehicle.h"

#include "VehicleParameters.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FVehicleParameters
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Make;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Model;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<ACarlaWheeledVehicle> Class;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 NumberOfWheels = 4;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 Generation = 0;

  /// (OPTIONAL) Use for custom classification of vehicles.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString ObjectType;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FColor> RecommendedColors;

  /// List of IDs of the drivers (pedestrians) supported by this vehicle, leave
  /// empty if no driver is supported.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<int32> SupportedDrivers;
};
