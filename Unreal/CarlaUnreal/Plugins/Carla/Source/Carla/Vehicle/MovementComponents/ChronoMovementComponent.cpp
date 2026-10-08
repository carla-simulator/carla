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

#include <util/ue-header-guard-begin.h>
#include "Misc/Paths.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include <util/ue-header-guard-end.h>

#include <util/disable-ue4-macros.h>
#include <carla/rpc/String.h>
#ifdef WITH_CHRONO
#include "chrono_thirdparty/rapidjson/document.h"
#endif
#include <util/enable-ue4-macros.h>

#ifdef WITH_CHRONO
// Declared here rather than by including chrono_vehicle/utils/ChUtilsJSON.h:
// that header pulls in the tracked-vehicle headers, whose ChTrackShoe.h
// destroys a std::vector<ChContactMaterialData> (a polymorphic type without a
// virtual destructor) inline, which Clang reports as
// -Wdelete-non-abstract-non-virtual-dtor inside libc++, where no pragma
// around the include reaches. These are the only JSON helpers used here.
namespace chrono {
namespace vehicle {
CH_VEHICLE_API void ReadFileJSON(const std::string& filename, rapidjson::Document& d);
CH_VEHICLE_API std::shared_ptr<ChEngine> ReadEngineJSON(const std::string& filename);
CH_VEHICLE_API std::shared_ptr<ChTransmission> ReadTransmissionJSON(const std::string& filename);
CH_VEHICLE_API std::shared_ptr<ChTire> ReadTireJSON(const std::string& filename);
}  // namespace vehicle
}  // namespace chrono
#endif


