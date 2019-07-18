// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// Workaround to fix Windows conflict: Windows change the name of some functions (DrawText, LoadLibrary...)
// with Unicode / ANSI versions for his own API (DrawTextW, DrawTextA, LoadLibraryW, LoadLibraryA...).
// But the changes are global for the compiler. Deep in headers, windows has something like:
// #ifdef UNICODE
//   #define DrawText  DrawTextW
//   #define LoadLibrary LoadLibraryW
// #else
//   #define DrawText  DrawTextA
//   #define LoadLibrary LoadLibraryA
// #endif
// Then linker tries to find function DrawTextW on external DLL and an unresolved external error happens because
// Unreal has no the function DrawTextW, it has just DrawText.
// We fix that just undefining the function that conflicts by name with the Windows API Unicode.
#undef DrawText

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
