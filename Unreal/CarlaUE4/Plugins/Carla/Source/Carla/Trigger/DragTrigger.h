// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "BaseTrigger.h"

#include "DragTrigger.generated.h"

UCLASS()
class CARLA_API ADragTrigger : public ABaseTrigger
{
  GENERATED_BODY()

public:

    virtual void OnTriggerBeginOverlap(
      UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult) override;

  virtual void OnTriggerEndOverlap(
      UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex) override;

  virtual void SetAttributes(const FActorDescription& Description) override;

protected:
  float OldLinearDrag = 0.0;

public:
  UPROPERTY(EditAnywhere)
  float LinearDrag = 0.01f;
};
