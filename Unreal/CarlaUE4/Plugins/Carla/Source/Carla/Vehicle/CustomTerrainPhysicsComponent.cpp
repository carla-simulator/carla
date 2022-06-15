// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CustomTerrainPhysicsComponent.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "Engine/CollisionProfile.h"
#include "CollisionQueryParams.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "Carla/Game/CarlaStatics.h"
// #include <carla/pytorch/pytorch.h>

#include <algorithm>

constexpr float ParticleDiameter = 0.04f;
constexpr float TerrainDepth = 0.40f;

void FHeightMapData::InitializeHeightmap(UTexture2D* Texture, FDVector Size, FDVector Origin)
{
  WorldSize = Size;
  Offset = Origin;
  Pixels.clear();
  const FColor* FormatedImageData = 
      static_cast<const FColor*>(
        Texture->PlatformData->Mips[0].BulkData.LockReadOnly());
  // const FFloat16Color* FormatedImageData = 
  //     static_cast<const FFloat16Color*>(
  //       HeightMap->PlatformData->Mips[0].BulkData.LockReadOnly());
  Size_X = Texture->GetSizeX();
  Size_Y = Texture->GetSizeY();
  for(uint32_t j = 0; j < Size_Y; j++)
  {
    for(uint32_t i = 0; i < Size_X; i++)
    {
      uint32_t idx = j*Size_X + i;
      float HeightLevel = FormatedImageData[idx].R/255.f;
      // float HeightLevel = FormatedImageData[idx].R.GetFloat();
      Pixels.emplace_back(HeightLevel);
    }
  }
  Texture->PlatformData->Mips[0].BulkData.Unlock();
  UE_LOG(LogCarla, Log, 
      TEXT("Height Map initialized with %d pixels"), Pixels.size());
}
float FHeightMapData::GetHeight(FDVector Position) const
{
  uint32_t Coord_X = (Position.X / WorldSize.X) * Size_X;
  uint32_t Coord_Y = (Position.Y / WorldSize.Y) * Size_Y;
  Coord_X = std::min(Coord_X, Size_X-1);
  Coord_Y = std::min(Coord_X, Size_Y-1);
  return Pixels[Coord_Y*Size_X + Coord_X];
}

void FHeightMapData::Clear()
{
  Pixels.clear();
}

void FDenseTile::InitializeTile(float ParticleSize, float Depth, 
      FDVector TileOrigin, FDVector TileEnd, const FHeightMapData &HeightMap)
{
  TilePosition = TileOrigin;
  uint32_t NumParticles_X = (TileEnd.X - TileOrigin.X) / ParticleSize;
  uint32_t NumParticles_Y = (TileEnd.Y - TileOrigin.Y) / ParticleSize;
  uint32_t NumParticles_Z = (Depth) / ParticleSize;
  UE_LOG(LogCarla, Log, TEXT("Initializing Tile with %d particles at location %s, size %f, depth %f. HeightMap at tile origin %f"), 
      NumParticles_X*NumParticles_Y*NumParticles_Z, *TileOrigin.ToString(), ParticleSize, Depth, HeightMap.GetHeight(TileOrigin));
  Particles = std::vector<FParticle>(NumParticles_X*NumParticles_Y*NumParticles_Z);
  for(uint32_t i = 0; i < NumParticles_X; i++)
  {
    for(uint32_t j = 0; j < NumParticles_Y; j++)
    {
      FDVector ParticlePosition = TileOrigin + FDVector(i*ParticleSize,j*ParticleSize, 0.f);
      float Height = HeightMap.GetHeight(ParticlePosition);
      for(uint32_t k = 0; k < NumParticles_Z; k++)
      {
        ParticlePosition.Z = Height - k*ParticleSize;
        Particles[k*NumParticles_X*NumParticles_Y + j*NumParticles_X + i] = 
            {ParticlePosition, ParticleSize/2.f};
      }
    }
  }
}

