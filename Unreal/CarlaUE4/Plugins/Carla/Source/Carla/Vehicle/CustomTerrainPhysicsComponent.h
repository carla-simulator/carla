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

#include <unordered_map>
#include <vector>
#include <string>

#include "CustomTerrainPhysicsComponent.generated.h"

struct FParticle
{
  // It formats particle data to "XValue YValue ZValue RadiusValue \n"
  std::string ToString() const;

  // String received must have format "XValue YValue ZValue RadiusValue \n"
  void ModifyDataFromString(const std::string& BaseString);

  FDVector Position; // position in m
  float Radius = 0.02f;

};

struct FHeightMapData
{
  void InitializeHeightmap(UTexture2D* Texture, FDVector Size, FDVector Origin);
  float GetHeight(FDVector Position) const; // get height at a given global 2d position
  void Clear();
// private:
  FDVector WorldSize;
  FDVector Offset;
  uint32_t Size_X;
  uint32_t Size_Y;
  std::vector<float> Pixels;
};

struct FDenseTile
{
  void InitializeTile(float ParticleSize, float Depth, 
      FDVector TileOrigin, FDVector TileEnd, const FHeightMapData &HeightMap);
  std::vector<FParticle*> GetParticlesInRadius(FDVector Position, float Radius);
  
  // Format DenseTile to "PosX PosY PosZ\n Particles"
  // WARNING LookAt ParticlesFormat
  std::string ToString() const;

  // String received must have format "PosX PosY PosZ\n ParticlesList"
  void ModifyDataFromString(const std::string& BaseString);

  std::vector<FParticle> Particles;
  FDVector TilePosition;
};

class FSparseHighDetailMap
{
public:

  inline float GetTileSize() const {
    return TileSize;
  }

  std::vector<FParticle*> GetParticlesInRadius(FDVector Position, float Radius);

  FDenseTile& GetTile(uint32_t Tile_X, uint32_t Tile_Y);
  FDenseTile& GetTile(FDVector Position);
  FDenseTile& GetTile(uint64_t TileId);

  FDenseTile& InitializeRegion(uint32_t Tile_X, uint32_t Tile_Y);
  FDenseTile& InitializeRegion(uint64_t TileId);

  uint64_t GetTileId(uint32_t Tile_X, uint32_t Tile_Y);
  uint64_t GetTileId(uint64_t TileId);
  uint64_t GetTileId(FDVector Position);
  FDVector GetTilePosition(uint64_t TileId);
  FDVector GetTilePosition(uint32_t Tile_X, uint32_t Tile_Y);

  void InitializeMap(UTexture2D* HeightMapTexture,
      FDVector Origin, FDVector MapSize, float Size = 1.f);

  void LoadTilesAtPosition(FDVector Position, float RadiusX = 100.0f, float RadiusY = 100.0f);
  
  void Update(FVector Position, float RadiusX, float RadiusY);

  void SaveMap();

  void Clear();

  void UpdateTexture(float RadiusX, float RadiusY);
private:
  std::unordered_map<uint64_t, FDenseTile> Map;
  std::unordered_map<uint64_t, FDenseTile> TilesToWrite;
  FDVector Tile0Position;
  FDVector Extension;
  float TileSize = 1.f; // 1m per tile
  FHeightMapData Heightmap;
  FVector PositionToUpdate;
  FCriticalSection Lock_Map; // UE4 Mutex
  FCriticalSection Lock_Position; // UE4 Mutex
  std::vector<double> Data;
  UTexture2D* TextureToUpdate;

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
  FVector GetTileCenter(FVector Position);

  UFUNCTION(BlueprintCallable, Category="Tiles")
  void LoadTilesAtPosition(FVector Position, float RadiusX = 100.0f, float RadiusY = 100.0f);

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D *HeightMap;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString NeuralModelFile = "";
  
  FVector LastUpdatedPosition;
private:

  void ApplyForces();

  UPROPERTY(EditAnywhere)
  TArray<FForceAtLocation> ForcesToApply;
  UPROPERTY(EditAnywhere)
  UPrimitiveComponent* RootComponent;
  UPROPERTY(EditAnywhere)
  float RayCastRange = 10.0f;
  UPROPERTY(EditAnywhere)
  FVector WorldSize = FVector(1000,1000,1000);
  UPROPERTY(EditAnywhere)
  FVector Radius = FVector(10,10,10);


  FSparseHighDetailMap SparseMap;
  TArray<ACarlaWheeledVehicle*> Vehicles;

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
  float RadiusX; 
  float RadiusY;
  bool bShouldContinue = true;
};
