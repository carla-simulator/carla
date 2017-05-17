// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/Actor.h"
#include "WalkerSpawnerBase.generated.h"

class AWalkerSpawnPoint;
class UBoxComponent;

/// Base class for spawning walkers. Implement SpawnWalker in derived
/// blueprints.
///
/// Walkers are spawned at a random AWalkerSpawnPoint present in the level, and
/// walk until its destination is reached at another random AWalkerSpawnPoint.
UCLASS(Abstract)
class CARLA_API AWalkerSpawnerBase : public AActor
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

  UFUNCTION(BlueprintCallable)
  const FRandomStream &GetRandomStream() const
  {
    return RandomStream;
  }

  UFUNCTION(BlueprintImplementableEvent)
  void SpawnWalker(const FTransform &SpawnTransform, ACharacter *&SpawnedCharacter);

  /// @}
  // ===========================================================================
  /// @name Other member functions
  // ===========================================================================
  /// @{
public:

  void SetNumberOfWalkers(int32 Count)
  {
    NumberOfWalkers = Count;
  }

private:

  void TryToSpawnRandomWalker();

  void SpawnWalkerAtSpawnPoint(const AWalkerSpawnPoint &SpawnPoint, const FVector &Destination);

  AWalkerSpawnPoint *GetRandomSpawnPoint() const;

  /// @}

private:

  /** If false, no walker will be spawned. */
  UPROPERTY(Category = "Walker Spawner", EditAnywhere)
  bool bSpawnWalkers = true;

  /** Number of walkers to be present within the volume. */
  UPROPERTY(Category = "Walker Spawner", EditAnywhere, meta = (EditCondition = bSpawnWalkers, ClampMin = "1"))
  int32 NumberOfWalkers = 10;

  /** Minimum walk distance in centimeters. */
  UPROPERTY(Category = "Walker Spawner", EditAnywhere, meta = (EditCondition = bSpawnWalkers))
  float MinimumWalkDistance = 1500.0f;

  /** If false, a random seed is generated each time. */
  UPROPERTY(Category = "Walker Spawner", EditAnywhere, meta = (EditCondition = bSpawnWalkers))
  bool bUseFixedSeed = true;

  /** Seed for spawning random walkers. */
  UPROPERTY(Category = "Walker Spawner", EditAnywhere, meta = (EditCondition = bUseFixedSeed))
  int32 Seed = 123456789;

  UPROPERTY()
  FRandomStream RandomStream;

  UPROPERTY(Category = "Walker Spawner", VisibleAnywhere, AdvancedDisplay)
  TArray<AWalkerSpawnPoint *> SpawnPoints;

  UPROPERTY(Category = "Walker Spawner", VisibleAnywhere, AdvancedDisplay)
  TArray<ACharacter *> Walkers;

  uint32 CurrentIndexToCheck = 0u;
};
