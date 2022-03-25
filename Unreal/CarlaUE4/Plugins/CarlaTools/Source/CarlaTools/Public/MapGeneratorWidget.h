// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once 

#include "CoreMinimal.h"

#include "EditorUtilityWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ProceduralFoliageSpawner.h"
#include "UnrealString.h"

#include "MapGeneratorWidget.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCarlaToolsMapGenerator, Log, All);

/// Struct used as container of basic map information
USTRUCT(BlueprintType)
struct CARLATOOLS_API FMapGeneratorMetaInfo
{
  GENERATED_USTRUCT_BODY();

  UPROPERTY(BlueprintReadWrite)
  FString DestinationPath;

  UPROPERTY(BlueprintReadWrite)
  FString MapName;

  UPROPERTY(BlueprintReadWrite)   
  int SizeX;

  UPROPERTY(BlueprintReadWrite)
  int SizeY;

  UPROPERTY(BlueprintReadWrite)
  TArray<UProceduralFoliageSpawner*> FoliageSpawners;
};

/// Struct used as container of basic tile information
USTRUCT(BlueprintType)
struct CARLATOOLS_API FMapGeneratorTileMetaInfo
{
  GENERATED_USTRUCT_BODY();

  UPROPERTY(BlueprintReadWrite)   
  bool bIsTiled = true;

  UPROPERTY(BlueprintReadWrite)   
  int IndexX;

  UPROPERTY(BlueprintReadWrite)
  int IndexY;
};

/// Class UMapGeneratorWidget extends the functionality of UEditorUtilityWidget
/// to be able to generate and manage maps and largemaps tiles for procedural
/// map generation
UCLASS(BlueprintType)
class CARLATOOLS_API UMapGeneratorWidget : public UEditorUtilityWidget
{
  GENERATED_BODY()

public:
  /// This function invokes a blueprint event defined in widget blueprint 
  /// event graph, which sets a heightmap to the @a Landscape using
  /// ALandscapeProxy::LandscapeImportHeightMapFromRenderTarget(...)
  /// function, which is not exposed to be used in C++ code, only blueprints
  /// @a metaTileInfo contains some useful info to execute this function
  UFUNCTION(BlueprintImplementableEvent)
  void AssignLandscapeHeightMap(ALandscape* Landscape, FMapGeneratorTileMetaInfo TileMetaInfo);

  /// Function called by Widget Blueprint which generates all tiles of map
  /// @a mapName, and saves them in @a destinationPath
  /// Returns a void string is success and an error message if the process failed
  UFUNCTION(Category="Map Generator",BlueprintCallable)
  void GenerateMapFiles(const FMapGeneratorMetaInfo& MetaInfo);

  /// Function called by Widget Blueprint used to start the whole vegetation
  /// process for map defined in @a MetaInfo
  UFUNCTION(Category="Map Generator",BlueprintCallable)
  void CookVegetation(const FMapGeneratorMetaInfo& MetaInfo);

  /// Function invoked by the widget that cooks the vegetation defined in
  /// @a FoliageSpawners only in the world opened in the editor
  UFUNCTION(Category="Map Generator", BlueprintCallable)
  void CookVegetationToCurrentTile(const TArray<UProceduralFoliageSpawner*> FoliageSpawners);

  /// Utils funtion to format @a InDirectory so it gets sanitized in a 
  /// format that unreal can access the directory, deleting unnecesary 
  /// characters such as final '/' or '\'
  UFUNCTION(Category="Map Generator", BlueprintCallable)
  FString SanitizeDirectory(FString InDirectory);

private:  
  /// Loads the base tile map and stores it in @a WorldAssetData
  /// The funtions return true is success, otherwise false
  UFUNCTION()
  bool LoadBaseTileWorld(FAssetData& WorldAssetData);

  /// Loads the base large map and stores it in @a WorldAssetData
  /// The funtions return true is success, otherwise false
  UFUNCTION()
  bool LoadBaseLargeMapWorld(FAssetData& WorldAssetData);

  /// Loads a UWorld object from @a BaseMapPath and returns 
  /// it in @a WorldAssetData
  /// The funtion returns true is success, otherwise false
  UFUNCTION()
  bool LoadWorld(FAssetData& WorldAssetData, const FString& BaseMapPath);

  /// Loads a bunch of world objects located in @a BaseMapPath and 
  /// returns them in @a WorldAssetsData.
  /// The function returns true if success, otherwise false
  UFUNCTION()
  bool LoadWorlds(TArray<FAssetData>& WorldAssetsData, const FString& BaseMapPath);

  /// Saves a world contained in @a WorldToBeSaved, in the path defined in @a DestinationPath
  /// named as @a WorldName, as a package .umap
  UFUNCTION()
  bool SaveWorld(FAssetData& WorldToBeSaved, const FString& DestinationPath, 
      const FString& WorldName, bool bCheckFileExists = false);

  /// Takes the name of the map from @a MetaInfo and created the main map
  /// including all the actors needed by large map system
  UFUNCTION()
  bool CreateMainLargeMap(const FMapGeneratorMetaInfo& MetaInfo);

  /// Takes @a MetaInfo as input and generates all tiles based on the
  /// dimensions specified for the map
  /// The funtions return true is success, otherwise false
  UFUNCTION()
  bool CreateTilesMaps(const FMapGeneratorMetaInfo& MetaInfo);

  /// Searches for the specified map in the specified path in @a MetaInfo
  /// and starts the vegetation cooking process for each of the tile.
  /// IMPORTANT: Only maps with '_Tile_' tag in it name are processed as
  /// vegetation is only applied to tiles.
  /// The function returns true if success, otherwise false
  UFUNCTION()
  bool CookVegetationToTiles(const FMapGeneratorMetaInfo& MetaInfo);

  /// Gets the Landscape from the input world @a WorldAssetData and
  /// applies the heightmap to it. The tile index is indexX and indexY in
  /// @a TileMetaInfo argument
  /// The funtions return true is success, otherwise false
  UFUNCTION()
  bool ApplyHeightMapToLandscape(FAssetData& WorldAssetData, FMapGeneratorTileMetaInfo TileMetaInfo);

  /// Instantiate a procedural foliage volume for each element in @a FoliageSpawners
  /// and cooks the corresponding vegetation to @a World
  /// Return true if success, false otherwise
  UFUNCTION()
  bool CookVegetationToWorld(UWorld* World, const TArray<UProceduralFoliageSpawner*> FoliageSpawners);

  /// Returns the world object in @a WorldAssetData
  UFUNCTION()
  UWorld* GetWorldFromAssetData(FAssetData& WorldAssetData);
};