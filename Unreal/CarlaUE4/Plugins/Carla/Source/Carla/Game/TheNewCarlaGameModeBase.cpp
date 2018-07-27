// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/TheNewCarlaGameModeBase.h"

ATheNewCarlaGameModeBase::ATheNewCarlaGameModeBase(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;
  bAllowTickBeforeBeginPlay = false;

  Episode = CreateDefaultSubobject<UCarlaEpisode>(TEXT("Episode"));
}

void ATheNewCarlaGameModeBase::InitGame(
    const FString &MapName,
    const FString &Options,
    FString &ErrorMessage)
{
  Super::InitGame(MapName, Options, ErrorMessage);

  check(Episode != nullptr);
  Episode->Initialize(MapName);

  GameInstance = Cast<UCarlaGameInstance>(GetGameInstance());
  checkf(
      GameInstance != nullptr,
      TEXT("GameInstance is not a UCarlaGameInstance, did you forget to set it in the project settings?"));

  SpawnActorFactories();
}

void ATheNewCarlaGameModeBase::BeginPlay()
{
  Super::BeginPlay();

  GameInstance->NotifyBeginEpisode(*Episode);
}

void ATheNewCarlaGameModeBase::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  GameInstance->Tick(DeltaSeconds);
}

void ATheNewCarlaGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  GameInstance->NotifyEndEpisode();

  Super::EndPlay(EndPlayReason);
}

void ATheNewCarlaGameModeBase::SpawnActorFactories()
{
  auto *World = GetWorld();
  check(World != nullptr);

  for (auto &FactoryClass : ActorFactories)
  {
    if (FactoryClass != nullptr)
    {
      auto *Factory = World->SpawnActor<ACarlaActorFactory>(FactoryClass);
      if (Factory != nullptr)
      {
        Episode->RegisterActorFactory(*Factory);
        ActorFactoryInstances.Add(Factory);
      }
      else
      {
        UE_LOG(LogCarla, Error, TEXT("Failed to spawn actor spawner"));
      }
    }
  }

}
