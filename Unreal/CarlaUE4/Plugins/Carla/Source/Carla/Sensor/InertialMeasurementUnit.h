// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"

#include "InertialMeasurementUnit.generated.h"

UCLASS()
class CARLA_API AInertialMeasurementUnit : public ASensor
{
  GENERATED_BODY()

public:

  AInertialMeasurementUnit(const FObjectInitializer &ObjectInitializer);

  static FActorDefinition GetSensorDefinition();

  void Set(const FActorDescription &ActorDescription) override;

  void SetOwner(AActor *Owner) override;

  void Tick(float DeltaTime) override;

  /// Based on OpenDRIVE's lon and lat, North is in (0.0f, -1.0f, 0.0f)
  static const FVector CarlaNorthVector;

private:

  void BeginPlay() override;

  FVector PrevVelocity;

};
