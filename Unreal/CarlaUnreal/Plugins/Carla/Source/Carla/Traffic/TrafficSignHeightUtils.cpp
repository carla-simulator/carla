// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficSignHeightUtils.h"

#include "Carla.h"
#include "TrafficSignBase.h"

#include <util/ue-header-guard-begin.h>
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include <util/ue-header-guard-end.h>

namespace TrafficSignHeightUtils
{
  bool AdjustLocationToGround(
      UWorld* World,
      FVector& Location,
      const TArray<AActor*>& IgnoredActors,
      const TArray<UPrimitiveComponent*>& IgnoredComponents)
  {
    if (!World)
    {
      return false;
    }

    const FVector Start = Location + FVector(0.0f, 0.0f, 200.0f);
    const FVector End = Location - FVector(0.0f, 0.0f, 10000.0f);

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.bTraceComplex = true;
    CollisionParams.bReturnPhysicalMaterial = false;
    CollisionParams.AddIgnoredActors(IgnoredActors);
    CollisionParams.AddIgnoredComponents(IgnoredComponents);

    constexpr float ZOffsetSignToGround = 0.5f;
    if (World->LineTraceSingleByChannel(
            HitResult, Start, End, ECC_WorldStatic, CollisionParams))
    {
      Location.Z = HitResult.Location.Z + ZOffsetSignToGround;
      return true;
    }
    return false;
  }

  bool AdjustSignToGround(
      UWorld* World,
      ATrafficSignBase* Sign,
      const TArray<AActor*>& IgnoredActors,
      const TArray<UPrimitiveComponent*>& IgnoredComponents)
  {
    // bGeneratedFromOpenDRIVE is the guarantee that only signals spawned from
    // the OpenDRIVE map are moved. ATrafficLightManager::TrafficSigns also
    // holds hand-placed level actors that were matched to a signal, so the
    // check belongs here, at the single choke point every caller goes through.
    if (!World || !IsValid(Sign) || Sign->bPositioned ||
        !Sign->bGeneratedFromOpenDRIVE)
    {
      return false;
    }

    const FVector OriginalLocation = Sign->GetActorLocation();
    FVector AdjustedLocation = OriginalLocation;

    if (!AdjustLocationToGround(
            World, AdjustedLocation, IgnoredActors, IgnoredComponents))
    {
      UE_LOG(LogCarla, Verbose,
          TEXT("Could not find ground for traffic sign %s at %s"),
          *Sign->GetName(), *OriginalLocation.ToString());
      return false;
    }

    const float ZOffset = AdjustedLocation.Z - OriginalLocation.Z;

    USceneComponent* Root = Sign->GetRootComponent();
    const EComponentMobility::Type OriginalMobility =
        Root ? Root->Mobility.GetValue() : EComponentMobility::Static;
    if (Root)
    {
      Root->SetMobility(EComponentMobility::Movable);
    }

    Sign->SetActorLocation(AdjustedLocation);

    // Shift only the trigger volumes (not every box component on the actor) so
    // they keep their world position while the visible mesh is grounded.
    for (UBoxComponent* BoxComp : Sign->GetTriggerVolumes())
    {
      if (!BoxComp)
      {
        continue;
      }
      FVector BoxLocation = BoxComp->GetRelativeLocation();
      BoxLocation.Z -= ZOffset;
      BoxComp->SetRelativeLocation(BoxLocation);
    }

    Sign->UpdateComponentTransforms();
    if (Root)
    {
      Root->SetMobility(OriginalMobility);
    }

    Sign->bPositioned = true;
    UE_LOG(LogCarla, Log, TEXT("Adjusted sign %s height by %f cm"),
        *Sign->GetName(), ZOffset);
    return true;
  }
}
