// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Containers/Array.h"
#include "GameFramework/HUD.h"

#include "CarlaHUD.generated.h"

struct HUDString
{
  FString Str { "" };
  FVector Location;
  FColor Color;
  double TimeToDie;
};

/// Class to draw on HUD
UCLASS()
class CARLA_API ACarlaHUD : public AHUD
{
  GENERATED_BODY()

public:

  ACarlaHUD(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
  {
    PrimaryActorTick.bCanEverTick = false;
  }

  virtual void DrawHUD() override;

  void AddHUDString(const FString Str, const FVector Location, const FColor Color, double LifeTime);

private:
  TArray<HUDString> StringList;
};
