// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "InertialMeasurementUnit.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Math.h"
#include "carla/geom/Vector3D.h"
#include <compiler/enable-ue4-macros.h>

#include "Carla/Vehicle/CarlaWheeledVehicle.h"
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
  PrimaryActorTick.TickGroup = TG_PostPhysics;
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
static FVector carla_GetActorAngularVelocityInRadians(AActor &Actor)
{
  const auto RootComponent = Cast<UPrimitiveComponent>(Actor.GetRootComponent());
  const FVector AngularVelocity =
      RootComponent != nullptr ?
          RootComponent->GetPhysicsAngularVelocityInRadians() :
          FVector { 0.0f, 0.0f, 0.0f };

  return AngularVelocity;
}

void AInertialMeasurementUnit::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  namespace cg = carla::geom;

  // Accelerometer measures linear acceleration in m/s2
  constexpr float TO_METERS = 1e-2;

  ACarlaWheeledVehicle *vehicle = Cast<ACarlaWheeledVehicle>(GetOwner());
  const FVector CurrentVelocity = vehicle->GetVehicleForwardSpeed() * vehicle->GetVehicleOrientation();
  float CurrentSimulationTime = GetWorld()->GetTimeSeconds();
  FVector FVectorAccelerometer =
      TO_METERS * (CurrentVelocity - PrevVelocity) / DeltaTime;

  PrevVelocity = CurrentVelocity;

  FQuat ImuRotation = GetRootComponent()->GetComponentTransform().GetRotation();
  FVectorAccelerometer = ImuRotation.UnrotateVector(FVectorAccelerometer);

  // Cast from FVector to our Vector3D to correctly send the data in m/s2
  const cg::Vector3D Accelerometer (
      FVectorAccelerometer.X,
      FVectorAccelerometer.Y,
      FVectorAccelerometer.Z
  );

  // Gyroscope measures angular velocity in rad/sec
  const FVector AngularVelocity =
      carla_GetActorAngularVelocityInRadians(*GetOwner());

  const FQuat SensorLocalRotation =
      RootComponent->GetRelativeTransform().GetRotation();

  const FVector FVectorGyroscope =
      SensorLocalRotation.RotateVector(AngularVelocity);

  // Cast from FVector to our Vector3D to correctly send the data in rad/s
  const cg::Vector3D Gyroscope (
      FVectorGyroscope.X,
      FVectorGyroscope.Y,
      FVectorGyroscope.Z
  );

  // Magnetometer: orientation with respect to the North in rad
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

  constexpr float TO_METERS = 1e-2;
  PrevVelocity = GetOwner()->GetVelocity();
}
