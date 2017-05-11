// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/Actor.h"
#include "WeatherDescription.h"
#include "DynamicWeather.generated.h"

class UArrowComponent;

UCLASS(Abstract)
class CARLA_API ADynamicWeather : public AActor
{
  GENERATED_BODY()

public:

  ADynamicWeather(const FObjectInitializer& ObjectInitializer);

  virtual void OnConstruction(const FTransform &Transform) override;

#if WITH_EDITOR

  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

  virtual void EditorApplyRotation(const FRotator & DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;

#endif // WITH_EDITOR

  void SetWeatherDescription(const FWeatherDescription &WeatherDescription)
  {
    Weather = WeatherDescription;
  }

  UFUNCTION(BlueprintCallable)
  const FWeatherDescription &GetWeatherDescription() const
  {
    return Weather;
  }

  UFUNCTION(BlueprintImplementableEvent)
  void RefreshWeather();

  UFUNCTION(BlueprintCallable)
  FVector GetSunDirection() const;

private:

  void Update();

  void AdjustSunPositionBasedOnActorRotation();

#if WITH_EDITORONLY_DATA
  UPROPERTY()
  UArrowComponent* ArrowComponent;
#endif // WITH_EDITORONLY_DATA

  UPROPERTY(Category = "Weather", EditAnywhere)
  FWeatherDescription Weather;
};
