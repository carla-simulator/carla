// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/GnssSensor.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/MapGen/LargeMapManager.h"

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

void AGnssSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AGnssSensor::PostPhysTick);

  FVector ActorLocation = GetActorLocation();
  ALargeMapManager * LargeMap = UCarlaStatics::GetLargeMapManager(GetWorld());
  if (LargeMap)
  {
    ActorLocation = LargeMap->LocalToGlobalLocation(ActorLocation);
  }
  carla::geom::Location Location = ActorLocation;
  carla::geom::GeoLocation CurrentLocation = CurrentGeoReference.Transform(Location);

  // Compute the noise for the sensor
  const float LatError = RandomEngine->GetNormalDistribution(0.0f, LatitudeDeviation);
  const float LonError = RandomEngine->GetNormalDistribution(0.0f, LongitudeDeviation);
  const float AltError = RandomEngine->GetNormalDistribution(0.0f, AltitudeDeviation);

  // Apply the noise to the sensor
  double Latitude = CurrentLocation.latitude + LatitudeBias + LatError;
  double Longitude = CurrentLocation.longitude + LongitudeBias + LonError;
  double Altitude = CurrentLocation.altitude + AltitudeBias + AltError;

  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("AGnssSensor Stream Send");
    auto Stream = GetDataStream(*this);
    Stream.Send(*this, carla::geom::GeoLocation{Latitude, Longitude, Altitude});
  }
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
