// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Util/BoundingBoxCalculator.h"

#include "Carla/Traffic/TrafficSignBase.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

FBoundingBox UBoundingBoxCalculator::GetActorBoundingBox(const AActor *Actor)
{
  if (Actor != nullptr)
  {
    // Vehicle.
    auto Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
    if (Vehicle != nullptr)
    {
      FVector Origin = Vehicle->GetVehicleBoundingBoxTransform().GetTranslation();
      FVector Extent = Vehicle->GetVehicleBoundingBoxExtent();
      return {Origin, Extent};
    }
    // Walker.
    auto Character = Cast<ACharacter>(Actor);
    if (Character != nullptr)
    {
      auto Capsule = Character->GetCapsuleComponent();
      if (Capsule != nullptr)
      {
        const auto Radius = Capsule->GetScaledCapsuleRadius();
        const auto HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
        // Characters have the pivot point centered.
        FVector Origin = {0.0f, 0.0f, 0.0f};
        FVector Extent = {Radius, Radius, HalfHeight};
        return {Origin, Extent};
      }
    }
    // Traffic sign.
    auto TrafficSign = Cast<ATrafficSignBase>(Actor);
    if (TrafficSign != nullptr)
    {
      auto TriggerVolume = TrafficSign->GetTriggerVolume();
      if (TriggerVolume != nullptr)
      {
        FVector Origin = TriggerVolume->GetRelativeTransform().GetTranslation();
        FVector Extent = TriggerVolume->GetScaledBoxExtent();
        return {Origin, Extent};
      }
      else
      {
        UE_LOG(LogCarla, Warning, TEXT("Traffic sign missing trigger volume: %s"), *Actor->GetName());
        return {};
      }
    }
  }
  return {};
}
