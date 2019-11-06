// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "InertialMeasurementUnit.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Vector3D.h"
#include "carla/geom/Math.h"
#include <compiler/enable-ue4-macros.h>

#include "Carla/Sensor/WorldObserver.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

// Based on OpenDRIVE's lon and lat
const FVector AInertialMeasurementUnit::CarlaNorthVector =
    FVector(0.0f, -1.0f, 0.0f);

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
  Super::Set(ActorDescription);
  // Fill the parameters that the user requested
  // Not currently needed in this sensor
}

void AInertialMeasurementUnit::SetOwner(AActor *Owner)
{
  Super::SetOwner(Owner);
}

// Copy of FWorldObserver_GetAngularVelocity but using radiants
static carla::geom::Vector3D GetActorAngularVelocityInRadians(AActor &Actor)
{
  const auto RootComponent = Cast<UPrimitiveComponent>(Actor.GetRootComponent());
  const FVector AngularVelocity =
      RootComponent != nullptr ?
          RootComponent->GetPhysicsAngularVelocityInRadians() :
          FVector{0.0f, 0.0f, 0.0f};

  return AngularVelocity;
}

void AInertialMeasurementUnit::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  namespace cg = carla::geom;

  // Accelerometer measures linear acceleration in m/s2
  FVector CurrentVelocity = GetOwner()->GetVelocity(); // cm/s
  float CurrentSimulationTime = GetWorld()->GetTimeSeconds();
  cg::Vector3D Accelerometer = 0.01f * (CurrentVelocity - PrevVelocity) / (CurrentSimulationTime - PrevSimulationTime);
  PrevVelocity = CurrentVelocity;
  PrevSimulationTime = CurrentSimulationTime;

  // Gyroscope measures angular velocity in degrees/sec
  const cg::Vector3D AngularVelocity =
      GetActorAngularVelocityInRadians(*GetOwner());

  const FTransform RotationTransform = FTransform(
      RootComponent->GetRelativeTransform().GetRotation());

  const auto Gyroscope = RotationTransform.TransformVectorNoScale({
      AngularVelocity.x,
      AngularVelocity.y,
      AngularVelocity.z
  });

  // Magnetometer: orientation with respect to the North
  const FVector ForwVect = GetActorForwardVector().GetSafeNormal2D();
  float Compass = std::acos(FVector::DotProduct(CarlaNorthVector, ForwVect));

  // Keep the angle between [0, 2pi)
  if (FVector::CrossProduct(CarlaNorthVector, ForwVect).Z > 0.0f)
  {
    Compass = cg::Math::Pi2<float>() - Compass;
  }

  auto Stream = GetDataStream(*this);
  Stream.Send(
      *this,
      Accelerometer,
      Gyroscope,
      Compass);
}

void AInertialMeasurementUnit::BeginPlay()
{
  Super::BeginPlay();

  PrevVelocity = GetVelocity();
  PrevSimulationTime = GetWorld()->GetTimeSeconds();

}