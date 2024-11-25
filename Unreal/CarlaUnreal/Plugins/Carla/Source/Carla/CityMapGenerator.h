// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "MapGen/CityMapMeshHolder.h"
#include "MapGen/DoublyConnectedEdgeList.h"
#include "MapGen/GraphParser.h"

#include <util/ue-header-guard-begin.h>
#include "UObject/ObjectSaveContext.h"
#include <util/ue-header-guard-end.h>

#include "CityMapGenerator.generated.h"

class URoadMap;

/// Generates a random city using the meshes provided.
///
/// @note At this point it only generates roads and sidewalks.
UCLASS(HideCategories=(Input,Rendering,Actor))
class CARLA_API ACityMapGenerator : public ACityMapMeshHolder
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{
public:

  ACityMapGenerator(const FObjectInitializer& ObjectInitializer);

  ~ACityMapGenerator();

  /// @}
  // ===========================================================================
  /// @name Overriden from UObject
  // ===========================================================================
  /// @{
public:

  virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;

  /// @}
  // ===========================================================================
  /// @name Overriden from ACityMapMeshHolder
  // ===========================================================================
  /// @{
private:

  virtual void UpdateMap() override;

  /// @}
  // ===========================================================================
  /// @name Road map
  // ===========================================================================
  /// @{
public:

  UFUNCTION(BlueprintCallable)
  URoadMap *GetRoadMap()
  {
    return RoadMap;
  }

  /// @}
  // ===========================================================================
  /// @name Map construction and update related methods
  // ===========================================================================
  /// @{
private:

  /// Update the random seeds. Generate random if no fixed seed is used.
  void UpdateSeeds();

  /// Regenerate the DCEL.
  void GenerateGraph();

  /// Add the road meshes to the scene based on the current DCEL.
  void GenerateRoads();

  /// Generate the road map image and save to disk if requested.
  void GenerateRoadMap();

  /// @}
  // ===========================================================================
  /// @name Map generation properties
  // ===========================================================================
  /// @{
private:

  /** Size X of the map in map units. The map unit is calculated based in the
    * tile mesh of the road (see Map Scale).
    */
  UPROPERTY(Category = "Map Generation", EditAnywhere, meta = (ClampMin = "10", ClampMax = "200"))
  uint32 MapSizeX = 20u;

  /** Size Y of the map in map units. The map unit is calculated based in the
    * tile mesh of the road (see Map Scale).
    */
  UPROPERTY(Category = "Map Generation", EditAnywhere, meta = (ClampMin = "10", ClampMax = "200"))
  uint32 MapSizeY = 20u;

  /** If false, no mesh is added, only the internal representation of road is
    * generated.
    */
  UPROPERTY(Category = "Map Generation", EditAnywhere)
  bool bGenerateRoads = true;

  /** If false, a random seed is generated each time. */
  UPROPERTY(Category = "Map Generation", EditAnywhere)
  bool bUseFixedSeed = true;

  /** Seed of the random map generated. */
  UPROPERTY(Category = "Map Generation", EditAnywhere, meta = (EditCondition = bUseFixedSeed))
  int32 Seed = 123456789;

  /// @}
  // ===========================================================================
  /// @name Road Map
  // ===========================================================================
  /// @{
private:

  /** Trigger the generation a the road map image of the current layout (used
    * for off-road and opposite lane invasion detection).
    */
  UPROPERTY(Category = "Road Map", EditAnywhere)
  bool bTriggerRoadMapGeneration = false;

  /** The resolution in pixels per map unit of the road map. The map unit is
    * calculated based in the tile mesh of the road (see Map Scale).
    */
  UPROPERTY(Category = "Road Map", EditAnywhere, meta = (ClampMin = "1", ClampMax = "500"))
  uint32 PixelsPerMapUnit = 50u;

  /** Whether the road map should be generated based on left-hand traffic. */
  UPROPERTY(Category = "Road Map", EditAnywhere)
  bool bLeftHandTraffic = false;

  /** If true, the road map encoded as an image is saved to disk. The image is
    * saved to the "Saved" folder of the project.
    */
  UPROPERTY(Category = "Road Map", EditAnywhere)
  bool bSaveRoadMapToDisk = true;

  /** If true, a debug point is drawn in the level for each pixel of the road
    * map.
    */
  UPROPERTY(Category = "Road Map", EditAnywhere)
  bool bDrawDebugPixelsToLevel = false;

  /** The road map is re-computed on save so we always store an up-to-date
    * version. Uncheck this only for testing purposes as the road map might get
    * out-of-sync with the current road layout.
    */
  UPROPERTY(Category = "Road Map", EditAnywhere, AdvancedDisplay)
  bool bGenerateRoadMapOnSave = true;

  /** If true, activate the custom depth pass of each tagged actor in the level.
    * This pass is necessary for rendering the semantic segmentation. However,
    * it may add a performance penalty since occlusion doesn't seem to be
    * applied to objects having this value active.
    */
  UPROPERTY(Category = "Road Map", EditAnywhere, AdvancedDisplay)
  bool bTagForSemanticSegmentation = false;

  UPROPERTY()
  URoadMap *RoadMap;

  /// @}
  // ===========================================================================
  /// @name Other private members
  // ===========================================================================
  /// @{
private:

  TUniquePtr<MapGen::DoublyConnectedEdgeList> Dcel;

  TUniquePtr<MapGen::GraphParser> DcelParser;
  /// @}
};
