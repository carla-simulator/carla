// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "InertialMeasurementUnit.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Vector3D.h"
#include <compiler/enable-ue4-macros.h>

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

AInertialMeasurementUnit::AInertialMeasurementUnit(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

FActorDefinition AInertialMeasurementUnit::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeGenericSensorDefinition(
      TEXT("other"),
      TEXT("imu"));
}

void AInertialMeasurementUnit::Set(const FActorDescription &ActorDescription)
{

}

void AInertialMeasurementUnit::SetOwner(AActor *Owner)
{
  Super::SetOwner(Owner);
}

void AInertialMeasurementUnit::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  namespace cg = carla::geom;

  cg::Vector3D accelerometer = GetActorLocation();
  cg::Vector3D gyroscope = FVector::ZeroVector;
  cg::Vector3D compass = FVector::OneVector;

  auto Stream = GetDataStream(*this);
  Stream.Send(
      *this,
      accelerometer,
      gyroscope,
      compass);
}
