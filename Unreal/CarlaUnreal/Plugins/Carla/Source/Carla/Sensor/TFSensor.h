// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"

#include "TFSensor.generated.h"

UCLASS()
class CARLA_API ATFSensor : public ASensor
{
  GENERATED_BODY()

public:

  ATFSensor(const FObjectInitializer &ObjectInitializer);

  static FActorDefinition GetSensorDefinition();

  virtual void Set(const FActorDescription &Description) override;

  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;

private:

  bool bIgnoreAltitude = false;
  bool bIgnoreTilt = false;
};
