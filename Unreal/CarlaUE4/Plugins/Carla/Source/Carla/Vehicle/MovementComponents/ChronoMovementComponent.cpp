// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ChronoMovementComponent.h"
#include "Carla/Util/RayTracer.h"

#ifdef WITH_CHRONO
using namespace chrono;
using namespace chrono::vehicle;
using namespace chrono::vehicle::hmmwv;
#endif

void UChronoMovementComponent::CreateChronoMovementComponent(
    ACarlaWheeledVehicle* Vehicle, uint64_t MaxSubsteps, float MaxSubstepDeltaTime)
{
  #ifdef WITH_CHRONO
  UChronoMovementComponent* ChronoMovementComponent = NewObject<UChronoMovementComponent>(Vehicle);
  ChronoMovementComponent->MaxSubsteps = MaxSubsteps;
  ChronoMovementComponent->MaxSubstepDeltaTime = MaxSubstepDeltaTime;
  ChronoMovementComponent->RegisterComponent();
  Vehicle->SetCarlaMovementComponent(ChronoMovementComponent);
  #else
  UE_LOG(LogCarla, Warning, TEXT("Error: Chrono is not enabled") );
  #endif
}

#ifdef WITH_CHRONO

constexpr double CMTOM = 0.01;
ChVector<> UE4LocationToChrono(const FVector& Location)
{
  return CMTOM*ChVector<>(Location.X, -Location.Y, Location.Z);
}
constexpr double MTOCM = 100;
FVector ChronoToUE4Location(const ChVector<>& position)
{
  return MTOCM*FVector(position.x(), -position.y(), position.z());
}

UERayCastTerrain::UERayCastTerrain(
    ACarlaWheeledVehicle* UEVehicle,
    chrono::vehicle::ChVehicle* ChrVehicle)
    : CarlaVehicle(UEVehicle), ChronoVehicle(ChrVehicle) {}

std::pair<bool, FHitResult>
    UERayCastTerrain::GetTerrainProperties(const FVector &Location) const
{
  const double MaxDistance = 1000000;
  FVector StartLocation = Location;
  FVector EndLocation = Location + FVector(0,0,-1)*MaxDistance; // search downwards
  FHitResult Hit;
  FCollisionQueryParams CollisionQueryParams;
  CollisionQueryParams.AddIgnoredActor(CarlaVehicle);
  bool bDidHit = CarlaVehicle->GetWorld()->LineTraceSingleByChannel(
      Hit,
      StartLocation,
      EndLocation,
      ECC_GameTraceChannel2, // camera (any collision)
      CollisionQueryParams,
      FCollisionResponseParams()
  );
  return std::make_pair(bDidHit, Hit);
}

double UERayCastTerrain::GetHeight(const ChVector<>& loc) const
{
  FVector Location = ChronoToUE4Location(loc);
  carla::log_warning("Get Height at", Location.X, Location.Y, Location.Z);
  auto point_pair = GetTerrainProperties(Location);
  if (point_pair.first)
  {
    double Height = CMTOM*point_pair.second.Location.Z;
    carla::log_warning("(",loc.x(),loc.y(),loc.z(),") Height:",Height);
    return Height;
  }
  return -1000000.0;
}
ChVector<> UERayCastTerrain::GetNormal(const ChVector<>& loc) const
{
  FVector Location = ChronoToUE4Location(loc);
  auto point_pair = GetTerrainProperties(Location);
  if (point_pair.first)
  {
    FVector Normal = point_pair.second.Normal;
    carla::log_warning("(",loc.x(),loc.y(),loc.z(),") Normal:",Normal.X,Normal.Y, Normal.Z);
    return UE4LocationToChrono(Normal);
  }
  return UE4LocationToChrono(FVector(0,1,0));
}
float UERayCastTerrain::GetCoefficientFriction(const ChVector<>& loc) const
{
  return 1;
}

