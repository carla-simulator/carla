// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Misc/ScopeLock.h"

#include "Carla/Math/DVector.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Carla/MapGen/LargeMapManager.h"
#ifdef WITH_PYTORCH
THIRD_PARTY_INCLUDES_START
#include <carla/pytorch/pytorch.h>
#endif
THIRD_PARTY_INCLUDES_END

#include <unordered_map>
#include <vector>
#include "Misc/ScopeLock.h"
#include <string>

#include "CustomTerrainPhysicsComponent.generated.h"

struct FParticle
{
  FDVector Position; // position in m
  FVector Velocity;
  float Radius = 0.02f;
};

struct FHeightMapData
{
  void InitializeHeightmap(
    UTexture2D* Texture, FDVector Size, FDVector Origin,
      float MinHeight, float MaxHeight, FDVector Tile0, float ScaleZ);
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
  void InitializeTile(uint32_t TextureSize, float AffectedRadius, float ParticleSize, float Depth, 
      FDVector TileOrigin, FDVector TileEnd, const FString& SavePath, const FHeightMapData &HeightMap);
  std::vector<FParticle*> GetParticlesInRadius(FDVector Position, float Radius);
  void GetParticlesInRadius(FDVector Position, float Radius, std::vector<FParticle*> &ParticlesInRadius);
  void GetParticlesInBox(const FOrientedBox& OBox, std::vector<FParticle*> &ParticlesInRadius);

  std::vector<FParticle> Particles;
  std::vector<float> ParticlesHeightMap;
  FDVector TilePosition;
  FString SavePath;
};

class FSparseHighDetailMap
{
public:
  friend struct FTilesWorker;

  FSparseHighDetailMap(float ParticleDiameter = 0.02f, float Depth = 0.4f)
    : ParticleSize(ParticleDiameter), TerrainDepth(Depth) {};

  void Init( uint32 NewTextureSize, float NewAffectedRadius, float ParticleDiameter, float Depth)
  {
    ParticleSize = ParticleDiameter;
    TerrainDepth = Depth;
    TextureSize = NewTextureSize;
    AffectedRadius = NewAffectedRadius;
    UE_LOG(LogCarla, Warning, 
        TEXT("ParticleSize %f"), ParticleSize);
  }

  inline float GetTileSize() const {
    return TileSize;
  }

  std::vector<FParticle*> GetParticlesInRadius(FDVector Position, float Radius);
  std::vector<FParticle*> GetParticlesInTileRadius(FDVector Position, float Radius);
  std::vector<FParticle*> GetParticlesInBox(const FOrientedBox& OBox);
  std::vector<float> GetParticlesHeightMapInTileRadius(FDVector Position, float Radius);

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
  FDVector GetTilePosition(uint64_t TileId);
  FDVector GetTilePosition(uint32_t Tile_X, uint32_t Tile_Y);

  float GetHeight(FDVector Position) {
    return Heightmap.GetHeight(Position);
  }

  void InitializeMap(UTexture2D* HeightMapTexture,
      FDVector Origin, FDVector MapSize, float Size, float MinHeight, float MaxHeight,
      float ScaleZ);

  void UpdateHeightMap(UTexture2D* HeightMapTexture,
      FDVector Origin, FDVector MapSize, float Size, float MinHeight, float MaxHeight,
      float ScaleZ);

  void LoadTilesAtPosition(FDVector Position, float RadiusX = 100.0f, float RadiusY = 100.0f);
  void ReloadCache(FDVector Position, float RadiusX = 100.0f, float RadiusY = 100.0f);

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

  std::unordered_map<uint64_t, FDenseTile> Map;
  std::unordered_map<uint64_t, FDenseTile> CacheMap;
  FString SavePath;
private:
  std::unordered_map<uint64_t, FDenseTile> TilesToWrite;
  FDVector Tile0Position;
  FDVector Extension;
  float TileSize = 1.f; // 1m per tile
  FHeightMapData Heightmap;
  float ParticleSize = 0.02f;
  float TerrainDepth = 0.4f;
  uint32 TextureSize = 0;
  float AffectedRadius = 0.0f;
  FVector PositionToUpdate;
  FCriticalSection Lock_Map; // UE4 Mutex
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
class UCustomTerrainPhysicsComponent : public UActorComponent
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
  TArray<FVector> GetParticlesInRadius(FVector Position, float Radius);

  UFUNCTION(BlueprintCallable)
  TArray<FVector> GetParticlesInTileRadius(FVector Position, float Radius);

