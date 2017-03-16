// Fill out your copyright notice in the Description page of Project Settings.

/**
 * Own SceneCapture, re-implementing some of the methods since ASceneCapture
 * cannot be subclassed.
 */

#pragma once

#include "GameFramework/Actor.h"
#include "StaticMeshResources.h"
#include "SceneCaptureCamera.generated.h"

class UDrawFrustumComponent;
class USceneCaptureComponent2D;
class UStaticMeshComponent;

UCLASS(hidecategories=(Collision, Material, Attachment, Actor))
class CARLA_API ASceneCaptureCamera : public AActor
{
  GENERATED_BODY()

public:

  ASceneCaptureCamera(const FObjectInitializer& ObjectInitializer);

protected:

  virtual void PostActorCreated() override;

public:

  virtual void BeginPlay() override;

  virtual void Tick(float Delta) override;

  uint32 GetImageSizeX() const
  {
    return SizeX;
  }

  uint32 GetImageSizeY() const
  {
    return SizeY;
  }

  const TArray<FColor> &GetImage() const
  {
    return ImageBitMap;
  }

private:

  /// Used to synchronize the DrawFrustumComponent with the
  /// SceneCaptureComponent2D settings.
  void UpdateDrawFrustum();

  UPROPERTY(Category = SceneCapture, EditAnywhere)
  uint32 SizeX;

  UPROPERTY(Category = SceneCapture, EditAnywhere)
  uint32 SizeY;

  /** To display the 3d camera in the editor. */
  UPROPERTY()
  class UStaticMeshComponent* MeshComp;

  /** To allow drawing the camera frustum in the editor. */
  UPROPERTY()
  class UDrawFrustumComponent* DrawFrustum;

  /** Render target necessary for scene capture */
  UPROPERTY(Transient)
  class UTextureRenderTarget2D* CaptureRenderTarget;

  /** Scene capture component. */
  UPROPERTY(Transient)
  class USceneCaptureComponent2D* CaptureComponent2D;

  UPROPERTY()
  TArray<FColor> ImageBitMap;
};