void UChronoMovementComponent::BeginPlay()
{
  Super::BeginPlay();

  DisableUE4VehiclePhysics();

  // // // Chrono system
  // sys.Set_G_acc(ChVector<>(0, 0, -9.81));
  sys.SetSolverType(ChSolver::Type::BARZILAIBORWEIN);
  sys.SetSolverMaxIterations(150);
  sys.SetMaxPenetrationRecoverySpeed(4.0);

  // // Create the HMMWV vehicle, set parameters, and initialize.
  // // Typical aerodynamic drag for HMMWV: Cd = 0.5 and area ~5 m2
  my_hmmwv = HMMWV_Full(&sys);
  my_hmmwv.SetContactMethod(ChContactMethod::NSC);
  my_hmmwv.SetChassisFixed(false);
  // Missing axis transformations to UE coordinate system
  FVector VehicleLocation = CarlaVehicle->GetActorLocation();
  my_hmmwv.SetInitPosition(ChCoordsys<>(ChVector<>(UE4LocationToChrono(VehicleLocation)), QUNIT));
  my_hmmwv.SetPowertrainType(PowertrainModelType::SHAFTS);
  my_hmmwv.SetDriveType(DrivelineType::FWD);
  my_hmmwv.SetTireType(TireModelType::PAC02);
  my_hmmwv.SetTireStepSize(MaxSubstepDeltaTime);
  my_hmmwv.SetAerodynamicDrag(0.5, 5.0, 1.2);
  my_hmmwv.Initialize();

  // Create the terrain
  terrain = chrono_types::make_shared<UERayCastTerrain>(CarlaVehicle, &my_hmmwv.GetVehicle());

  carla::log_warning("ChronoBeginPlay");
}

void UChronoMovementComponent::ProcessControl(FVehicleControl &Control)
{
  double Time = my_hmmwv.GetSystem()->GetChTime();

  double Throttle = Control.Throttle;
  double Steering = Control.Steer;
  double Brake = Control.Brake + Control.bHandBrake;

  my_hmmwv.Synchronize(Time, {Throttle, Steering, Brake}, *terrain.get());
  carla::log_warning("ChronoProcessControl");
}

void UChronoMovementComponent::TickComponent(float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction)
{
  carla::log_warning("DeltaTime:", DeltaTime);
  if (DeltaTime > MaxSubstepDeltaTime)
  {
    uint64_t NumberSubSteps = FGenericPlatformMath::FloorToInt(DeltaTime/MaxSubstepDeltaTime);
    if (NumberSubSteps < MaxSubsteps)
    {
      for (uint64_t i = 0; i < NumberSubSteps; ++i)
      {
        AdvanceChronoSimulation(MaxSubstepDeltaTime);
      }
      float RemainingTime = DeltaTime - NumberSubSteps*MaxSubstepDeltaTime;
      AdvanceChronoSimulation(RemainingTime);
      carla::log_warning("NumberSubSteps:", NumberSubSteps);
    }
    else
    {
      double SubDelta = DeltaTime / MaxSubsteps;
      for (uint64_t i = 0; i < MaxSubsteps; ++i)
      {
        AdvanceChronoSimulation(SubDelta);
      }
      carla::log_warning("MaxSubsteps limit, SubDelta:", SubDelta);
    }
  }
  else
  {
    carla::log_warning("Single step");
    AdvanceChronoSimulation(DeltaTime);
  }

  auto* vehicle = &my_hmmwv.GetVehicle();
  auto VehiclePos = vehicle->GetVehiclePos();
  auto VehicleRot = vehicle->GetVehicleRot();
  double Time = my_hmmwv.GetSystem()->GetChTime();
  carla::log_warning("Time:", Time);
  carla::log_warning("vehicle pos (", VehiclePos.x(), VehiclePos.y(), VehiclePos.z(), ")");
  carla::log_warning("vehicle rot (", VehicleRot.e1(), VehicleRot.e2(), VehicleRot.e3(), VehicleRot.e0(), ")");
  FVector NewLocation = ChronoToUE4Location(VehiclePos);
  if(NewLocation.ContainsNaN())
  {
    carla::log_warning("Error: Chrono vehicle position contains NaN");
    UDefaultMovementComponent::CreateDefaultMovementComponent(CarlaVehicle);
    return;
  }
  CarlaVehicle->SetActorLocation(NewLocation);
  // CarlaVehicle->SetActorRotation(FQuat(VehicleRot.e1(), VehicleRot.e2(), VehicleRot.e3(), VehicleRot.e0()));

  carla::log_warning("ChronoTick");
}

void UChronoMovementComponent::AdvanceChronoSimulation(float StepSize)
{
  my_hmmwv.Advance(StepSize);
  sys.DoStepDynamics(StepSize);
}
#endif
