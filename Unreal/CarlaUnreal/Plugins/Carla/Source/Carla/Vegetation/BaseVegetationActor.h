// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "SpringBasedVegetationComponent.h"

#include <util/ue-header-guard-begin.h>
#include "GameFramework/Actor.h"
#include <util/ue-header-guard-end.h>

#include "BaseVegetationActor.generated.h"

USTRUCT(BlueprintType)
struct FSpringBasedVegetationParameters
{
  GENERATED_BODY()
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float Beta = 0.5f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float Alpha = 0.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  FVector Gravity = FVector(0,0,-1);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float BaseSpringStrength = 10000.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float MinSpringStrength = 2000.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float HorizontalFallof = 0.1f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float VerticalFallof = 0.1f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float RestFactor = 0.5f;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float DeltaTimeOverride = -1.f;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float CollisionForceParameter = 10.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float CollisionForceMinVel = 1.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float ForceDistanceFalloffExponent = 1.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float ForceMaxDistance = 180.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float MinForceFactor = 0.01;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float LineTraceMaxDistance = 180.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float CapsuleRadius = 6.0f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float MinBoneLength = 10.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  FVector SpringStrengthMulFactor = FVector(1,1,1);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float VehicleCenterZOffset = 120.f;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  FSkeletonHierarchy Skeleton;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  bool bAutoComputeStrength = true;

};

UCLASS()
class CARLA_API ABaseVegetationActor : public AActor
{
  GENERATED_BODY()

public:

  virtual void BeginPlay() override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation") 
  FSpringBasedVegetationParameters SpringParameters;

  UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spring Based Vegetation")
  void GetParametersFromComponent();

  UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spring Based Vegetation")
  void SetParametersToComponent();

  UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spring Based Vegetation")
  void UpdateSkeletonAndParameters();

};
