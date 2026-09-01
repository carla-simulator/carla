// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProceduralMeshComponent.h"
#include "Math/Vector2D.h"
#include "EditorUtilityActor.h"
#include "EditorUtilityObject.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

#include <util/disable-ue4-macros.h>
#include <carla/road/Map.h>
#include <carla/rpc/OpendriveGenerationParameters.h>
#include <util/enable-ue4-macros.h>
#include "TextureResource.h"
#include <optional>
#include "Generation/OpenDriveFileGenerationParameters.h"
#include "Utils/DGTImplementable.h"
#include "OpenDriveToMap.generated.h"

USTRUCT(BlueprintType)
struct FRoadSignInfo
{
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly)
  FString SignId;

  UPROPERTY(BlueprintReadOnly)
  FString RoadId;

  UPROPERTY(BlueprintReadOnly)
  double S;

  UPROPERTY(BlueprintReadOnly)
  double T;

  UPROPERTY(BlueprintReadOnly)
  FString Orientation;

  UPROPERTY(BlueprintReadOnly)
  FTransform Transform;
};

class UProceduralMeshComponent;
class UMeshComponent;
class UCustomFileDownloader;
class UMaterialInstance;
class USplineComponent;

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class CARLATOOLS_API UOpenDriveToMap : public UEditorUtilityObject
{
  GENERATED_BODY()

public:
  UOpenDriveToMap();
  ~UOpenDriveToMap();

  UFUNCTION()
  void ConvertOSMInOpenDrive();

  UFUNCTION( BlueprintCallable )
  void CreateMap();

  UFUNCTION(BlueprintCallable)
  void CreateTerrain(const int NumberOfTerrainX, const int NumberOfTerrainY, const float MeshGridResolution);

  UFUNCTION(BlueprintCallable)
  void CreateTerrainMesh(const int MeshIndex, const FVector2D Offset,  const int TileSizeX, const int TileSizeY, const float GridSectionSize);

  UFUNCTION(BlueprintCallable)
  float GetHeight(float PosX, float PosY, bool bDrivingLane = false);

  UFUNCTION(BlueprintCallable)
  static AActor* SpawnActorWithCheckNoCollisions(UClass* ActorClassToSpawn, FTransform Transform);

  UFUNCTION(BlueprintCallable)
  float GetDistanceToDrivingLaneBorder(FVector Location) const{
    return DistanceToLaneBorder(CarlaMap, Location);
  }

  UFUNCTION(BlueprintCallable)
  bool GetIsInRoad(FVector Location) const {
    return IsInRoad(CarlaMap, Location);
  }

  UFUNCTION(BlueprintCallable)
  void GenerateTileStandalone();

  UFUNCTION(BlueprintCallable)
  void GenerateTile();

  UFUNCTION(BlueprintCallable)
  bool GoNextTile();

  UFUNCTION(BlueprintCallable)
  void ReturnToMainLevel();

  UFUNCTION(BlueprintCallable)
  void CorrectPositionForAllActorsInCurrentTile();

  UFUNCTION(BlueprintCallable)
  FString GetStringForCurrentTile();

  UFUNCTION(BlueprintCallable)
  AActor* SpawnActorInEditorWorld(UClass* Class, FVector Location, FRotator Rotation);

  UFUNCTION(BlueprintCallable)
  static UWorld* GetEditorWorld();

  UFUNCTION(BlueprintCallable)
  static UWorld* GetGameWorld();

  UFUNCTION(BlueprintCallable)
  TArray<FRoadSignInfo> GetAllRoadSignsInfo();

  UFUNCTION(BlueprintCallable, Category = "Assets Placement")
  static void MoveActorsToSubLevelWithLargeMap(TArray<AActor*> Actors, ALargeMapManager* LargeMapManager);

  UFUNCTION(BlueprintCallable, Category = "HoudiniImporterWidget")
  static void UpdateGenericActorCoordinates(AActor* Actor, FVector TileOrigin);

  UFUNCTION(BlueprintCallable, Category = "Assets Placement")
  static void UpdateInstancedMeshCoordinates(
      UHierarchicalInstancedStaticMeshComponent* Component, FVector TileOrigin);

  UFUNCTION(BlueprintCallable, Category = "Assets Placement")
  static FVector DisplaceLocationOutsideNeighboringRoads(const UObject* WorldContextObject, FVector InLocation, float SteppingPercentage = 0.66f, float RoadLimitPadding = 10.0f);


  UFUNCTION(BlueprintCallable)
  void UnloadWorldPartitionRegion(const FBox& RegionBox);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="File")
  FString FilePath;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="File")
  FString MapName;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Settings" )
  FString Url;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Settings" )
  FString LocalFilePath;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
  FVector2D FinalGeoCoordinates;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
  FString PythonBinPath;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
  FVector2D OriginGeoCoordinates;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
  UMaterialInstance* DefaultRoadMaterial;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
  UMaterialInstance* DefaultLaneMarksWhiteMaterial;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
  UMaterialInstance* DefaultLaneMarksYellowMaterial;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
  UMaterialInstance* DefaultSidewalksMaterial;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
  UMaterialInstance* DefaultLandscapeMaterial;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
  FOpenDriveFileGenerationParameters OpenDriveGenParams;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Settings" )
  float DistanceBetweenTrees = 50.0f;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Settings" )
  float DistanceFromRoadEdge = 3.0f;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Stage" )
  bool bHasStarted = false;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Stage" )
  bool bRoadsFinished = false;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Stage" )
  bool bMapLoaded = false;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Mitsuba" )
  bool bUseMitsuba = false;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Digital Twins Tools" )
  TArray<TSubclassOf<class UDGTImplementable>> ToolsClasses;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  FVector MinPosition;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  FVector MaxPosition;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  float TileSize;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  FVector Tile0Offset;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  bool bTileFinished;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  FIntVector NumTilesInXY;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  FIntVector CurrentTilesInXY;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  FString BaseLevelName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
  UTexture2D* DefaultHeightmap;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Heightmap" )
  FVector WorldEndPosition;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Heightmap" )
  FVector WorldOriginPosition;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Heightmap" )
  float MinHeight;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Heightmap" )
  float MaxHeight;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Heightmap" )
  class AStreetMapActor* StreetMapActorReference;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Heightmap" )
  TArray<USplineComponent*> GeneratedSplines;

