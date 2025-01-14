// Copyright (c) 2019 Intel Labs.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/ShaderBasedSensor.h"
#include "Carla/Actor/ActorDefinition.h"
#include "LaneInvasionSensor.generated.h"

/// LaneInvasion sensor representation
/// The actual position calculation is done one client side
UCLASS()
class CARLA_API ALaneInvasionSensor : public ASensor
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

  ALaneInvasionSensor(const FObjectInitializer &ObjectInitializer);
};
