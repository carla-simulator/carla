// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
#pragma once

#include "Carla/Vehicle/VehicleSpawnPoint.h"

#include <util/disable-ue4-macros.h>
#include "carla/road/Map.h"
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include <util/ue-header-guard-end.h>

#include <optional>

#include "OpenDriveGenerator.generated.h"

class UMaterialInterface;
class UBoxComponent;
class UPCGComponent;
class UPCGGraphInterface;

UCLASS()
class CARLA_API AProceduralMeshActor : public AActor
{
  GENERATED_BODY()
public:
  AProceduralMeshActor();

  UPROPERTY(Category = "Procedural Mesh Actor", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TObjectPtr<UProceduralMeshComponent> MeshComponent;
};

/// Invisible marker placed along a generated road to anchor PCG-scattered
/// street furniture (lamps, signage, vegetation). Carries no mesh of its
/// own -- the PCG graph selects these by tag (see
/// AOpenDriveGenerator::GeneratePoles) via a "Get Actor Data" node and
/// spawns the actual content at each one. Does nothing on its own, same
/// spirit as AWalkerAIController's server-side stub.
UCLASS()
class CARLA_API AProceduralFurnitureAnchor : public AActor
{
  GENERATED_BODY()
public:
  AProceduralFurnitureAnchor();
};

UCLASS()
class CARLA_API AOpenDriveGenerator : public AActor
{
  GENERATED_BODY()

public:

  AOpenDriveGenerator(const FObjectInitializer &ObjectInitializer);

  /// Set the OpenDRIVE information as string and generates the
  /// queryable map structure.
  bool LoadOpenDrive(const FString &OpenDrive);

  /// Get the OpenDRIVE information as string.
  const FString &GetOpenDrive() const;

  /// Checks if the OpenDrive has been loaded and it's valid.
  bool IsOpenDriveValid() const;

  /// Generates the road and sidewalk mesh based on the OpenDRIVE information.
  void GenerateRoadMesh();

  /// Generates the crosswalk mesh based on the OpenDRIVE information.
  void GenerateCrosswalkMesh();

  /// Generates the lane-marking meshes (white/yellow) based on the
  /// OpenDRIVE information.
  void GenerateLaneMarkings();

  /// Scatters street furniture (lamp poles, roadside vegetation, signage)
  /// along the generated road network. Emits tagged AProceduralFurnitureAnchor
  /// markers via Map::GetTreesTransform (reused as-is: it already walks each
  /// non-junction road's outermost driving lane at regular s-intervals and
  /// offsets outward past the sidewalk -- exactly what furniture placement
  /// needs, no new LibCarla code required), then triggers the runtime PCG
  /// graph (see PCGComponent/StreetFurnitureGraph) that samples them by tag
  /// and spawns the actual content.
  void GeneratePoles();

  /// Generates spawn points along the road.
  void GenerateSpawnPoints();

  void GenerateAll();

protected:

  virtual void BeginPlay() override;

  /// Determine the height where the spawners will be placed, relative to each
  /// RoutePlanner
  UPROPERTY(Category = "Spawners", EditAnywhere)
  float SpawnersHeight = 300.f;

  UPROPERTY(Category = "Spawners", EditAnywhere)
  TArray<TObjectPtr<AVehicleSpawnPoint>> VehicleSpawners;

  UPROPERTY(EditAnywhere)
  FString OpenDriveData;

  UPROPERTY(EditAnywhere)
  TArray<TObjectPtr<AActor>> ActorMeshList;

  /// Material applied to driving-lane mesh sections.
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> RoadMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Rural_A.MI_Road_Rural_A")));

  /// Material applied to sidewalk mesh sections.
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> SidewalkMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Sidewalk/MI_Sidewalk_Apartment.MI_Sidewalk_Apartment")));

  /// Material applied to the crosswalk mesh. Content/Carla does not ship a
  /// dedicated flat "zebra crossing" ground material yet, so this defaults
  /// to a light lane-marking asphalt material as a visible placeholder.
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> CrosswalkMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Asphalt_B_LaneMarkingWhite.MI_Road_Asphalt_B_LaneMarkingWhite")));

  /// Material applied to white lane-marking meshes.
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> LaneMarkingWhiteMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Rural_A_LaneMarkingWhite.MI_Road_Rural_A_LaneMarkingWhite")));

  /// Material applied to yellow lane-marking meshes (centre line).
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> LaneMarkingYellowMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Rural_A_LaneMarkingYellow.MI_Road_Rural_A_LaneMarkingYellow")));

