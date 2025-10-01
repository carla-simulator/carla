// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/ShaderBasedSensor_WideAngleLens.h"

#include "Carla/Actor/ActorDefinition.h"

#include "NormalsCamera_WideAngleLens.generated.h"

/// Sensor that produces "normals" images.
UCLASS()
class CARLA_API ANormalsCamera_WideAngleLens : public AShaderBasedSensor_WideAngleLens
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

  ANormalsCamera_WideAngleLens(const FObjectInitializer &ObjectInitializer);

protected:

  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;
};
