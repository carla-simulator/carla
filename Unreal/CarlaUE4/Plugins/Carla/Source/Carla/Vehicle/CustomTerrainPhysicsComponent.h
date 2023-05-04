// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Misc/ScopeLock.h"

#include "Carla/Math/DVector.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "Engine/DataAsset.h"
#include "Async/Future.h"
#ifdef WITH_PYTORCH
THIRD_PARTY_INCLUDES_START
#include <carla/pytorch/pytorch.h>
THIRD_PARTY_INCLUDES_END
#endif

#include <unordered_map>
#include <vector>
#include "Misc/ScopeLock.h"
#include <string>

#include "CustomTerrainPhysicsComponent.generated.h"


UENUM(BlueprintType)
enum EDefResolutionType
{
  E256M = 0   UMETA(DisplayName = "256M"),
  E512M = 1   UMETA(DisplayName = "512M"),
  E1K = 2     UMETA(DisplayName = "1K"),
  E2K = 3     UMETA(DisplayName = "2K"),
};

UCLASS(BlueprintType)
class UHeightMapDataAsset : public UPrimaryDataAsset
{
  GENERATED_BODY()
public:

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HeightMapDataAsset)
  int SizeX = 0;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HeightMapDataAsset)
  int SizeY = 0;
  UPROPERTY(BlueprintReadWrite, Category = HeightMapDataAsset)
  TArray<float> HeightValues;
};

struct FParticle
{
  FDVector Position; // position in m
  FVector Velocity;
  float Radius = 0.02f;
};

struct FHeightMapData
{
  void InitializeHeightmap(
      UHeightMapDataAsset* DataAsset, FDVector Size, FDVector Origin,
      FDVector Tile0, float ScaleZ);
  float GetHeight(FDVector Position) const; // get height at a given global 2d position
  void Clear();
// private:
  FDVector WorldSize;
  FDVector Offset;
  uint32_t Size_X;
  uint32_t Size_Y;
  float MinHeight = 0.0f;
  float MaxHeight = 10.0f;
  float Scale_Z = 1;
  FDVector Tile0Position;
  std::vector<float> Pixels;
};

struct FDenseTile
{
  FDenseTile();
  ~FDenseTile();
  FDenseTile(const FDenseTile& Origin);
  FDenseTile(FDenseTile&& Origin);
  FDenseTile& operator=(FDenseTile&& Origin);

  void InitializeTile(uint32_t TextureSize, float AffectedRadius, float ParticleSize, float Depth, 
      FDVector TileOrigin, FDVector TileEnd, const FString& SavePath, const FHeightMapData &HeightMap);
  std::vector<FParticle*> GetParticlesInRadius(FDVector Position, float Radius);
  void GetParticlesInRadius(FDVector Position, float Radius, std::vector<FParticle*> &ParticlesInRadius);
  void GetParticlesInBox(const FOrientedBox& OBox, std::vector<FParticle*> &ParticlesInRadius);
  void GetAllParticles(std::vector<FParticle*> &ParticlesInRadius);
  void InitializeDataStructure();

  void UpdateLocalHeightmap();
  std::vector<FParticle> Particles;
  std::vector<float> ParticlesHeightMap;
  std::vector<std::multiset<float,std::greater<float>>> ParticlesZOrdered;
  bool bParticlesZOrderedInitialized = false;
  FDVector TilePosition;
  FString SavePath;
  bool bHeightmapNeedToUpdate = false;
  uint32_t PartialHeightMapSize = 0;
  uint32_t TileSize = 0;
};

class FSparseHighDetailMap
{
public:
  friend struct FTilesWorker;

  FSparseHighDetailMap(float ParticleDiameter = 0.02f, float Depth = 0.4f)
    : ParticleSize(ParticleDiameter), TerrainDepth(Depth) {};

  void Init( uint32 NewTextureSize, float NewAffectedRadius, float ParticleDiameter, 
    float Depth, float NewFloorHeight )
  {
    ParticleSize = ParticleDiameter;
    TerrainDepth = Depth;
    TextureSize = NewTextureSize;
    AffectedRadius = NewAffectedRadius;
    FloorHeight = NewFloorHeight;
    UE_LOG(LogCarla, Warning, 
        TEXT("ParticleSize %f"), ParticleSize);
  }

