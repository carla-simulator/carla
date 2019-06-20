// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/PixelReader.h"
#include "Carla/Sensor/Sensor.h"

#include "SceneCaptureSensor.generated.h"

class UDrawFrustumComponent;
class USceneCaptureComponent2D;
class UStaticMeshComponent;
class UTextureRenderTarget2D;

/// Base class for sensors using a USceneCaptureComponent2D for rendering the
/// scene. This class does not capture data, use
/// `FPixelReader::SendPixelsInRenderThread(*this)` in derived classes.
///
/// To access the USceneCaptureComponent2D override the
/// SetUpSceneCaptureComponent function.
///
/// @warning All the setters should be called before BeginPlay.
UCLASS(Abstract)
class CARLA_API ASceneCaptureSensor : public ASensor
{
  GENERATED_BODY()

  friend class FPixelReader;

public:

  ASceneCaptureSensor(const FObjectInitializer &ObjectInitializer);

  void Set(const FActorDescription &ActorDescription) override;

  void SetImageSize(uint32 Width, uint32 Height);

  uint32 GetImageWidth() const
  {
    return ImageWidth;
  }

  uint32 GetImageHeight() const
  {
    return ImageHeight;
  }

  UFUNCTION(BlueprintCallable)
  void EnablePostProcessingEffects(bool Enable = true)
  {
    bEnablePostProcessingEffects = Enable;
  }

  UFUNCTION(BlueprintCallable)
  bool ArePostProcessingEffectsEnabled() const
  {
    return bEnablePostProcessingEffects;
  }

  UFUNCTION(BlueprintCallable)
  void SetFOVAngle(float FOVAngle);

  UFUNCTION(BlueprintCallable)
  float GetFOVAngle() const;

  UFUNCTION(BlueprintCallable)
  void SetTargetGamma(float InTargetGamma)
  {
    TargetGamma = InTargetGamma;
  }

  UFUNCTION(BlueprintCallable)
  float GetTargetGamma() const
  {
    return TargetGamma;
  }

  UFUNCTION(BlueprintCallable)
  void SetMotionBlurIntensity(float Intensity);

  UFUNCTION(BlueprintCallable)
  float GetMotionBlurIntensity() const;

  UFUNCTION(BlueprintCallable)
  void SetMotionBlurMaxDistortion(float MaxDistortion);

  UFUNCTION(BlueprintCallable)
  float GetMotionBlurMaxDistortion() const;

  UFUNCTION(BlueprintCallable)
  void SetMotionBlurMinObjectScreenSize(float ScreenSize);

  UFUNCTION(BlueprintCallable)
  float GetMotionBlurMinObjectScreenSize() const;

  /// Use for debugging purposes only.
  UFUNCTION(BlueprintCallable)
  bool ReadPixels(TArray<FColor> &BitMap) const
  {
    check(CaptureRenderTarget != nullptr);
    return FPixelReader::WritePixelsToArray(*CaptureRenderTarget, BitMap);
  }

  /// Use for debugging purposes only.
  UFUNCTION(BlueprintCallable)
  void SaveCaptureToDisk(const FString &FilePath) const
  {
    check(CaptureRenderTarget != nullptr);
    FPixelReader::SavePixelsToDisk(*CaptureRenderTarget, FilePath);
  }

protected:

  virtual void BeginPlay() override;

  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  virtual void SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture) {}

private:

  /// Image width in pixels.
  UPROPERTY(EditAnywhere)
  uint32 ImageWidth = 800u;

  /// Image height in pixels.
  UPROPERTY(EditAnywhere)
  uint32 ImageHeight = 600u;

  /// Whether to render the post-processing effects present in the scene.
  UPROPERTY(EditAnywhere)
  bool bEnablePostProcessingEffects = true;

  UPROPERTY(EditAnywhere)
  float TargetGamma = 2.2f;

  /// Render target necessary for scene capture.
  UPROPERTY(EditAnywhere)
  UTextureRenderTarget2D *CaptureRenderTarget = nullptr;

  /// Scene capture component.
  UPROPERTY(EditAnywhere)
  USceneCaptureComponent2D *CaptureComponent2D = nullptr;
};
