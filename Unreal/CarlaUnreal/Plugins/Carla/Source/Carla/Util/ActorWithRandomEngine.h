// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"

#include "ActorWithRandomEngine.generated.h"

class URandomEngine;

/// Base class for actors containing a random engine with a fixed seed.
UCLASS(Abstract)
class CARLA_API AActorWithRandomEngine : public AActor
{
  GENERATED_BODY()

public:

  AActorWithRandomEngine(const FObjectInitializer& ObjectInitializer);

protected:

  virtual void OnConstruction(const FTransform &Transform) override;

#if WITH_EDITOR
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:

  UFUNCTION(BlueprintCallable)
  URandomEngine *GetRandomEngine()
  {
    return RandomEngine;
  }

  UFUNCTION(BlueprintCallable)
  int32 GetSeed() const
  {
    return Seed;
  }

  UFUNCTION(BlueprintCallable)
  void SetSeed(int32 InSeed);

private:

  /** Set a random seed. */
  UPROPERTY(Category = "Random Engine", EditAnywhere)
  bool bGenerateRandomSeed = false;

  /** Seed of the pseudo-random engine. */
  UPROPERTY(Category = "Random Engine", EditAnywhere)
  int32 Seed = 123456789;

  UPROPERTY()
  URandomEngine *RandomEngine;
};
