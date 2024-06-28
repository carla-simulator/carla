
#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/PixelReader.h"
#include "Carla/Sensor/ShaderBasedSensor.h"

#include "IRCamera.generated.h"

/// A sensor that captures images from the scene.
UCLASS()
class CARLA_API AIRCamera : public AShaderBasedSensor
{
	GENERATED_BODY()

protected:
	UMaterialParameterCollection* IRMPC;

public:

	
	static FActorDefinition GetSensorDefinition();

	AIRCamera(const FObjectInitializer &ObjectInitializer);

protected:
	
#ifdef CARLA_HAS_GBUFFER_API
	virtual void SendGBufferTextures(FGBufferRequest& GBuffer) override;
#endif

	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PrePhysTick(float DeltaSeconds) override;
	void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;
  
	virtual void OnFirstClientConnected() override;
	virtual void OnLastClientDisconnected() override;

private:
};
