// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Sensor/PixelReader.h"
#include "Carla/Sensor/ShaderBasedSensor_WideAngleLens.h"

#include "SceneCaptureCamera_WideAngleLens.generated.h"

/// A sensor that captures images from the scene.
UCLASS()
class CARLA_API ASceneCaptureCamera_WideAngleLens : public AShaderBasedSensor_WideAngleLens
{
	GENERATED_BODY()

public:

	static FActorDefinition GetSensorDefinition();

	ASceneCaptureCamera_WideAngleLens(const FObjectInitializer& ObjectInitializer);

protected:

	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void PostPhysTick(UWorld* World, ELevelTick TickType, float DeltaSeconds) override;

	virtual void OnFirstClientConnected() override;
	virtual void OnLastClientDisconnected() override;

private:
};
