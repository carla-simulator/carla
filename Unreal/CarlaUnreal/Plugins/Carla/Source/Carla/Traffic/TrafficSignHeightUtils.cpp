// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficSignHeightUtils.h"

#include "Carla.h"
#include "TrafficSignBase.h"
#include "Carla/Game/Tagger.h"

#include <util/ue-header-guard-begin.h>
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Landscape.h"
#include <util/ue-header-guard-end.h>

namespace crp = carla::rpc;

namespace TrafficSignHeightUtils
{
  namespace
  {
    /// Whether a hit surface is ground that a sign may stand on.
    ///
    /// The downward trace crosses every static object under the sign, so a
    /// fence or any other prop beside the pole can answer before the ground
    /// does. Taking the closest hit therefore lands the sign on top of that
    /// prop, so each candidate is classified by its semantic tag instead.
    bool IsGroundHit(const FHitResult &HitResult)
    {
      // Landscapes are not tagged by ATagger, so they carry no tag at all.
      const AActor *HitActor = HitResult.GetActor();
      if (HitActor != nullptr && HitActor->IsA<ALandscape>())
      {
        return true;
      }

      const UPrimitiveComponent *Component = HitResult.GetComponent();
      if (Component == nullptr)
      {
        return false;
      }

      switch (ATagger::GetTagOfTaggedComponent(*Component))
      {
        case crp::CityObjectLabel::Roads:
        case crp::CityObjectLabel::RoadLines:
        case crp::CityObjectLabel::Sidewalks:
        case crp::CityObjectLabel::Ground:
        case crp::CityObjectLabel::Terrain:
        case crp::CityObjectLabel::Bridge:
        case crp::CityObjectLabel::RailTrack:
          return true;
        default:
          return false;
      }
    }
  }

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

    FCollisionQueryParams CollisionParams;
    CollisionParams.bTraceComplex = true;
    CollisionParams.bReturnPhysicalMaterial = false;
    CollisionParams.AddIgnoredActors(IgnoredActors);
    CollisionParams.AddIgnoredComponents(IgnoredComponents);

    // Collect every surface under the sign rather than only the closest one,
    // then keep the first that is ground. The hits come back ordered along the
    // ray, so this is the highest ground below the sign.
    //
    // The trace must use the overlap channel, the same one URayTracer::CastRay
    // uses. ECC_WorldStatic blocks, so a multi trace on it still stops at the
    // first solid surface and a prop beside the pole hides the ground behind
    // a single hit that the filter below then rejects, leaving no candidate.
    TArray<FHitResult> HitResults;
    World->LineTraceMultiByChannel(
        HitResults, Start, End, ECC_GameTraceChannel3, CollisionParams);

    constexpr float ZOffsetSignToGround = 0.5f;
    for (const FHitResult &HitResult : HitResults)
    {
      if (IsGroundHit(HitResult))
      {
        Location.Z = HitResult.Location.Z + ZOffsetSignToGround;
        return true;
      }
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
