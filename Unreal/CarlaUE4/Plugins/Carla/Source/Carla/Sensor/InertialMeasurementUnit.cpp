// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/InertialMeasurementUnit.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Sensor/WorldObserver.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Math.h"
#include <compiler/enable-ue4-macros.h>

// Based on OpenDRIVE's lon and lat
const FVector AInertialMeasurementUnit::CarlaNorthVector =
    FVector(0.0f, -1.0f, 0.0f);

AInertialMeasurementUnit::AInertialMeasurementUnit(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PostPhysics;
  RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));
}

FActorDefinition AInertialMeasurementUnit::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeIMUDefinition();
}

void AInertialMeasurementUnit::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  UActorBlueprintFunctionLibrary::SetIMU(ActorDescription, this);
}

void AInertialMeasurementUnit::SetOwner(AActor *Owner)
{
  Super::SetOwner(Owner);
}

// Copy of FWorldObserver_GetAngularVelocity but using radiants
static FVector FIMU_GetActorAngularVelocityInRadians(
    AActor &Actor)
{
  const auto RootComponent = Cast<UPrimitiveComponent>(Actor.GetRootComponent());
  const FVector AngularVelocity =
      RootComponent != nullptr ?
          RootComponent->GetPhysicsAngularVelocityInRadians() :
          FVector { 0.0f, 0.0f, 0.0f };

  return AngularVelocity;
}

const carla::geom::Vector3D AInertialMeasurementUnit::ComputeAccelerometerNoise(
    const FVector &Accelerometer)
{
  // mean = 0.0f
  // norm_distr => (mean, standard_deviation)
  // Noise function = ang_vel + Bias + norm_distr
  constexpr float Mean = 0.0f;
  return carla::geom::Vector3D {
      Accelerometer.X + RandomEngine->GetNormalDistribution(Mean, StdDevAccel.X),
      Accelerometer.Y + RandomEngine->GetNormalDistribution(Mean, StdDevAccel.Y),
      Accelerometer.Z + RandomEngine->GetNormalDistribution(Mean, StdDevAccel.Z) + 9.81f
  };
}

const carla::geom::Vector3D AInertialMeasurementUnit::ComputeGyroscopeNoise(
    const FVector &Gyroscope)
{
  // mean = 0.0f
  // norm_distr => (mean, standard_deviation)
  // Noise function = ang_vel + Bias + norm_distr
  constexpr float Mean = 0.0f;
  return carla::geom::Vector3D {
      Gyroscope.X + BiasGyro.X + RandomEngine->GetNormalDistribution(Mean, StdDevGyro.X),
      Gyroscope.Y + BiasGyro.Y + RandomEngine->GetNormalDistribution(Mean, StdDevGyro.Y),
      Gyroscope.Z + BiasGyro.Z + RandomEngine->GetNormalDistribution(Mean, StdDevGyro.Z)
  };
}

void AInertialMeasurementUnit::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  namespace cg = carla::geom;

  // Accelerometer measures linear acceleration in m/s2
  constexpr float TO_METERS = 1e-2;
  const float InvDeltaTime = 1.0f / DeltaTime;

  ACarlaWheeledVehicle *vehicle = Cast<ACarlaWheeledVehicle>(GetOwner());

  cg::Vector3D Accelerometer { 0.0f, 0.0f, 0.0f };

  if(PrevLocation.Num() < 2){
    PrevLocation.Add(GetActorLocation());
    PrevDeltaTime = DeltaTime;
  } else {
    const FVector CurrentLocation = GetActorLocation();

    const FVector y2 = PrevLocation[0];
    const FVector y1 = PrevLocation[1];
    const FVector y0 = CurrentLocation;
    const float h1 = DeltaTime;
    const float h2 = PrevDeltaTime;

    const FVector a = y1 / ( h1 * h2 );
    const FVector b = y2 / ( h2 * (h2 + h1) );
    const FVector c = y0 / ( h1 * (h2 + h1) );
    FVector FVectorAccelerometer = TO_METERS * -2.0f * ( a - b - c );

    PrevLocation[0] = PrevLocation[1];
    PrevLocation[1] = CurrentLocation;
    PrevDeltaTime = DeltaTime;

    FQuat ImuRotation = GetRootComponent()->GetComponentTransform().GetRotation();
    FVectorAccelerometer = ImuRotation.UnrotateVector(FVectorAccelerometer);

    // Cast from FVector to our Vector3D to correctly send the data in m/s2
    // and apply the desired noise function, in this case a normal distribution
    Accelerometer = ComputeAccelerometerNoise(FVectorAccelerometer);

  }


  // Gyroscope measures angular velocity in rad/sec
  const FVector AngularVelocity =
      FIMU_GetActorAngularVelocityInRadians(*GetOwner());

  const FQuat SensorLocalRotation =
      RootComponent->GetRelativeTransform().GetRotation();

  const FVector FVectorGyroscope =
      SensorLocalRotation.RotateVector(AngularVelocity);

  // Cast from FVector to our Vector3D to correctly send the data in rad/s
  // and apply the desired noise function, in this case a normal distribution
  const cg::Vector3D Gyroscope = ComputeGyroscopeNoise(FVectorGyroscope);

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

void AInertialMeasurementUnit::SetAccelerationStandardDeviation(const FVector &Vec)
{
  StdDevAccel = Vec;
}

void AInertialMeasurementUnit::SetGyroscopeStandardDeviation(const FVector &Vec)
{
  StdDevGyro = Vec;
}

void AInertialMeasurementUnit::SetGyroscopeBias(const FVector &Vec)
{
  BiasGyro = Vec;
}

const FVector &AInertialMeasurementUnit::GetAccelerationStandardDeviation() const
{
  return StdDevAccel;
}

const FVector &AInertialMeasurementUnit::GetGyroscopeStandardDeviation() const
{
  return StdDevGyro;
}

const FVector &AInertialMeasurementUnit::GetGyroscopeBias() const
{
  return BiasGyro;
}

void AInertialMeasurementUnit::BeginPlay()
{
  Super::BeginPlay();

  constexpr float TO_METERS = 1e-2;
  PrevLocation.Add(GetActorLocation());
}
