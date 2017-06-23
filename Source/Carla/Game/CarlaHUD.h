// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/HUD.h"
#include "CarlaHUD.generated.h"

class FTexture;

UCLASS()
class ACarlaHUD : public AHUD
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
