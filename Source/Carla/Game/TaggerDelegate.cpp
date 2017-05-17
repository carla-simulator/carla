// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "TaggerDelegate.h"

#include "Engine/World.h"
#include "Tagger.h"

UTaggerDelegate::UTaggerDelegate() :
  ActorSpawnedDelegate(FOnActorSpawned::FDelegate::CreateUObject(this, &UTaggerDelegate::OnActorSpawned)) {}

void UTaggerDelegate::RegisterSpawnHandler(UWorld *InWorld)
{
  InWorld->AddOnActorSpawnedHandler(ActorSpawnedDelegate);
}

void UTaggerDelegate::OnActorSpawned(AActor* InActor)
{
  if (InActor != nullptr) {
    ATagger::TagActor(*InActor, bSemanticSegmentationEnabled);

    TArray<ECityObjectLabel> Tags;
    ATagger::GetTagsOfTaggedActor(*InActor, Tags);
    UE_LOG(LogCarla, Warning, TEXT("Tagged recently spawned actor \"%s\""), *InActor->GetName());
    for (auto Tag : Tags) {
      UE_LOG(LogCarla, Warning, TEXT("  - %d"), (uint8)Tag);
    }
  }
}
