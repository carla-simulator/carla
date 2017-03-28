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

  bool IsVisible() const
  {
    return bIsVisible;
  }

  UFUNCTION()
  void ToggleHUDView();

  UFUNCTION()
  void SetVisible(bool IsVisible);

private:

  bool bIsVisible = true;
};
