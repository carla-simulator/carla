// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "TrafficLightComponent.h"
#include "TrafficLightController.h"
#include "TrafficLightGroup.generated.h"

class ATrafficLightManager;

/// Class which implements the state changing of traffic lights
UCLASS()
class CARLA_API ATrafficLightGroup : public AActor
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ATrafficLightGroup();

  UFUNCTION(Category = "Traffic Group", BlueprintPure)
  TArray<UTrafficLightController*>& GetControllers()
  {
    return Controllers;
  }

  UFUNCTION(Category = "Traffic Group", BlueprintCallable)
  void SetFrozenGroup(bool InFreeze);

  UFUNCTION(Category = "Traffic Group", BlueprintCallable)
  bool IsFrozen() const;

  UFUNCTION(Category = "Traffic Group", BlueprintPure)
  int GetJunctionId() const;

  UFUNCTION(Category = "Traffic Group", BlueprintCallable)
  void ResetGroup();

  UFUNCTION(Category = "Traffic Group", BlueprintCallable)
  void AddController(UTrafficLightController* Controller);

protected:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

private:

  friend ATrafficLightManager;

  UPROPERTY(Category = "Traffic Group", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  USceneComponent *SceneComponent;

  UPROPERTY(Category = "Traffic Group", EditAnywhere)
  TArray<UTrafficLightController*> Controllers;

  UPROPERTY(Category = "Traffic Group", VisibleAnywhere)
  int CurrentController = 0;

  UFUNCTION()
  void NextController();

  UPROPERTY(Category = "Traffic Group", EditAnywhere)
  bool bIsFrozen = false;

  UPROPERTY(Category = "Traffic Group", EditAnywhere)
  int JunctionId = -1;
};
