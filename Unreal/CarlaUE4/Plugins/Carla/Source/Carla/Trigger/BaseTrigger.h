// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "BaseTrigger.generated.h"

UCLASS()
class CARLA_API ABaseTrigger : public AActor
{
  GENERATED_BODY()

private:

  virtual void Init();

public:

  ABaseTrigger(const FObjectInitializer &ObjectInitializer);

  UFUNCTION()
  void OnTriggerBeginOverlap_Callback(
      UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  virtual void OnTriggerBeginOverlap(
      UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult) {}

  UFUNCTION()
  void OnTriggerEndOverlap_Callback(
      UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex);

  virtual void OnTriggerEndOverlap(
      UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex) {}

  void SetEpisode(const UCarlaEpisode &InEpisode)
  {
    Episode = &InEpisode;
  }

  void SetBoxExtent(const FVector &Extent)
  {
    TriggerVolume->SetBoxExtent(Extent);
  }

  virtual void SetAttributes(const FActorDescription& Description) {}

protected:

  virtual void BeginPlay() override;

  virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

  virtual void Tick(float DeltaTime) override;

public:

  UPROPERTY(EditAnywhere)
  UBoxComponent *TriggerVolume;

  const UCarlaEpisode *Episode = nullptr;
};
