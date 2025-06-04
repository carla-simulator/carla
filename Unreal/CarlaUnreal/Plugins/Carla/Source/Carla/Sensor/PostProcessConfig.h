#pragma once
#include "Engine/Scene.h"



class CARLA_API FPostProcessConfig
{
  void EnablePostProcessingEffects();
  void DisablePostProcessingEffects();

public:
	
  FPostProcessConfig(
    FPostProcessSettings& InPostProcessSettings,
    FEngineShowFlags& InEngineShowFlags);
  ~FPostProcessConfig() = default;

  void UpdateFromSceneCaptureComponent(const class USceneCaptureComponent& Component);
  void UpdateFromSceneCaptureComponent2D(const class USceneCaptureComponent2D& Component);
  void EnablePostProcessingEffects(bool Enable);

  FPostProcessSettings PostProcessSettings;
  FEngineShowFlags EngineShowFlags;

};