std::vector<FParticle*> FDenseTile::GetParticlesInRadius(FDVector Position, float Radius)
{
  std::vector<FParticle*> ParticlesInRadius;
  for (FParticle& particle : Particles)
  {
    if((particle.Position - Position).SizeSquared() < Radius*Radius)
    {
      ParticlesInRadius.emplace_back(&particle);
    }
  }
  return ParticlesInRadius;
}

std::vector<FParticle*> FSparseHighDetailMap::
    GetParticlesInRadius(FDVector Position, float Radius)
{
  uint64_t TileId = GetTileId(Position);
  uint32_t Tile_X = (uint32_t)(TileId >> 32);
  uint32_t Tile_Y = (uint32_t)(TileId & (uint32_t)(~0));
  // UE_LOG(LogCarla, Log, TEXT("Accessing Tile (%d, %d)"), Tile_X, Tile_Y);
  FDenseTile& Tile = GetTile(TileId);
  // FDVector TilePosition = GetTilePosition(TileId);
  // FDVector LocalTilePosition = Position - TilePosition;
  return Tile.GetParticlesInRadius(Position, Radius);
}

uint64_t FSparseHighDetailMap::GetTileId(uint32_t Tile_X, uint32_t Tile_Y)
{
  return (uint64_t) Tile_X << 32 | Tile_Y;
}

uint64_t FSparseHighDetailMap::GetTileId(FDVector Position)
{
  uint32_t Tile_X = static_cast<uint32_t>((Position.X - Tile0Position.X) / TileSize);
  uint32_t Tile_Y = static_cast<uint32_t>((Position.Y - Tile0Position.Y) / TileSize);
  // UE_LOG(LogCarla, Log, TEXT("Getting tile at location %s, (%d, %d)"), *Position.ToString(), Tile_X, Tile_Y);
  return GetTileId(Tile_X, Tile_Y);
}

FDVector FSparseHighDetailMap::GetTilePosition(uint64_t TileId)
{
  uint32_t Tile_X = (uint32_t)(TileId >> 32);
  uint32_t Tile_Y = (uint32_t)(TileId & (uint32_t)(~0));
  return GetTilePosition(Tile_X, Tile_Y);
}

FDVector FSparseHighDetailMap::GetTilePosition(uint32_t Tile_X, uint32_t Tile_Y)
{
  FDVector Position = Tile0Position + FDVector(Tile_X*TileSize, Tile_Y*TileSize, 0);
  // UE_LOG(LogCarla, Log, TEXT("Getting location from id (%d, %d) %s"), Tile_X, Tile_Y, *Position.ToString());
  return Position;
}

FDenseTile& FSparseHighDetailMap::GetTile(FDVector Position)
{
  uint64_t TileId = GetTileId(Position);
  return GetTile(TileId);
}

FDenseTile& FSparseHighDetailMap::GetTile(uint32_t Tile_X, uint32_t Tile_Y)
{
  uint64_t TileId = GetTileId(Tile_X, Tile_Y);
  return GetTile(TileId);
}
FDenseTile& FSparseHighDetailMap::GetTile(uint64_t TileId)
{
  auto Iterator = Map.find(TileId);
  if (Iterator == Map.end())
  {
    return InitializeRegion(TileId);
  }
  return Iterator->second;
}

FDenseTile& FSparseHighDetailMap::InitializeRegion(uint32_t Tile_X, uint32_t Tile_Y)
{
  uint64_t TileId = GetTileId(Tile_X, Tile_Y);
  return InitializeRegion(TileId);
}

FDenseTile& FSparseHighDetailMap::InitializeRegion(uint64_t TileId)
{
  FDenseTile& Tile = Map[TileId]; // default constructor
  FDVector TileCenter = GetTilePosition(TileId);
  Tile.InitializeTile(
      ParticleDiameter, TerrainDepth,
      TileCenter, TileCenter + FDVector(TileSize, TileSize, 0.f),
      Heightmap);
  return Tile;
}

void FSparseHighDetailMap::InitializeMap(UTexture2D* HeightMapTexture,
      FDVector Origin, FDVector MapSize, float Size)
{
  Tile0Position = Origin;
  TileSize = Size;
  Extension = MapSize;
  Heightmap.InitializeHeightmap(HeightMapTexture, Extension, Tile0Position);
  UE_LOG(LogCarla, Log, 
      TEXT("Sparse Map initialized"));
}
void FSparseHighDetailMap::Clear()
{
  Heightmap.Clear();
  Map.clear();
}

