// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"

#include "FrictionLayer.generated.h"

UCLASS()
class CARLA_API AFrictionLayer : public AActor
{
  GENERATED_BODY()

private:
  void Init();

public:

  AFrictionLayer(const FObjectInitializer &ObjectInitializer);

  UFUNCTION()
  void OnTriggerBeginOverlap(
      UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  UFUNCTION()
  void OnTriggerEndOverlap(
      UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex);

protected:

  virtual void BeginPlay() override;

public:

  UPROPERTY(EditAnywhere)
  UBoxComponent *TriggerVolume;

  virtual void Tick(float DeltaTime) override;



};
