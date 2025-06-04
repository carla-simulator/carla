// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ActorWithRandomEngine.h"
#include "Carla.h"
#include "Carla/Util/RandomEngine.h"

AActorWithRandomEngine::AActorWithRandomEngine(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));
}

void AActorWithRandomEngine::OnConstruction(const FTransform &Transform)
{
  Super::OnConstruction(Transform);
  check(RandomEngine != nullptr);
  RandomEngine->Seed(Seed);
}

#if WITH_EDITOR
void AActorWithRandomEngine::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  if (PropertyChangedEvent.Property) {
    if (bGenerateRandomSeed) {
      Seed = URandomEngine::GenerateRandomSeed();
      bGenerateRandomSeed = false;
    }
    check(RandomEngine != nullptr);
    RandomEngine->Seed(Seed);
  }
}
#endif // WITH_EDITOR

void AActorWithRandomEngine::SetSeed(const int32 InSeed)
{
  check(RandomEngine != nullptr);
  Seed = InSeed;
  RandomEngine->Seed(InSeed);
}
