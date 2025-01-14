// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "SignComponent.h"

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include <util/ue-header-guard-end.h>

#include "SpeedLimitComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API USpeedLimitComponent : public USignComponent
{
  GENERATED_BODY()

public:

  virtual void InitializeSign(const carla::road::Map &Map) override;

  void SetSpeedLimit(float Limit);

private:

  void GenerateSpeedBox(const FTransform BoxTransform, float BoxSize);

  UFUNCTION(BlueprintCallable)
  void OnOverlapBeginSpeedLimitBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  UPROPERTY(Category = "Speed Limit", EditAnywhere)
  float SpeedLimit = 30;

};
