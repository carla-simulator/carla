// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"

#include <util/disable-ue4-macros.h>
#include "carla/geom/GeoLocation.h"
#include <util/enable-ue4-macros.h>

#include "GnssSensor.generated.h"

/// Gnss sensor representation
/// The actual position calculation is done one server side
UCLASS()
class CARLA_API AGnssSensor : public ASensor
{
  GENERATED_BODY()

public:

  AGnssSensor(const FObjectInitializer &ObjectInitializer);

  static FActorDefinition GetSensorDefinition();

  void Set(const FActorDescription &ActorDescription);

  // virtual void PrePhysTick(float DeltaSeconds) override;
  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;

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

protected:

  virtual void BeginPlay() override;

private:

  carla::geom::GeoLocation CurrentGeoReference;

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