  /// Vertical offset (cm) applied to crosswalk and lane-marking meshes above
  /// the road surface, to avoid z-fighting with the road mesh section.
  UPROPERTY(Category = "Materials", EditAnywhere)
  float DecalZOffset = 1.5f;

  /// Drives the runtime PCG scatter (see GeneratePoles). Non-partitioned by
  /// default (bIsComponentPartitioned = false on UPCGComponent), so this
  /// works on legacy non-World-Partition levels like OpenDriveMap.umap --
  /// no PCGWorldActor is required. Generation is triggered explicitly by
  /// calling PCGComponent->Generate() once GeneratePoles() has spawned all
  /// the anchor markers; the graph asset's own GenerationTrigger should be
  /// left at "Generate On Demand" (not "Generate On Load" or "Generate At
  /// Runtime", which assume a World-Partition streaming source).
  UPROPERTY(Category = "Street Furniture", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TObjectPtr<UPCGComponent> PCGComponent;

  /// Provides the PCG component's generation bounds: PCG derives its grid
  /// bounds from the owner actor's component bounds, and this actor has no
  /// visible geometry of its own -- without a bounded component the graph
  /// aborts at Generate() with "Component has invalid bounds, not registered
  /// nor updated" and never schedules a task. Sized to the generated road
  /// network (anchor bounding box, padded) right before the scatter runs.
  UPROPERTY(Category = "Street Furniture", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TObjectPtr<UBoxComponent> PCGBoundsComponent;

  /// Accumulated bounding box of every furniture anchor spawned for the
  /// current generation, in world space; drives PCGBoundsComponent.
  FBox FurnitureAnchorBounds = FBox(ForceInit);

  /// PCG graph asset that samples the tagged anchors below (via a "Get
  /// Actor Data" node per category, ActorSelector = {ByTag, AllWorldActors,
  /// <tag>, bSelectMultiple=true}, Mode = GetSinglePoint) and spawns lamp,
  /// vegetation, and signage actors/meshes at each point. Left unset by
  /// default -- author this graph in-editor (see design doc) and assign it
  /// on the placed AOpenDriveGenerator instance in OpenDriveMap.umap.
  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  TSoftObjectPtr<UPCGGraphInterface> StreetFurnitureGraph;

  /// Tag used to mark lamp-pole anchor points for the PCG graph to sample.
  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  FName LampAnchorTag = FName("PCG_StreetLampAnchor");

  /// Spacing (m) between lamp anchors along each road.
  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  float LampAnchorSpacing = 30.0f;

  /// Lateral offset (m) of lamp anchors from the outer edge of the
  /// outermost driving lane -- calibrated to clear a typical CARLA
  /// sidewalk (OpenDriveToMap's tree placement uses 3m for the same
  /// purpose; lamps use a little more so the pole base sits past the
  /// sidewalk rather than on it).
  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  float LampAnchorOffset = 4.0f;

  /// Tag used to mark roadside-vegetation anchor points.
  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  FName VegetationAnchorTag = FName("PCG_VegetationAnchor");

  /// Spacing (m) between vegetation anchors along each road. Denser than
  /// lamps by default; the PCG graph can layer jitter/density-filter nodes
  /// on top of these seed points for a less regular look.
  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  float VegetationAnchorSpacing = 15.0f;

  /// Lateral offset (m) of vegetation anchors -- set past the lamp row so
  /// poles and trees don't overlap.
  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  float VegetationAnchorOffset = 6.0f;

  /// Tag used to mark decorative-signage anchor points. Traffic-control
  /// signs (stop/yield/speed limit) already come from OpenDRIVE <signal>
  /// data via the existing traffic-light/-sign pipeline; this is only for
  /// ambient roadside signage PCG can scatter for visual density.
  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  FName SignageAnchorTag = FName("PCG_SignageAnchor");

  /// Spacing (m) between signage anchors along each road.
  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  float SignageAnchorSpacing = 60.0f;

  /// Lateral offset (m) of signage anchors, same row as lamps by default.
  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  float SignageAnchorOffset = 4.0f;

  /// Spawns one AProceduralFurnitureAnchor per transform returned by
  /// Map::GetTreesTransform for the given spacing/offset, tagged for the
  /// PCG graph to pick up. Returns the number of anchors spawned.
  int32 GenerateFurnitureAnchors(const FName &Tag, float Spacing, float Offset);

};
