// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/LightState.h>
#include <compiler/enable-ue4-macros.h>

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CarlaLight.generated.h"



#define CARLA_ENUM_FROM_RPC(e) static_cast<uint8>(carla::rpc::LightState::LightGroup:: e)

UENUM(BlueprintType)
enum class ELightType : uint8
{
  Null      = 0, // Workarround for UE4.24 issue with enums
  Vehicle   = CARLA_ENUM_FROM_RPC(Vehicle)    UMETA(DisplayName = "Vehicle"),
  Street    = CARLA_ENUM_FROM_RPC(Street)     UMETA(DisplayName = "Street"),
  Building  = CARLA_ENUM_FROM_RPC(Building)   UMETA(DisplayName = "Building"),
  Other     = CARLA_ENUM_FROM_RPC(Other)      UMETA(DisplayName = "Other"),
};

#undef CARLA_ENUM_FROM_RPC

// Class representing a light in the scene
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARLA_API UCarlaLight : public UActorComponent
{
  GENERATED_BODY()

public:
  UCarlaLight();

  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  void OnComponentDestroyed(bool bDestroyingHierarchy) override;

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void RegisterLight();

  UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Carla Light")
  void UpdateLights();

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightIntensity(float Intensity);

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  float GetLightIntensity() const;

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightColor(FLinearColor Color);

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  FLinearColor GetLightColor() const;

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightOn(bool bOn);

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  bool GetLightOn() const;

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightType(ELightType Type);

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  ELightType GetLightType() const;

  carla::rpc::LightState GetLightState();

  void SetLightState(carla::rpc::LightState LightState);

  FVector GetLocation() const;

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  int GetId() const;

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetId(int InId);

protected:

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  ELightType LightType = ELightType::Street;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  float LightIntensity;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  FLinearColor LightColor;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  bool bLightOn;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  int Id = -1;

  private:

  void RecordLightChange() const;

  bool bRegistered = false;
};
