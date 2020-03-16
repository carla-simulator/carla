// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
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

  UFUNCTION(BlueprintPure)
  TArray<UTrafficLightController*>& GetControllers()
  {
    return Controllers;
  }

  UFUNCTION(BlueprintCallable)
  void SetFrozenGroup(bool InFreeze);

  UFUNCTION(BlueprintPure)
  int GetJunctionId() const;

  UFUNCTION(BlueprintCallable)
  void ResetGroup();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

  // Called every frame
  virtual void Tick(float DeltaTime) override;

private:

  friend ATrafficLightManager;
  UPROPERTY(Category = "Traffic Group", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  USceneComponent *SceneComponent;

  UPROPERTY(Category = "Traffic Group", EditAnywhere)
  TArray<UTrafficLightController*> Controllers;

  UPROPERTY()
  float Timer = 0;

  UPROPERTY()
  int CurrentController = 0;

  UFUNCTION()
  void NextCycleStep();

  UFUNCTION()
  void NextController();

  UPROPERTY(Category = "Traffic Group", EditAnywhere)
  bool bIsFrozen = false;

  UPROPERTY(Category = "Traffic Group", EditAnywhere)
  int JunctionId = -1;
};
