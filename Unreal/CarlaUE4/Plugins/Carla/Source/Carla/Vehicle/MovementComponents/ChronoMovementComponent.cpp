// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ChronoMovementComponent.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Vehicle/MovementComponents/DefaultMovementComponent.h"

#include "compiler/disable-ue4-macros.h"
#include <carla/rpc/String.h>
#ifdef WITH_CHRONO
#include "chrono_vehicle/utils/ChUtilsJSON.h"
#endif
#include "compiler/enable-ue4-macros.h"
#include "Carla/Util/RayTracer.h"


void UChronoMovementComponent::CreateChronoMovementComponent(
    ACarlaWheeledVehicle* Vehicle,
    uint64_t MaxSubsteps,
    float MaxSubstepDeltaTime,
    FString VehicleJSON,
    FString PowertrainJSON,
    FString TireJSON,
    FString BaseJSONPath)
{
  #ifdef WITH_CHRONO
  UChronoMovementComponent* ChronoMovementComponent = NewObject<UChronoMovementComponent>(Vehicle);
  if (!VehicleJSON.IsEmpty())
  {
    ChronoMovementComponent->VehicleJSON = VehicleJSON;
  }
  if (!PowertrainJSON.IsEmpty())
  {
    ChronoMovementComponent->PowertrainJSON = PowertrainJSON;
  }
  if (!TireJSON.IsEmpty())
  {
    ChronoMovementComponent->TireJSON = TireJSON;
  }
  if (!BaseJSONPath.IsEmpty())
  {
    ChronoMovementComponent->BaseJSONPath = BaseJSONPath;
  }
  ChronoMovementComponent->MaxSubsteps = MaxSubsteps;
  ChronoMovementComponent->MaxSubstepDeltaTime = MaxSubstepDeltaTime;
  ChronoMovementComponent->RegisterComponent();
  Vehicle->SetCarlaMovementComponent(ChronoMovementComponent);
  #else
  UE_LOG(LogCarla, Warning, TEXT("Error: Chrono is not enabled") );
  #endif
}

#ifdef WITH_CHRONO

using namespace chrono;
using namespace chrono::vehicle;

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

double UERayCastTerrain::GetHeight(const ChVector<>& loc) const
{
  FVector Location = ChronoToUE4Location(loc + ChVector<>(0,0,0.5)); // small offset to detect the ground properly
  auto point_pair = GetTerrainProperties(Location);
  if (point_pair.first)
  {
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

  // // // Chrono System
  Sys.Set_G_acc(ChVector<>(0, 0, -9.81));
  Sys.SetSolverType(ChSolver::Type::BARZILAIBORWEIN);
  Sys.SetSolverMaxIterations(150);
  Sys.SetMaxPenetrationRecoverySpeed(4.0);

  InitializeChronoVehicle();

  // Create the terrain
  Terrain = chrono_types::make_shared<UERayCastTerrain>(CarlaVehicle, Vehicle.get());

  CarlaVehicle->OnActorHit.AddDynamic(
      this, &UChronoMovementComponent::OnVehicleHit);
  CarlaVehicle->GetMesh()->OnComponentBeginOverlap.AddDynamic(
      this, &UChronoMovementComponent::OnVehicleOverlap);
  CarlaVehicle->GetMesh()->SetCollisionResponseToChannel(
      ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
}

void UChronoMovementComponent::InitializeChronoVehicle()
{
  // Initial location with small offset to prevent falling through the ground
  FVector VehicleLocation = CarlaVehicle->GetActorLocation() + FVector(0,0,25);
  FQuat VehicleRotation = CarlaVehicle->GetActorRotation().Quaternion();
  auto ChronoLocation = UE4LocationToChrono(VehicleLocation);
  auto ChronoRotation = UE4QuatToChrono(VehicleRotation);

  // Set base path for vehicle JSON files
  vehicle::SetDataPath(carla::rpc::FromFString(BaseJSONPath));

  std::string BasePath_string = carla::rpc::FromFString(BaseJSONPath);

  // Create full path for json files
  // Do NOT use vehicle::GetDataFile() as strings from chrono lib
  // messes with unreal's std lib
  std::string VehicleJSON_string = carla::rpc::FromFString(VehicleJSON);
  std::string VehiclePath_string = BasePath_string + VehicleJSON_string;
  FString VehicleJSONPath = carla::rpc::ToFString(VehiclePath_string);

  std::string PowerTrainJSON_string = carla::rpc::FromFString(PowertrainJSON);
  std::string PowerTrain_string = BasePath_string + PowerTrainJSON_string;
  FString PowerTrainJSONPath = carla::rpc::ToFString(PowerTrain_string);

  std::string TireJSON_string = carla::rpc::FromFString(TireJSON);
  std::string Tire_string = BasePath_string + TireJSON_string;
  FString TireJSONPath = carla::rpc::ToFString(Tire_string);

  UE_LOG(LogCarla, Log, TEXT("Loading Chrono files: Vehicle: %s, PowerTrain: %s, Tire: %s"),
      *VehicleJSONPath,
      *PowerTrainJSONPath,
      *TireJSONPath);
  // Create JSON vehicle
  Vehicle = chrono_types::make_shared<WheeledVehicle>(
      &Sys,
      VehiclePath_string);
  Vehicle->Initialize(ChCoordsys<>(ChronoLocation, ChronoRotation));
  Vehicle->GetChassis()->SetFixed(false);
  // Create and initialize the powertrain System
  auto powertrain = ReadPowertrainJSON(
      PowerTrain_string);
  Vehicle->InitializePowertrain(powertrain);
  // Create and initialize the tires
  for (auto& axle : Vehicle->GetAxles()) {
      for (auto& wheel : axle->GetWheels()) {
          auto tire = ReadTireJSON(Tire_string);
          Vehicle->InitializeTire(tire, wheel, VisualizationType::MESH);
      }
  }
}

void UChronoMovementComponent::ProcessControl(FVehicleControl &Control)
{
  VehicleControl = Control;
  auto PowerTrain = Vehicle->GetPowertrain();
  if (PowerTrain)
  {
    if (VehicleControl.bReverse)
    {
      PowerTrain->SetDriveMode(ChPowertrain::DriveMode::REVERSE);
    }
    else
    {
      PowerTrain->SetDriveMode(ChPowertrain::DriveMode::FORWARD);
    }
  }
}

void UChronoMovementComponent::TickComponent(float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UChronoMovementComponent::TickComponent);
  if (DeltaTime > MaxSubstepDeltaTime)
  {
    uint64_t NumberSubSteps =
        FGenericPlatformMath::FloorToInt(DeltaTime/MaxSubstepDeltaTime);
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
    }
    else
    {
      double SubDelta = DeltaTime / MaxSubsteps;
      for (uint64_t i = 0; i < MaxSubsteps; ++i)
      {
        AdvanceChronoSimulation(SubDelta);
      }
    }
  }
  else
  {
    AdvanceChronoSimulation(DeltaTime);
  }

  auto VehiclePos = Vehicle->GetVehiclePos() - ChVector<>(0,0,0.5);
  auto VehicleRot = Vehicle->GetVehicleRot();
  double Time = Vehicle->GetSystem()->GetChTime();

  FVector NewLocation = ChronoToUE4Location(VehiclePos);
  FQuat NewRotation = ChronoToUE4Quat(VehicleRot);
  if(NewLocation.ContainsNaN() || NewRotation.ContainsNaN())
  {
    UE_LOG(LogCarla, Warning, TEXT(
        "Error: Chrono vehicle position or rotation contains NaN. Disabling chrono physics..."));
    UDefaultMovementComponent::CreateDefaultMovementComponent(CarlaVehicle);
    return;
  }
  CarlaVehicle->SetActorLocation(NewLocation);
  CarlaVehicle->SetActorRotation(NewRotation);
}

