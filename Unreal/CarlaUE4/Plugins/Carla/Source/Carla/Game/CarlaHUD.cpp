// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaHUD.h"

#include "GameFramework/PlayerController.h"

void ACarlaHUD::DrawHUD()
{
  Super::DrawHUD();

  auto Player = GetOwningPlayerController();
  if (Player == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("Can't find player controller!"));
    return;
  }

  if(DebugVehicle) {
    float YL = 1600.0f;
    float Y0 = 0.0f;
    DebugVehicle->DrawDebug(Canvas, YL, Y0);
  }

  double Now = FPlatformTime::Seconds();
  int i = 0;
  while (i < StringList.Num())
  {
    // project position from camera
    FVector2D Screen;
    if (Player->ProjectWorldLocationToScreen(StringList[i].Location, Screen, true))
    {
      DrawText(StringList[i].Str, StringList[i].Color, Screen.X, Screen.Y);
    }

    // check to remove the string
    if (Now >= StringList[i].TimeToDie)
    {
      StringList.RemoveAt(i);
    }
    else
      ++i;
  }

  while (i < LineList.Num())
  {
    // project position from camera
    FVector2D Begin, End;
    if (Player->ProjectWorldLocationToScreen(LineList[i].Begin, Begin, true) &&
        Player->ProjectWorldLocationToScreen(LineList[i].End, End, true))
    {
      DrawLine(Begin.X, Begin.Y, End.X, End.Y, LineList[i].Color, LineList[i].Thickness);
    }

    // check to remove the string
    if (Now >= LineList[i].TimeToDie)
    {
      LineList.RemoveAt(i);
    }
    else
      ++i;
  }
}

void ACarlaHUD::AddHUDString(const FString Str, const FVector Location, const FColor Color, double LifeTime)
{
  double Now = FPlatformTime::Seconds();
  HUDString Obj { Str, Location, Color, Now + LifeTime };
  StringList.Add(std::move(Obj));
}

void ACarlaHUD::AddHUDLine(const FVector Begin, const FVector End, const float Thickness, const FColor Color, double LifeTime)
{
  double Now = FPlatformTime::Seconds();
  HUDLine Obj { Begin, End, Thickness, Color, Now + LifeTime };
  LineList.Add(std::move(Obj));
}
