// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Autoware/Data/MgrsDataAsset.h"

#include <util/disable-ue4-macros.h>
#include <carla/geom/GeoProjection.h>
#include <util/enable-ue4-macros.h>

#include "AutowareGnssSensor.generated.h"

/// Autoware GNSS sensor ("sensor.other.autoware_gnss"): streams GeoLocation
/// to the client like the regular GNSS sensor, but on the ROS2 side publishes
/// the sensor pose in the map frame (world transform + MGRS offset) as
/// <base>/pose and <base>/pose_with_covariance. Ported from
/// tier4/autoware-support.
UCLASS()
class CARLA_API AAutowareGnssSensor : public ASensor
{
  GENERATED_BODY()

public:

  AAutowareGnssSensor(const FObjectInitializer &ObjectInitializer);

  static FActorDefinition GetSensorDefinition();

  void Set(const FActorDescription &ActorDescription);

  void SetLatitudeDeviation(float Value);
  void SetLongitudeDeviation(float Value);
  void SetAltitudeDeviation(float Value);

  void SetLatitudeBias(float Value);
  void SetLongitudeBias(float Value);
  void SetAltitudeBias(float Value);

  float GetLatitudeDeviation() const;
  float GetLongitudeDeviation() const;
  float GetAltitudeDeviation() const;

  float GetLatitudeBias() const;
  float GetLongitudeBias() const;
  float GetAltitudeBias() const;

  double GetLatitudeValue() const;
  double GetLongitudeValue() const;
  double GetAltitudeValue() const;

  UFUNCTION(BlueprintCallable)
  bool IsNoiseErrorEnabled() const;

  UFUNCTION(BlueprintCallable)
  void LoadMgrsData();

protected:

  virtual void BeginPlay() override;

  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;

  UFUNCTION(BlueprintCallable)
  void SetNoiseErrorActive(const bool bEnabled);

private:

  UPROPERTY()
  TObjectPtr<UMgrsDataAsset> MgrsDataAsset = nullptr;

  carla::geom::GeoProjection CurrentGeoProjection;

  bool bApplyNoiseError = false;

  float LatitudeDeviation;
  float LongitudeDeviation;
  float AltitudeDeviation;

  float LatitudeBias;
  float LongitudeBias;
  float AltitudeBias;

  double LatitudeValue;
  double LongitudeValue;
  double AltitudeValue;
};
