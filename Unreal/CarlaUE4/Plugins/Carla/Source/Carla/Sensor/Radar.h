// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/sensor/s11n/RadarData.h>
#include <compiler/enable-ue4-macros.h>

#include "Radar.generated.h"

/// A ray-cast based Radar sensor.
UCLASS()
class CARLA_API ARadar : public ASensor
{
  GENERATED_BODY()

  using FRadarData = carla::sensor::s11n::RadarData;

public:

  static FActorDefinition GetSensorDefinition();

  ARadar(const FObjectInitializer &ObjectInitializer);

  void Set(const FActorDescription &Description) override;

  UFUNCTION(BlueprintCallable, Category = "Radar")
  void SetFOVAndSteps(float NewFov, int NewSteps);

  UFUNCTION(BlueprintCallable, Category = "Radar")
  void SetDistance(float NewDistance);

  UFUNCTION(BlueprintCallable, Category = "Radar")
  void SetAperture(int NewAperture);

protected:

  void BeginPlay() override;

  virtual void Tick(float DeltaTime) override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Detection")
  float Distance;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Detection")
  float FOV;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Detection")
  int Aperture;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Detection")
  int Steps;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
  bool ShowDebug = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug", meta=(EditCondition="ShowDebug"))
  int ShowDebugDelay = 5;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug", meta=(EditCondition="ShowDebug"))
  bool ShowDebugLines = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug", meta=(EditCondition= "ShowDebug & ShowDebugLines"))
  bool ShowCompleteLines = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug", meta=(EditCondition= "ShowDebug & ShowDebugLines"))
  float LineThickness = 4.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug", meta=(EditCondition= "ShowDebug"))
  bool ShowDebugHits = true;

private:

  void CalculateCurrentVelocity(const float DeltaTime);

  void PreCalculateCosSin();

  void PreCalculateLineTraceIncrement();

  void SendLineTraces(float DeltaSeconds);

  float CalculateRelativeVelocity(const FHitResult& OutHit, const FVector& ForwardVector);

  FRadarData RadarData;

  FCollisionQueryParams TraceParams;

  // Current Radar Velocity
  FVector CurrentVelocity;

  /// Used to compute the velocity of the radar
  FVector PrevLocation;

  FVector2D CosSinIncrement;

  UWorld* World;

  float LineTraceIncrement;

  int CurrentDebugDelay = 0;




};
