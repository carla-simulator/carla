#pragma once

#include "Carla/Sensor/ShaderBasedSensor.h"

#include "Carla/Actor/ActorDefinition.h"

#include "OpticalFlowCamera.generated.h"

/// Sensor that produces "optical flow" images.
UCLASS()
class CARLA_API AOpticalFlowCamera : public AShaderBasedSensor
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

  AOpticalFlowCamera(const FObjectInitializer &ObjectInitializer);

  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;

  void SendPixelsInRenderThread(const TArray<FFloat16Color>& AtlasPixels, uint32 AtlasTextureWidth) override;

  void CopyOpticalFlowFromAtlas(carla::Buffer &Buffer,
      const TArray<FFloat16Color>& AtlasPixels,
      uint32 AtlasTextureWidth);
};
