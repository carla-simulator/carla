// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "SceneCaptureSensor.generated.h"

class UDrawFrustumComponent;
class USceneCaptureComponent2D;
class UStaticMeshComponent;
class UTextureRenderTarget2D;

/// Base class for sensors using a USceneCaptureComponent2D for rendering the
/// scene. Images are automatically serialized and sent.
///
/// To access the USceneCaptureComponent2D override the
/// SetUpSceneCaptureComponent function.
///
/// @warning All the setters should be called before BeginPlay.
UCLASS(Abstract)
class CARLA_API ASceneCaptureSensor : public ASensor
{
  GENERATED_BODY()

public:

  ASceneCaptureSensor(const FObjectInitializer& ObjectInitializer);

  void SetImageSize(uint32 Width, uint32 Height);

  uint32 GetImageWidth() const
  {
    return ImageWidth;
  }

  uint32 GetImageHeight() const
  {
    return ImageHeight;
  }

  void EnablePostProcessingEffects(bool Enable = true)
  {
    bEnablePostProcessingEffects = Enable;
  }

  bool ArePostProcessingEffectsEnabled() const
  {
    return bEnablePostProcessingEffects;
  }

  void SetFOVAngle(float FOVAngle);

  float GetFOVAngle() const;

protected:

  virtual void PostActorCreated() override;

  virtual void BeginPlay() override;

  virtual void Tick(float DeltaSeconds) override;

  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  virtual void SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture) {}

private:

  /// Used to synchronize the DrawFrustumComponent with the
  /// SceneCaptureComponent2D settings.
  void UpdateDrawFrustum();

  /// Image width in pixels.
  UPROPERTY(Category = "Scene Capture", EditAnywhere)
  uint32 ImageWidth = 800u;

  /// Image height in pixels.
  UPROPERTY(Category = "Scene Capture", EditAnywhere)
  uint32 ImageHeight = 600u;

  /// Whether to render the post-processing effects present in the scene.
  UPROPERTY(Category = "Scene Capture", EditAnywhere)
  bool bEnablePostProcessingEffects = true;

  /// Render target necessary for scene capture.
  UPROPERTY()
  UTextureRenderTarget2D *CaptureRenderTarget = nullptr;

  /// Scene capture component.
  UPROPERTY(EditAnywhere)
  USceneCaptureComponent2D *CaptureComponent2D = nullptr;

  /// To display the 3d camera in the editor.
  UPROPERTY()
  UStaticMeshComponent *MeshComp = nullptr;

  /// To allow drawing the camera frustum in the editor.
  UPROPERTY()
  UDrawFrustumComponent *DrawFrustum = nullptr;
};