protected:

  UFUNCTION(BlueprintCallable, Category = "Assets Placement")
  void RenderRoadToTexture(FVector MinLocation, FVector MaxLocation);

  UFUNCTION(BlueprintCallable, Category = "Assets Placement")
  void RunPythonScript(FString ScriptPath, FString Args);

  UFUNCTION(BlueprintCallable, Category = "Assets Placement")
  void RunPythonMergeTiles();

  UFUNCTION(BlueprintCallable, Category = "Assets Placement")
  void RunPythonRoadEdges();

  UFUNCTION(BlueprintCallable, Category = "Assets Placement")
  void GenerateCurbSplines();

  UFUNCTION(BlueprintCallable, Category = "Assets Placement")
  void GenerateCurbSplinesFromRoadRenders();

  UFUNCTION(BlueprintCallable, Category = "Mitsuba")
  void ExportStaticMeshToOBJ(UStaticMesh *StaticMesh, const FString &OutputPath);

  UFUNCTION(BlueprintCallable, Category = "Mitsuba")
  void MergeRoads();

  UFUNCTION(BlueprintCallable, Category = "Mitsuba")
  void RunPythonRoadSegmentation();

  UFUNCTION(BlueprintCallable, Category = "Mitsuba")
  void RunPythonMitsubaOptimization();

  UFUNCTION(BlueprintCallable, Category = "Mitsuba")
  void MitsubaMeshOptimization();

  UFUNCTION(BlueprintCallable)
  TArray<AActor*> GenerateMiscActors(float Offset, FVector MinLocation, FVector MaxLocation );

  UFUNCTION( BlueprintImplementableEvent )
  void GenerationFinished(FVector MinLocation, FVector MaxLocation);

  UFUNCTION( BlueprintImplementableEvent )
  void DownloadFinished();

  UFUNCTION( BlueprintImplementableEvent )
  void InitializeBuildingsInBP();

  UFUNCTION( BlueprintImplementableEvent )
  void ExecuteTileCommandlet();

  UFUNCTION( BlueprintCallable )
  void MoveActorsToSubLevels(TArray<AActor*> ActorsToMove);


private:

  UFUNCTION()
  void OpenFileDialog();

  UFUNCTION(BlueprintCallable)
  void LoadMap();

  void GenerateAll(const std::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation);
  void GenerateRoadMesh(const std::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation);
  // void GenerateSpawnPoints(const carla::road::Map& ParamCarlaMap, FVector MinLocation, FVector MaxLocation);
  void GenerateTreePositions(const std::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation);
  void GenerateLaneMarks(const std::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation);
  void GenerateTrafficLights(const std::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation);
  bool TryResolveContentFileAnywhere(const FString& RelativePathUnderContent, FString& OutAbsPath);


  FTransform GetSnappedPosition(FTransform Origin);

  float GetHeightForLandscape(FVector Origin);

  float DistanceToLaneBorder(
      const std::optional<carla::road::Map>& CarlaMap,
      FVector &location,
      int32_t lane_type = static_cast<int32_t>(carla::road::Lane::LaneType::Driving)) const;

  bool IsInRoad(
      const std::optional<carla::road::Map>& ParamCarlaMap,
      FVector &location) const;

  void InitTextureData();

  void ImportXODR();
  void ImportOSM();

  carla::rpc::OpendriveGenerationParameters opg_parameters;

  std::optional<carla::road::Map> CarlaMap;

  UPROPERTY()
  UCustomFileDownloader* FileDownloader;

  UPROPERTY()
  TArray<AActor*> Landscapes;

  UPROPERTY()
  UTexture2D* Heightmap;

  UPROPERTY()
  TArray<UDGTImplementable*> ToolInstances;

  FSharedImageConstRef HeightmapCopy;
  TArrayView64<const uint16> HeightmapPixels;
  int32 HeightmapWidth = 0;
  int32 HeightmapHeight = 0;
};
