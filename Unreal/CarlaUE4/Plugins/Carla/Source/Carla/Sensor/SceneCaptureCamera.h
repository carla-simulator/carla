// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/DeprecatedSensor.h"

#include "Carla/Settings/CameraDescription.h"

#include "StaticMeshResources.h"

#include "SceneCaptureCamera.generated.h"

class UDrawFrustumComponent;
class USceneCaptureComponent2D;
class UStaticMeshComponent;
class UTextureRenderTarget2D;

/// Own SceneCapture, re-implementing some of the methods since ASceneCapture
/// cannot be subclassed.
UCLASS(hidecategories=(Collision, Attachment, Actor))
class CARLA_API ASceneCaptureCamera : public ADeprecatedSensor
{
  GENERATED_BODY()

public:

  ASceneCaptureCamera(const FObjectInitializer& ObjectInitializer);

protected:

  virtual void PostActorCreated() override;

public:

  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void Tick(float DeltaSeconds) override;

  uint32 GetImageSizeX() const
  {
    return SizeX;
  }

  uint32 GetImageSizeY() const
  {
    return SizeY;
  }

  EPostProcessEffect GetPostProcessEffect() const
  {
    return PostProcessEffect;
  }

  float GetFOVAngle() const;

  void SetImageSize(uint32 SizeX, uint32 SizeY);

  void SetPostProcessEffect(EPostProcessEffect PostProcessEffect);

  void SetFOVAngle(float FOVAngle);

  void SetTargetGamma(float TargetGamma);

  void Set(const UCameraDescription &CameraDescription);

  bool ReadPixels(TArray<FColor> &BitMap) const;

protected:
  static uint32 NumSceneCapture;

private:

  /// Read the camera buffer and write it to the client with no lock of the
  /// resources (for Vulkan API).
  void WritePixelsNonBlocking(
      uint64 FrameNumber,
      FRHICommandListImmediate& rhi_cmd_list) const;

  /// Read the camera buffer and write it to the client with opengl or direct3d.
  void WritePixels(uint64 FrameNumber) const;

  /// Used to synchronize the DrawFrustumComponent with the
  /// SceneCaptureComponent2D settings.
  void UpdateDrawFrustum();

  UPROPERTY(Category = "Scene Capture", EditAnywhere)
  uint32 SizeX;

  UPROPERTY(Category = "Scene Capture", EditAnywhere)
  uint32 SizeY;

  UPROPERTY(Category = "Scene Capture", EditAnywhere)
  EPostProcessEffect PostProcessEffect;

  /** To display the 3d camera in the editor. */
  UPROPERTY()
  UStaticMeshComponent* MeshComp;

  /** To allow drawing the camera frustum in the editor. */
  UPROPERTY()
  UDrawFrustumComponent* DrawFrustum;

  /** Render target necessary for scene capture */
  UPROPERTY()
  UTextureRenderTarget2D* CaptureRenderTarget;

  /** Scene capture component. */
  UPROPERTY(EditAnywhere)
  USceneCaptureComponent2D* CaptureComponent2D;

  UPROPERTY()
  UMaterial *PostProcessDepth;

  UPROPERTY()
  UMaterial *PostProcessSemanticSegmentation;
};
