// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "VehicleSpawnerBase.h"

#include "Engine/PlayerStartPIE.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Character.h"

// Sets default values
AVehicleSpawnerBase::AVehicleSpawnerBase(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer),
  RandomStream(Seed)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;

}


void AVehicleSpawnerBase::BeginPlay()
{
  Super::BeginPlay();

  // Allocate space for walkers.
  Vehicles.Reserve(NumberOfVehicles);

  // Set seed for random numbers.
  if (!bUseFixedSeed) {
    RandomStream.GenerateNewSeed();
  } else {
    RandomStream.Initialize(Seed);
  }

  // Find spawn points present in level.
  for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It) {
    SpawnPoints.Add(*It);
  }

  UE_LOG(LogCarla, Log, TEXT("Found %d positions for spawning vehilces"), SpawnPoints.Num());

  if (SpawnPoints.Num() < 2) {
    bSpawnVehicles = false;
    UE_LOG(LogCarla, Error, TEXT("We don't have enough spawn points for vehicles!"));
  }

  while (bSpawnVehicles && (NumberOfVehicles > Vehicles.Num())) {
    // Try to spawn one walker.
    TryToSpawnRandomVehicle();
  }
}

void AVehicleSpawnerBase::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
}

// =============================================================================
// -- Other member functions ---------------------------------------------------
// =============================================================================

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
  AWheeledVehicle *Vehicle;
  SpawnVehicle(SpawnPoint.GetActorTransform(), Vehicle);
  if ((Vehicle != nullptr) && !Vehicle->IsPendingKill()) {
    Vehicle->AIControllerClass = AAICarlaVehicleController::StaticClass();
    Vehicle->SpawnDefaultController();
    auto Controller = GetVehicleController(Vehicle);
    if (Controller != nullptr) { // Sometimes fails...
      Vehicles.Add(Vehicle);
      } else {
      UE_LOG(LogCarla, Error, TEXT("Something went wrong creating the controller for the new vehicle"));
      Vehicle->Destroy();
    }
  }
}

APlayerStart *AVehicleSpawnerBase::GetRandomSpawnPoint() const
{
  return (SpawnPoints.Num() > 0 ?
      SpawnPoints[RandomStream.RandRange(0, SpawnPoints.Num() - 1)] :
      nullptr);
}