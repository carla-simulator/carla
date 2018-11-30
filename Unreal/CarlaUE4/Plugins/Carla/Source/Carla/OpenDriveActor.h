// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Traffic/RoutePlanner.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/geom/Math.h>
#include <carla/opendrive/OpenDrive.h>
#include <compiler/enable-ue4-macros.h>

#include "OpenDriveActor.generated.h"

UCLASS()
class CARLA_API AOpenDriveActor : public AActor
{
  GENERATED_BODY()

private:

  TArray<ARoutePlanner *> RoutePlanners;

#if WITH_EDITOR
  UPROPERTY(Category = "Generate", EditAnywhere)
  bool bGenerateRoutes = false;
#endif

public:

  // Sets default values for this actor's properties
  AOpenDriveActor();

  void BuildRoutes();

  virtual void BeginPlay() override;

  virtual void BeginDestroy() override;

  virtual void OnConstruction(const FTransform &transform) override;

#if WITH_EDITOR
  void PostEditChangeProperty(struct FPropertyChangedEvent&);
#endif

  ARoutePlanner *GenerateRoutePlanner(const TArray<FVector> &waypoints);

  TArray<carla::road::element::DirectedPoint> GenerateLaneZeroPoints(
      const carla::road::element::RoadSegment *road);

  TArray<TArray<FVector>> GenerateRightLaneWaypoints(
      const carla::road::element::RoadSegment *road,
      const TArray<carla::road::element::DirectedPoint> &laneZeroPoints);

  TArray<TArray<FVector>> GenerateLeftLaneWaypoints(
      const carla::road::element::RoadSegment *road,
      const TArray<carla::road::element::DirectedPoint> &laneZeroPoints);

  void GenerateWaypointsJunction(
      const carla::road::element::RoadSegment *road,
      TArray<TArray<FVector>> &waypoints);

  void GenerateWaypointsRoad(const carla::road::element::RoadSegment *road);
};
