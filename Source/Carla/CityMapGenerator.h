// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "MapGen/CityMapMeshHolder.h"
#include "MapGen/DoublyConnectedEdgeList.h"
#include "MapGen/GraphParser.h"
#include "CityMapGenerator.generated.h"

UCLASS(HideCategories=(Rendering, Input))
class CARLA_API ACityMapGenerator : public ACityMapMeshHolder
{
  GENERATED_BODY()

public:

  ACityMapGenerator(const FObjectInitializer& ObjectInitializer);

  ~ACityMapGenerator();

private:

#if WITH_EDITOR
  /// Called after a property change in editor.
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

  /// Update the map based on the current settings.
  void UpdateMap();

  /// Update the random seeds. Generate random if no fixed seed is used.
  void UpdateSeeds();

  /// Regenerate the DCEL.
  void GenerateGraph();

  /// Add the road meshes to the scene based on the current DCEL.
  void GenerateRoads();

  /// @name Map Generation
  /// @{
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

  /// @name Map Generation - Advance Display
  /// @{
  UPROPERTY(Category = "Map Generation", EditAnywhere, AdvancedDisplay, meta = (EditCondition = bUseFixedSeed))
  bool bUseMultipleFixedSeeds = false;

  UPROPERTY(Category = "Map Generation", EditAnywhere, AdvancedDisplay, meta = (EditCondition = bUseMultipleFixedSeeds))
  int32 RoadPlanningSeed;

  UPROPERTY(Category = "Map Generation", EditAnywhere, AdvancedDisplay, meta = (EditCondition = bUseMultipleFixedSeeds))
  int32 BuildingGenerationSeed;
  /// @}

  /// @name Other private members
  /// @{
  TUniquePtr<MapGen::DoublyConnectedEdgeList> Dcel;

  TUniquePtr<MapGen::GraphParser> DcelParser;
  /// @}
};
