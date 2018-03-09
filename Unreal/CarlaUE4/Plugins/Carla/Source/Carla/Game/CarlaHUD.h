// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/HUD.h"
#include "CarlaHUD.generated.h"

class FTexture;

UCLASS()
class CARLA_API ACarlaHUD : public AHUD
{
  GENERATED_BODY()

public:

  ACarlaHUD();

  UPROPERTY(EditAnywhere)
  UFont* HUDFont;

  virtual void DrawHUD() override;

  UFUNCTION(BlueprintCallable)
  bool IsVisible() const
  {
    return bIsVisible;
  }

  UFUNCTION(BlueprintCallable)
  void SetVisible(bool bInIsVisible)
  {
    bIsVisible = bInIsVisible;
  }

  UFUNCTION(BlueprintCallable)
  void ToggleHUDView()
  {
    SetVisible(!bIsVisible);
  }

private:

  bool bIsVisible;
};
