// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ChronoMovementComponent.h"
#include "Carla/Util/RayTracer.h"
#include "compiler/disable-ue4-macros.h"
#include "carla/rpc/String.h"
#include "compiler/enable-ue4-macros.h"

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
ChVector<> UE4DirectionToChrono(const FVector& Location)
{
  return ChVector<>(Location.X, -Location.Y, Location.Z);
}
FVector ChronoToUE4Direction(const ChVector<>& position)
{
  return FVector(position.x(), -position.y(), position.z());
}
ChQuaternion<> UE4QuatToChrono(const FQuat& Quat)
{
  return ChQuaternion<>(Quat.W, -Quat.X, Quat.Y, -Quat.Z);
}
FQuat ChronoToUE4Quat(const ChQuaternion<>& quat)
{
  return FQuat(-quat.e1(), quat.e2(), -quat.e3(), quat.e0());
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

void DrawPoint(UWorld* World, FVector Location, FColor Color, float Size, float LifeTime)
{
  World->PersistentLineBatcher->DrawPoint(
      Location,
      Color,
      Size,
      SDPG_World,
      LifeTime);
}
void DrawLine(UWorld* World, FVector Start, FVector End, FColor Color, float Thickness, float LifeTime)
{
  World->PersistentLineBatcher->DrawLine(
      Start,
      End,
      Color,
      SDPG_World,
      Thickness,
      LifeTime);
}

double UERayCastTerrain::GetHeight(const ChVector<>& loc) const
{
  // DrawPoint(CarlaVehicle->GetWorld(), ChronoToUE4Location(loc), FColor(0,255,0), 5, 0.1);
  FVector Location = ChronoToUE4Location(loc + ChVector<>(0,0,0.5)); // small offset to detect the ground properly
  auto point_pair = GetTerrainProperties(Location);
  if (point_pair.first)
  {
    // DrawPoint(CarlaVehicle->GetWorld(), point_pair.second.Location, FColor(255,0,0), 5, 0.1);
    double Height = CMTOM*static_cast<double>(point_pair.second.Location.Z);
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
    // DrawLine(CarlaVehicle->GetWorld(), point_pair.second.Location, point_pair.second.Location+Normal*10, FColor(0,0,255), 1, 0.003);
    auto ChronoNormal = UE4DirectionToChrono(Normal);
    return ChronoNormal;
  }
  return UE4DirectionToChrono(FVector(0,0,1));
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
  sys.Set_G_acc(ChVector<>(0, 0, -9.81));
  sys.SetSolverType(ChSolver::Type::BARZILAIBORWEIN);
  sys.SetSolverMaxIterations(150);
  sys.SetMaxPenetrationRecoverySpeed(4.0);

  FVector VehicleLocation = CarlaVehicle->GetActorLocation();
  FQuat VehicleRotation = CarlaVehicle->GetActorRotation().Quaternion();
  auto ChronoLocation = UE4LocationToChrono(VehicleLocation);
  auto ChronoRotation = UE4QuatToChrono(VehicleRotation);
  // // Create the HMMWV vehicle, set parameters, and initialize.
  // // Typical aerodynamic drag for HMMWV: Cd = 0.5 and area ~5 m2
  my_hmmwv = HMMWV_Full(&sys);
  my_hmmwv.SetContactMethod(ChContactMethod::NSC);
  my_hmmwv.SetChassisFixed(false);
  // Missing axis transformations to UE coordinate system
  my_hmmwv.SetInitPosition(ChCoordsys<>(
      ChVector<>(ChronoLocation.x(), ChronoLocation.y(), ChronoLocation.z() + 0.5),
      ChronoRotation));
  my_hmmwv.SetPowertrainType(PowertrainModelType::SHAFTS);
  my_hmmwv.SetDriveType(DrivelineTypeWV::FWD);
  my_hmmwv.SetTireType(TireModelType::PAC02);
  my_hmmwv.SetTireStepSize(MaxSubstepDeltaTime);
  my_hmmwv.SetAerodynamicDrag(0.5, 5.0, 1.2);
  my_hmmwv.Initialize();
  // vehicle = WheeledVehicle(vehicle::GetDataFile(carla::rpc::FromFString(VehicleFilePath)));
  // vehicle.Initialize(ChCoordsys<>(ChronoLocation, ChronoRotation));
  // vehicle.GetChassis()->SetFixed(false);


  // Create the terrain
  terrain = chrono_types::make_shared<UERayCastTerrain>(CarlaVehicle, &my_hmmwv.GetVehicle());

  // carla::log_warning("ChronoBeginPlay");
}

void UChronoMovementComponent::ProcessControl(FVehicleControl &Control)
{
  VehicleControl = Control;
  // my_hmmwv.Synchronize(Time, {Throttle, Steering, Brake}, *terrain.get());
  // carla::log_warning("ChronoProcessControl");
}

void UChronoMovementComponent::TickComponent(float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction)
{
  // carla::log_warning("DeltaTime:", DeltaTime);
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
      if (RemainingTime > 0)
      {
        AdvanceChronoSimulation(RemainingTime);
      }
      // carla::log_warning("NumberSubSteps:", NumberSubSteps);
    }
    else
    {
      double SubDelta = DeltaTime / MaxSubsteps;
      for (uint64_t i = 0; i < MaxSubsteps; ++i)
      {
        AdvanceChronoSimulation(SubDelta);
      }
      // carla::log_warning("MaxSubsteps limit, SubDelta:", SubDelta);
    }
  }
  else
  {
    // carla::log_warning("Single step");
    AdvanceChronoSimulation(DeltaTime);
  }

  auto* vehicle = &my_hmmwv.GetVehicle();
  auto VehiclePos = vehicle->GetVehiclePos() - ChVector<>(0,0,0.5);
  auto VehicleRot = vehicle->GetVehicleRot();
  double Time = my_hmmwv.GetSystem()->GetChTime();
  // carla::log_warning("Time:", Time);
  // carla::log_warning("vehicle pos (", VehiclePos.x(), VehiclePos.y(), VehiclePos.z(), ")");
  // carla::log_warning("vehicle rot (", VehicleRot.e1(), VehicleRot.e2(), VehicleRot.e3(), VehicleRot.e0(), ")");
  FVector NewLocation = ChronoToUE4Location(VehiclePos);
  FQuat NewRotation = ChronoToUE4Quat(VehicleRot);
  if(NewLocation.ContainsNaN() || NewRotation.ContainsNaN())
  {
    UE_LOG(LogCarla, Warning, TEXT("Error: Chrono vehicle position or rotation contains NaN. Disabling chrono physics..."));
    UDefaultMovementComponent::CreateDefaultMovementComponent(CarlaVehicle);
    return;
  }
  CarlaVehicle->SetActorLocation(NewLocation);
  CarlaVehicle->SetActorRotation(NewRotation);

  // carla::log_warning("ChronoTick");
}

void UChronoMovementComponent::AdvanceChronoSimulation(float StepSize)
{
  double Time = my_hmmwv.GetSystem()->GetChTime();
  double Throttle = VehicleControl.Throttle;
  double Steering = -VehicleControl.Steer; // RHF to LHF
  double Brake = VehicleControl.Brake + VehicleControl.bHandBrake;
  my_hmmwv.Synchronize(Time, {Steering, Throttle, Brake}, *terrain.get());
  my_hmmwv.Advance(StepSize);
  sys.DoStepDynamics(StepSize);
}
#endif
