// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Character.h"

#include "PedestrianParameters.generated.h"

UENUM(BlueprintType)
enum class EPedestrianGender : uint8
{
  Other       UMETA(DisplayName = "Other"),
  Female      UMETA(DisplayName = "Female"),
  Male        UMETA(DisplayName = "Male"),

  SIZE        UMETA(Hidden),
  INVALID     UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct CARLA_API FPedestrianParameters
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Id;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<ACharacter> Class;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  EPedestrianGender Gender = EPedestrianGender::Other;
};
