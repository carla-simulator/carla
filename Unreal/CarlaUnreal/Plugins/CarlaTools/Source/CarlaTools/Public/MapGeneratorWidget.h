// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once 

#include "RegionOfInterest.h"

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ProceduralFoliageSpawner.h"
#include <Containers/UnrealString.h>
#include <util/ue-header-guard-end.h>

#include "MapGeneratorWidget.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCarlaToolsMapGenerator, Log, All);

struct FSoilTerramechanicsProperties;

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
  int SizeX = 0;

  UPROPERTY(BlueprintReadWrite)
  int SizeY = 0;

  UPROPERTY(BlueprintReadWrite)
  TArray<UProceduralFoliageSpawner*> FoliageSpawners;

  UPROPERTY(BlueprintReadWrite)
  FSoilTerramechanicsProperties GeneralSoilType;

  UPROPERTY(BlueprintReadWrite)
  UTextureRenderTarget2D* GlobalHeightmap = nullptr;

  UPROPERTY(BlueprintReadWrite)
  TMap<FRoiTile, FTerrainROI> TerrainRoisMap;

  UPROPERTY(BlueprintReadWrite)
  TMap<FRoiTile, FVegetationROI> VegetationRoisMap;

  UPROPERTY(BlueprintReadWrite)
  TMap<FRoiTile, FSoilTypeROI> SoilTypeRoisMap;

  UPROPERTY(BlueprintReadWrite)
  TMap<FRoiTile, FMiscSpecificLocationActorsROI> MiscSpecificLocationActorsRoisMap;

  UPROPERTY(BlueprintReadWrite)
  TMap<FRoiTile, FMiscSpreadedActorsROI> MiscSpreadedActorsRoisMap;

  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<AActor> VegetationManagerBpClass;

  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<AActor> TerramechanicsBpClass;

  UPROPERTY(BlueprintReadWrite)
  float RiverChanceFactor = 0.0F;

  UPROPERTY(BlueprintReadWrite)
  float RiverFlateningFactor = 0.0F;
};

/// Struct used as container of basic tile information
USTRUCT(BlueprintType)
struct CARLATOOLS_API FMapGeneratorTileMetaInfo
{
  GENERATED_USTRUCT_BODY();

  UPROPERTY(BlueprintReadWrite)
  FMapGeneratorMetaInfo MapMetaInfo;

  UPROPERTY(BlueprintReadWrite)   
  bool bIsTiled = true;

  UPROPERTY(BlueprintReadWrite)   
  int IndexX = 0;

  UPROPERTY(BlueprintReadWrite)
  int IndexY = 0;

  UPROPERTY(BlueprintReadWrite)
  bool ContainsRiver = false;

  UPROPERTY(BlueprintReadWrite)
  FString RiverPreset;
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FMapGeneratorWidgetState
{
  GENERATED_USTRUCT_BODY();

  // General Fields
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  bool IsPersistentState = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  FString MapName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  FString WorkingPath;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  FString ActiveTabName;

  // Terrain
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainGeneralSize = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainGeneralSlope = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainGeneralHeight = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainGeneralMinHeight = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainGeneralMaxHeight = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainGeneralInvert = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallSeed = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallScale = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallSlope = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallHeight = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallMinHeight = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallMaxHeight = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallInvert = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedSeed = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedScale = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedSlope = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedHeight = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedMinHeight = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedMaxHeight = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedInvert = 0.0F;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  bool bShowDetailedNoise = 0.0F;
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FMapGeneratorPreset
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|TerrainPresets")
  FString PresetName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|TerrainPresets")
  FMapGeneratorWidgetState WidgetState;
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FTileBoundariesInfo
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY()
  TArray<uint16> RightHeightData;

  UPROPERTY()
  TArray<uint16> BottomHeightData;
};

UENUM(BlueprintType)
enum EMapGeneratorToolMode
{
  NEW_MAP,
  MISC_EDITOR
};

/// Class UMapGeneratorWidget extends the functionality of UEditorUtilityWidget
/// to be able to generate and manage maps and largemaps tiles for procedural
/// map generation
UCLASS(BlueprintType)
class CARLATOOLS_API UMapGeneratorWidget : public UEditorUtilityWidget
{
  GENERATED_BODY()

public:
  /// Sets the specified material to @a Landscape
  UFUNCTION(BlueprintImplementableEvent)
  void AssignLandscapeMaterial(ALandscape* Landscape);

  UFUNCTION(BlueprintImplementableEvent)
  void InstantiateRiverSublevel(UWorld* World, const FMapGeneratorTileMetaInfo TileMetaInfo);

  /// PROVISIONAL
  UFUNCTION(BlueprintImplementableEvent)
  void UpdateTileRT(const FMapGeneratorTileMetaInfo& TileMetaInfo);

