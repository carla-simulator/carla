// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "WalkerSpawnerBase.h"

#include "Components/BoxComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"

#include "WalkerAIController.h"
#include "WalkerSpawnPoint.h"

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

static AWalkerAIController *GetController(ACharacter *Walker)
{
  return (Walker != nullptr ? Cast<AWalkerAIController>(Walker->GetController()) : nullptr);
}

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

AWalkerSpawnerBase::AWalkerSpawnerBase(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer),
  RandomStream(Seed)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;
}

// =============================================================================
// -- Overriden from AActor ----------------------------------------------------
// =============================================================================

void AWalkerSpawnerBase::BeginPlay()
{
  Super::BeginPlay();

  // Allocate space for walkers.
  Walkers.Reserve(NumberOfWalkers);

  // Set seed for random numbers.
  if (!bUseFixedSeed) {
    RandomStream.GenerateNewSeed();
  } else {
    RandomStream.Initialize(Seed);
  }

  // Find spawn points present in level.
  for (TActorIterator<AWalkerSpawnPoint> It(GetWorld()); It; ++It) {
    SpawnPoints.Add(*It);
  }
  UE_LOG(LogCarla, Log, TEXT("Found %d positions for spawning walkers"), SpawnPoints.Num());

  if (SpawnPoints.Num() < 2) {
    bSpawnWalkers = false;
    UE_LOG(LogCarla, Error, TEXT("We don't have enough spawn points for walkers!"));
  }
}

void AWalkerSpawnerBase::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  if (bSpawnWalkers && (NumberOfWalkers > Walkers.Num())) {
    // Try to spawn one walker.
    TryToSpawnRandomWalker();
  }

  if (Walkers.Num() > 0) {
    // Check one walker and kill it if necessary.
    const int32 Index = (++CurrentIndexToCheck % Walkers.Num());
    auto Walker = Walkers[Index];
    auto Controller = GetController(Walker);
    if ((Controller == nullptr) || (Controller->GetMoveStatus() != EPathFollowingStatus::Moving)) {
      Walkers.RemoveAtSwap(Index);
      if (Walker != nullptr) {
        Walker->Destroy();
      }
    }
  }
}

// =============================================================================
// -- Other member functions ---------------------------------------------------
// =============================================================================

void AWalkerSpawnerBase::TryToSpawnRandomWalker()
{
  auto SpawnPoint = GetRandomSpawnPoint();
  auto DestinationPoint = GetRandomSpawnPoint();
  if ((SpawnPoint != nullptr) && (DestinationPoint != nullptr)) {
    const auto StraightDistance =
        DestinationPoint->GetActorLocation() -
        SpawnPoint->GetActorLocation();
    if (StraightDistance.Size() >= MinimumWalkDistance) {
      SpawnWalkerAtSpawnPoint(*SpawnPoint, DestinationPoint->GetActorLocation());
    }
  } else {
    UE_LOG(LogCarla, Error, TEXT("Unable to find spawn point"));
  }
}

void AWalkerSpawnerBase::SpawnWalkerAtSpawnPoint(
    const AWalkerSpawnPoint &SpawnPoint,
    const FVector &Destination)
{
  ACharacter *Walker;
  SpawnWalker(SpawnPoint.GetActorTransform(), Walker);
  if ((Walker != nullptr) && !Walker->IsPendingKill()) {
    Walker->AIControllerClass = AWalkerAIController::StaticClass();
    Walker->SpawnDefaultController();
    auto Controller = GetController(Walker);
    if (Controller != nullptr) { // Sometimes fails...
      Controller->MoveToLocation(Destination);
      Walkers.Add(Walker);
    } else {
      UE_LOG(LogCarla, Error, TEXT("Something went wrong creating the controller for the new walker"));
      Walker->Destroy();
    }
  }
}

AWalkerSpawnPoint *AWalkerSpawnerBase::GetRandomSpawnPoint() const
{
  return (SpawnPoints.Num() > 0 ?
      SpawnPoints[RandomStream.RandRange(0, SpawnPoints.Num() - 1)] :
      nullptr);
}
