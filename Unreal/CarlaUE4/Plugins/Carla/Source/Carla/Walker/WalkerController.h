// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Walker/WalkerControl.h"
#include "Carla/Walker/WalkerBoneControl.h"

#include "CoreMinimal.h"
#include "GameFramework/Controller.h"

#include <compiler/disable-ue4-macros.h>
#include <boost/variant.hpp>
#include <compiler/enable-ue4-macros.h>

#include "WalkerController.generated.h"

UCLASS()
class CARLA_API AWalkerController : public AController
{
  GENERATED_BODY()

private:

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
  void ApplyWalkerControl(const FWalkerControl &InControl)
  {
    Control = InControl;
  }

  void ApplyWalkerControl(const FWalkerBoneControl &InBoneControl)
  {
    Control = InBoneControl;
  }

  UFUNCTION(BlueprintCallable)
  const FWalkerControl GetWalkerControl() const
  {
    return Control.which() == 0u ? boost::get<FWalkerControl>(Control) : FWalkerControl{};
  }

  UFUNCTION(BlueprintCallable)
  const FWalkerBoneControl GetBoneWalkerControl() const
  {
    return Control.which() == 1u ? boost::get<FWalkerBoneControl>(Control) : FWalkerBoneControl{};
  }

  UFUNCTION(BlueprintCallable)
  void SetManualBones(const bool bIsEnabled);

private:

  boost::variant<FWalkerControl, FWalkerBoneControl> Control;

  bool bManualBones;
};
