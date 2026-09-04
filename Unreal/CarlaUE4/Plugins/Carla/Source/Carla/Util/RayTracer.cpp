// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//
// Additional functionality added by AVL List GmbH under the terms of the MIT license.

#include "Carla.h"
#include "Carla/Util/RayTracer.h"

#include "Carla/Game/CarlaStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


namespace crp = carla::rpc;

std::vector<crp::LabelledPoint> URayTracer::CastRay(
    FVector StartLocation, FVector EndLocation, UWorld * World)
{
  TArray<FHitResult> OutHits;
  World->LineTraceMultiByChannel(
      OutHits,
      StartLocation,
      EndLocation,
      ECC_GameTraceChannel3, // overlap channel
      FCollisionQueryParams(),
      FCollisionResponseParams()
  );
  std::vector<crp::LabelledPoint> result;
  for (auto& Hit : OutHits)
  {
    UPrimitiveComponent* Component = Hit.GetComponent();
    crp::CityObjectLabel ComponentTag =
        ATagger::GetTagOfTaggedComponent(*Component);

    FVector UELocation = Hit.Location;
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(World);
    ALargeMapManager* LargeMap = GameMode->GetLMManager();
    if (LargeMap)
    {
      UELocation = LargeMap->LocalToGlobalLocation(UELocation);
    }
    result.emplace_back(crp::LabelledPoint(UELocation, ComponentTag));
  }
  return result;
}

std::pair<bool, crp::LabelledPoint> URayTracer::ProjectPoint(
    FVector StartLocation, FVector Direction, float MaxDistance, UWorld * World)
{
  FHitResult Hit;
  bool bDidHit = World->LineTraceSingleByChannel(
      Hit,
      StartLocation,
      StartLocation + Direction.GetSafeNormal() * MaxDistance,
      ECC_GameTraceChannel2, // camera
      FCollisionQueryParams(),
      FCollisionResponseParams()
  );
  if (bDidHit)
  {
    UPrimitiveComponent* Component = Hit.GetComponent();
    crp::CityObjectLabel ComponentTag =
        ATagger::GetTagOfTaggedComponent(*Component);

    FVector UELocation = Hit.Location;
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(World);
    ALargeMapManager* LargeMap = GameMode->GetLMManager();
    if (LargeMap)
    {
      UELocation = LargeMap->LocalToGlobalLocation(UELocation);
    }
    return std::make_pair(bDidHit, crp::LabelledPoint(UELocation, ComponentTag));
  }
  return std::make_pair(bDidHit, crp::LabelledPoint(FVector(0.0f,0.0f,0.0f), crp::CityObjectLabel::None));
}

std::vector<std::pair<bool, crp::ContactPoint>> URayTracer::ProjectPoints(
    const std::vector<FVector>& StartLocations,
    const std::vector<FVector>& Directions,
    float MaxDistance,
    UWorld * World,
    const std::vector<const AActor *>& IgnoredActors)
{
    std::vector<std::pair<bool, crp::ContactPoint>> Result;
    Result.reserve(StartLocations.size());

    // prepare for the ray tracing
    FHitResult Hit;

    FCollisionQueryParams QueryParams;
    QueryParams.bReturnPhysicalMaterial = true;
    for (const AActor * IgnoredActor : IgnoredActors) {
        QueryParams.AddIgnoredActor(IgnoredActor);
    }

    size_t NumLocations = StartLocations.size();
    size_t NumDirections = Directions.size();
    for (size_t i= 0; i < NumLocations; ++i) {
        const auto& StartLocation = StartLocations[i];
        const auto& Direction = i < NumDirections ? Directions[i] : Directions[NumDirections - 1];
        bool bDidHit = World->LineTraceSingleByChannel(
            Hit,
            StartLocation,
            StartLocation + Direction.GetSafeNormal() * MaxDistance,
            ECC_Visibility,
            QueryParams
        );

        if (!bDidHit)
        {
            Result.emplace_back(false, crp::ContactPoint());
            continue;
        }

        UPhysicalMaterial * Material = Hit.PhysMaterial.Get();
        float Friction = Material ? Material->Friction : -1.0f;

        Result.emplace_back(true, crp::ContactPoint(Hit.Location, crp::Vector3D(Hit.Normal.X, Hit.Normal.Y, Hit.Normal.Z), Friction));
    }

    return Result;
}
