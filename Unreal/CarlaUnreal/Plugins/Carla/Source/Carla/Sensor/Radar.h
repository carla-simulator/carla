// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"
#include "Carla/Actor/ActorDefinition.h"

#include <util/disable-ue4-macros.h>
#include <carla/sensor/data/RadarData.h>
#include <util/enable-ue4-macros.h>

#include "Radar.generated.h"

/// A ray-cast based Radar sensor.
UCLASS()
class CARLA_API ARadar : public ASensor
{
  GENERATED_BODY()

  using FRadarData = carla::sensor::data::RadarData;

public:

  static FActorDefinition GetSensorDefinition();

  ARadar(const FObjectInitializer &ObjectInitializer);

  void Set(const FActorDescription &Description) override;

  UFUNCTION(BlueprintCallable, Category = "Radar")
  void SetHorizontalFOV(float NewHorizontalFOV);

  UFUNCTION(BlueprintCallable, Category = "Radar")
  void SetVerticalFOV(float NewVerticalFOV);

  UFUNCTION(BlueprintCallable, Category = "Radar")
  void SetRange(float NewRange);

  UFUNCTION(BlueprintCallable, Category = "Radar")
  void SetPointsPerSecond(int NewPointsPerSecond);

  const FRadarData& GetRadarData() const;
protected:

  void BeginPlay() override;

  // virtual void PrePhysTick(float DeltaTime) override;
  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Detection")
  float Range;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection")
  float HorizontalFOV;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection")
  float VerticalFOV;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection")
  int PointsPerSecond;

private:

  void CalculateCurrentVelocity(const float DeltaTime);

  void SendLineTraces(float DeltaTime);

  float CalculateRelativeVelocity(const FHitResult& OutHit, const FVector& RadarLocation);

  FRadarData RadarData;

  FCollisionQueryParams TraceParams;

  FVector CurrentVelocity;

  /// Used to compute the velocity of the radar
  FVector PrevLocation;

  struct RayData {
    float Radius;
    float Angle;
    bool Hitted;
    float RelativeVelocity;
    FVector2D AzimuthAndElevation;
    float Distance;
  };

  std::vector<RayData> Rays;
};
