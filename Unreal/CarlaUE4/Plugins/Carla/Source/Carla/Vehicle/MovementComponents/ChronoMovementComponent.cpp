// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ChronoMovementComponent.h"

#ifdef WITH_CHRONO
using namespace chrono;
using namespace chrono::vehicle;
using namespace chrono::vehicle::hmmwv;
#endif

void UChronoMovementComponent::CreateChronoMovementComponent(
    ACarlaWheeledVehicle* Vehicle)
{
  #ifdef WITH_CHRONO
  UChronoMovementComponent* ChronoMovementComponent = NewObject<UChronoMovementComponent>(Vehicle);
  ChronoMovementComponent->RegisterComponent();
  Vehicle->SetCarlaMovementComponent(ChronoMovementComponent);
  #else
  UE_LOG(LogCarla, Warning, TEXT("Error: Chrono is not enabled") );
  #endif
}

#ifdef WITH_CHRONO

constexpr double CMTOM = 0.01;
ChVector<> UE4LocationToChrono(FVector& Location)
{
  return CMTOM*ChVector<>(Location.Y, Location.Z, -Location.X);
}
constexpr double MTOCM = 100;
FVector ChronoToUE4Location(ChVector<>& position)
{
  return MTOCM*FVector(-position.z(), position.x(), position.y());
}

void UChronoMovementComponent::BeginPlay()
{
  Super::BeginPlay();

  DisableUE4VehiclePhysics();

  // // // Chrono system
  sys.Set_G_acc(ChVector<>(0, 0, 0));
  sys.SetSolverType(ChSolver::Type::BARZILAIBORWEIN);
  sys.SetSolverMaxIterations(150);
  sys.SetMaxPenetrationRecoverySpeed(4.0);

  // // Create the HMMWV vehicle, set parameters, and initialize.
  // // Typical aerodynamic drag for HMMWV: Cd = 0.5 and area ~5 m2
  my_hmmwv = HMMWV_Full(&sys);
  my_hmmwv.SetContactMethod(ChMaterialSurface::NSC);
  my_hmmwv.SetChassisFixed(false);
  // Missing axis transformations to UE coordinate system
  FVector VehicleLocation = CarlaVehicle->GetActorLocation();
  my_hmmwv.SetInitPosition(ChCoordsys<>(ChVector<>(UE4LocationToChrono(VehicleLocation)), QUNIT));
  my_hmmwv.SetPowertrainType(PowertrainModelType::SHAFTS);
  my_hmmwv.SetDriveType(DrivelineType::FWD);
  my_hmmwv.SetTireType(TireModelType::PAC02);
  my_hmmwv.SetTireStepSize(0.001);
  my_hmmwv.SetAerodynamicDrag(0.5, 5.0, 1.2);
  my_hmmwv.Initialize();

  // Create the terrain
  terrain = chrono_types::make_shared<RigidTerrain>(&sys);
  auto patch = terrain->AddPatch(ChCoordsys<>(ChVector<>(0, 0, 0), QUNIT), ChVector<>(200, 100, -10));
  patch->SetContactFrictionCoefficient(0.9f);
  patch->SetContactRestitutionCoefficient(0.01f);
  patch->SetContactMaterialProperties(2e7f, 0.3f);
  terrain->Initialize();
  carla::log_warning("ChronoBeginPlay");
}

void UChronoMovementComponent::ProcessControl(FVehicleControl &Control)
{
  double Time = my_hmmwv.GetSystem()->GetChTime();

  double Throttle = Control.Throttle;
  double Steering = Control.Steer;
  double Brake = Control.Brake + Control.bHandBrake;

  my_hmmwv.Synchronize(Time, {Throttle, Steering, Brake}, *terrain.get());
  terrain->Synchronize(Time);
  carla::log_warning("ChronoProcessControl");
}

void UChronoMovementComponent::TickComponent(float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction)
{

  // Maximum delta time for the simulation to be stable
  // TODO: make this a customizable value
  const float MaxDeltaTime = 0.002;
  if (DeltaTime > MaxDeltaTime)
  {
    uint64_t NumberSubSteps = FGenericPlatformMath::FloorToInt(DeltaTime/MaxDeltaTime);
    carla::log_warning("Number of chrono substeps:", NumberSubSteps);
    for (uint64_t i = 0; i < NumberSubSteps; ++i)
    {
      AdvanceChronoSimulation(MaxDeltaTime);
    }
    float RemainingTime = DeltaTime - NumberSubSteps*MaxDeltaTime;
    AdvanceChronoSimulation(RemainingTime);
  }
  else
  {
    AdvanceChronoSimulation(DeltaTime);
  }

  auto* vehicle = &my_hmmwv.GetVehicle();
  auto VehiclePos = vehicle->GetVehiclePos();
  auto VehicleRot = vehicle->GetVehicleRot();
  double Time = my_hmmwv.GetSystem()->GetChTime();
  carla::log_warning("Time:", Time);
  carla::log_warning("vehicle pos (", VehiclePos.x(), VehiclePos.y(), VehiclePos.z(), ")");
  carla::log_warning("vehicle rot (", VehicleRot.e1(), VehicleRot.e2(), VehicleRot.e3(), VehicleRot.e0(), ")");
  CarlaVehicle->SetActorLocation(ChronoToUE4Location(VehiclePos));
  CarlaVehicle->SetActorRotation(FQuat(VehicleRot.e1(), VehicleRot.e2(), VehicleRot.e3(), VehicleRot.e0()));

  carla::log_warning("ChronoTick");
}

void UChronoMovementComponent::AdvanceChronoSimulation(float StepSize)
{
  terrain->Advance(StepSize);
  my_hmmwv.Advance(StepSize);
  sys.DoStepDynamics(StepSize);
}
#endif
