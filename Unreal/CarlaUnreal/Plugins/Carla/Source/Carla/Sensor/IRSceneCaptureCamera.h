
#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/PixelReader.h"
#include "Carla/Sensor/ShaderBasedSensor.h"

#include "IRSceneCaptureCamera.generated.h"

/// A sensor that captures images from the scene.
UCLASS()
class CARLA_API AIRSceneCaptureCamera : public AShaderBasedSensor
{
	GENERATED_BODY()

public:


	static FActorDefinition GetSensorDefinition();

	AIRSceneCaptureCamera(const FObjectInitializer &ObjectInitializer);

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
