// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.



#include "LargeMapManager.h"
#include "LargeMapUtils.h"

#include "Engine/WorldComposition.h"

#include "UncenteredPivotPointMesh.h"

#include "Walker/WalkerBase.h"


// Sets default values
ALargeMapManager::ALargeMapManager()
{
  TileManager = CreateDefaultSubobject<ULargeMapTileManager>(TEXT("TileManager"));
  ActorManager = CreateDefaultSubobject<ULargeMapActorManager>(TEXT("ActorManager"));

  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickInterval = TickInterval;
}

ALargeMapManager::~ALargeMapManager()
{
  /* Remove delegates */
  // Origin rebase
  FCoreDelegates::PostWorldOriginOffset.RemoveAll(this);
  // Level added/removed from world
  FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);
  FWorldDelegates::LevelAddedToWorld.RemoveAll(this);
}

// Called when the game starts or when spawned
void ALargeMapManager::BeginPlay()
{
	Super::BeginPlay();

  UWorld* World = GetWorld();
  /* Setup delegates */
  // Origin rebase
  FCoreDelegates::PostWorldOriginOffset.AddUObject(this, &ALargeMapManager::PostWorldOriginOffset);
  // Level added/removed from world
  FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ALargeMapManager::OnLevelAddedToWorld);
  FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &ALargeMapManager::OnLevelRemovedFromWorld);

  // Setup Origin rebase settings
  UWorldComposition* WorldComposition = World->WorldComposition;
  WorldComposition->bRebaseOriginIn3DSpace = true;
  WorldComposition->RebaseOriginDistance = RebaseOriginDistance;

}

void ALargeMapManager::PostWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin)
{
  CurrentOriginInt = InDstOrigin;
  CurrentOriginD = FDVector(InDstOrigin);

  check(TileManager);
  TileManager->ChangeWorldOrigin(InSrcOrigin, InDstOrigin);
  check(ActorManager);
  ActorManager->ChangeWorldOrigin(InSrcOrigin, InDstOrigin);
}

void ALargeMapManager::OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld)
{
  check(TileManager);
  TileManager->OnLevelAddedToWorld(InLevel);
}

void ALargeMapManager::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
  check(TileManager);
  TileManager->OnLevelRemovedFromWorld(InLevel);
}

void ALargeMapManager::OnActorSpawned(const FActorView& ActorView)
{
  check(ActorManager);
  bool HeroDetected = ActorManager->OnActorSpawned(ActorView);

  if(HeroDetected)
  {
    ActorManager->GetActorsToConsiderPosition(ActorsLocation);
    TileManager->UpdateTilesState(ActorsLocation);

    // Wait until the pending levels changes are finished to avoid spawning
    // the car without ground underneath
    GetWorld()->FlushLevelStreaming();
  }
}

void ALargeMapManager::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  ActorManager->UpdateAndGetActorsToConsiderPosition(ActorsLocation);
  TileManager->UpdateTilesState(ActorsLocation);

  EvaluatePossibleRebase();

#if WITH_EDITOR
  if (bPrintMapInfo) PrintMapInfo();
#endif // WITH_EDITOR

}

void ALargeMapManager::GenerateMap(const FString& InAssetsPath)
{
  check(TileManager);
  TileManager->GenerateMap(InAssetsPath);

  ActorManager->Reset();
}

void ALargeMapManager::AddActor(const FActorView& ActorView, const FTransform& Transform)
{
  check(ActorManager);
  ActorManager->AddActor(ActorView, Transform);
}

void ALargeMapManager::AddActorToConsider(AActor* InActor)
{
  check(ActorManager);
  ActorManager->AddActorToConsider(InActor);
}

void ALargeMapManager::RemoveActorToConsider(AActor* InActor)
{
  check(ActorManager);
  ActorManager->RemoveActorToConsider(InActor);
}

void ALargeMapManager::EvaluatePossibleRebase()
{
  // TODO: choose ego vehicle
  const TArray<FActorToConsider>& ActorsToConsider = ActorManager->GetActorsToConsider();
  if(ActorsToConsider.Num() > 0)
  {
    UWorldComposition* WorldComposition = GetWorld()->WorldComposition;
    AActor* ActorToConsider = ActorsToConsider[0].Actor;
    if( IsValid(ActorToConsider) )
    {
      FVector ActorLocation = ActorToConsider->GetActorLocation();
      FIntVector ILocation = FIntVector(ActorLocation.X, ActorLocation.Y, ActorLocation.Z);
      //WorldComposition->EvaluateWorldOriginLocation(ActorToConsider->GetActorLocation());
      if (ActorLocation.SizeSquared() > FMath::Square(RebaseOriginDistance)) {

        GetWorld()->SetNewWorldOrigin(ILocation + CurrentOriginInt);
      }
    }
  }
}

#if WITH_EDITOR

void ALargeMapManager::PrintMapInfo()
{
  const float MsgTime = 1.0f;
  const int32 MaxNumMsg = 100;
  const int32 FirstTileMngMsgIndex = 100;
  const int32 FirstActorMngMsgIndex = 200;


  TileManager->PrintMapInfo(MsgTime, FirstTileMngMsgIndex, MaxNumMsg);
  ActorManager->PrintMapInfo(MsgTime, FirstActorMngMsgIndex, MaxNumMsg);
}

#endif // WITH_EDITOR