FString UChronoMovementComponent::CreateChronoMovementComponent(
    ACarlaWheeledVehicle* Vehicle,
    uint64_t MaxSubsteps,
    float MaxSubstepDeltaTime,
    FString VehicleJSON,
    FString PowertrainJSON,
    FString TireJSON,
    FString BaseJSONPath)
{
  #ifdef WITH_CHRONO
  auto Fail = [](const FString& Error)
  {
    UE_LOG(LogCarla, Error, TEXT("%s; Chrono physics not enabled."), *Error);
    return Error;
  };
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
  else
  {
    // The templates CARLA ships, in the source tree this server was built
    // from (<repository>/Unreal/CarlaUnreal/ is the project directory).
    ChronoMovementComponent->BaseJSONPath = FPaths::ConvertRelativePathToFull(
        FPaths::ProjectDir() / TEXT("../../Co-Simulation/Chrono/Vehicles/"));
    if (!ChronoMovementComponent->BaseJSONPath.EndsWith(TEXT("/")))
    {
      ChronoMovementComponent->BaseJSONPath += TEXT("/");
    }
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
    return Fail(FString::Printf(TEXT(
        "Chrono powertrain template %s must name an \"Engine Input File\" and a "
        "\"Transmission Input File\""),
        *carla::rpc::ToFString(PowertrainPath)));
  }
  ChronoMovementComponent->EngineJSON =
      carla::rpc::ToFString(Powertrain["Engine Input File"].GetString());
  ChronoMovementComponent->TransmissionJSON =
      carla::rpc::ToFString(Powertrain["Transmission Input File"].GetString());

  // Chrono's Read*JSON helpers return null for a file they cannot read, and
  // ChWheeledVehicle dereferences that without checking, taking the server
  // down. They also only assert() the template's "Type" (compiled out in a
  // release build) and read "Template" unchecked, so an engine JSON passed
  // as a tire is accepted and the vehicle falls through the ground. Check
  // every template up front instead.
  const std::pair<const FString*, const char*> Templates[] = {
      {&ChronoMovementComponent->VehicleJSON, "Vehicle"},
      {&ChronoMovementComponent->TireJSON, "Tire"},
      {&ChronoMovementComponent->EngineJSON, "Engine"},
      {&ChronoMovementComponent->TransmissionJSON, "Transmission"}};
  for (const auto& [Template, ExpectedType] : Templates)
  {
    const std::string TemplatePath =
        carla::rpc::FromFString(ChronoMovementComponent->BaseJSONPath) +
        carla::rpc::FromFString(*Template);
    rapidjson::Document Document;
    chrono::vehicle::ReadFileJSON(TemplatePath, Document);
    if (!Document.IsObject())
    {
      return Fail(FString::Printf(
          TEXT("Could not read Chrono template %s"),
          *carla::rpc::ToFString(TemplatePath)));
    }
    if (!Document.HasMember("Type") || !Document["Type"].IsString() ||
        std::string(Document["Type"].GetString()) != ExpectedType ||
        !Document.HasMember("Template") || !Document["Template"].IsString())
    {
      return Fail(FString::Printf(
          TEXT("Chrono template %s is not a \"%s\" template (its \"Type\" must be "
               "\"%s\" and it must name a \"Template\")"),
          *carla::rpc::ToFString(TemplatePath),
          *carla::rpc::ToFString(ExpectedType),
          *carla::rpc::ToFString(ExpectedType)));
    }
  }

  // Build the Chrono vehicle before the component replaces the current one,
  // so whatever Chrono rejects still leaves the vehicle on the physics it
  // has, and the client hears about it.
  ChronoMovementComponent->CarlaVehicle = Vehicle;
  try
  {
    ChronoMovementComponent->InitializeChronoVehicle();
  }
  catch (const std::exception& Exception)
  {
    return Fail(FString::Printf(
        TEXT("Chrono could not build the vehicle: %s"),
        *carla::rpc::ToFString(Exception.what())));
  }

  Vehicle->SetCarlaMovementComponent(ChronoMovementComponent);
  ChronoMovementComponent->RegisterComponent();
  return FString();
  #else
  const FString Error = TEXT(
      "Chrono is not enabled in this build; configure CARLA with -DENABLE_CHRONO=ON");
  UE_LOG(LogCarla, Warning, TEXT("Error: %s"), *Error);
  return Error;
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
  CollisionQueryParams.bReturnPhysicalMaterial = true;
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

void UERayCastTerrain::GetProperties(
    const ChVector3d& loc,
    ChVector3d& point,
    double& height,
    ChVector3d& normal,
    float& friction) const
{
  // One trace answers every query. Chrono 10's tire models read the contact
  // point as well as the height (ChTire::DiscTerrainCollision uses it for the
  // penetration depth and the moment arm), and ChTerrain's default GetPoint()
  // puts it on the z=0 plane, so anything not overridden here makes the car
  // fall through or bounce off ground that is not at z=0.
  // Start slightly above the query point to detect the ground properly.
  FVector Location = ChronoToUE4Location(loc + ChVector3d(0,0,0.5));
  auto point_pair = GetTerrainProperties(Location);
  if (point_pair.first)
  {
    point = UE4LocationToChrono(point_pair.second.Location);
    height = point.z();
    normal = UE4DirectionToChrono(point_pair.second.Normal);
    // The friction of the surface hit, as Chaos would see it. Chrono's tire
    // models scale their grip by this over the tire's own mu0, so a fixed
    // value ignored CARLA's surfaces and, at 1.0 against the sedan tires'
    // 0.8, gave them about 25% more grip than specified.
    const UPhysicalMaterial* Material = point_pair.second.PhysMaterial.Get();
    friction = Material ? Material->Friction : DefaultFriction;
    return;
  }
  friction = DefaultFriction;
  height = -1000000.0;
  point = ChVector3d(loc.x(), loc.y(), height);
  normal = UE4DirectionToChrono(FVector(0,0,1));
}

double UERayCastTerrain::GetHeight(const ChVector3d& loc) const
{
  ChVector3d Point, Normal;
  double Height;
  float Friction;
  GetProperties(loc, Point, Height, Normal, Friction);
  return Height;
}
ChVector3d UERayCastTerrain::GetPoint(const ChVector3d& loc) const
{
  ChVector3d Point, Normal;
  double Height;
  float Friction;
  GetProperties(loc, Point, Height, Normal, Friction);
  return Point;
}
ChVector3d UERayCastTerrain::GetNormal(const ChVector3d& loc) const
{
  ChVector3d Point, Normal;
  double Height;
  float Friction;
  GetProperties(loc, Point, Height, Normal, Friction);
  return Normal;
}
float UERayCastTerrain::GetCoefficientFriction(const ChVector3d& loc) const
{
  ChVector3d Point, Normal;
  double Height;
  float Friction;
  GetProperties(loc, Point, Height, Normal, Friction);
  return Friction;
}

void UChronoMovementComponent::BeginPlay()
{
  Super::BeginPlay();

  DisableUE4VehiclePhysics();

  // The Chrono vehicle was built by CreateChronoMovementComponent, before
  // this component replaced the previous one.

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
  // // // Chrono System
  // Chrono 9 stopped giving a system a collision system by default; keep the
  // Bullet one Chrono 6 used to create implicitly.
  Sys.SetCollisionSystemType(ChCollisionSystem::Type::BULLET);
  Sys.SetGravitationalAcceleration(ChVector3d(0, 0, -9.81));
  Sys.SetSolverType(ChSolver::Type::BARZILAIBORWEIN);
  Sys.GetSolver()->AsIterative()->SetMaxIterations(150);
  Sys.SetMaxPenetrationRecoverySpeed(4.0);

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
  if (!Engine || !Transmission)
  {
    throw std::runtime_error("could not read the engine or transmission template");
  }
  Vehicle->InitializePowertrain(
      chrono_types::make_shared<ChPowertrainAssembly>(Engine, Transmission));
  // Create and initialize the tires. Unreal renders the vehicle, so Chrono
  // gets no visualization: a MESH one would load the tire .obj on the server
  // and crash it (ChTire::AddVisualizationMesh) when the file is missing.
  for (auto& axle : Vehicle->GetAxles()) {
      for (auto& wheel : axle->GetWheels()) {
          auto tire = ReadTireJSON(Tire_string);
          if (!tire)
          {
            throw std::runtime_error("could not read the tire template");
          }
          Vehicle->InitializeTire(tire, wheel, VisualizationType::NONE);
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
  // A diverging simulation flings the vehicle through huge but finite
  // positions (kilometres per step) before it reaches Inf and then NaN, and
  // handing Chaos a vehicle out there makes Chaos produce NaN in turn. So
  // check for a non-finite pose, an implausible speed, or a jump in position
  // or orientation no vehicle could make in one tick (a forced divergence
  // stood the car on its nose, 1.3 to 84.8 degrees of pitch, in one 0.1 s
  // step), and do it before the actor is moved: the vehicle is handed back
  // where it last was valid, at rest.
  // DisableChronoPhysics() re-enables the Chaos simulation; creating the
  // default movement component alone leaves the vehicle without physics.
  constexpr double MaxPlausibleSpeed = 150.0; // m/s, 540 km/h
  constexpr double MaxPlausibleTurnRate = 4.0 * PI; // rad/s, 720 deg/s
  auto IsFinite = [](double X, double Y, double Z, double W = 0.0)
  {
    return FMath::IsFinite(X) && FMath::IsFinite(Y) &&
        FMath::IsFinite(Z) && FMath::IsFinite(W);
  };
  const double ChronoSpeed = Vehicle->GetPointVelocity(ChVector3d(0,0,0)).Length();
  FRotator NewRotator = NewRotation.Rotator();
  // adding small rotation to compensate chrono offset
  const float ChronoPitchOffset = 2.5f;
  NewRotator.Add(ChronoPitchOffset, 0.f, 0.f);
  const double JumpSpeed = DeltaTime > 0.f ?
      CMTOM * FVector::Dist(NewLocation, CarlaVehicle->GetActorLocation()) / DeltaTime :
      0.0;
  const double TurnRate = DeltaTime > 0.f ?
      CarlaVehicle->GetActorQuat().AngularDistance(NewRotator.Quaternion()) / DeltaTime :
      0.0;
  if (!IsFinite(NewLocation.X, NewLocation.Y, NewLocation.Z) ||
      !IsFinite(NewRotation.X, NewRotation.Y, NewRotation.Z, NewRotation.W) ||
      !FMath::IsFinite(ChronoSpeed) || ChronoSpeed > MaxPlausibleSpeed ||
      JumpSpeed > MaxPlausibleSpeed || TurnRate > MaxPlausibleTurnRate)
  {
    UE_LOG(LogCarla, Warning, TEXT(
        "Error: Chrono simulation diverged (non-finite pose, a speed of %g m/s, "
        "a jump of %g m/s or a turn of %g deg/s). Disabling chrono physics..."),
        ChronoSpeed, JumpSpeed, FMath::RadiansToDegrees(TurnRate));
    DisableChronoPhysics(true);
    return;
  }
  CarlaVehicle->SetActorLocation(NewLocation);
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

void UChronoMovementComponent::DisableChronoPhysics(bool bResetVelocity)
{
  this->SetComponentTickEnabled(false);
  // Read before the swap below, while GetVelocity() is still Chrono's.
  const FVector Velocity = bResetVelocity ? FVector::ZeroVector : GetVelocity();
  CarlaVehicle->OnActorHit.RemoveDynamic(this, &UChronoMovementComponent::OnVehicleHit);
  CarlaVehicle->GetMesh()->OnComponentBeginOverlap.RemoveDynamic(
      this, &UChronoMovementComponent::OnVehicleOverlap);
  CarlaVehicle->GetMesh()->SetCollisionResponseToChannel(
      ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
  // Swap the movement component before Chaos is re-enabled: recreating the
  // Chaos physics state reads the vehicle's velocity, which goes through the
  // current movement component, and a diverged Chrono one answers NaN. This
  // destroys this component, but it stays valid until garbage collection.
  UDefaultMovementComponent::CreateDefaultMovementComponent(CarlaVehicle);
  EnableUE4VehiclePhysics(Velocity, bResetVelocity);
}

void UChronoMovementComponent::OnVehicleHit(AActor *Actor,
    AActor *OtherActor,
    FVector NormalImpulse,
    const FHitResult &Hit)
{
  carla::log_warning("Chrono physics does not support collisions yet, reverting to the default physics.");
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
    carla::log_warning("Chrono physics does not support collisions yet, reverting to the default physics.");
    DisableChronoPhysics();
  }
}
