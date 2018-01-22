// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "RoadIntersection.generated.h"

UENUM(BlueprintType)
enum class ERoadIntersectionItem : uint8
{
  Lane0           UMETA(DisplayName = "Lane 1"),
  Lane1           UMETA(DisplayName = "Lane 2"),
  Lane2           UMETA(DisplayName = "Lane 3"),
  Lane3           UMETA(DisplayName = "Lane 4"),
  Sidewalk0       UMETA(DisplayName = "Sidewalk 1"),
  Sidewalk1       UMETA(DisplayName = "Sidewalk 2"),
  Sidewalk2       UMETA(DisplayName = "Sidewalk 3"),
  Sidewalk3       UMETA(DisplayName = "Sidewalk 4"),
  LaneMarking     UMETA(DisplayName = "LaneMarking"),
};

/// A road intersection.
UCLASS()
class CARLA_API ARoadIntersection : public AActor
{
  GENERATED_BODY()

public:

  ARoadIntersection(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

  UFUNCTION(BlueprintCallable)
  void SetStaticMesh(ERoadIntersectionItem Item, UStaticMesh *StaticMesh);

private:

  void UpdateMeshes();

  UPROPERTY()
  TArray<UStaticMeshComponent *> StaticMeshComponents;

  UPROPERTY(Category = "Meshes", EditAnywhere)
  TMap<ERoadIntersectionItem, UStaticMesh *> StaticMeshes;
};
