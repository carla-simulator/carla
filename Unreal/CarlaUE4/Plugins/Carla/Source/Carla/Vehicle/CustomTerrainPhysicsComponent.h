// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Carla/Math/DVector.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Engine/TextureRenderTarget2D.h"
THIRD_PARTY_INCLUDES_START
#include <carla/pytorch/pytorch.h>
THIRD_PARTY_INCLUDES_END

#include <unordered_map>
#include <vector>

#include "CustomTerrainPhysicsComponent.generated.h"

struct FParticle
{
  FDVector Position; // position in m
  FVector Velocity;
  float Radius = 0.02f;
};
struct FHeightMapData
{ 
  void InitializeHeightmap(UTexture2D* Texture, FDVector Size, FDVector Origin);
  float GetHeight(FDVector Position) const; // get height at a given global 2d position
  void Clear();
private:
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
  void GetParticlesInRadius(FDVector Position, float Radius, std::vector<FParticle*> &ParticlesInRadius);
  void GetParticlesInBox(const FOrientedBox& OBox, std::vector<FParticle*> &ParticlesInRadius);
  std::vector<FParticle> Particles;
  FDVector TilePosition;
};
class FSparseHighDetailMap
{
public:

  FSparseHighDetailMap(float ParticleDiameter = 0.02f, float Depth = 0.4f)
    : ParticleSize(ParticleDiameter), TerrainDepth(Depth) {};
  std::vector<FParticle*> GetParticlesInRadius(FDVector Position, float Radius);
  std::vector<FParticle*> GetParticlesInBox(const FOrientedBox& OBox);

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

  void Clear();

private:
  std::unordered_map<uint64_t, FDenseTile> Map;
  FDVector Tile0Position;
  FDVector Extension;
  float TileSize = 1.f; // 1m per tile
  FHeightMapData Heightmap;
  float ParticleSize = 0.02f;
  float TerrainDepth = 0.4f;
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

  UCustomTerrainPhysicsComponent(const FObjectInitializer& ObjectInitializer);

  virtual void BeginPlay() override;
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

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D *HeightMap;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D *TextureToUpdate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString NeuralModelFile = "";

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

  UPROPERTY(EditAnywhere)
  TArray<FForceAtLocation> ForcesToApply;
  UPROPERTY(EditAnywhere)
  UPrimitiveComponent* RootComponent;
  UPROPERTY(EditAnywhere)
  float RayCastRange = 10.0f;
  UPROPERTY(EditAnywhere)
  FVector WorldSize = FVector(1000,1000,1000);
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


  FSparseHighDetailMap SparseMap;

  TArray<ACarlaWheeledVehicle*> Vehicles;
  carla::learning::NeuralModel TerramechanicsModel;

};
