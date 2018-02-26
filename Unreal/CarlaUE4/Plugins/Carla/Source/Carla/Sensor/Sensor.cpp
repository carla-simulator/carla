// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Sensor.h"

ASensor::ASensor(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer),
    Id(0u) {}

void ASensor::AttachToActor(AActor *Actor)
{
  check(Actor != nullptr);
  Super::AttachToActor(Actor, FAttachmentTransformRules::KeepRelativeTransform);
  SetOwner(Actor);
  Actor->AddTickPrerequisiteActor(this);
}
