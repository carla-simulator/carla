// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Walker/WalkerBoneControlIn.h"
#include "Carla/Walker/WalkerBoneControlOut.h"
#include "Carla/Walker/WalkerControl.h"

#include <util/disable-ue4-macros.h>
#include <carla/rpc/WalkerBoneControlIn.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "GameFramework/Controller.h"
#include <util/ue-header-guard-end.h>

#include "WalkerController.generated.h"

UCLASS()
class CARLA_API AWalkerController : public AController
{
  GENERATED_BODY()

public:

  AWalkerController(const FObjectInitializer &ObjectInitializer);

  void OnPossess(APawn *InPawn) override;

  void Tick(float DeltaSeconds) override;

  /// Maximum walk speed allowed in centimetres per second.
  UFUNCTION(BlueprintCallable)
  float GetMaximumWalkSpeed() const
  {
    return 4096.0f; // ~147 km/h
  }

  UFUNCTION(BlueprintCallable)
  void ApplyWalkerControl(const FWalkerControl &InControl);

  UFUNCTION(BlueprintCallable)
  const FWalkerControl GetWalkerControl() const
  {
    return Control;
  }

  UFUNCTION(BlueprintCallable)
  void GetBonesTransform(FWalkerBoneControlOut &WalkerBones);

  UFUNCTION(BlueprintCallable)
  void SetBonesTransform(const FWalkerBoneControlIn &WalkerBones);

  UFUNCTION(BlueprintCallable)
  void BlendPose(float Blend);

  UFUNCTION(BlueprintCallable)
  void GetPoseFromAnimation();

private:

  FWalkerControl Control;
};
