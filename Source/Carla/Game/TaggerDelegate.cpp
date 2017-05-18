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
  }
}
