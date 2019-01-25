// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Walker/WalkerControl.h"

#include "CoreMinimal.h"
#include "GameFramework/Controller.h"

#include "WalkerController.generated.h"

UCLASS()
class CARLA_API AWalkerController : public AController
{
  GENERATED_BODY()

public:

  AWalkerController(const FObjectInitializer& ObjectInitializer);

  void Possess(APawn *InPawn) override;

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

  UFUNCTION(BlueprintCallable)
  const FWalkerControl &GetWalkerControl() const
  {
    return Control;
  }

private:

  UPROPERTY(VisibleAnywhere)
  FWalkerControl Control;
};