  UFUNCTION(BlueprintCallable)
  FVector GetTileCenter(FVector Position);

  UFUNCTION(BlueprintCallable, Category="Tiles")
  void LoadTilesAtPosition(FVector Position, float RadiusX = 100.0f, float RadiusY = 100.0f);

  UFUNCTION(BlueprintCallable, Category="Tiles")
  void ReloadCache(FVector Position, float RadiusX = 100.0f, float RadiusY = 100.0f);

  UFUNCTION(BlueprintCallable, Category="Texture")
  void InitTexture();

  UFUNCTION(BlueprintCallable, Category="Texture")
  void UpdateTexture();

  UFUNCTION(BlueprintCallable, Category="Texture")
  void UpdateTextureData();

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D *HeightMap;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaterialParameters")
  UTexture2D* TextureToUpdate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaterialParameters")
  UMaterialParameterCollection* MPC;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString NeuralModelFile = "";

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FVector NextPositionToUpdate = FVector(0,0,0);
  
  FVector LastUpdatedPosition;

  FString SavePath;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float ForceMulFactor = 1.0;
  UPROPERTY(EditAnywhere)
  bool NNVerbose = false;

private:

  void RunNNPhysicsSimulation(
      ACarlaWheeledVehicle *Vehicle, float DeltaTime);
  // TArray<FParticle*> GetParticlesInRange(...);
  void SetUpParticleArrays(std::vector<FParticle*>& ParticlesIn, 
      TArray<float>& ParticlePosOut, 
      TArray<float>& ParticleVelOut);
  void SetUpWheelArrays(ACarlaWheeledVehicle *Vehicle, int WheelIdx,
      TArray<float>& WheelPos, 
      TArray<float>& WheelOrientation, 
      TArray<float>& WheelLinearVelocity, 
      TArray<float>& WheelAngularVelocity);
  void UpdateParticles(
      std::vector<FParticle*> Particles, std::vector<float> Forces,
      float DeltaTime);
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
  void DrawParticles(UWorld* World, std::vector<FParticle*>& Particles);
  void DrawOrientedBox(UWorld* World, const TArray<FOrientedBox>& Boxes);

  void UpdateTilesHeightMaps( const std::vector<FParticle*>& Particles);
  UPROPERTY(EditAnywhere)
  TArray<FForceAtLocation> ForcesToApply;
  UPROPERTY(EditAnywhere)
  UPrimitiveComponent* RootComponent;
  UPROPERTY(EditAnywhere)
  float RayCastRange = 10.0f;
  UPROPERTY(EditAnywhere)
  FVector WorldSize = FVector(200000,200000,0);

  // Radius of the data loaded in memory
  UPROPERTY(EditAnywhere, Category="Tiles")
  FVector TileRadius = FVector( 5, 5, 0 );
  // Radius of the data loaded in memory
  UPROPERTY(EditAnywhere, Category="Tiles")
  FVector CacheRadius = FVector( 50, 50, 0 );
  UPROPERTY(EditAnywhere, Category="Tiles")
  int32 TileSize = 1;

  // Radius of the data collected by the texture in METERS
  UPROPERTY(EditAnywhere, Category="MaterialParameters")
  float TextureRadius = 4.0f;
  // Scalar Factor of deformation effect applied in the landscape
  UPROPERTY(EditAnywhere, Category="MaterialParameters")
  float EffectMultiplayer = 10.0f;

  UPROPERTY()
  UMaterialParameterCollectionInstance* MPCInstance;
  


  UPROPERTY(EditAnywhere)
  float SearchRadius = 100;
  UPROPERTY(EditAnywhere)
  float ParticleDiameter = 2;
  UPROPERTY(EditAnywhere)
  float TerrainDepth = 40;
  UPROPERTY(EditAnywhere)
  AActor *FloorActor = nullptr;
  UPROPERTY(EditAnywhere)
  bool bUpdateParticles = false;
  UPROPERTY(EditAnywhere)
  bool bUseDynamicModel = false;

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
  bool DrawDebugInfo = true;
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
  UPROPERTY(VisibleAnywhere)
  FIntVector CurrentLargeMapTileId = FIntVector(-1,-1,0);
  UPROPERTY(VisibleAnywhere)
  ALargeMapManager* LargeMapManager = nullptr;

  TArray<ACarlaWheeledVehicle*> Vehicles;
  FSparseHighDetailMap SparseMap;
  TArray<uint8> Data;
  #ifdef WITH_PYTORCH
  carla::learning::NeuralModel TerramechanicsModel;
  #endif

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

  bool bShouldContinue = true;
};
