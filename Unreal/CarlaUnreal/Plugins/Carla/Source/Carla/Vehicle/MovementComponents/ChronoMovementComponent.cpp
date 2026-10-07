// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ChronoMovementComponent.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Vehicle/MovementComponents/DefaultMovementComponent.h"
#include "Carla/Util/RayTracer.h"

#include <util/disable-ue4-macros.h>
#include <carla/rpc/String.h>
#ifdef WITH_CHRONO
#include "chrono_vehicle/utils/ChUtilsJSON.h"
#endif
#include <util/enable-ue4-macros.h>


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

  // Since Chrono 8 a powertrain is an engine plus a transmission. The
  // powertrain template names the two, with the same keys a Chrono vehicle
  // JSON uses for its "Powertrain" block, so the RPC keeps taking one file.
  // Resolve it before the component replaces the current one, so a bad
  // template leaves the vehicle on the physics it already has.
  const std::string PowertrainPath =
      carla::rpc::FromFString(ChronoMovementComponent->BaseJSONPath) +
      carla::rpc::FromFString(ChronoMovementComponent->PowertrainJSON);
  rapidjson::Document Powertrain;
  chrono::vehicle::ReadFileJSON(PowertrainPath, Powertrain);
  if (!Powertrain.IsObject() ||
      !Powertrain.HasMember("Engine Input File") ||
      !Powertrain["Engine Input File"].IsString() ||
      !Powertrain.HasMember("Transmission Input File") ||
      !Powertrain["Transmission Input File"].IsString())
  {
    UE_LOG(LogCarla, Error, TEXT(
        "Chrono powertrain template %s must name an \"Engine Input File\" and a "
        "\"Transmission Input File\"; Chrono physics not enabled."),
        *carla::rpc::ToFString(PowertrainPath));
    return;
  }
  ChronoMovementComponent->EngineJSON =
      carla::rpc::ToFString(Powertrain["Engine Input File"].GetString());
  ChronoMovementComponent->TransmissionJSON =
      carla::rpc::ToFString(Powertrain["Transmission Input File"].GetString());

  Vehicle->SetCarlaMovementComponent(ChronoMovementComponent);
  ChronoMovementComponent->RegisterComponent();
  #else
  UE_LOG(LogCarla, Warning, TEXT("Error: Chrono is not enabled") );
  #endif
}

#ifdef WITH_CHRONO

using namespace chrono;
using namespace chrono::vehicle;

constexpr double CMTOM = 0.01;
ChVector3d UE4LocationToChrono(const FVector& Location)
{
  return CMTOM*ChVector3d(Location.X, -Location.Y, Location.Z);
}
constexpr double MTOCM = 100;
FVector ChronoToUE4Location(const ChVector3d& position)
{
  return MTOCM*FVector(position.x(), -position.y(), position.z());
}
ChVector3d UE4DirectionToChrono(const FVector& Location)
{
  return ChVector3d(Location.X, -Location.Y, Location.Z);
}
FVector ChronoToUE4Direction(const ChVector3d& position)
{
  return FVector(position.x(), -position.y(), position.z());
}
ChQuaterniond UE4QuatToChrono(const FQuat& Quat)
{
  return ChQuaterniond(Quat.W, -Quat.X, Quat.Y, -Quat.Z);
}
FQuat ChronoToUE4Quat(const ChQuaterniond& quat)
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

double UERayCastTerrain::GetHeight(const ChVector3d& loc) const
{
  FVector Location = ChronoToUE4Location(loc + ChVector3d(0,0,0.5)); // small offset to detect the ground properly
  auto point_pair = GetTerrainProperties(Location);
  if (point_pair.first)
  {
    double Height = CMTOM*static_cast<double>(point_pair.second.Location.Z);
    return Height;
  }
  return -1000000.0;
}
ChVector3d UERayCastTerrain::GetNormal(const ChVector3d& loc) const
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
float UERayCastTerrain::GetCoefficientFriction(const ChVector3d& loc) const
{
  return 1;
}

