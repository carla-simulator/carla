// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ChronoMovementComponent.h"

using namespace chrono;
using namespace chrono::vehicle;
using namespace chrono::vehicle::hmmwv;

void UChronoMovementComponent::CreateChronoMovementComponent(
    ACarlaWheeledVehicle* Vehicle)
{
  UChronoMovementComponent* ChronoMovementComponent = NewObject<UChronoMovementComponent>(Vehicle);
  ChronoMovementComponent->RegisterComponent();
  Vehicle->SetCarlaMovementComponent(ChronoMovementComponent);
}

void UChronoMovementComponent::BeginPlay()
{
  Super::BeginPlay();

  // DisableUE4VehiclePhysics();

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
  my_hmmwv.SetInitPosition(ChCoordsys<>(ChVector<>(10, 10, 0), QUNIT));
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
  double Time = my_hmmwv.GetSystem()->GetChTime(); // this line crashes, my_hmmwv.GetSystem() returns an invalid pointer

  double Throttle = 0; //Control.Throttle;
  double Steering = 0; //Control.Steer;
  double Brake = 0; //Control.Brake + Control.bHandBrake;

  my_hmmwv.Synchronize(Time, {Throttle, Steering, Brake}, *terrain.get());
  terrain->Synchronize(Time);
  carla::log_warning("ChronoProcessControl");
}

void UChronoMovementComponent::TickComponent(float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction)
{
  double Time = my_hmmwv.GetSystem()->GetChTime();  // this line crashes, my_hmmwv.GetSystem() returns an invalid pointer
  terrain->Advance(DeltaTime);
  my_hmmwv.Advance(DeltaTime);
  sys.DoStepDynamics(DeltaTime);

  auto* vehicle = &my_hmmwv.GetVehicle(); // this line crashes, my_hmmwv.GetVehicle() also returns an invalid pointer
  if(vehicle)
  {
  auto VehiclePos = vehicle->GetVehiclePos();
  auto VehicleRot = vehicle->GetVehicleRot();
  carla::log_warning("Time:", Time);
  carla::log_warning("vehicle pos (", VehiclePos.x(), VehiclePos.y(), VehiclePos.z(), ")");
  carla::log_warning("vehicle rot (", VehicleRot.e1(), VehicleRot.e2(), VehicleRot.e3(), VehicleRot.e0(), ")");
  CarlaVehicle->SetActorLocation(FVector(VehiclePos.x(), VehiclePos.y(), VehiclePos.z()));
  CarlaVehicle->SetActorRotation(FQuat(VehicleRot.e1(), VehicleRot.e2(), VehicleRot.e3(), VehicleRot.e0()));
  }
  else
  {
    carla::log_warning("vehicle not initialized");
  }

  carla::log_warning("ChronoTick");
}