  UFUNCTION(BlueprintImplementableEvent)
  void UpdateTileRoiRT(const FMapGeneratorTileMetaInfo& TileMetaInfo, UMaterialInstanceDynamic* RoiMeterialInstance);

  UFUNCTION(BlueprintImplementableEvent)
  void UpdateRiverActorSplinesEvent(AActor* RiverActor);

  /// Function called by Widget Blueprint which generates all tiles of map
  /// @a mapName, and saves them in @a destinationPath
  /// Returns a void string is success and an error message if the process failed
  UFUNCTION(Category="Map Generator",BlueprintCallable)
  void GenerateMapFiles(const FMapGeneratorMetaInfo& MetaInfo);

  /// Function called by Widget Blueprint used to start the whole vegetation
  /// process for map defined in @a MetaInfo
  UFUNCTION(Category="Map Generator|Vegetation",BlueprintCallable)
  void CookVegetation(const FMapGeneratorMetaInfo& MetaInfo);

  UFUNCTION(Category="Map Generator|Soil Terramechanics", BlueprintCallable)
  void CookSoilTypeToMaps(const FMapGeneratorMetaInfo& MetaInfo);

  UFUNCTION(Category="Map Generator|Miscellaneous", BlueprintCallable)
  void CookMiscSpreadedInformationToTiles(const FMapGeneratorMetaInfo& MetaInfo);

  UFUNCTION(Category="Map Generator|Miscellaneous", BlueprintCallable)
  void CookMiscSpecificLocationInformationToTiles(const FMapGeneratorMetaInfo& MetaInfo);

  UFUNCTION(Category="Map Generator|Miscellaneous", BlueprintCallable)
  void DeleteAllSpreadedActors(const FMapGeneratorMetaInfo& MetaInfo);

  /// Function invoked by the widget that cooks the vegetation defined in
  /// @a FoliageSpawners only in the world opened in the editor
  UFUNCTION(Category="Map Generator", BlueprintCallable)
  void CookVegetationToCurrentTile(const TArray<UProceduralFoliageSpawner*> FoliageSpawners);

  /// Recalculate the collisions of the loaded map
  UFUNCTION(Category="Map Generator", BlueprintCallable)
  bool RecalculateCollision();

  /// Traverse all tiles of map in @a MetaInfo and recalculates
  /// the collisions of all of them. Can be called from blueprints
  UFUNCTION(Category = "MapGenerator", BlueprintCallable)
  void CookTilesCollisions(const FMapGeneratorMetaInfo& MetaInfo);

  /// Utils funtion to format @a InDirectory so it gets sanitized in a 
  /// format that unreal can access the directory, deleting unnecesary 
  /// characters such as final '/' or '\'
  UFUNCTION(Category="Map Generator", BlueprintCallable)
  FString SanitizeDirectory(FString InDirectory);

  /// Used to find if the @a InDirectory contains a map. If does return true and
  /// @a OutMapSize and @OutFoundMapName stores the information of the found map 
  UFUNCTION(Category="Map Generator", BlueprintCallable)
  bool LoadMapInfoFromPath(FString InDirectory, int& OutMapSize, FString& OutFoundMapName);

  /// Spawns rivers of types @a RiverClass
  UFUNCTION(Category="MapGenerator|Water", BlueprintCallable)
  AActor* GenerateWater(TSubclassOf<class AActor> RiverClass);

  UFUNCTION(Category="MapGenerator|Water", BlueprintCallable)
  bool GenerateWaterFromWorld(UWorld* RiversWorld, TSubclassOf<class AActor> RiverClass);

  UFUNCTION(Category="MapGenerator", BlueprintCallable)
  UWorld* DuplicateWorld(const FString BaseWorldPath, const FString TargetWorldPath, const FString NewWorldName);

  /// Adds weather actor of type @a WeatherActorClass and sets the @a SelectedWeather
  /// to the map specified in @a MetaInfo. Ifthe actor already exists on the map
  /// then it is returned so only one weather actor is spawned in each map
  UFUNCTION(Category="MapGenerator|Weather", BlueprintCallable)
  AActor* AddWeatherToExistingMap(TSubclassOf<class AActor> WeatherActorClass, 
      const FMapGeneratorMetaInfo& MetaInfo, const FString SelectedWeather);

  UFUNCTION(Category="MapGenerator|ROIs", BlueprintCallable)
  TMap<FRoiTile, FVegetationROI> CreateVegetationRoisMap(TArray<FVegetationROI> VegetationRoisArray);

  UFUNCTION(Category="MapGenerator|ROIs", BlueprintCallable)
  TMap<FRoiTile, FTerrainROI> CreateTerrainRoisMap(TArray<FTerrainROI> TerrainRoisArray);

  UFUNCTION(Category="MapGenerator|ROIs", BlueprintCallable)
  TMap<FRoiTile, FMiscSpreadedActorsROI> CreateMiscSpreadedActorsRoisMap(TArray<FMiscSpreadedActorsROI> SpreadedActorsRoisArray);

