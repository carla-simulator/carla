// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Util/ActorWithRandomEngine.h"

#include "WalkerSpawnerBase.generated.h"

class AWalkerSpawnPoint;
class AWalkerSpawnPointBase;
class UBoxComponent;

/// Base class for spawning walkers. Implement SpawnWalker in derived
/// blueprints.
///
/// Walkers are spawned at a random AWalkerSpawnPoint present in the level, and
/// walk until its destination is reached at another random AWalkerSpawnPoint.
UCLASS(Abstract)
class CARLA_API AWalkerSpawnerBase : public AActorWithRandomEngine
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{
public:

  AWalkerSpawnerBase(const FObjectInitializer& ObjectInitializer);

  /// @}
  // ===========================================================================
  /// @name Overriden from AActor
  // ===========================================================================
  /// @{
protected:

  virtual void BeginPlay() override;

public:

  virtual void Tick(float DeltaTime) override;


  /// @}
  // ===========================================================================
  /// @name Blueprintable functions
  // ===========================================================================
  /// @{
protected:

  UFUNCTION(BlueprintImplementableEvent)
  void SpawnWalker(const FTransform &SpawnTransform, ACharacter *&SpawnedCharacter);

  /// @}
  // ===========================================================================
  /// @name Other member functions
  // ===========================================================================
  /// @{
public:

  void SetNumberOfWalkers(int32 Count);

  int32 GetCurrentNumberOfWalkers() const
  {
    return Walkers.Num() + WalkersBlackList.Num();
  }

  const TArray<ACharacter *> &GetWalkersWhiteList() const {
    return Walkers;
  }

  const TArray<ACharacter *> &GetWalkersBlackList() const {
    return WalkersBlackList;
  }

private:

  const AWalkerSpawnPointBase &GetRandomSpawnPoint();

  bool TryGetValidDestination(const FVector &Origin, FVector &Destination);

  bool TryToSpawnWalkerAt(const AWalkerSpawnPointBase &SpawnPoint);

  bool TrySetDestination(ACharacter &Walker);

  bool SetRandomWalkerDestination(ACharacter * Walker);
  /// @}

private:

  /** If false, no walker will be spawned at all. */
  UPROPERTY(Category = "Walker Spawner", EditAnywhere)
  bool bSpawnWalkers = true;

  /** If false, no walker will be spawned at begin play. */
  UPROPERTY(Category = "Walker Spawner", EditAnywhere, meta = (EditCondition = bSpawnWalkers))
  bool bSpawnWalkersAtBeginPlay = true;

  /** Number of walkers to be present within the volume. */
  UPROPERTY(Category = "Walker Spawner", EditAnywhere, meta = (EditCondition = bSpawnWalkers, ClampMin = "1"))
  int32 NumberOfWalkers = 10;

  /** Minimum walk distance in centimeters. */
  UPROPERTY(Category = "Walker Spawner", EditAnywhere, meta = (EditCondition = bSpawnWalkers))
  float MinimumWalkDistance = 1500.0f;

  UPROPERTY(Category = "Walker Spawner", VisibleAnywhere, AdvancedDisplay)
  TArray<AWalkerSpawnPoint *> SpawnPoints;

  UPROPERTY(Category = "Walker Spawner", VisibleAnywhere, AdvancedDisplay)
  TArray<ACharacter *> Walkers;

  UPROPERTY(Category = "Walker Spawner", VisibleAnywhere, AdvancedDisplay)
  TArray<ACharacter *> WalkersBlackList;

  uint32 CurrentWalkerIndexToCheck = 0u;

  uint32 CurrentBlackWalkerIndexToCheck = 0u;
};