void UCustomTerrainPhysicsComponent::BeginPlay()
{
  Super::BeginPlay();
  // torch::Tensor tensor = torch::rand({2, 3});
  // std::cout << tensor << std::endl;
  
  // carla::learning::test_learning();
  // carla::learning::NeuralModel Model;
  // Model.LoadModel(TCHAR_TO_ANSI(*NeuralModelFile));

 
  SparseMap.Clear();
  RootComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
  if (!RootComponent)
  {
    UE_LOG(LogCarla, Error, 
        TEXT("UCustomTerrainPhysicsComponent: Root component is not a UPrimitiveComponent"));
  }
  ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(GetWorld());
  if(LargeMap)
  {
    FIntVector NumTiles = LargeMap->GetNumTilesInXY();
    // WorldSize = FVector(NumTiles) * LargeMap->GetTileSize();
    UE_LOG(LogCarla, Log, 
        TEXT("World Size %s"), *(WorldSize.ToString()));
  }
  SparseMap.InitializeMap(HeightMap, FDVector(0.f,0.f,0.f), WorldSize/100.f);
}

void UCustomTerrainPhysicsComponent::TickComponent(float DeltaTime, 
    ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
  Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
  for (ACarlaWheeledVehicle* Vehicle : Vehicles)
  {

  }
}

// TArray<FHitResult> UCustomTerrainPhysicsComponent::SampleTerrainRayCast(
//     const TArray<FVector> &Locations)
// {
//   TArray<FHitResult> HitResult;
//   HitResult.SetNum(Locations.Num());
//   GetWorld()->GetPhysicsScene()->GetPxScene()->lockRead();
//   {
//     TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent_ParallelFor);
//     ParallelFor(Locations.Num(), [&](int32 Index) {
//       TRACE_CPUPROFILER_EVENT_SCOPE(ParallelForTask);

//       FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Terrain")), true, GetOwner());
//       TraceParams.bTraceComplex = true;
//       TraceParams.bReturnPhysicalMaterial = false;

//       FHitResult HitInfo;
//       const FVector& Location = Locations[Index];

//       GetWorld()->ParallelLineTraceSingleByChannel(
//           HitInfo,
//           Location,
//           Location + RayCastRange*FVector(0,0,-1),
//           ECC_GameTraceChannel2,
//           TraceParams,
//           FCollisionResponseParams::DefaultResponseParam);
//       UE_LOG(LogCarla, Warning, 
//           TEXT("Paralel for id %d"), Index);
//       HitResult[Index] = HitInfo;
//     });
//   }
//   GetWorld()->GetPhysicsScene()->GetPxScene()->unlockRead();
//   return HitResult;
// }
TArray<FVector> UCustomTerrainPhysicsComponent::GetParticlesInRadius(FVector Position, float Radius)
{
  std::vector<FParticle*> Particles = SparseMap.GetParticlesInRadius(Position/100.f, Radius/100.f);
  TArray<FVector> ParticlePositions;
  for(FParticle* Particle : Particles)
  {
    ParticlePositions.Add(100.f*Particle->Position.ToFVector());
  }
  return ParticlePositions;
}

FVector UCustomTerrainPhysicsComponent::GetTileCenter(FVector Position)
{
  
  return 100.f*SparseMap.GetTilePosition(SparseMap.GetTileId(Position/100.f)).ToFVector();
}

void UCustomTerrainPhysicsComponent::AddForces(
    const TArray<FForceAtLocation> &Forces)
{
  for (const FForceAtLocation& Force : Forces)
  {
    ForcesToApply.Add(Force);
  }
}

void UCustomTerrainPhysicsComponent::ApplyForces()
{
  for (const FForceAtLocation& Force : ForcesToApply)
  {
    RootComponent->AddForceAtLocationLocal(Force.Force, Force.Location);
  }
  ForcesToApply.Empty();
}