void UChronoMovementComponent::BeginPlay()
{
  Super::BeginPlay();

  DisableUE4VehiclePhysics();

  // // // Chrono System
  // Chrono 9 stopped giving a system a collision system by default; keep the
  // Bullet one Chrono 6 used to create implicitly.
  Sys.SetCollisionSystemType(ChCollisionSystem::Type::BULLET);
  Sys.SetGravitationalAcceleration(ChVector3d(0, 0, -9.81));
  Sys.SetSolverType(ChSolver::Type::BARZILAIBORWEIN);
  Sys.GetSolver()->AsIterative()->SetMaxIterations(150);
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
  SetVehicleDataPath(carla::rpc::FromFString(BaseJSONPath));

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
  // Engine and transmission named by the powertrain template, resolved in
  // CreateChronoMovementComponent.
  std::string EngineJSON_string =
      BasePath_string + carla::rpc::FromFString(EngineJSON);
  std::string TransmissionJSON_string =
      BasePath_string + carla::rpc::FromFString(TransmissionJSON);

  // Create JSON vehicle. Its powertrain and tires come from the templates
  // passed to the RPC, not from the vehicle JSON.
  Vehicle = chrono_types::make_shared<WheeledVehicle>(
      &Sys,
      VehiclePath_string,
      false,
      false);
  Vehicle->Initialize(ChCoordsysd(ChronoLocation, ChronoRotation));
  Vehicle->GetChassis()->SetFixed(false);
  // Create and initialize the powertrain System
  auto Engine = ReadEngineJSON(EngineJSON_string);
  auto Transmission = ReadTransmissionJSON(TransmissionJSON_string);
  Vehicle->InitializePowertrain(
      chrono_types::make_shared<ChPowertrainAssembly>(Engine, Transmission));
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
  auto Transmission = Vehicle ? Vehicle->GetTransmission() : nullptr;
  if (Transmission && Transmission->IsAutomatic())
  {
    Transmission->asAutomatic()->SetDriveMode(VehicleControl.bReverse ?
        ChAutomaticTransmission::DriveMode::REVERSE :
        ChAutomaticTransmission::DriveMode::FORWARD);
  }
  // ACarlaWheeledVehicle::FlushVehicleControl() rebuilds bReverse from Gear
  // after every flush, so report the gear back as the default movement
  // component does; otherwise reverse is dropped on the next tick. Chrono uses
  // the same convention: -1 reverse, 0 neutral, 1+ forward.
  if (Transmission)
  {
    Control.Gear = Transmission->GetCurrentGear();
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

  const auto ChronoPositionOffset = ChVector3d(0,0,-0.25f);
  auto VehiclePos = Vehicle->GetPos() + ChronoPositionOffset;
  auto VehicleRot = Vehicle->GetRot();
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
  FRotator NewRotator = NewRotation.Rotator();
  // adding small rotation to compensate chrono offset
  const float ChronoPitchOffset = 2.5f;
  NewRotator.Add(ChronoPitchOffset, 0.f, 0.f); 
  CarlaVehicle->SetActorRotation(NewRotator);
}

void UChronoMovementComponent::AdvanceChronoSimulation(float StepSize)
{
  double Time = Vehicle->GetSystem()->GetChTime();
  double Throttle = VehicleControl.Throttle;
  double Steering = -VehicleControl.Steer; // RHF to LHF
  double Brake = VehicleControl.Brake + VehicleControl.bHandBrake;
  Vehicle->Synchronize(Time, {Steering, Throttle, Brake, 0.0}, *Terrain.get());
  Vehicle->Advance(StepSize);
  Sys.DoStepDynamics(StepSize);
}

FVector UChronoMovementComponent::GetVelocity() const
{
  if (Vehicle)
  {
    return ChronoToUE4Location(
        Vehicle->GetPointVelocity(ChVector3d(0,0,0)));
  }
  return FVector();
}

int32 UChronoMovementComponent::GetVehicleCurrentGear() const
{
  if (Vehicle)
  {
    auto Transmission = Vehicle->GetTransmission();
    if (Transmission)
    {
      return Transmission->GetCurrentGear();
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
    Super::EndPlay(EndPlayReason);
    return;
  }
  // reset callbacks to react to collisions
  CarlaVehicle->OnActorHit.RemoveDynamic(
      this, &UChronoMovementComponent::OnVehicleHit);
  CarlaVehicle->GetMesh()->OnComponentBeginOverlap.RemoveDynamic(
      this, &UChronoMovementComponent::OnVehicleOverlap);
  CarlaVehicle->GetMesh()->SetCollisionResponseToChannel(
      ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
  Super::EndPlay(EndPlayReason);
}
#endif

void UChronoMovementComponent::DisableSpecialPhysics()
{
  DisableChronoPhysics();
}

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
}

void UChronoMovementComponent::OnVehicleHit(AActor *Actor,
    AActor *OtherActor,
    FVector NormalImpulse,
    const FHitResult &Hit)
{
  carla::log_warning("Chrono physics does not support collisions yet, reverting to default PhysX physics.");
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
    carla::log_warning("Chrono physics does not support collisions yet, reverting to default PhysX physics.");
    DisableChronoPhysics();
  }
}