void UChronoMovementComponent::AdvanceChronoSimulation(float StepSize)
{
  double Time = Vehicle->GetSystem()->GetChTime();
  double Throttle = VehicleControl.Throttle;
  double Steering = -VehicleControl.Steer; // RHF to LHF
  double Brake = VehicleControl.Brake + VehicleControl.bHandBrake;
  Vehicle->Synchronize(Time, {Steering, Throttle, Brake}, *Terrain.get());
  Vehicle->Advance(StepSize);
  Sys.DoStepDynamics(StepSize);
}

FVector UChronoMovementComponent::GetVelocity() const
{
  if (Vehicle)
  {
    return ChronoToUE4Location(
        Vehicle->GetVehiclePointVelocity(ChVector<>(0,0,0)));
  }
  return FVector();
}

int32 UChronoMovementComponent::GetVehicleCurrentGear() const
{
  if (Vehicle)
  {
    auto PowerTrain = Vehicle->GetPowertrain();
    if (PowerTrain)
    {
      return PowerTrain->GetCurrentTransmissionGear();
    }
  }
  return 0;
}

float UChronoMovementComponent::GetVehicleForwardSpeed() const
{
  if (Vehicle)
  {
    return GetVelocity().X;
  }
  return 0.f;
}

void UChronoMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  if(!CarlaVehicle)
  {
    return;
  }
  // reset callbacks to react to collisions
  CarlaVehicle->OnActorHit.RemoveDynamic(
      this, &UChronoMovementComponent::OnVehicleHit);
  CarlaVehicle->GetMesh()->OnComponentBeginOverlap.RemoveDynamic(
      this, &UChronoMovementComponent::OnVehicleOverlap);
  CarlaVehicle->GetMesh()->SetCollisionResponseToChannel(
      ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
}
#endif

void UChronoMovementComponent::DisableChronoPhysics()
{
  this->SetComponentTickEnabled(false);
  EnableUE4VehiclePhysics(true);
  CarlaVehicle->OnActorHit.RemoveDynamic(this, &UChronoMovementComponent::OnVehicleHit);
  CarlaVehicle->GetMesh()->OnComponentBeginOverlap.RemoveDynamic(
      this, &UChronoMovementComponent::OnVehicleOverlap);
  CarlaVehicle->GetMesh()->SetCollisionResponseToChannel(
      ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
  UDefaultMovementComponent::CreateDefaultMovementComponent(CarlaVehicle);
  carla::log_warning("Chrono physics does not support collisions yet, reverting to default PhysX physics.");
}

void UChronoMovementComponent::OnVehicleHit(AActor *Actor,
    AActor *OtherActor,
    FVector NormalImpulse,
    const FHitResult &Hit)
{
  DisableChronoPhysics();
}

// On car mesh overlap, only works when carsim is enabled
// (this event triggers when overlapping with static environment)
void UChronoMovementComponent::OnVehicleOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult & SweepResult)
{
  if (OtherComp->GetCollisionResponseToChannel(
      ECollisionChannel::ECC_WorldDynamic) ==
      ECollisionResponse::ECR_Block)
  {
    DisableChronoPhysics();
  }
}
