// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// Workaround to fix Windows conflict: Windows changes the name of some functions (DrawText, LoadLibrary...)
// with Unicode / ANSI versions for his own API (DrawTextW, DrawTextA, LoadLibraryW, LoadLibraryA...).
// But the changes are global for the compiler. Deep in headers, Windows has something like:
// #ifdef UNICODE
//   #define DrawText  DrawTextW
//   #define LoadLibrary LoadLibraryW
// #else
//   #define DrawText  DrawTextA
//   #define LoadLibrary LoadLibraryA
// #endif
// Then the linker tries to find the function DrawTextW on an external DLL and an unresolved external error happens because
// Unreal has no function DrawTextW, it has just DrawText.
// We can fix that by just undefining the function that conflicts with the name of the Windows API in Unicode.
#undef DrawText

#include "Containers/Array.h"
#include "GameFramework/HUD.h"
#include "WheeledVehicleMovementComponent.h"
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

  UWheeledVehicleMovementComponent* DebugVehicle{nullptr};
  void AddDebugVehicleForTelemetry(UWheeledVehicleMovementComponent* Veh) { DebugVehicle = Veh; }

  void AddHUDString(const FString Str, const FVector Location, const FColor Color, double LifeTime);

private:
  TArray<HUDString> StringList;
};