  inline float GetTileSize() const {
    return TileSize;
  }

  std::vector<FParticle*> GetParticlesInRadius(FDVector Position, float Radius);
  std::vector<FParticle*> GetParticlesInTileRadius(FDVector Position, float Radius);
  std::vector<FParticle*> GetParticlesInBox(const FOrientedBox& OBox);
  std::vector<uint64_t> GetIntersectingTiles(const FOrientedBox& OBox);
  std::vector<uint64_t> GetLoadedTilesInRange(FDVector Position, float Radius);


  FDenseTile& GetTile(uint32_t Tile_X, uint32_t Tile_Y);
  FDenseTile& GetTile(FDVector Position);
  FDenseTile& GetTile(uint64_t TileId);

  FDenseTile& InitializeRegion(uint32_t Tile_X, uint32_t Tile_Y);
  FDenseTile& InitializeRegion(uint64_t TileId);
  FDenseTile& InitializeRegionInCache(uint64_t TileId);

  uint64_t GetTileId(uint32_t Tile_X, uint32_t Tile_Y);
  uint64_t GetTileId(uint64_t TileId);
  uint64_t GetTileId(FDVector Position);
  FIntVector GetVectorTileId(FDVector Position);
  FIntVector GetVectorTileId(uint64_t TileId);
  FDVector GetTilePosition(uint64_t TileId);
  FDVector GetTilePosition(uint32_t Tile_X, uint32_t Tile_Y);

  float GetHeight(FDVector Position) {
    return Heightmap.GetHeight(Position);
  }

  void InitializeMap(UHeightMapDataAsset* DataAsset,
      FDVector Origin, FDVector MapSize, float Size, float ScaleZ);

  void UpdateHeightMap(UHeightMapDataAsset* DataAsset,
      FDVector Origin, FDVector MapSize, float Size, float ScaleZ);

  void UpdateMaps(FDVector Position, float RadiusX, float RadiusY, float CacheRadiusX, float CacheRadiusY);

  void Update(FVector Position, float RadiusX, float RadiusY);

  void SaveMap();

  void Clear();

  void LockMutex()
  {
    Lock_Map.Lock();
  }

  void UnLockMutex()
  {
    Lock_Map.Unlock();
  }

  std::vector<uint64_t> GetTileIdInMap()
  {
    std::vector<uint64_t> Result;
    for (auto& Iter : Map)
    {
      Result.emplace_back(Iter.first);
    }
    return Result;
  }
  std::vector<uint64_t> GetTileIdInCache()
  {
    std::vector<uint64_t> Result;
    for (auto& Iter : CacheMap)
    {
      Result.emplace_back(Iter.first);
    }
    return Result;
  }

  std::unordered_map<uint64_t, FDenseTile> Map;
  std::unordered_map<uint64_t, FDenseTile> CacheMap;
  FString SavePath;
  FCriticalSection Lock_Particles;
private:
  std::unordered_map<uint64_t, FDenseTile> TilesToWrite;
  FDVector Tile0Position;
  FDVector Extension;
  float TileSize = 1.f; // 1m per tile
  FHeightMapData Heightmap;
  float ParticleSize = 0.02f;
  float TerrainDepth = 0.4f;
  float FloorHeight = 0.0f;
  uint32 TextureSize = 0;
  float AffectedRadius = 0.0f;
  FVector PositionToUpdate;
  FCriticalSection Lock_Map; // UE4 Mutex
  FCriticalSection Lock_CacheMap; // UE4 Mutex
  FCriticalSection Lock_GetTile;
  FCriticalSection Lock_Position; // UE4 Mutex

};

