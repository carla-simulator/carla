// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB), and the INTEL Visual Computing Lab.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "VehicleSpawnerBase.h"

#include "AI/WheeledVehicleAIController.h"
#include "CarlaWheeledVehicle.h"
#include "Util/RandomEngine.h"

#include "Engine/PlayerStartPIE.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

static bool VehicleIsValid(const ACarlaWheeledVehicle *Vehicle)
{
  return ((Vehicle != nullptr) && !Vehicle->IsPendingKill());
}

static AWheeledVehicleAIController *GetController(ACarlaWheeledVehicle *Vehicle)
{
  return (VehicleIsValid(Vehicle) ? Cast<AWheeledVehicleAIController>(Vehicle->GetController()) : nullptr);
}

// =============================================================================
// -- AVehicleSpawnerBase ------------------------------------------------------
// =============================================================================

// Sets default values
AVehicleSpawnerBase::AVehicleSpawnerBase(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer) {}

void AVehicleSpawnerBase::BeginPlay()
{
  Super::BeginPlay();

  NumberOfVehicles = FMath::Max(0, NumberOfVehicles);

  // Allocate space for walkers.
  Vehicles.Reserve(NumberOfVehicles);

  // Find spawn points present in level.
  for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It) {
    SpawnPoints.Add(*It);
  }

  UE_LOG(LogCarla, Log, TEXT("Found %d positions for spawning vehicles"), SpawnPoints.Num());

  if (SpawnPoints.Num() < NumberOfVehicles) {
    bSpawnVehicles = false;
    UE_LOG(LogCarla, Error, TEXT("We don't have enough spawn points for vehicles!"));
  }

  if (bSpawnVehicles) {
    const int32 MaximumNumberOfAttempts = 4 * NumberOfVehicles;
    int32 NumberOfAttempts = 0;
    while ((NumberOfVehicles > Vehicles.Num()) && (NumberOfAttempts < MaximumNumberOfAttempts)) {
      // Try to spawn one vehicle.
      TryToSpawnRandomVehicle();
      ++NumberOfAttempts;
    }

    if (NumberOfVehicles > Vehicles.Num()) {
      UE_LOG(LogCarla, Error, TEXT("Requested %d vehicles, but we were only able to spawn %d"), NumberOfVehicles, Vehicles.Num());
    }
  }
}

void AVehicleSpawnerBase::SetNumberOfVehicles(const int32 Count)
{
  if (Count > 0) {
    bSpawnVehicles = true;
    NumberOfVehicles = Count;
  } else {
    bSpawnVehicles = false;
  }
}

void AVehicleSpawnerBase::TryToSpawnRandomVehicle()
{
  auto SpawnPoint = GetRandomSpawnPoint();
  if ((SpawnPoint != nullptr)) {
      SpawnVehicleAtSpawnPoint(*SpawnPoint);
  } else {
    UE_LOG(LogCarla, Error, TEXT("Unable to find spawn point"));
  }
}

void AVehicleSpawnerBase::SpawnVehicleAtSpawnPoint(
    const APlayerStart &SpawnPoint)
{
  ACarlaWheeledVehicle *Vehicle;
  SpawnVehicle(SpawnPoint.GetActorTransform(), Vehicle);
  if ((Vehicle != nullptr) && !Vehicle->IsPendingKill()) {
    Vehicle->AIControllerClass = AWheeledVehicleAIController::StaticClass();
    Vehicle->SpawnDefaultController();
    auto Controller = GetController(Vehicle);
    if (Controller != nullptr) { // Sometimes fails...
      Controller->GetRandomEngine()->Seed(GetRandomEngine()->GenerateSeed());
      Controller->SetRoadMap(GetRoadMap());
      Controller->SetAutopilot(true);
      Vehicles.Add(Vehicle);
    } else {
      UE_LOG(LogCarla, Error, TEXT("Something went wrong creating the controller for the new vehicle"));
      Vehicle->Destroy();
    }
  }
}

APlayerStart *AVehicleSpawnerBase::GetRandomSpawnPoint()
{
  return (SpawnPoints.Num() > 0 ? GetRandomEngine()->PickOne(SpawnPoints) : nullptr);
}
