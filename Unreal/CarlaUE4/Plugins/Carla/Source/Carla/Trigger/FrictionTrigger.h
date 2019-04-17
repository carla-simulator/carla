// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"

#include "FrictionTrigger.generated.h"

UCLASS()
class CARLA_API AFrictionTrigger : public AActor
{
  GENERATED_BODY()

private:

  void Init();

public:

  AFrictionTrigger(const FObjectInitializer &ObjectInitializer);

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

  void SetEpisode(const UCarlaEpisode &InEpisode)
  {
    Episode = &InEpisode;
  }

protected:

  virtual void BeginPlay() override;

  virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:

  UPROPERTY(EditAnywhere)
  UBoxComponent *TriggerVolume;

  virtual void Tick(float DeltaTime) override;

  const UCarlaEpisode *Episode = nullptr;
};
