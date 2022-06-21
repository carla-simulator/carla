// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "TaggerDelegate.h"

#include "Game/Tagger.h"

#include "Engine/World.h"

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
