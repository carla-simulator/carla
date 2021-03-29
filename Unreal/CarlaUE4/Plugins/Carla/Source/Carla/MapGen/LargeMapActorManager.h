// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "LargeMapDataTypes.h"

#include "LargeMapActorManager.generated.h"

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
  FIntVector CurrentOriginInt{ 0 }; // TODO: remove
  FDVector CurrentOriginD; // TODO: remove

#if WITH_EDITOR
public:

  void PrintMapInfo(float MsgTime, int32 FirstMsgIndex, int32 MsgLimit);

#endif // WITH_EDITOR

};
