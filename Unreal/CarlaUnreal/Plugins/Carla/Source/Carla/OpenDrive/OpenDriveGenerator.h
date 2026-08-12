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
class CARLA_API ACarlaProceduralMeshActor : public AActor
{
  GENERATED_BODY()
public:
  ACarlaProceduralMeshActor();

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

  /// Generates a terrain-following heightfield beneath the generated road
  /// network so gaps in the road/sidewalk mesh (roundabout centers, area
  /// beyond the shoulders, etc.) don't show blue sky through the ground.
  /// The terrain surface is a multilevel B-spline approximation (Lee,
  /// Wolberg & Shin 1997) fit to the road height samples in
  /// GroundHeightSampleGrid: a coarse-to-fine hierarchy of uniform cubic
  /// B-spline lattices, each level fit to the residual left by the previous
  /// one, giving a C2-smooth surface that follows the road network's
  /// vertical curvature instead of a piecewise-linear patchwork that pokes
  /// through sag curves and drops away under crests. Near roads the surface
  /// is additionally clamped to GroundPlaneZOffset below the lowest nearby
  /// road sample so smoothing can never push terrain up through the road.
  /// Must run after GenerateRoadMesh(), which is what populates
  /// RoadMeshBounds and GroundHeightSampleGrid.
  void GenerateGroundPlane();

  /// Fills the narrow unmapped strips between parallel carriageway roads
  /// with road-material surface. Real-world OpenDRIVE exports (NuRec
  /// DeepMap) model each direction of a dual carriageway as a separate
  /// road and leave the median/gore strip between them unmapped -- bare
  /// terrain showed through at road level and read as broken road
  /// surface. Queries RoadRaster for terrain cells trapped between
  /// driving surfaces on opposite sides within MedianFillMaxWidth whose
  /// heights agree (rejects gaps under overpasses). Must run after
  /// GenerateRoadMesh.
  void GenerateMedianFill();

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

  /// World-space bounding box of every road/sidewalk vertex generated by
  /// GenerateRoadMesh, accumulated there and consumed by
  /// GenerateGroundPlane to size and position the fill quad -- mirrors how
  /// FurnitureAnchorBounds drives PCGBoundsComponent below.
  FBox RoadMeshBounds = FBox(ForceInit);

  /// The authoritative "what is road surface, where, at what height"
  /// tool the rest of generation consumes. GenerateRoadMesh rasterizes
  /// every road/sidewalk *triangle* (vertices alone leave unconstrained
  /// holes inside lanes -- the mesh is dense along s but a full lane wide
  /// laterally) into a regular XY grid with barycentric-interpolated
  /// surface heights, then a chamfer distance transform gives every cell
  /// its distance to the driving footprint and to any pavement, plus the
  /// height of the nearest pavement. Terrain generation, furniture
  /// placement and the generation QA all query this one structure instead
  /// of re-deriving geometry their own (subtly different) ways.
  struct FRoadSurfaceRaster
  {
    struct FCell
    {
      float DriveMinZ = 0.0f;
      float DriveMaxZ = 0.0f;
      float PavedMinZ = 0.0f;      // min over driving + sidewalk surfaces
      float DistToDrive = -1.0f;   // cm to the driving footprint, 0 inside
      float DistToPaved = -1.0f;   // cm to any pavement, 0 inside
      float NearestPavedMinZ = 0.0f;
      /// Lowest median-fill / under-road-blanket quad emitted in this cell
      /// (FLT_MAX if none). Written by GenerateMedianFill, which runs
      /// before GenerateGroundPlane; the terrain clamps itself below these
      /// quads so it cannot z-fight up through the fill.
      float FillMinZ = FLT_MAX;
      uint8 bDrive = 0;
      uint8 bPaved = 0;
      /// Like bDrive but WITHOUT the rasterizer's negative barycentric
      /// tolerance: set only when the cell center is genuinely inside a
      /// driving triangle. bDrive's tolerance lets grazing triangles claim
      /// cells whose surface is mostly uncovered; gap-filling must use the
      /// strict flag or those cells read as "road" and never get filled.
      uint8 bDriveCovered = 0;
    };

    float CellSize = 250.0f;
    float OriginX = 0.0f;
    float OriginY = 0.0f;
    int32 Width = 0;
    int32 Height = 0;
    TArray<FCell> Cells;

    bool IsValid() const { return Width > 0 && Height > 0; }
    void Initialize(const FBox &Bounds, float InCellSize);
    /// Splats one triangle of road ("drive") or sidewalk surface into the
    /// grid: conservative 2D rasterization, per-cell-center barycentric z.
    void RasterizeTriangle(const FVector &A, const FVector &B, const FVector &C, bool bIsDrive);
    /// Two-pass chamfer distance transform filling DistToDrive/DistToPaved
    /// and NearestPavedMinZ. Call once after all triangles are in.
    void BuildDistanceFields();
    const FCell *CellAtWorld(float X, float Y) const;
  };

public:
  /// Debug: log the raster's weld/height record in a 3x3 cell block around
  /// (X, Y) in UE cm. Bound to the `carla.DumpRasterAt` console command.
  void DumpRasterAt(float X, float Y) const;

protected:
  FRoadSurfaceRaster RoadRaster;

