// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Carla/Game/CarlaEpisode.h"

#include "FrictionTrigger.generated.h"

UCLASS()
class CARLA_API AFrictionTrigger : public AActor
{
  GENERATED_BODY()

private:

  void Init();

  void UpdateWheelsFriction(AActor *OtherActor, TArray<float>& NewFriction);

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

  void SetBoxExtent(const FVector &Extent)
  {
    TriggerVolume->SetBoxExtent(Extent);
  }

  void SetFriction(float NewFriction)
  {
    Friction = NewFriction;
  }

protected:

  virtual void BeginPlay() override;

  virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

  virtual void Tick(float DeltaTime) override;

  // Save old frictions
  TArray<float> OldFrictionValues;

public:

  UPROPERTY(EditAnywhere)
  float Friction = 0.0f;

  UPROPERTY(EditAnywhere)
  UBoxComponent *TriggerVolume;

  const UCarlaEpisode *Episode = nullptr;
};
