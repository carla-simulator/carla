// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/ShaderBasedSensor.h"

#include <util/ue-header-guard-begin.h>
#include "Actor/ActorBlueprintFunctionLibrary.h"
#include <util/ue-header-guard-end.h>

#include "SceneCaptureCamera.generated.h"

/// A sensor that captures images from the scene.
UCLASS()
class CARLA_API ASceneCaptureCamera : public AShaderBasedSensor
{
  GENERATED_BODY()

public:


  static FActorDefinition GetSensorDefinition();

  ASceneCaptureCamera(const FObjectInitializer &ObjectInitializer);

protected:
	
#ifdef CARLA_HAS_GBUFFER_API
  virtual void SendGBufferTextures(FGBufferRequest& GBuffer) override;
#endif

  void BeginPlay() override;
  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;
  
  virtual void OnFirstClientConnected() override;
  virtual void OnLastClientDisconnected() override;

private:
};
