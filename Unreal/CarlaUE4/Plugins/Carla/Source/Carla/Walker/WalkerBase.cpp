// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "WalkerBase.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStatics.h"


AWalkerBase::AWalkerBase(const FObjectInitializer &ObjectInitializer)
        : Super(ObjectInitializer)
{
}

void AWalkerBase::TagWheelchair()
{
  auto* Episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
  if (!Episode) {
    return;
  }

  FCarlaActor* CarlaActor = Episode->FindCarlaActor(this);
  if (!CarlaActor){
    return;
  } 

  ATagger::TagActor(*this, true, CarlaActor->GetActorId());
}