// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaHUD.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/Engine.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GameFramework/PlayerController.h"
#include <util/ue-header-guard-end.h>

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
    DrawDebug(YL, Y0);
  }

  double Now = FPlatformTime::Seconds();
  int i = 0;
  while (i < StringList.Num())
  {
    // project position from camera
    FVector2D Screen;
    if (Player->ProjectWorldLocationToScreen(StringList[i].Location, Screen, true))
    {
      // draw text
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
}

void ACarlaHUD::AddHUDString(const FString Str, const FVector Location, const FColor Color, double LifeTime)
{
  double Now = FPlatformTime::Seconds();
  HUDString Obj { Str, Location, Color, Now + LifeTime };
  StringList.Add(std::move(Obj));
}

// Debug
void ACarlaHUD::DrawDebug(float& YL, float& YPos)
{
	using namespace Chaos;

	ensure(IsInGameThread());

	if (!DebugVehicle->HasValidPhysicsState()){
		UE_LOG(LogCarla, Log, TEXT("Invalid state"));
		return;
	}

	UFont* RenderFont = GEngine->GetMediumFont();
	// draw drive data
	{
		Canvas->SetDrawColor(FColor::White);
	
		YPos += 16;
		for (int i = 0; i < DebugVehicle->GetNumWheels(); i++)
		{
			YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("WheelLoad: [%d] %1.f N"), i, CmToM(DebugVehicle->GetWheelState(i).SpringForce)), 4, YPos);
		}

		YPos += 16;
		for (int i = 0; i < DebugVehicle->GetNumWheels(); i++)
		{
			if (DebugVehicle->GetWheelState(i).PhysMaterial.IsValid())
			{
				YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("SurfaceFriction: [%d] %.2f"), i, DebugVehicle->GetWheelState(i).PhysMaterial->Friction), 4, YPos);
			}
		}
	}
}
