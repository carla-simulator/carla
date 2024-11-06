// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/MovementComponent.h"
#include "Carla/Vehicle/VehicleControl.h"
#include "BaseCarlaMovementComponent.generated.h"

class ACarlaWheeledVehicle;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class CARLA_API UBaseCarlaMovementComponent : public UMovementComponent
{
  GENERATED_BODY()

protected:

  UPROPERTY()
  ACarlaWheeledVehicle* CarlaVehicle;

public:

  virtual void BeginPlay() override;

  virtual void ProcessControl(FVehicleControl &Control);

  virtual FVector GetVelocity() const;

  virtual int32 GetVehicleCurrentGear() const;

  virtual float GetVehicleForwardSpeed() const;

  virtual void DisableSpecialPhysics() {};

protected:

  void DisableUE4VehiclePhysics();

  void EnableUE4VehiclePhysics(bool bResetVelocity = true);
};
