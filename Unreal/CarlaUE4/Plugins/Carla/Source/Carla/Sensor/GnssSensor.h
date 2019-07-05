// Copyright (c) 2019 Intel Labs.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/ShaderBasedSensor.h"

#include "Carla/Actor/ActorDefinition.h"

#include "GnssSensor.generated.h"

/// Gnss sensor representation
/// The actual position calculation is done one client side
UCLASS()
class CARLA_API AGnssSensor : public ASensor
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

  AGnssSensor(const FObjectInitializer &ObjectInitializer);
};
