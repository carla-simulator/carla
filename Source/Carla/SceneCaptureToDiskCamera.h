// Fill out your copyright notice in the Description page of Project Settings.

/**
 * Own SceneCapture, re-implementing some of the methods since ASceneCapture
 * cannot be subclassed.
 */

#pragma once

#include "GameFramework/Actor.h"
#include "StaticMeshResources.h"
#include "SceneCaptureToDiskCamera.generated.h"

class UDrawFrustumComponent;
class USceneCaptureComponent2D;
class UStaticMeshComponent;

UCLASS(hidecategories=(Collision, Material, Attachment, Actor))
class CARLA_API ASceneCaptureToDiskCamera : public AActor
{
  GENERATED_BODY()

  float ElapsedTimeSinceLastCapture = 0.0f;

  size_t CaptureFileNameCount = 0u;

public:

  UPROPERTY(Category = SceneCapture, EditAnywhere)
  bool bCaptureScene = true;

  UPROPERTY(Category = SceneCapture, EditAnywhere, meta = (EditCondition = bCaptureScene))
  FString SaveToFolder;

  UPROPERTY(Category = SceneCapture, EditAnywhere, meta = (EditCondition = bCaptureScene))
  FString FileName;

  UPROPERTY(Category = SceneCapture, EditAnywhere, meta = (EditCondition = bCaptureScene))
  float CapturesPerSecond;

  UPROPERTY(Category = SceneCapture, EditAnywhere, meta = (EditCondition = bCaptureScene))
  uint32 SizeX;

  UPROPERTY(Category = SceneCapture, EditAnywhere, meta = (EditCondition = bCaptureScene))
  uint32 SizeY;

private:
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

public:

  ASceneCaptureToDiskCamera(const FObjectInitializer& ObjectInitializer);

  ~ASceneCaptureToDiskCamera();

  virtual void PostActorCreated() override;

  virtual void BeginPlay() override;

  virtual void Tick(float Delta) override;

  /** Used to synchronize the DrawFrustumComponent with the SceneCaptureComponent2D settings. */
  void UpdateDrawFrustum();

  UFUNCTION(BlueprintCallable, Category="Rendering")
  void OnInterpToggle(bool bEnable);

  /** Returns CaptureComponent2D subobject **/
  USceneCaptureComponent2D* GetCaptureComponent2D() const;

  /** Returns DrawFrustum subobject **/
  UDrawFrustumComponent* GetDrawFrustum() const;

  /** Returns MeshComp subobject **/
  UStaticMeshComponent* GetMeshComp() const;
};
