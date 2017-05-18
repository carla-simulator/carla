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

  /// If none is found, the default one is added.
  static void LoadWeatherDescriptionsFromFile(TArray<FWeatherDescription> &Descriptions);

  ADynamicWeather(const FObjectInitializer& ObjectInitializer);

  virtual void OnConstruction(const FTransform &Transform) override;

  virtual void BeginPlay() override;

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

  void AdjustSunPositionBasedOnActorRotation();

#if WITH_EDITOR

  void Update();

  bool LoadFromConfigFile();

  bool SaveToConfigFile() const;

#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA

  UPROPERTY()
  UArrowComponent *ArrowComponent;

  /** If true, the weather is refreshed on construction and at begin play.
    * Useful for editing the weather (Editor only).
    */
  UPROPERTY(Category = "Weather Description", EditAnywhere)
  bool bRefreshAutomatically = false;

  /** Load the section with the currently set name. */
  UPROPERTY(Category = "Weather Description", EditAnywhere)
  bool bLoadFromConfigFile = false;

  /** Save current settings to disk. */
  UPROPERTY(Category = "Weather Description", EditAnywhere)
  bool bSaveToConfigFile = false;

#endif // WITH_EDITORONLY_DATA

  UPROPERTY(Category = "Weather Description", EditAnywhere)
  FWeatherDescription Weather;
};
