// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaHUD.h"

void ACarlaHUD::DrawHUD()
{
  Super::DrawHUD();

  auto Player = GetOwningPlayerController();
  if (Player == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("Can't find player controller!"));
    return;
  }

  // get viewport size for culling
  int ScreenWidth = 0;
  int ScreenHeight = 0;
  Player->GetViewportSize(ScreenWidth, ScreenHeight);

  double Now = FPlatformTime::Seconds();
  int i = 0;
  while (i < StringList.Num())
  {
    // project position from camera
    FVector2D Screen;
    Player->ProjectWorldLocationToScreen(StringList[i].Location, Screen, true);
    if (Screen.X >= 0 && Screen.Y >= 0 && Screen.X < ScreenWidth && Screen.Y < ScreenHeight)
    {
      // draw text
      DrawText(StringList[i].Str, StringList[i].Color, Screen.X, Screen.Y, nullptr, 1.0f, false);
    }

    // check to remove the string
    if (Now >= StringList[i].TimeToDie)
    {
      StringList.RemoveAt(i);
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