  /// Material applied to driving-lane mesh sections.
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> RoadMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_RoadAsphalt_Town15.MI_RoadAsphalt_Town15")));

  /// Material applied to sidewalk mesh sections.
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> SidewalkMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Sidewalk/MI_Sidewalk_Apartment.MI_Sidewalk_Apartment")));

  /// Material applied to the ground plane (see GenerateGroundPlane). Picked
  /// from Content/Carla/Static/GenericMaterials/Ground -- a plain
  /// M_GeneralMaster instance (not a landscape auto-material, which expects
  /// per-vertex layer weightmaps this flat quad doesn't have) authored for
  /// large tiled areas. Alternates in the same folder: MI_Grass_Cutted_Yard
  /// (more manicured), MI_Dirt, MI_LargeLandscape_Rural/_Highway/_Mountain
  /// (biome variants) -- swap per-map if grass doesn't fit.
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> GroundMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Ground/MI_LargeLandscape_Grass.MI_LargeLandscape_Grass")));

  /// Material applied to the crosswalk mesh. Uses the zebra-stripe texture
  /// set authored for the speed-bump crosswalk mesh (SM_Crosswalk_Bump) --
  /// it's a real painted-crosswalk diffuse/normal/ORM set on a standard
  /// (non-decal) material instance, so it applies cleanly here even though
  /// the diffuse was baked with the speed bump's raised shading in mind.
  /// Alternate: RoadLine/DissappearingRoadlines/DisRoadlinesCrosswalk is a
  /// decal-domain material (drives a scrolling/dissolve effect via a
  /// MaterialParameterCollection) and is not a drop-in base-color swap.
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> CrosswalkMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/Static/Materials/Crosswalk_SpeedBump/MI_Crosswalk_SpeedBump.MI_Crosswalk_SpeedBump")));

  /// Material applied to white lane-marking meshes. The Rural_A marking
  /// variants carry a heavy paint-damage mask keyed to the road atlas UV
  /// layout; on the generated markings' along-road UVs the mask samples as
  /// dense black blotches across the line, so use the clean Asphalt_B
  /// variants instead.
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> LaneMarkingWhiteMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Asphalt_B_LaneMarkingWhite.MI_Road_Asphalt_B_LaneMarkingWhite")));

  /// Material applied to yellow lane-marking meshes (centre line).
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> LaneMarkingYellowMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Asphalt_B_LaneMarkingYellow.MI_Road_Asphalt_B_LaneMarkingYellow")));

  /// Vertical offset (cm) applied to crosswalk and lane-marking meshes above
  /// the road surface, to avoid z-fighting with the road mesh section.
  UPROPERTY(Category = "Materials", EditAnywhere)
  float DecalZOffset = 1.5f;

  /// Vertical offset (cm) applied *below* the road surface height where
  /// the terrain runs under road geometry -- large enough to avoid
  /// z-fighting, small enough that the road visibly rests on the ground
  /// (the generation QA requires terrain within 10cm of the road
  /// underside, see RunGenerationQA).
  UPROPERTY(Category = "Materials", EditAnywhere)
  float GroundPlaneZOffset = 5.0f;

  /// Widest gap (cm) between opposing driving surfaces that
  /// GenerateMedianFill closes with road surface; wider separations are
  /// genuine verges and stay terrain.
  UPROPERTY(Category = "Materials", EditAnywhere)
  float MedianFillMaxWidth = 600.0f;

  /// Max height split (cm) of the driving surface samples WITHIN a single
  /// raster cell near a gap cell for it to be filled -- a gap under an
  /// overpass sees
  /// both decks and must not be bridged at the wrong height.
  UPROPERTY(Category = "Materials", EditAnywhere)
  float MedianFillMaxHeightDelta = 100.0f;

  /// Width (cm) of the blend band past the road footprint over which the
  /// terrain transitions from exactly hugging the road underside to the
  /// free B-spline surface. Within the band the surface is also clamped
  /// below the nearest road sample, so the transition can never rise
  /// through a road.
  UPROPERTY(Category = "Materials", EditAnywhere)
  float GroundSkirtWidth = 1000.0f;

  /// XY cell size (cm) of RoadRaster. 2.5m resolves both the vertical
  /// curvature of ramps/crests and lane-scale footprint detail.
  UPROPERTY(Category = "Materials", EditAnywhere)
  float GroundHeightSampleCellSize = 250.0f;

