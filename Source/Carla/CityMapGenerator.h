// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "MapGen/CityMapMeshHolder.h"
#include "MapGen/DoublyConnectedEdgeList.h"
#include "MapGen/GraphParser.h"
#include "CityMapGenerator.generated.h"

class URoadMap;

/// Generates a random city using the meshes provided.
///
/// @note At this point it only generates roads and sidewalks.
UCLASS(HideCategories=(Input))
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
  /// @name Map construction and update related methods
  // ===========================================================================
  /// @{
public:

  URoadMap *GetRoadMap()
  {
    return RoadMap;
  }

private:

  virtual void BeginPlay() override;

  /// Update the map based on the current settings.
  virtual void UpdateMap() override;

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

  UPROPERTY(Category = "Map Generation", EditAnywhere, meta = (ClampMin = "10", ClampMax = "200"))
  uint32 MapSizeX = 20u;

  UPROPERTY(Category = "Map Generation", EditAnywhere, meta = (ClampMin = "10", ClampMax = "200"))
  uint32 MapSizeY = 20u;

  UPROPERTY(Category = "Map Generation", EditAnywhere)
  bool bGenerateRoads = true;

  UPROPERTY(Category = "Map Generation", EditAnywhere)
  bool bUseFixedSeed = true;

  UPROPERTY(Category = "Map Generation", EditAnywhere, meta = (EditCondition = bUseFixedSeed))
  int32 Seed = 123456789;

  /// @}
  // ===========================================================================
  /// @name Map generation properties - advance display
  // ===========================================================================
  /// @{
private:

  UPROPERTY(Category = "Map Generation", EditAnywhere, AdvancedDisplay, meta = (EditCondition = bUseFixedSeed))
  bool bUseMultipleFixedSeeds = false;

  UPROPERTY(Category = "Map Generation", EditAnywhere, AdvancedDisplay, meta = (EditCondition = bUseMultipleFixedSeeds))
  int32 RoadPlanningSeed;

  /// @}
  // ===========================================================================
  /// @name Road Map
  // ===========================================================================
  /// @{
private:

  UPROPERTY(Category = "Road Map", EditAnywhere)
  bool bGenerateRoadMap = false;

  UPROPERTY(Category = "Road Map", EditAnywhere, meta = (ClampMin = "1", ClampMax = "500", EditCondition = bGenerateRoadMap))
  uint32 PixelsPerMapUnit = 50u;

  UPROPERTY(Category = "Road Map", EditAnywhere, meta = (EditCondition = bGenerateRoadMap))
  bool bSaveRoadMapToDisk = true;

  UPROPERTY(Category = "Road Map", VisibleAnywhere, AdvancedDisplay)
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
