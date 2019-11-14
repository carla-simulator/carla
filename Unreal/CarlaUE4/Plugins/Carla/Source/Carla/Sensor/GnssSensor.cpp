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
}

FActorDefinition AGnssSensor::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeGenericSensorDefinition(
    TEXT("other"),
    TEXT("gnss"));
}

void AGnssSensor::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  carla::geom::Location location = GetActorLocation();

  carla::geom::GeoLocation current_location = CurrentGeoLocation.Transform(location);

  auto Stream = GetDataStream(*this);
  double latitude = current_location.latitude;
  double longitude = current_location.longitude;
  double altitude = current_location.altitude;
  Stream.Send(*this, carla::geom::GeoLocation{latitude, longitude, altitude});

}

void AGnssSensor::BeginPlay()
{
  Super::BeginPlay();

  const UCarlaEpisode* episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
  CurrentGeoLocation = episode->GetGeoReference();
}
