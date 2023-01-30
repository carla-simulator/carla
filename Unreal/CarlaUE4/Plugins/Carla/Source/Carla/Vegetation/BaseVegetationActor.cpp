// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "BaseVegetationActor.h"



void ABaseVegetationActor::BeginPlay()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ABaseVegetationActor::BeginPlay);
  Super::BeginPlay();
}

void ABaseVegetationActor::GetParametersFromComponent()
{
  UActorComponent* Component = 
      GetComponentByClass(USpringBasedVegetationComponent::StaticClass());
  USpringBasedVegetationComponent* SpringComponent = 
      Cast<USpringBasedVegetationComponent>(Component);
  
  if (!SpringComponent)
  {
    UE_LOG(LogCarla, Error, 
        TEXT("ABaseVegetationActor::GetParametersFromComponent Component not found"));
    return;
  }

  SpringParameters.Beta = SpringComponent->Beta;
  SpringParameters.Alpha = SpringComponent->Alpha;
  SpringParameters.Gravity = SpringComponent->Gravity;
  SpringParameters.BaseSpringStrength = SpringComponent->BaseSpringStrength;
  SpringParameters.MinSpringStrength = SpringComponent->MinSpringStrength;
  SpringParameters.HorizontalFallof = SpringComponent->HorizontalFallof;
  SpringParameters.VerticalFallof = SpringComponent->VerticalFallof;
  SpringParameters.RestFactor = SpringComponent->RestFactor;
  SpringParameters.DeltaTimeOverride = SpringComponent->DeltaTimeOverride;
  SpringParameters.CollisionForceParameter = SpringComponent->CollisionForceParameter;
  SpringParameters.CollisionForceMinVel = SpringComponent->CollisionForceMinVel;
  SpringParameters.ForceDistanceFalloffExponent = SpringComponent->ForceDistanceFalloffExponent;
  SpringParameters.ForceMaxDistance = SpringComponent->ForceMaxDistance;
  SpringParameters.MinForceFactor = SpringComponent->MinForceFactor;
  SpringParameters.LineTraceMaxDistance = SpringComponent->LineTraceMaxDistance;
  SpringParameters.CapsuleRadius = SpringComponent->CapsuleRadius;
  SpringParameters.MinBoneLength = SpringComponent->MinBoneLength;
  SpringParameters.SpringStrengthMulFactor = SpringComponent->SpringStrengthMulFactor;
  SpringParameters.VehicleCenterZOffset = SpringComponent->VehicleCenterZOffset;
  SpringParameters.Skeleton = SpringComponent->Skeleton;
  SpringParameters.bAutoComputeStrength = SpringComponent->bAutoComputeStrength;

  UE_LOG(LogCarla, Log, 
      TEXT("ABaseVegetationActor::GetParametersFromComponent Copy successful"));
}

void ABaseVegetationActor::SetParametersToComponent()
{
  UActorComponent* Component = 
      GetComponentByClass(USpringBasedVegetationComponent::StaticClass());
  USpringBasedVegetationComponent* SpringComponent = 
      Cast<USpringBasedVegetationComponent>(Component);
  if (!SpringComponent)
  {
    UE_LOG(LogCarla, Error, TEXT("ABaseVegetationActor::SetParametersToComponent Component not found"));
    return;
  }

  SpringComponent->Beta = SpringParameters.Beta;
  SpringComponent->Alpha = SpringParameters.Alpha;
  SpringComponent->Gravity = SpringParameters.Gravity;
  SpringComponent->BaseSpringStrength = SpringParameters.BaseSpringStrength;
  SpringComponent->MinSpringStrength = SpringParameters.MinSpringStrength;
  SpringComponent->HorizontalFallof = SpringParameters.HorizontalFallof;
  SpringComponent->VerticalFallof = SpringParameters.VerticalFallof;
  SpringComponent->RestFactor = SpringParameters.RestFactor;
  SpringComponent->DeltaTimeOverride = SpringParameters.DeltaTimeOverride;
  SpringComponent->CollisionForceParameter = SpringParameters.CollisionForceParameter;
  SpringComponent->CollisionForceMinVel = SpringParameters.CollisionForceMinVel;
  SpringComponent->ForceDistanceFalloffExponent = SpringParameters.ForceDistanceFalloffExponent;
  SpringComponent->ForceMaxDistance = SpringParameters.ForceMaxDistance;
  SpringComponent->MinForceFactor = SpringParameters.MinForceFactor;
  SpringComponent->LineTraceMaxDistance = SpringParameters.LineTraceMaxDistance;
  SpringComponent->CapsuleRadius = SpringParameters.CapsuleRadius;
  SpringComponent->MinBoneLength = SpringParameters.MinBoneLength;
  SpringComponent->SpringStrengthMulFactor = SpringParameters.SpringStrengthMulFactor;
  SpringComponent->VehicleCenterZOffset = SpringParameters.VehicleCenterZOffset;
  SpringComponent->Skeleton = SpringParameters.Skeleton;
  SpringComponent->bAutoComputeStrength = SpringParameters.bAutoComputeStrength;

}

void ABaseVegetationActor::UpdateSkeletonAndParameters()
{
  UActorComponent* Component = 
      GetComponentByClass(USpringBasedVegetationComponent::StaticClass());
  USpringBasedVegetationComponent* SpringComponent = 
      Cast<USpringBasedVegetationComponent>(Component);
  if (!SpringComponent)
  {
    UE_LOG(LogCarla, Error, TEXT("ABaseVegetationActor::UpdateSkeletonAndParameters Component not found"));
    return;
  }
  SetParametersToComponent();
  SpringComponent->GenerateSkeletonHierarchy();
  SpringComponent->ComputeSpringStrengthForBranches();
  GetParametersFromComponent();
}
