// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/GnssSensor.h"
#include "Carla/Game/CarlaEpisode.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Vector3D.h"
#include <compiler/enable-ue4-macros.h>

AGnssSensor::AGnssSensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));
}

FActorDefinition AGnssSensor::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeGnssDefinition();
}

void AGnssSensor::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  UActorBlueprintFunctionLibrary::SetGnss(ActorDescription, this);
}

void AGnssSensor::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  carla::geom::Location location = GetActorLocation();

  carla::geom::GeoLocation current_location = CurrentGeoReference.Transform(location);

  auto Stream = GetDataStream(*this);
  float lat_error = RandomEngine->GetNormalDistribution(0.0f, LatitudeDeviation);
  float long_error = RandomEngine->GetNormalDistribution(0.0f, LongitudeDeviation);
  float alt_error = RandomEngine->GetNormalDistribution(0.0f, AltitudeDeviation);

  UE_LOG(LogCarla, Warning, TEXT("lat = %f - long = %f - alt = %f"), lat_error, long_error, alt_error);
  UE_LOG(LogCarla, Warning, TEXT("lat_dev = %f - long_dev = %f - alt_dev = %f"), LatitudeDeviation, LongitudeDeviation, AltitudeDeviation);

  double latitude = current_location.latitude + LatitudeBias + lat_error;
  double longitude = current_location.longitude + LongitudeBias + long_error;
  double altitude = current_location.altitude + AltitudeBias + alt_error;
  Stream.Send(*this, carla::geom::GeoLocation{latitude, longitude, altitude});

}

void AGnssSensor::SetLatitudeDeviation(float Value)
{
  LatitudeDeviation = Value;
}

void AGnssSensor::SetLongitudeDeviation(float Value)
{
  LongitudeDeviation = Value;
}

void AGnssSensor::SetAltitudeDeviation(float Value)
{
  AltitudeDeviation = Value;
}


void AGnssSensor::SetLatitudeBias(float Value)
{
  LatitudeBias = Value;
}

void AGnssSensor::SetLongitudeBias(float Value)
{
  LongitudeBias = Value;
}

void AGnssSensor::SetAltitudeBias(float Value)
{
  AltitudeBias = Value;
}

float AGnssSensor::GetLatitudeDeviation() const
{
  return LatitudeDeviation;
}
float AGnssSensor::GetLongitudeDeviation() const
{
  return LongitudeDeviation;
}
float AGnssSensor::GetAltitudeDeviation() const
{
  return AltitudeDeviation;
}

float AGnssSensor::GetLatitudeBias() const
{
  return LatitudeBias;
}
float AGnssSensor::GetLongitudeBias() const
{
  return LongitudeBias;
}
float AGnssSensor::GetAltitudeBias() const
{
  return AltitudeBias;
}

void AGnssSensor::BeginPlay()
{
  Super::BeginPlay();

  const UCarlaEpisode* episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
  CurrentGeoReference = episode->GetGeoReference();
}
