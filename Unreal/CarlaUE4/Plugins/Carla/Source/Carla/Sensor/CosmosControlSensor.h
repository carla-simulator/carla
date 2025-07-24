// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Carla/Sensor/ShaderBasedSensor.h"
#include "Sensor/Sensor.h"
#include "Carla/Actor/ActorDefinition.h"
#include "CosmosControlSensor.generated.h"

/**
 * Sensor that produces an input control visualization for Cosmos
 */
UCLASS()
class CARLA_API ACosmosControlSensor : public AShaderBasedSensor
{
	GENERATED_BODY()

public:

	static FActorDefinition GetSensorDefinition();

  ACosmosControlSensor(const FObjectInitializer& ObjectInitializer);
  void BeginDestroy() override;

protected:

  void SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture) override;
  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;

private:
  bool added_persisted_stop_lines;
  bool added_persisted_route_lines;
  bool added_persisted_crosswalks;
};
