// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/ShaderBasedSensor.h"
#include "Carla/Actor/ActorDefinition.h"
#include "NormalsCamera.generated.h"

/// Sensor that produces "normals" images.
UCLASS()
class CARLA_API ANormalsCamera : public AShaderBasedSensor
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

  ANormalsCamera(const FObjectInitializer &ObjectInitializer);

protected:

  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;
};
