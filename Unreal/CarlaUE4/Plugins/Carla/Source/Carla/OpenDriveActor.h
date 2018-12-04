// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BillboardComponent.h"

#include "Traffic/RoutePlanner.h"

#include "Vehicle/VehicleSpawnPoint.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/geom/Math.h>
#include <carla/opendrive/OpenDrive.h>
#include <compiler/enable-ue4-macros.h>

#include "OpenDriveActor.generated.h"

UCLASS()
class CARLA_API AOpenDriveActor : public AActor
{
  GENERATED_BODY()

protected:
  // A UBillboardComponent to hold Icon sprite
  UBillboardComponent* SpriteComponent;
  // Sprite for the Billboard Component
  UTexture2D* SpriteTexture;

private:

  UPROPERTY()
  TArray<ARoutePlanner *> RoutePlanners;

  UPROPERTY()
  TArray<AVehicleSpawnPoint *> VehicleSpawners;

#if WITH_EDITORONLY_DATA

  UPROPERTY(Category = "Generate", EditAnywhere)
  bool bGenerateRoutes = false;

  UPROPERTY(Category = "Generate", EditAnywhere, meta = (ClampMin = "0.01", UIMin = "0.01"))
  float RoadAccuracy = 2.0f;

  UPROPERTY(Category = "Generate", EditAnywhere)
  bool bRemoveRoutes = false;

  UPROPERTY(Category = "Spawners", EditAnywhere)
  bool bAddSpawners = true;

  UPROPERTY(Category = "Spawners", EditAnywhere)
  float SpawnersHeight = 300.0;

  UPROPERTY(Category = "Spawners", EditAnywhere)
  bool bRemoveCurrentSpawners = false;

  UPROPERTY(Category = "Debug", EditAnywhere)
  bool bShowDebug = true;
#endif // WITH_EDITORONLY_DATA

public:

  using RoadSegment = carla::road::element::RoadSegment;
  using DirectedPoint = carla::road::element::DirectedPoint;
  using LaneInfo = carla::road::element::LaneInfo;
  using RoadGeneralInfo = carla::road::element::RoadGeneralInfo;
  using RoadInfoLane = carla::road::element::RoadInfoLane;
  using IdType = carla::road::element::id_type;
  using CarlaMath = carla::geom::Math;

  AOpenDriveActor(const FObjectInitializer& ObjectInitializer);

  virtual void BeginDestroy() override;

  void BuildRoutes();

  void RemoveRoutes();

  void DebugRoutes() const;

  void RemoveDebugRoutes() const;

#if WITH_EDITOR
  void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif // WITH_EDITOR

  ARoutePlanner *GenerateRoutePlanner(const TArray<DirectedPoint> &waypoints);

  TArray<DirectedPoint> GenerateLaneZeroPoints(
      const RoadSegment *road);

  TArray<TArray<DirectedPoint>> GenerateRightLaneWaypoints(
      const RoadSegment *road,
      const TArray<DirectedPoint> &laneZeroPoints);

  TArray<TArray<DirectedPoint>> GenerateLeftLaneWaypoints(
      const RoadSegment *road,
      const TArray<DirectedPoint> &laneZeroPoints);

  void GenerateWaypointsJunction(
      const RoadSegment *road,
      TArray<TArray<DirectedPoint>> &waypoints);

  void GenerateWaypointsRoad(const RoadSegment *road);

  void AddSpawners();

  void RemoveSpawners();

};
