// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Math/DVector.h"

#include "LargeMapActorManager.generated.h"

USTRUCT()
struct FActorToConsider
{
  GENERATED_BODY()

  FActorToConsider() {}

  FActorToConsider(AActor* InActor) : Actor(InActor) {}

  AActor* Actor = nullptr;

  // Absolute location = Current Origin + Relative Tile Location
  FDVector Location;

  bool operator==(const FActorToConsider& Other)
  {
    return Actor == Other.Actor &&
           Location == Other.Location;
  }
};

/*
  Actor that was spawned or queried to be spawn at some point but it was so far away
  from the origin that was removed from the level (or not spawned).
  It is possible that the actor keeps receiving updates, eg, traffic manager.
  FGhostActor is a wrapper of the info and state of the actor in case it needs to be re-spawned.
*/
struct FGhostActor
{
  FGhostActor(const FTransform& InTransform, const FActorView& InActorView) :
    Transform(InTransform),
    ActorView(InActorView) {}

  FTransform Transform;

  FActorView ActorView;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API ULargeMapActorManager : public UActorComponent
{
  GENERATED_BODY()

public:
  // Sets default values for this component's properties
  ULargeMapActorManager();

  const TArray<FActorToConsider>& GetActorsToConsider() const { return ActorsToConsider; }

protected:
  // Called when the game starts
  void BeginPlay() override;

public:

  void ChangeWorldOrigin(FIntVector InSrcOrigin, FIntVector InDstOrigin);

  /**
   * @brief Returns true if new hero vehicle was added
   *
   */
  bool OnActorSpawned(const FActorView& ActorView);

  void AddActor(const FActorView& ActorView, const FTransform& Transform);

  void AddActorToConsider(AActor* InActor);

  void RemoveActorToConsider(AActor* InActor);

  void Reset();

  void UpdateAndGetActorsToConsiderPosition(TArray<FDVector>& OutActorsLocation);

  void GetActorsToConsiderPosition(TArray<FDVector>& OutActorsLocation) const;

private:

  TArray<FActorToConsider> ActorsToConsider;

  TMap<uint32, FGhostActor> GhostActors;

  // Current Origin after rebase
  FIntVector CurrentOriginInt{ 0 };
  FDVector CurrentOriginD;

#if WITH_EDITOR
public:

  void PrintMapInfo(float MsgTime, int32 FirstMsgIndex, int32 MsgLimit);

#endif // WITH_EDITOR

};