USTRUCT(BlueprintType)
struct FForceAtLocation
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FVector Force;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FVector Location;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API UCustomTerrainPhysicsComponent : public UActorComponent
{
  GENERATED_BODY()
  friend struct FTilesWorker;
public:

  UCustomTerrainPhysicsComponent();
  
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  virtual void TickComponent(float DeltaTime,
      ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

  // UFUNCTION(BlueprintCallable)
  // TArray<FHitResult> SampleTerrainRayCast(const TArray<FVector> &Locations);

  UFUNCTION(BlueprintCallable)
  void AddForces(const TArray<FForceAtLocation> &Forces);

  UFUNCTION(BlueprintCallable)
  static void BuildLandscapeHeightMapDataAasset(ALandscapeProxy* Landscape, 
      int Resolution, FVector MapSize, FString AssetPath, FString AssetName);

  UFUNCTION(BlueprintCallable)
  float GetHeightAtLocation(ALandscapeProxy * Landscape, FVector Location);

  UFUNCTION(BlueprintCallable)
  TArray<FVector> GetParticlesInRadius(FVector Position, float Radius);

  UFUNCTION(BlueprintCallable)
  TArray<FVector> GetParticlesInTileRadius(FVector Position, float Radius);

  UFUNCTION(BlueprintCallable)
  FVector GetTileCenter(FVector Position);

  UFUNCTION(BlueprintCallable, Category="Tiles")
  void UpdateMaps(FVector Position, float RadiusX, float RadiusY, float CacheRadiusX, float CacheRadiusY);

  UFUNCTION(BlueprintCallable, Category="Texture")
  void InitTexture();

  UFUNCTION(BlueprintCallable, Category="Texture")
  void UpdateTexture();

  UFUNCTION(BlueprintCallable, Category="Texture")
  void UpdateLoadedTextureDataRegions();
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UHeightMapDataAsset* DataAsset;

  UFUNCTION(BlueprintCallable, Category="Texture")
  void UpdateLargeTexture();
  
  UFUNCTION(BlueprintCallable, Category="Texture")
  void UpdateLargeTextureData();

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaterialParameters")
  UTexture2D* TextureToUpdate;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaterialParameters")
  float MinDisplacement = -100;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaterialParameters")
  float MaxDisplacement = 100;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaterialParameters")
  UTexture2D* LargeTextureToUpdate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaterialParameters")
  UMaterialParameterCollection* MPC;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString NeuralModelFile = "";

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FVector NextPositionToUpdate = FVector(0,0,0);
  
  FVector LastUpdatedPosition;
  FVector CachePosition;

  FString SavePath;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float ForceMulFactor = 1.0;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float ParticleForceMulFactor = 1.0;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int SoilType = 0;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bUseSoilType = false;
  UPROPERTY(EditAnywhere)
  bool NNVerbose = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bUseLocalFrame = false;
private:

  void RunNNPhysicsSimulation(
      ACarlaWheeledVehicle *Vehicle, float DeltaTime);
  // TArray<FParticle*> GetParticlesInRange(...);
  void SetUpParticleArrays(std::vector<FParticle*>& ParticlesIn, 
      TArray<float>& ParticlePosOut, 
      TArray<float>& ParticleVelOut,
      const FTransform &WheelTransform);
  void SetUpWheelArrays(ACarlaWheeledVehicle *Vehicle, int WheelIdx,
      TArray<float>& WheelPos, 
      TArray<float>& WheelOrientation, 
      TArray<float>& WheelLinearVelocity, 
      TArray<float>& WheelAngularVelocity);
  void UpdateParticles(
      std::vector<FParticle*> Particles, std::vector<float> Forces,
      float DeltaTime, const FTransform& WheelTransform);
  void ApplyForcesToVehicle(
      ACarlaWheeledVehicle *Vehicle,
      FVector ForceWheel0, FVector ForceWheel1, FVector ForceWheel2, FVector ForceWheel3,
      FVector TorqueWheel0, FVector TorqueWheel1, FVector TorqueWheel2, FVector TorqueWheel3);
  void ApplyMeanAccelerationToVehicle(
      ACarlaWheeledVehicle *Vehicle,
      FVector ForceWheel0, FVector ForceWheel1, FVector ForceWheel2, FVector ForceWheel3);
  void ApplyAccelerationToVehicle(
      ACarlaWheeledVehicle *Vehicle,
      FVector ForceWheel0, FVector ForceWheel1, FVector ForceWheel2, FVector ForceWheel3);
      
  void ApplyForces();
  void LimitParticlesPerWheel(std::vector<FParticle*> &Particles);
  void DrawParticles(UWorld* World, std::vector<FParticle*>& Particles, 
      FLinearColor Color = FLinearColor(1.f, 0.f, 0.f));
  void DrawParticlesArray(UWorld* World, TArray<float>& ParticlesArray, 
      FLinearColor Color = FLinearColor(1.f, 0.f, 0.f));
  void DrawOrientedBox(UWorld* World, const TArray<FOrientedBox>& Boxes);
  void DrawTiles(UWorld* World, const std::vector<uint64_t>& TilesIds, float Height = 0,
    FLinearColor Color = FLinearColor(0.0,1.0,0.0,1.0));
  void GenerateBenchmarkParticles(std::vector<FParticle>& BenchParticles, 
      std::vector<FParticle*> &ParticlesWheel0, std::vector<FParticle*> &ParticlesWheel1,
      std::vector<FParticle*> &ParticlesWheel2, std::vector<FParticle*> &ParticlesWheel3,
      FOrientedBox &BboxWheel0, FOrientedBox &BboxWheel1, 
      FOrientedBox &BboxWheel2, FOrientedBox &BboxWheel3);

  void UpdateParticlesDebug(std::vector<FParticle*> Particles);
  
  void OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld);
  
  void UpdateTilesHeightMaps( const std::vector<FParticle*>& Particles);
  void RemoveParticlesFromOrderedContainer(const std::vector<FParticle*>& Particles);
  void AddParticlesToOrderedContainer(const std::vector<FParticle*>& Particles);
  void FlagTilesToRedoOrderedContainer(const std::vector<FParticle*>& Particles);
  void UpdateTilesHeightMapsInRadius(FDVector Position, uint32 Rad );
  
  void AddForceToSingleWheel(USkeletalMeshComponent* SkeletalMeshComponent,
    FVector WheelPosition, FVector WheelNormalForce);
  
  UPROPERTY(EditAnywhere)
  TArray<FForceAtLocation> ForcesToApply;
  UPROPERTY(EditAnywhere)
  UPrimitiveComponent* RootComponent;
  UPROPERTY(EditAnywhere)
  float RayCastRange = 10.0f;

public:
  UPROPERTY(EditAnywhere)
  FVector WorldSize = FVector(200000,200000,0);
  UPROPERTY(EditAnywhere)
  bool DrawDebugInfo = true;
  UPROPERTY(EditAnywhere)
  bool bUpdateParticles = false;
  // Radius of the data loaded in memory
  UPROPERTY(EditAnywhere, Category="Tiles")
  FVector TileRadius = FVector( 100, 100, 0 );
  // Radius of the data loaded in memory
  UPROPERTY(EditAnywhere, Category="Tiles")
  FVector CacheRadius = FVector( 50, 50, 0 );
  UPROPERTY(EditAnywhere, Category="Tiles")
  bool bDrawLoadedTiles = false;
  UPROPERTY(EditAnywhere, Category="Tiles")
  int32 TileSize = 1;
  UPROPERTY(EditAnywhere, Category="Tiles")
  bool bRemoveLandscapeColliders = false;
private:
  // TimeToTriggerCacheReload In seconds
  UPROPERTY(EditAnywhere, Category="Tiles")
  float TimeToTriggerCacheReload = 20.0f;
  // TimeToTriggerLoadTiles in MS
  UPROPERTY(EditAnywhere, Category="Tiles")
  float TimeToTriggerLoadTiles = 1.0f;
  UPROPERTY(EditAnywhere, Category="Tiles")
  float TimeToTriggerUnLoadTiles = 5.0f;
  // Radius of the data collected by the texture in METERS
  UPROPERTY(EditAnywhere, Category="MaterialParameters")
  float TextureRadius = 4.0f;
  // Radius of the data collected by the texture in METERS
  UPROPERTY(EditAnywhere, Category="MaterialParameters")
  float LargeTextureRadius = 50.0f;
  // Scalar Factor of deformation effect applied in the landscape
  UPROPERTY(EditAnywhere, Category="MaterialParameters")
  float EffectMultiplayer = 10.0f;

  UPROPERTY(EditAnywhere, Category="MaterialParameters")
  TEnumAsByte<EDefResolutionType> ChosenRes = EDefResolutionType::E1K;
  UPROPERTY(EditAnywhere, Category="MaterialParameters")
  TMap<TEnumAsByte<EDefResolutionType>, UTexture2D*> TexturesRes;

  bool bVisualization = false;

  UPROPERTY(EditAnywhere, Category="DeformationMesh")
  bool bUseDeformationPlane = false;
  UPROPERTY(EditAnywhere, Category="DeformationMesh")
  UStaticMesh* DeformationPlaneMesh = nullptr;
  UPROPERTY(EditAnywhere, Category="DeformationMesh")
  UMaterialInstance* DeformationPlaneMaterial = nullptr;
  UPROPERTY(VisibleAnywhere, Category="DeformationMesh")
  AStaticMeshActor* DeformationPlaneActor = nullptr;

  UPROPERTY()
  UMaterialParameterCollectionInstance* MPCInstance;

  UPROPERTY(EditAnywhere, Category="Forces")
  float NormalForceIntensity = 100;
  
  UPROPERTY(EditAnywhere)
  float SearchRadius = 100;
  UPROPERTY(EditAnywhere)
  float ParticleDiameter = 2;
  UPROPERTY(EditAnywhere)
  float TerrainDepth = 40;
  UPROPERTY(EditAnywhere)
  AActor *FloorActor = nullptr;
  UPROPERTY(EditAnywhere)
  bool bUseDynamicModel = false;
  UPROPERTY(EditAnywhere)
  bool bUseCUDAModel = false;

  UPROPERTY(EditAnywhere)
  float TireRadius = 33.0229f;
  UPROPERTY(EditAnywhere)
  float TireWidth = 21.21f;
  UPROPERTY(EditAnywhere)
  float BoxSearchForwardDistance = 114.39f;
  UPROPERTY(EditAnywhere)
  float BoxSearchLateralDistance = 31.815f;
  UPROPERTY(EditAnywhere)
  float BoxSearchDepthDistance = 20.f;
  UPROPERTY(EditAnywhere)
  bool bDisableVehicleGravity = false;
  UPROPERTY(EditAnywhere)
  float MaxForceMagnitude = 1000000.f;
  UPROPERTY(EditAnywhere)
  float FloorHeight = 0.f;
  UPROPERTY(EditAnywhere)
  bool bUseImpulse = false;
  UPROPERTY(EditAnywhere)
  bool bUseMeanAcceleration = false;
  UPROPERTY(EditAnywhere)
  bool bShowForces = true;
  UPROPERTY(EditAnywhere)
  float MinHeight = 0;
  UPROPERTY(EditAnywhere)
  float MaxHeight = 10;
  UPROPERTY(EditAnywhere)
  FVector Tile0Origin;
  UPROPERTY(EditAnywhere)
  bool bDrawHeightMap = false;
  UPROPERTY(EditAnywhere)
  FVector DrawStart = FVector(0);
  UPROPERTY(EditAnywhere)
  FVector DrawEnd = FVector(1000, 1000, 0);
  UPROPERTY(EditAnywhere)
  FVector DrawInterval = FVector(100,100,0);
  UPROPERTY(EditAnywhere)
  int CUDADevice = 0;
  UPROPERTY(EditAnywhere)
  FVector HeightMapScaleFactor = FVector(1, 1, 1);
  UPROPERTY(EditAnywhere)
  FVector HeightMapOffset = FVector(0, 0, 0);
  UPROPERTY(EditAnywhere)
  bool bBenchMark = false;
  UPROPERTY(EditAnywhere)
  int MaxParticlesPerWheel = 6000;

  UPROPERTY(EditAnywhere)
  FVector Radius = FVector(10,10,10);
  
  UPROPERTY(VisibleAnywhere)
  FIntVector CurrentLargeMapTileId = FIntVector(-1,-1,0);
  UPROPERTY(VisibleAnywhere)
  ALargeMapManager* LargeMapManager = nullptr;

  TArray<ACarlaWheeledVehicle*> Vehicles;
  FSparseHighDetailMap SparseMap;
  TArray<uint8> Data;
  TArray<uint8> LargeData;
  #ifdef WITH_PYTORCH
  carla::learning::NeuralModel TerramechanicsModel;
  #endif

  TFuture<bool> IterationCompleted;

  class FRunnableThread* Thread;
  struct FTilesWorker* TilesWorker;
};

struct FTilesWorker : public FRunnable
{
	FTilesWorker(class UCustomTerrainPhysicsComponent* TerrainComp, FVector NewPosition, float NewRadiusX, float NewRadiusY );

	virtual ~FTilesWorker() override;

	virtual uint32 Run() override; 

  class UCustomTerrainPhysicsComponent* CustomTerrainComp;
  FVector Position;

  double RadiusX; 
  double RadiusY;

  volatile bool bShouldContinue = true;
};