  /// Per-category clearance (cm) beyond the lane half-width that street
  /// furniture must keep from the nearest driving-lane centerline; anchors
  /// closer than this are discarded so scattered objects can't stand on or
  /// overhang the roadway where lanes widen, merge or overlap. Sized to
  /// the spawned content's bounding-box half-extent (an anchor is a point;
  /// the bus shelter it spawns is ~8m long), not a nominal margin.
  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  float LampRoadClearance = 150.0f;

  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  float VegetationRoadClearance = 250.0f;

  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  float SignageRoadClearance = 450.0f;

  /// Evaluated ground heightfield (final terrain surface z per grid vertex,
  /// row-major, GroundGridNumY rows of GroundGridNumX), retained after
  /// GenerateGroundPlane so GenerateFurnitureAnchors can re-ground
  /// scattered objects onto the terrain they'll actually stand on.
  TArray<float> GroundGridHeights;
  int32 GroundGridNumX = 0;
  int32 GroundGridNumY = 0;
  FVector2D GroundGridOrigin = FVector2D::ZeroVector;
  float GroundGridStepX = 0.0f;
  float GroundGridStepY = 0.0f;

  /// Bilinear interpolation of GroundGridHeights at a world XY position.
  /// Only valid once GenerateGroundPlane has run (GroundGridNumX > 0).
  float SampleGroundGridHeight(float X, float Y) const;

  /// How far (cm) the ground plane extends past the generated road/sidewalk
  /// bounding box on each side, so the fill doesn't end exactly at the
  /// outermost shoulder.
  UPROPERTY(Category = "Materials", EditAnywhere)
  float GroundPlanePadding = 5000.0f;

  /// World-space size (cm) of one UV tile on the ground plane, i.e. how
  /// large the material's texture repeats on the ground -- the plane spans
  /// the whole map, so a single 0-1 UV tile would stretch the texture to
  /// the point of being unrecognizable.
  UPROPERTY(Category = "Materials", EditAnywhere)
  float GroundUVTileSize = 500.0f;

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

  /// Spacing (m) between signage anchors along each road. Sparse: this row
  /// spawns full bus-stop shelters, and a shelter every 60m read as a
  /// repeating wall of street furniture.
  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  float SignageAnchorSpacing = 150.0f;

  /// Lateral offset (m) of signage anchors, same row as lamps by default.
  UPROPERTY(Category = "Street Furniture", EditAnywhere)
  float SignageAnchorOffset = 4.0f;

  /// Spawns one AProceduralFurnitureAnchor per transform returned by
  /// Map::GetTreesTransform for the given spacing/offset, tagged for the
  /// PCG graph to pick up, re-grounded onto pavement/terrain and filtered
  /// against driving lanes with the given clearance (cm) past the lane
  /// half-width. Returns the number of anchors spawned.
  int32 GenerateFurnitureAnchors(const FName &Tag, float Spacing, float Offset, float RoadClearance);

public:

  /// Systematic post-generation validation, run automatically shortly
  /// after GenerateAll (delayed so the async PCG scatter has spawned its
  /// content) and re-runnable on demand via the console command
  /// "carla.MapGenQA". Checks, exhaustively rather than by spot samples:
  ///   A) road support -- every road/sidewalk sample cell must have
  ///      terrain 0..QASupportGapMax below its lowest surface (floating
  ///      road if the gap is larger, terrain invasion if negative);
  ///      stacked-road cells (MaxZ - MinZ > QAStackedRoadGap) only check
  ///      the lower deck, the upper one is a legitimate bridge.
  ///   B) object placement -- the bounding-box footprint of every static
  ///      primitive not generated by this actor (street furniture from
  ///      the PCG scatter, incl. per-instance foliage ISMs) must stay
  ///      clear of every driving lane's corridor.
  /// Logs a summary and writes the full violation list with world
  /// locations to Saved/mapgen_qa.json for external tooling.
  void RunGenerationQA();

protected:

  /// Max allowed gap (cm) between a road cell's lowest surface and the
  /// terrain below it before the cell counts as a floating road. Must stay
  /// above the ground plane's near-pavement clearance (3x GroundPlaneZOffset
  /// = 15cm) plus the fill-quad clamp's reach (fill sits 1-3cm under the
  /// lowest neighbouring lane, terrain 10cm under the fill, so a graded
  /// cell's own surface can be ~grade x 2.5m + 13cm above the terrain).
  UPROPERTY(Category = "Generation QA", EditAnywhere)
  float QASupportGapMax = 30.0f;

  /// Min z spread (cm) within one sample cell that marks it as stacked
  /// roads (bridge over road) rather than a single surface.
  UPROPERTY(Category = "Generation QA", EditAnywhere)
  float QAStackedRoadGap = 400.0f;

};