  UFUNCTION(Category="MapGenerator|ROIs", BlueprintCallable)
  TMap<FRoiTile, FMiscSpecificLocationActorsROI> CreateMiscSpecificLocationActorsRoisMap(TArray<FMiscSpecificLocationActorsROI> SpecificLocationActorsRoisArray);

  UFUNCTION(Category="MapGenerator|ROIs", BlueprintCallable)
  TMap<FRoiTile, FSoilTypeROI> CreateSoilTypeRoisMap(TArray<FSoilTypeROI> SoilTypeRoisArray);

  UFUNCTION(Category="MapGenerator|Vegetation", BlueprintCallable)
  bool DeleteAllVegetationInMap(const FString Path, const FString MapName);

  UFUNCTION(Category="MapGenerator|JsonLibrary", BlueprintCallable)
  bool GenerateWidgetStateFileFromStruct(FMapGeneratorWidgetState WidgetState, const FString JsonPath);

  UFUNCTION(Category="MapGenerator|JsonLibrary", BlueprintCallable)
  FMapGeneratorWidgetState LoadWidgetStateStructFromFile(const FString JsonPath);

  UFUNCTION(Category="MapGenerator|JsonLibrary|Misc", BlueprintCallable)
  bool GenerateMiscStateFileFromStruct(FMiscWidgetState MiscState, const FString JsonPath);

  UFUNCTION(Category="MapGenerator|JsonLibrary|Misc", BlueprintCallable)
  FMiscWidgetState LoadMiscStateStructFromFile(const FString JsonPath);

  UFUNCTION(Category="MapGenerator|TerrainPresets", BlueprintCallable)
  bool GenerateTerrainPresetFileFromStruct(FMapGeneratorPreset Preset, const FString JsonPath);

  UFUNCTION(Category="MapGenerator|TerrainPresets", BlueprintCallable)
  FMapGeneratorPreset LoadTerrainPresetStructFromFile(const FString JsonPath);

private:  
  UPROPERTY()
  TMap<FRoiTile, FTileBoundariesInfo> BoundariesInfo;

  /// Loads a bunch of world objects located in @a BaseMapPath and 
  /// returns them in @a WorldAssetsData.
  /// The function returns true if success, otherwise false
  UFUNCTION()
  bool LoadWorlds(TArray<FAssetData>& WorldAssetsData, const FString& BaseMapPath, bool bRecursive = true);

  /// Saves @a WorldToBeSaved. It also saves all dirty packages to asure
  /// a correct managements of landscapes
  UFUNCTION()
  bool SaveWorld(UWorld* WorldToBeSaved);

  // UFUNCTION()
  // bool SaveWorldPackage

  /// Takes the name of the map from @a MetaInfo and created the main map
  /// including all the actors needed by large map system
  UFUNCTION()
  bool CreateMainLargeMap(const FMapGeneratorMetaInfo& MetaInfo);

  /// Creates an Opendrive file for the created map map 
  /// specified in @a MetaInfo
  UFUNCTION()
  bool CreateOpenDriveFile(const FMapGeneratorMetaInfo& MetaInfo);

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

  /// Instantiate a procedural foliage volume for each element in @a FoliageSpawners
  /// and cooks the corresponding vegetation to @a World
  /// Return true if success, false otherwise
  UFUNCTION()
  bool CookVegetationToWorld(UWorld* World, const TArray<UProceduralFoliageSpawner*> FoliageSpawners);

  UFUNCTION()
  bool CookMiscSpreadedActors(const FMapGeneratorMetaInfo& MetaInfo);

  /// Returns the world object in @a WorldAssetData
  UFUNCTION()
  UWorld* GetWorldFromAssetData(FAssetData& WorldAssetData);

  /// Returns the Z coordinate of the landscape located in @a World at position
  /// @a x and @a y.
  UFUNCTION()
  float GetLandscapeSurfaceHeight(UWorld* World, float x, float y, bool bDrawDebugLines);

  UFUNCTION()
  float GetLandscapeSurfaceHeightFromRayCast(UWorld* World, float x, float y, bool bDrawDebugLines);

  UFUNCTION()
  void ExtractCoordinatedFromMapName(const FString MapName, int& X, int& Y);

  UFUNCTION()
  void SmoothHeightmap(TArray<uint16> HeightData, TArray<uint16>& OutHeightData);

  UFUNCTION()
  void SewUpperAndLeftTiles(TArray<uint16> HeightData, TArray<uint16>& OutHeightData, int IndexX, int IndexY);

  // Converting a 2D coordinate to a 1D coordinate.
  UFUNCTION()
  FORCEINLINE int Convert2DTo1DCoord(int IndexX, int IndexY, int TileSize)
  {
    return (IndexX * TileSize) + IndexY;
  }
};
