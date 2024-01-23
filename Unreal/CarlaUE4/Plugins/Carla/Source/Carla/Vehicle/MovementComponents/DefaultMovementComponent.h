// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "BaseCarlaMovementComponent.h"
#include "DefaultMovementComponent.generated.h"


UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent) )
class CARLA_API UDefaultMovementComponent : public UBaseCarlaMovementComponent
{
  GENERATED_BODY()

public:

  static void CreateDefaultMovementComponent(ACarlaWheeledVehicle* Vehicle);

  virtual void BeginPlay() override;

  void ProcessControl(FVehicleControl &Control) override;

  // FVector GetVelocity() const override;

  int32 GetVehicleCurrentGear() const override;

  float GetVehicleForwardSpeed() const override;

};

