// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CarlaLight.generated.h"


UENUM(BlueprintType)
enum class ELightType : uint8
{
  Null = 0, // Workarround for UE4.24 issue with enums
  Street      UMETA(DisplayName = "Street"),
  Building    UMETA(DisplayName = "Building"),
};

// Class representing a light in the scene
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARLA_API UCarlaLight : public UActorComponent
{
  GENERATED_BODY()

public:
  UCarlaLight();

  UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Carla Light")
  void UpdateLights();

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightIntensity(float Intensity);

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  float GetLightIntensity();

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightColor(FLinearColor Color);

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  FLinearColor GetLightColor();

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightOn(bool bOn);

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  bool GetLightOn();

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightType(ELightType Type);

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  ELightType GetLightType();

protected:

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  ELightType LightType = ELightType::Street;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  float LightIntensity;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  FLinearColor LightColor;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  bool bLightOn;

};
