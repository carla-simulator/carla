// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "FisheyeSensor.generated.h"


class USceneCaptureComponentCube;
class UDrawFrustumComponent;
class UTextureRenderTargetCube;
class UStaticMeshComponent;

UCLASS()
class CARLA_API AFisheyeSensor : public ASensor
{
  GENERATED_BODY()

  friend class FReader;

public:

  AFisheyeSensor (const FObjectInitializer &ObjectInitializer);

  static FActorDefinition GetSensorDefinition();

  UFUNCTION(BlueprintCallable)
  float GetImageWidth() const;

  UFUNCTION(BlueprintCallable)
  float GetImageHeight() const;

  UFUNCTION(BlueprintCallable)
  float GetFOV() const;

  template <typename TSensor>
  static void SendPixelsInRenderThread(TSensor &Sensor, float MaxAngle, float SizeX, float SizeY, float Fx, float Fy, float Cx, float Cy, float D1, float D2, float D3, float D4);

  
  void Set(const FActorDescription &ActorDescription) override;

  UPROPERTY(EditAnywhere)
  UTextureRenderTargetCube *CaptureRenderTarget = nullptr;

  UPROPERTY(EditAnywhere)
  USceneCaptureComponentCube *Fisheye = nullptr;

  UPROPERTY(EditAnywhere)
  float MaxAngle = 0;

  UPROPERTY(EditAnywhere)
  float XSize = 0;

  UPROPERTY(EditAnywhere)
  float YSize = 0;

  UPROPERTY(EditAnywhere)
  float Fx = 0;

  UPROPERTY(EditAnywhere)
  float Fy = 0;

  UPROPERTY(EditAnywhere)
  float Cx = 0;

  UPROPERTY(EditAnywhere)
  float Cy = 0;

  UPROPERTY(EditAnywhere)
  float D1 = 0;

  UPROPERTY(EditAnywhere)
  float D2 = 0;

  UPROPERTY(EditAnywhere)
  float D3 = 0;

  UPROPERTY(EditAnywhere)
  float D4 = 0;

protected:

  virtual void BeginPlay() override;

  virtual void PrePhysTick(float DeltaTime) override;

  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;

  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  bool ReadyToCapture = false;

private:

};
