// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#undef CreateDirectory

#include "CustomTerrainPhysicsComponent.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "Engine/CollisionProfile.h"
#include "CollisionQueryParams.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/MapGen/SoilTypeManager.h"
#include "carla/rpc/String.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/RunnableThread.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "LandscapeComponent.h"

#include "RHICommandList.h"
#include "TextureResource.h"
#include "Rendering/Texture2DResource.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
// #include <carla/pytorch/pytorch.h>

#include "Components/SkinnedMeshComponent.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Async/Async.h"
#include "Async/Future.h"
#include "LandscapeProxy.h"


#include "Carla/Game/CarlaStatics.h"
#include "carla/rpc/String.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/RunnableThread.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture.h"
#include "Rendering/Texture2DResource.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CommandLine.h"
#include "Components/LineBatchComponent.h"
#include "Math/OrientedBox.h"
#include "Misc/DateTime.h"
#include "EngineUtils.h"
#include <algorithm>
#include <fstream>

#include <thread>
#include <chrono>

constexpr float MToCM = 100.f;
constexpr float CMToM = 0.01f;
 
const int CacheExtraRadius = 10;

#ifdef _WIN32
      std::string _filesBaseFolder = std::string(getenv("USERPROFILE")) + "/carlaCache/";
#else
      std::string _filesBaseFolder = std::string(getenv("HOME")) + "/carlaCache/";
#endif

FVector SIToUEFrame(const FVector& In)
{
  return MToCM * FVector(In.X, -In.Y, In.Z);
}
float SIToUEFrame(const float& In) { return MToCM * In; }
FVector SIToUEFrameDirection(const FVector& In)
{
  return FVector(In.X, -In.Y, In.Z);
}

FVector UEFrameToSI(const FVector& In)
{
  return CMToM*FVector(In.X, -In.Y, In.Z);
}
float UEFrameToSI(const float& In) { return CMToM * In; }
FVector UEFrameToSIDirection(const FVector& In)
{
  return FVector(In.X, -In.Y, In.Z);
}

void FHeightMapData::InitializeHeightmap(
    UHeightMapDataAsset* DataAsset, FDVector Size, FDVector Origin,
    FDVector Tile0, float ScaleZ)
{
  Tile0Position = Tile0;
  WorldSize = Size;
  Offset = Origin;
  Size_X = DataAsset->SizeX;
  Size_Y = DataAsset->SizeY;
  // Pixels = DataAsset->HeightValues;
  Pixels.clear();
  Pixels.reserve(DataAsset->HeightValues.Num());
  for (float Height : DataAsset->HeightValues)
  {
    Pixels.emplace_back(UEFrameToSI(Height));
  }
}

float FHeightMapData::GetHeight(FDVector Position) const
{
  Position = Position - Tile0Position;
  uint32_t Coord_X = (Position.X / WorldSize.X) * Size_X;
  uint32_t Coord_Y = (1.f - Position.Y / WorldSize.Y) * Size_Y;
  Coord_X = std::min(Coord_X, Size_X-1);
  Coord_Y = std::min(Coord_Y, Size_Y-1);
  return Pixels[Coord_X*Size_Y + Coord_Y];
}

void FHeightMapData::Clear()
{
  Pixels.clear();
}

FDenseTile::FDenseTile(){
  Particles.clear();
  ParticlesHeightMap.clear();
  TilePosition = FDVector(0.0,0.0,0.0);
  SavePath = FString("NotValidPath");
  bHeightmapNeedToUpdate = false;
}

FDenseTile::~FDenseTile(){
  Particles.clear();
  ParticlesHeightMap.clear();
  ParticlesZOrdered.clear();
  TilePosition = FDVector(0.0,0.0,0.0);
  SavePath = FString("NotValidPath");
  bHeightmapNeedToUpdate = false;
}

FDenseTile::FDenseTile(const FDenseTile& Origin){
  TilePosition = Origin.TilePosition;
  SavePath = Origin.SavePath;
  bHeightmapNeedToUpdate = false;
  Particles = Origin.Particles;
  ParticlesHeightMap = Origin.ParticlesHeightMap;
  ParticlesZOrdered = Origin.ParticlesZOrdered;
}

FDenseTile::FDenseTile(FDenseTile&& Origin){
  TilePosition = Origin.TilePosition;
  SavePath = Origin.SavePath;
  bHeightmapNeedToUpdate = false;
  Particles = std::move(Origin.Particles);
  ParticlesHeightMap = std::move(Origin.ParticlesHeightMap);
  ParticlesZOrdered = std::move(Origin.ParticlesZOrdered);
}

FDenseTile& FDenseTile::operator=(FDenseTile&& Origin)
{
  TilePosition = Origin.TilePosition;
  SavePath = Origin.SavePath;
  bHeightmapNeedToUpdate = false;
  Particles = std::move(Origin.Particles);
  ParticlesHeightMap = std::move(Origin.ParticlesHeightMap);
  ParticlesZOrdered = std::move(Origin.ParticlesZOrdered);
  return *this;
 }
  
void FDenseTile::InitializeTile(uint32_t TextureSize, float AffectedRadius, float ParticleSize, float Depth, 
    FDVector TileOrigin, FDVector TileEnd, 
    const FString& SavePath, const FHeightMapData &HeightMap)
{

  TileSize = (TileEnd.X - TileOrigin.X );
  PartialHeightMapSize = TileSize * TextureSize / (2*AffectedRadius);
  std::string FileName = std::string(TCHAR_TO_UTF8(*( SavePath + TileOrigin.ToString() + ".tile" ) ) );
  
  //UE_LOG(LogCarla, Log, TEXT("Tile origin %s"), *TileOrigin.ToString() );
  if( FPaths::FileExists(FString(FileName.c_str())) )
  {
    
    TRACE_CPUPROFILER_EVENT_SCOPE(DenseTile::InitializeTile::Read);
    std::ifstream ReadStream(FileName);
    FVector VectorToRead;
    ReadFVector(ReadStream, VectorToRead );
    TilePosition = FDVector(VectorToRead);
    ReadStdVector<FParticle> (ReadStream, Particles);
    //UE_LOG(LogCarla, Log, TEXT("Reading data, got %d particles"), Particles.size());
  }
  else
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(DenseTile::InitializeTile::Create);

    TilePosition = TileOrigin;
    uint32_t NumParticles_X = (TileEnd.X - TileOrigin.X) / ParticleSize;
    uint32_t NumParticles_Y = FMath::Abs(TileEnd.Y - TileOrigin.Y) / ParticleSize;
    uint32_t NumParticles_Z = (Depth) / ParticleSize;
    Particles = std::vector<FParticle>(NumParticles_X*NumParticles_Y*NumParticles_Z);

    //UE_LOG(LogCarla, Log, TEXT("Initializing Tile with (%d,%d,%d) particles at location %s, size %f, depth %f, HeightMap at tile origin %f"), 
    //   NumParticles_X,NumParticles_Y,NumParticles_Z, *TileOrigin.ToString(), ParticleSize, Depth,  HeightMap.GetHeight(TileOrigin)  );

    for(uint32_t i = 0; i < NumParticles_X; i++)
    {
      for(uint32_t j = 0; j < NumParticles_Y; j++)
      {
        FDVector ParticleLocalPosition = FDVector(i*ParticleSize, j*ParticleSize, 0.0f);
        FDVector ParticlePosition = TileOrigin + ParticleLocalPosition;
        float Height = HeightMap.GetHeight(ParticlePosition);
        for(uint32_t k = 0; k < NumParticles_Z; k++)
        {
          ParticlePosition.Z = TileOrigin.Z + Height - k*ParticleSize;
          Particles[k*NumParticles_X*NumParticles_Y + j*NumParticles_X + i] = 
            {ParticlePosition, FVector(0), ParticleSize/2.f};
        }
      }
    }

    //UE_LOG(LogCarla, Log, TEXT("Building local heightMap of %d pixels"), PartialHeightMapSize);

  }
  ParticlesZOrdered.resize( PartialHeightMapSize*PartialHeightMapSize );

  for(float& Height : ParticlesHeightMap)
  {
    Height = 0;
  }

  bParticlesZOrderedInitialized = false;
  bHeightmapNeedToUpdate = true;
}

void FDenseTile::InitializeDataStructure()
{
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(DenseTile::InitializeTile::ParticlesZOrdered);
    ParticlesHeightMap.clear();
    ParticlesHeightMap.resize( PartialHeightMapSize*PartialHeightMapSize );
    float InverseTileSize = 1.f/TileSize;
    float Transformation = InverseTileSize * PartialHeightMapSize;

    for (size_t i = 0; i < Particles.size(); i++)
    {
      const FParticle& P = Particles[i];
      FDVector ParticleLocalPosition = P.Position - TilePosition;
      // Recalculate position to get it into heightmap coords
      FIntVector HeightMapCoords = FIntVector(
          ParticleLocalPosition.X * Transformation,
          ParticleLocalPosition.Y * Transformation, 0);

      uint32_t Index = HeightMapCoords.Y * PartialHeightMapSize + HeightMapCoords.X;
      // Compare to the current value, if higher replace 
      if(Index < ParticlesZOrdered.size() ){
        ParticlesZOrdered[Index].insert(P.Position.Z);
      }
    }
    
  }
  bParticlesZOrderedInitialized = true;
  bHeightmapNeedToUpdate = true;
}

// revise coordinates
void FDenseTile::GetParticlesInRadius(FDVector Position, float Radius, std::vector<FParticle*> &ParticlesInRadius)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FDenseTile::GetParticlesInRadius);
  for (FParticle& particle : Particles)
  {
    if((particle.Position - Position).SizeSquared() < Radius*Radius)
    {
      ParticlesInRadius.emplace_back(&particle);
    }
  }
}
// revise coordinates
std::vector<FParticle*> FDenseTile::GetParticlesInRadius(FDVector Position, float Radius)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FDenseTile::GetParticlesInRadius);
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

void FDenseTile::GetParticlesInBox(
    const FOrientedBox& OBox, std::vector<FParticle*> &ParticlesInRadius)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FDenseTile::GetParticlesInBox);
  for (FParticle& Particle : Particles)
  {
    FVector PToCenter = SIToUEFrame(Particle.Position.ToFVector()) - OBox.Center;
    if((FMath::Abs(FVector::DotProduct(PToCenter, OBox.AxisX)) < OBox.ExtentX) &&
       (FMath::Abs(FVector::DotProduct(PToCenter, OBox.AxisY)) < OBox.ExtentY) &&
       (FMath::Abs(FVector::DotProduct(PToCenter, OBox.AxisZ)) < OBox.ExtentZ))
    {
      ParticlesInRadius.emplace_back(&Particle);
    }
  }
}

void FDenseTile::GetAllParticles(std::vector<FParticle*> &ParticlesInRadius)
{
  for (FParticle& Particle : Particles)
  {
    ParticlesInRadius.emplace_back(&Particle);
  }
}

void FDenseTile::UpdateLocalHeightmap()
{
  if( bHeightmapNeedToUpdate && bParticlesZOrderedInitialized ){
    TRACE_CPUPROFILER_EVENT_SCOPE(FDenseTile::UpdateLocalHeightmap);
    for( uint32_t i = 0; i < ParticlesHeightMap.size() ; ++i ){
      if( ParticlesZOrdered.size() == ParticlesHeightMap.size() ){
        float Value = * ( ParticlesZOrdered[i].begin() );
        ParticlesHeightMap[i] = Value;
      }
    }
    bHeightmapNeedToUpdate = false;
  }

}

// revise coordinates
std::vector<FParticle*> FSparseHighDetailMap::
    GetParticlesInRadius(FDVector Position, float Radius)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::GetParticlesInRadius);
 
  uint64_t TileId = GetTileId(Position);
  uint32_t Tile_X = (uint32_t)(TileId >> 32);
  uint32_t Tile_Y = (uint32_t)(TileId & (uint32_t)(~0));

  // FDenseTile& Tile = GetTile(TileId);
  // return Tile.GetParticlesInRadius(Position, Radius);
  std::vector<FParticle*> ParticlesInRadius;

  GetTile(Tile_X-1, Tile_Y-1).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X, Tile_Y-1).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X+1, Tile_Y-1).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X-1, Tile_Y).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X, Tile_Y).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X+1, Tile_Y).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X-1, Tile_Y+1).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X, Tile_Y+1).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X+1, Tile_Y+1).GetParticlesInRadius(Position, Radius, ParticlesInRadius);

  return ParticlesInRadius;
}

std::vector<FParticle*> FSparseHighDetailMap::
    GetParticlesInTileRadius(FDVector Position, float Radius)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::GetParticlesInRadius);
 
  uint64_t TileId = GetTileId(Position);
  uint32_t Tile_X = (uint32_t)(TileId >> 32);
  uint32_t Tile_Y = (uint32_t)(TileId & (uint32_t)(~0));

  uint32_t RadiusInTiles = (Radius/TileSize);
  uint32_t MinX,MinY,MaxX,MaxY = 0; 
  
  if( Tile_X < RadiusInTiles){
    MinX = 0;
  }else{
    MinX = Tile_X - RadiusInTiles;
  }
  
  if( Tile_Y < RadiusInTiles){
    MinY = 0;
  }else{
    MinY = Tile_Y - RadiusInTiles;
  }

  if( ((Extension.X) / TileSize - RadiusInTiles) < Tile_X ){
    MaxX = (Extension.X) / TileSize;
  }else{
    MaxX = Tile_X + RadiusInTiles;
  }

  if( ((-Extension.Y) / TileSize)  - RadiusInTiles < Tile_Y  ){
    MaxY = (-Extension.Y) / TileSize;
  }else{
    MaxY = Tile_Y + RadiusInTiles;
  }
  /*
  UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::GetParticlesInTileRadius MinX %zu MaxX: %zu, MinY %zu MaxY %zu"),
      MinX, MaxX, MinY, MaxY);
  UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::GetParticlesInTileRadius Extension X: %f, Y %f"), Extension.X, Extension.Y);
  UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::GetParticlesInTileRadius Position X: %f, Y %f"), Position.X, Position.Y);
  UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::GetParticlesInTileRadius RadiusInTiles %d"), RadiusInTiles);
  UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::GetParticlesInTileRadius TileId %lld TileX: %d, TileY %d"),
      TileId, Tile_X, Tile_Y);


  */
  // FDenseTile& Tile = GetTile(TileId);
  // return Tile.GetParticlesInRadius(Position, Radius);
  std::vector<FParticle*> ParticlesInRadius;
  for( uint32_t X = MinX; X <= MaxX; ++X )
  {
    for( uint32_t Y = MinY; Y <= MaxY; ++Y )
    {
      uint64_t CurrentTileId = GetTileId(X,Y);
      if( Map.count(CurrentTileId) )
      {
        GetTile(X, Y).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
      }
    }
  }
  return ParticlesInRadius;
}

std::vector<FParticle*> FSparseHighDetailMap::
    GetParticlesInBox(const FOrientedBox& OBox)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::GetParticlesInBox);
  std::vector<uint64_t> TilesToCheck = GetIntersectingTiles(OBox);
  
  std::vector<FParticle*> ParticlesInRadius;
  for(uint64_t TileId : TilesToCheck)
  {
    GetTile(TileId).GetParticlesInBox(OBox, ParticlesInRadius);
  }
  return ParticlesInRadius;
}

std::vector<uint64_t> FSparseHighDetailMap::GetIntersectingTiles(
    const FOrientedBox& OBox)
{
TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::GetIntersectingTiles);
  std::vector<uint64_t> IntersectingTiles;

  FVector BoxCenter = UEFrameToSI(OBox.Center);
  float ExtentX = UEFrameToSI(OBox.ExtentX);
  float ExtentXSqr = ExtentX*ExtentX;
  FVector AxisX = UEFrameToSIDirection(OBox.AxisX);
  float ExtentY = UEFrameToSI(OBox.ExtentY);
  float ExtentYSqr = ExtentY*ExtentY;
  FVector AxisY = UEFrameToSIDirection(OBox.AxisY);

  uint64_t CenterTileId = GetTileId(BoxCenter);
  uint32_t CenterTile_X = (uint32_t)(CenterTileId >> 32);
  uint32_t CenterTile_Y = (uint32_t)(CenterTileId & (uint32_t)(~0));
  uint32_t TileRange = 1;
  IntersectingTiles.emplace_back(CenterTileId);
  FVector2D BoxVert0 = FVector2D(BoxCenter - AxisX*ExtentX - AxisY*ExtentY);
  FVector2D BoxVert1 = FVector2D(BoxCenter + AxisX*ExtentX - AxisY*ExtentY);
  FVector2D BoxVert2 = FVector2D(BoxCenter + AxisX*ExtentX + AxisY*ExtentY);
  FVector2D BoxVert3 = FVector2D(BoxCenter - AxisX*ExtentX + AxisY*ExtentY);

  // Use separate axis theorem to detect intersecting tiles with OBox
  struct F2DRectangle
  {
    FVector2D V1, V2, V3, V4;
    std::vector<FVector2D> ComputeNormals() const
    {
      FVector2D V12 = (V2 - V1).GetSafeNormal();
      FVector2D V23 = (V3 - V2).GetSafeNormal();
      return {FVector2D(-V12.Y, V12.X), FVector2D(-V23.Y, V23.X)};
    }
  };
  auto SATtest = [&](const FVector2D& Axis, const F2DRectangle &Shape,
      float &MinAlong, float &MaxAlong)
  {
    for (const FVector2D& Vert : {Shape.V1, Shape.V2, Shape.V3, Shape.V4})
    {
      float DotVal = FVector2D::DotProduct(Vert, Axis);
      if( DotVal < MinAlong ) 
        MinAlong = DotVal;
      if( DotVal > MaxAlong ) 
        MaxAlong = DotVal;
    }
  };
  auto IsBetweenOrdered = [&]( float Val, float LowerBound, float UpperBound ) -> bool
  {
    return LowerBound <= Val && Val <= UpperBound ;
  };
  auto Overlaps = [&]( float Min1, float Max1, float Min2, float Max2 ) -> bool
  {
    return IsBetweenOrdered( Min2, Min1, Max1 ) || IsBetweenOrdered( Min1, Min2, Max2 ) ;
  };
  auto RectangleIntersect = [&](
    const F2DRectangle& Rectangle1, 
    const F2DRectangle& Rectangle2) -> bool
  {
    for (const FVector2D& Normal : Rectangle1.ComputeNormals())
    {
      constexpr float LargeNumber = 10000000;
      float Shape1Min = LargeNumber, Shape1Max = -LargeNumber;
      float Shape2Min = LargeNumber, Shape2Max = -LargeNumber;
      SATtest(Normal, Rectangle1, Shape1Min, Shape1Max);
      SATtest(Normal, Rectangle2, Shape2Min, Shape2Max);
      if (!Overlaps(Shape1Min, Shape1Max, Shape2Min, Shape2Max))
      {
        return false;
      }
    }
    for (const FVector2D& Normal : Rectangle2.ComputeNormals())
    {
      constexpr float LargeNumber = 10000000;
      float Shape1Min = LargeNumber, Shape1Max = -LargeNumber;
      float Shape2Min = LargeNumber, Shape2Max = -LargeNumber;
      SATtest(Normal, Rectangle1, Shape1Min, Shape1Max);
      SATtest(Normal, Rectangle2, Shape2Min, Shape2Max);
      if (!Overlaps(Shape1Min, Shape1Max, Shape2Min, Shape2Max))
      {
        return false;
      }
    }
    return true;
  };
  // check surrounding tiles except CenterTile (as it is always included)
  std::vector<std::pair<uint32_t, uint32_t>> TilesToCheck = {
      {CenterTile_X-1, CenterTile_Y-1}, {CenterTile_X, CenterTile_Y-1},
      {CenterTile_X+1, CenterTile_Y-1}, {CenterTile_X-1, CenterTile_Y},
      {CenterTile_X+1, CenterTile_Y}, {CenterTile_X-1, CenterTile_Y+1},
      {CenterTile_X, CenterTile_Y+1}, {CenterTile_X+1, CenterTile_Y+1}};
  for (auto& TileIdPair : TilesToCheck)
  {
    uint32_t &Tile_X = TileIdPair.first;
    uint32_t &Tile_Y = TileIdPair.second;
    FVector2D V1 = FVector2D(GetTilePosition(Tile_X, Tile_Y).ToFVector());
    FVector2D V2 = FVector2D(GetTilePosition(Tile_X+1, Tile_Y).ToFVector());
    FVector2D V3 = FVector2D(GetTilePosition(Tile_X+1, Tile_Y+1).ToFVector());
    FVector2D V4 = FVector2D(GetTilePosition(Tile_X, Tile_Y+1).ToFVector());
    if (RectangleIntersect(
        F2DRectangle{BoxVert0,BoxVert1,BoxVert2,BoxVert3},
        F2DRectangle{V1,V2,V3,V4}))
    {
      IntersectingTiles.emplace_back(GetTileId(Tile_X, Tile_Y));
    }
  }

  return IntersectingTiles;
}

std::vector<uint64_t> FSparseHighDetailMap::GetLoadedTilesInRange(FDVector Position, float Radius)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::GetLoadedTilesInRange);
 
  uint64_t TileId = GetTileId(Position);
  uint32_t Tile_X = (uint32_t)(TileId >> 32);
  uint32_t Tile_Y = (uint32_t)(TileId & (uint32_t)(~0));

  uint32_t RadiusInTiles = (Radius/TileSize);
  uint32_t MinX = 0,MinY = 0,MaxX = 0,MaxY = 0; 
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(Comparisons);
    if( Tile_X < RadiusInTiles){
      MinX = 0;
    }else{
      MinX = Tile_X - RadiusInTiles;
    }
    
    if( Tile_Y < RadiusInTiles){
      MinY = 0;
    }else{
      MinY = Tile_Y - RadiusInTiles;
    }

    MaxX = Tile_X + RadiusInTiles;
    MaxY = Tile_Y + RadiusInTiles;
  }

  std::vector<uint64_t> LoadedTiles;
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(Looping);
    for( uint32_t X = MinX; X < MaxX; ++X )
    {
      for( uint32_t Y = MinY; Y < MaxY; ++Y )
      {
        uint64_t CurrentTileId = GetTileId(X,Y);
        if( Map.find(CurrentTileId) != Map.end() )
        {
          LoadedTiles.emplace_back(CurrentTileId);
        }
      }
    }
  }
  return LoadedTiles;
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
  FDVector Position = FDVector(Tile_X*TileSize, Tile_Y*TileSize, FloorHeight);
  Position = Position + Tile0Position;
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
    TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::GetTile);
    // FScopeLock Lock(&Lock_CacheMap);
    Lock_GetTile.Lock();
    bool bGotCacheLock = Lock_CacheMap.TryLock();
    if(bGotCacheLock)
    {
      auto CacheIterator = CacheMap.find(TileId);
      if (CacheIterator != CacheMap.end())
      {
        Map.emplace(TileId, std::move(CacheIterator->second));
        CacheMap.erase(CacheIterator);
        Lock_CacheMap.Unlock();
        Lock_GetTile.Unlock();
        return Map[TileId];
      }
      Lock_CacheMap.Unlock();
    }
    FDenseTile& Tile = Map[TileId];
    Lock_GetTile.Unlock();
    return InitializeRegion(TileId);
  }
  return Iterator->second;
}

FIntVector FSparseHighDetailMap::GetVectorTileId(FDVector Position)
{
  uint32_t Tile_X = static_cast<uint32_t>((Position.X - Tile0Position.X) / TileSize);
  uint32_t Tile_Y = static_cast<uint32_t>((Position.Y - Tile0Position.Y) / TileSize);
  return FIntVector(Tile_X, Tile_Y, 0);
}
FIntVector FSparseHighDetailMap::GetVectorTileId(uint64_t TileId)
{
  uint32_t Tile_X = (uint32_t)(TileId >> 32);
  uint32_t Tile_Y = (uint32_t)(TileId & (uint32_t)(~0));
  return FIntVector(Tile_X, Tile_Y, 0);
}

FDenseTile& FSparseHighDetailMap::InitializeRegion(uint32_t Tile_X, uint32_t Tile_Y)
{
  uint64_t TileId = GetTileId(Tile_X, Tile_Y);
  return InitializeRegion(TileId);
}

FDenseTile& FSparseHighDetailMap::InitializeRegion(uint64_t TileId)
{
  FDVector TileCenter = GetTilePosition(TileId);
  FDenseTile& Tile = Map[TileId]; 
  //UE_LOG(LogCarla, Log, TEXT("InitializeRegion Tile with (%f,%f,%f)"), 
  //  TileCenter.X,TileCenter.Y,TileCenter.Z);
  Tile.InitializeTile(
      TextureSize, AffectedRadius,
      ParticleSize, TerrainDepth,
      TileCenter, TileCenter + FDVector(TileSize, TileSize, 0.f),
      SavePath, Heightmap);
  return Tile;
}

FDenseTile& FSparseHighDetailMap::InitializeRegionInCache(uint64_t TileId)
{
  FDVector TileCenter = GetTilePosition(TileId);
  FDenseTile& Tile = CacheMap[TileId]; 
  //UE_LOG(LogCarla, Log, TEXT("InitializeRegionInCache Tile with (%f,%f,%f)"), 
  //  TileCenter.X,TileCenter.Y,TileCenter.Z);
  
  Tile.InitializeTile(
      TextureSize, AffectedRadius,
      ParticleSize, TerrainDepth,
      TileCenter, TileCenter + FDVector(TileSize, TileSize, 0.f),
      SavePath, Heightmap);
  Lock_CacheMap.Unlock();
  return Tile;
}

void FSparseHighDetailMap::InitializeMap(UHeightMapDataAsset* DataAsset,
      FDVector Origin, FDVector MapSize, float Size, float ScaleZ)
{
  Tile0Position = Origin;
  TileSize = Size;
  Extension = MapSize;
  Heightmap.InitializeHeightmap(
      DataAsset, Extension, Tile0Position, 
      Tile0Position, ScaleZ);
  UE_LOG(LogCarla, Log, 
      TEXT("Sparse Map initialized"));
  UE_LOG(LogCarla, Log, 
      TEXT("Map Extension %f %f %f"), MapSize.X, MapSize.Y, MapSize.Z );
}

void FSparseHighDetailMap::UpdateHeightMap(UHeightMapDataAsset* DataAsset,
      FDVector Origin, FDVector MapSize, float Size,
      float ScaleZ)
{
  Heightmap.Clear();
  Heightmap.InitializeHeightmap(
      DataAsset, Extension, Origin, 
      Origin, ScaleZ);
  UE_LOG(LogCarla, Log, 
      TEXT("Height map updated"));
}
void FSparseHighDetailMap::Clear()
{
  Heightmap.Clear();
  Map.clear();
}

void FSparseHighDetailMap::UpdateMaps(
    FDVector Position, float RadiusX, float RadiusY, float CacheRadiusX, float CacheRadiusY)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::UpdateMaps);
  double MinX = Position.X - RadiusX;
  double MinY = Position.Y - RadiusY;
  double MaxX = Position.X + RadiusX;
  double MaxY = Position.Y + RadiusY;

  FIntVector MinVector = GetVectorTileId(FDVector(MinX, MinY, 0));
  FIntVector MaxVector = GetVectorTileId(FDVector(MaxX, MaxY, 0));

  FIntVector CacheMinVector = GetVectorTileId(
      FDVector(Position.X - CacheRadiusX, Position.Y - CacheRadiusY, 0));
  FIntVector CacheMaxVector = GetVectorTileId(
      FDVector(Position.X + CacheRadiusX, Position.Y + CacheRadiusY, 0));

  auto IsInCacheRange = [&](int32_t Tile_X, int32_t Tile_Y) -> bool
  {
    return Tile_X >= CacheMinVector.X && Tile_X <= CacheMaxVector.X &&
           Tile_Y >= CacheMinVector.Y && Tile_Y <= CacheMaxVector.Y;
  };
  auto IsInMapRange = [&](int32_t Tile_X, int32_t Tile_Y) -> bool
  {
    return Tile_X >= MinVector.X && Tile_X <= MaxVector.X &&
           Tile_Y >= MinVector.Y && Tile_Y <= MaxVector.Y;
  };
  {
    FScopeLock ScopeLock(&Lock_Map);
    FScopeLock ScopeCacheLock(&Lock_CacheMap);
    TRACE_CPUPROFILER_EVENT_SCOPE(UpdateMap);
    // unload extra tiles
    std::vector<uint64_t> TilesToErase;
    for (auto &Element : Map)
    {
      uint64_t TileId = Element.first;
      FIntVector VectorTileId = GetVectorTileId(TileId);
      if (!IsInMapRange(VectorTileId.X, VectorTileId.Y))
      {
        CacheMap.emplace(TileId, std::move(Element.second));
        TilesToErase.emplace_back(TileId);
      }
    }
    for (uint64_t TileId : TilesToErase)
    {
      Map.erase(TileId);
    }
  }
  {
    FScopeLock ScopeCacheLock(&Lock_CacheMap);
    TRACE_CPUPROFILER_EVENT_SCOPE(UpdateCache);
    
    // unload extra tiles
    std::vector<uint64_t> TilesToErase;
    {
      TRACE_CPUPROFILER_EVENT_SCOPE(GetTilesToErase);
      for (auto &Element : CacheMap)
      {
        uint64_t TileId = Element.first;
        FIntVector VectorTileId = GetVectorTileId(TileId);
        if (!IsInCacheRange(VectorTileId.X, VectorTileId.Y))
        {
          TilesToErase.emplace_back(TileId);
        }
      }
    }

    {
      TRACE_CPUPROFILER_EVENT_SCOPE(EraseTiles);
      ParallelFor(TilesToErase.size(), [&](int32 Idx)
      {
        TRACE_CPUPROFILER_EVENT_SCOPE(SaveData);
        uint64_t TileId = TilesToErase[Idx];
        auto& Tile = CacheMap[TileId];
        std::string FileToSavePath = std::string(
              TCHAR_TO_UTF8(*( SavePath + Tile.TilePosition.ToString() + ".tile")));
        std::ofstream OutputStream(FileToSavePath.c_str());
        WriteFVector(OutputStream, Tile.TilePosition.ToFVector());
        WriteStdVector<FParticle> (OutputStream, Tile.Particles);
        OutputStream.close();
      });
      {
        TRACE_CPUPROFILER_EVENT_SCOPE(CacheMap.erase);
        for (uint64_t TileId : TilesToErase)
        {
          CacheMap.erase(TileId); 
        }
      }
    }
  }

}

void FSparseHighDetailMap::Update(FVector Position, float RadiusX, float RadiusY)
{
  FVector PositionTranslated;
  PositionTranslated.X = ( Position.X * 0.01 ) + (Extension.X * 0.5f);
  PositionTranslated.Y = (-Position.Y * 0.01 ) + ( (-Extension.Y) * 0.5f);
  PositionTranslated.Z = ( Position.Z * 0.01 ) + (Extension.Z * 0.5f);
  PositionToUpdate = PositionTranslated;

  double MinX = std::min( std::max( PositionTranslated.X - RadiusX, 0.0f) , static_cast<float>(Extension.X - 1.0f) );
  double MinY = std::min( std::max( PositionTranslated.Y - RadiusY, 0.0f) , static_cast<float>(-Extension.Y + 1.0f) );
  double MaxX = std::min( std::max( PositionTranslated.X + RadiusX, 0.0f) , static_cast<float>(Extension.X - 1.0f) );
  double MaxY = std::min( std::max( PositionTranslated.Y + RadiusY, 0.0f) , static_cast<float>(-Extension.Y + 1.0f) );

  FIntVector MinVector = GetVectorTileId(FDVector(std::floor(MinX), std::floor(MinY), 0));
  FIntVector MaxVector = GetVectorTileId(FDVector(std::floor(MaxX), std::floor(MaxY), 0));
  
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("FSparseHighDetailMap::WaitUntilProperTilesAreLoaded"));
    for(int32_t X = MinVector.X; X < MaxVector.X; X++ )
    {
      for(int32_t Y = MinVector.Y; Y < MaxVector.Y; Y++ )
      {
        bool ConditionToStopWaiting = true;
        // Check if tiles are already loaded
        // If they are send position
        // If not wait until loaded
        while(ConditionToStopWaiting) 
        {
          uint64_t CurrentTileID = GetTileId(X,Y);
          {
            TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("FSparseHighDetailMap::WaitUntilProperTilesAreLoaded::TimeLocked"));
            ConditionToStopWaiting = Map.find(CurrentTileID) == Map.end();
          }
          
        }
      }
    }
  }

}

void FSparseHighDetailMap::SaveMap()
{
  UE_LOG(LogCarla, Warning, TEXT("Save directory %s"), *SavePath );
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::SaveMap);
  ParallelFor(Map.size(), [this](int32 Idx)
  {
    std::pair<uint64_t, FDenseTile> it = *(std::next(this->Map.begin(), Idx) );
    std::string FileToSavePath = std::string(TCHAR_TO_UTF8(*( this->SavePath + it.second.TilePosition.ToString() + ".tile")));
    std::ofstream OutputStream(FileToSavePath.c_str());
    WriteFVector(OutputStream, it.second.TilePosition.ToFVector());
    WriteStdVector<FParticle> (OutputStream, it.second.Particles);
    OutputStream.close();
  });
  
  ParallelFor(CacheMap.size(), [this](int32 Idx)
  {
    std::pair<uint64_t, FDenseTile> it = *(std::next(this->CacheMap.begin(), Idx) );
    std::string FileToSavePath = std::string(TCHAR_TO_UTF8(*( this->SavePath + it.second.TilePosition.ToString() + ".tile")));
    std::ofstream OutputStream(FileToSavePath.c_str());
    WriteFVector(OutputStream, it.second.TilePosition.ToFVector());
    WriteStdVector<FParticle> (OutputStream, it.second.Particles);
    OutputStream.close();
  });

}

void UCustomTerrainPhysicsComponent::UpdateTexture()
{  
  UpdateLoadedTextureDataRegions();

  ENQUEUE_RENDER_COMMAND(UpdateDynamicTextureCode)
  (
    [NewData=Data, Texture=TextureToUpdate](auto &InRHICmdList) mutable
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UCustomTerrainPhysicsComponent::TickComponent Renderthread"));
    FUpdateTextureRegion2D region;
    region.SrcX = 0;
    region.SrcY = 0;
    region.DestX = 0;
    region.DestY = 0;
    region.Width = Texture->GetSizeX();
    region.Height = Texture->GetSizeY();

    FTexture2DResource* resource = (FTexture2DResource*)Texture->Resource;
    RHIUpdateTexture2D(
        resource->GetTexture2DRHI(), 0, region, region.Width * sizeof(uint8_t), &NewData[0]); 
  });
}

void UCustomTerrainPhysicsComponent::InitTexture(){
  if( Data.Num() == 0 && TextureToUpdate ){
    float LimitX = TextureToUpdate->GetSizeX(); 
    float LimitY = TextureToUpdate->GetSizeY();
    Data.Init(128, LimitX * LimitY);
  }

  if( LargeData.Num() == 0 && LargeTextureToUpdate ){
    float LimitX = LargeTextureToUpdate->GetSizeX(); 
    float LimitY = LargeTextureToUpdate->GetSizeY();
    LargeData.Init(128, LimitX * LimitY);
  }
}

void UCustomTerrainPhysicsComponent::UpdateLoadedTextureDataRegions()
{
  TRACE_CPUPROFILER_EVENT_SCOPE("UCustomTerrainPhysicsComponent::UpdateLoadedTextureDataRegions");
  const int32_t TextureSizeX = TextureToUpdate->GetSizeX();
  if ( TextureSizeX == 0) return;
  
  
  FDVector TextureCenterPosition = UEFrameToSI(GetTileCenter(LastUpdatedPosition));
  
  std::vector<uint64_t> LoadedTiles = 
      SparseMap.GetLoadedTilesInRange(TextureCenterPosition, TextureRadius );
  FDVector TextureOrigin = TextureCenterPosition - FDVector(TextureRadius, TextureRadius, 0);
  float GlobalTexelSize = (2.0f * TextureRadius) / TextureSizeX;
  int32_t PartialHeightMapSize = std::floor( SparseMap.GetTileSize() * TextureSizeX / (2*TextureRadius) );
  
  float LocalTexelSize = SparseMap.GetTileSize() / PartialHeightMapSize;
  LocalTexelSize = std::floor( LocalTexelSize * 1000.0f ) / 1000.0f;
  
  Data.Init( 128, Data.Num() );
  float DisplacementRange = MaxDisplacement - MinDisplacement;
  float InverseDisplacementRange = 1.0f / DisplacementRange;

  for (uint64_t TileId : LoadedTiles)
  {
    FDenseTile& CurrentTile = SparseMap.GetTile(TileId);

    if (!CurrentTile.bParticlesZOrderedInitialized)
    {
      continue;
    }

    FDVector& TilePosition = CurrentTile.TilePosition;
    for (int32_t Local_Y = 0; Local_Y < PartialHeightMapSize; ++Local_Y)
    {
      for (int32_t Local_X = 0; Local_X < PartialHeightMapSize; ++Local_X)
      {
        int32_t LocalIndex = Local_Y * PartialHeightMapSize + Local_X;
        float Height = CurrentTile.ParticlesHeightMap[LocalIndex];
        FDVector LocalTexelPosition = 
            TilePosition + FDVector(Local_X*LocalTexelSize, Local_Y*LocalTexelSize, 0);
        int32_t Coord_X = std::floor( (LocalTexelPosition.X - TextureOrigin.X ) / GlobalTexelSize );
        int32_t Coord_Y = std::floor( (LocalTexelPosition.Y - TextureOrigin.Y ) / GlobalTexelSize );
        
        if ( Coord_X >= 0 && Coord_X < TextureSizeX &&
            Coord_Y >= 0 && Coord_Y < TextureToUpdate->GetSizeY() )
        {
          float OriginalHeight = SparseMap.GetHeight(LocalTexelPosition);
          float Displacement = Height - OriginalHeight;
          float Fraction = (Displacement - MinDisplacement) * InverseDisplacementRange; 
          Fraction = FMath::Clamp(Fraction, 0.f, 1.f) * 255;
          Data[Coord_X * TextureToUpdate->GetSizeY() + Coord_Y] = static_cast<uint8_t>(Fraction );
        }
      }
    }
  }
}

void UCustomTerrainPhysicsComponent::UpdateLargeTexture()
{  
  UpdateLargeTextureData();

  ENQUEUE_RENDER_COMMAND(UpdateDynamicTextureCode)
  (
    [NewData=LargeData, Texture=LargeTextureToUpdate](auto &InRHICmdList) mutable
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UCustomTerrainPhysicsComponent::TickComponent Renderthread"));
    FUpdateTextureRegion2D region;
    region.SrcX = 0;
    region.SrcY = 0;
    region.DestX = 0;
    region.DestY = 0;
    region.Width = Texture->GetSizeX();
    region.Height = Texture->GetSizeY();

    FTexture2DResource* resource = (FTexture2DResource*)Texture->Resource;
    RHIUpdateTexture2D(
        resource->GetTexture2DRHI(), 0, region, region.Width * sizeof(uint8_t), &NewData[0]); 
  });
}

void UCustomTerrainPhysicsComponent::UpdateLargeTextureData()
{
  uint32_t NumberOfParticlesIn1AxisInLocalHeightmap = TextureToUpdate->GetSizeX() / (TextureRadius * 2) / TileSize;
  uint32_t NumberOfParticlesIn1AxisInLargeHeightmap = LargeTextureToUpdate->GetSizeX() / (CacheRadius.X * 2) / TileSize;

  std::vector<float> ParticlesPositions;
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UCustomTerrainPhysicsComponent::UpdateLargeTextureData::GettingHeightMaps"));
    FVector OriginPosition;     
    OriginPosition.X =  CachePosition.X + (WorldSize.X * 0.5f);
    OriginPosition.Y =  -CachePosition.Y - (WorldSize.Y * 0.5f);

    //UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::UpdateTextureData OriginPosition X: %f, Y %f"), OriginPosition.X, OriginPosition.Y);
  }

  int32_t OffsetX = 0;
  int32_t OffsetY = 0;
  float* Ptr = &ParticlesPositions[0];
  while( OffsetY < LargeTextureToUpdate->GetSizeY() )
  {
    for(uint32_t y = 0; y < NumberOfParticlesIn1AxisInLocalHeightmap; y++)
    {
      float LocalYTransformed = std::floor(y / NumberOfParticlesIn1AxisInLocalHeightmap * NumberOfParticlesIn1AxisInLargeHeightmap);
      for(uint32_t x = 0; x < NumberOfParticlesIn1AxisInLocalHeightmap; x++)
      {
        uint32_t LocalIndex = x + y * NumberOfParticlesIn1AxisInLocalHeightmap;
        float LocalXTransformed = std::floor(x / NumberOfParticlesIn1AxisInLocalHeightmap * NumberOfParticlesIn1AxisInLargeHeightmap);
        //uint32_t AbsolutIndex = (LocalXTransformed + OffsetX) + (LocalYTransformed + OffsetY) * 

      }
    }

    Ptr += (NumberOfParticlesIn1AxisInLocalHeightmap* NumberOfParticlesIn1AxisInLocalHeightmap);
    
    OffsetX += NumberOfParticlesIn1AxisInLargeHeightmap;
    if( OffsetX >= LargeTextureToUpdate->GetSizeX() )
    {
      OffsetX = 0;
      OffsetY += NumberOfParticlesIn1AxisInLargeHeightmap;
    }
  }
}


UCustomTerrainPhysicsComponent::UCustomTerrainPhysicsComponent()
  : Super()
{
  PrimaryComponentTick.bCanEverTick = true;
}
 
void UCustomTerrainPhysicsComponent::BeginPlay()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::BeginPlay);
  Super::BeginPlay();
  //GEngine->Exec( GetWorld(), TEXT( "Trace.Start default,gpu" ) );

  SparseMap.Clear();
  RootComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
  if (!RootComponent)
  {
    UE_LOG(LogCarla, Error, 
        TEXT("UCustomTerrainPhysicsComponent: Root component is not a UPrimitiveComponent"));
  }
#ifndef WITH_EDITOR
  bUpdateParticles = true;
  DrawDebugInfo = false;
  bUseDynamicModel = false;
  bDisableVehicleGravity = false;
  NNVerbose = false;
  bUseImpulse = false;
  bUseMeanAcceleration = false;
  bShowForces = true;
  bBenchMark = false;
  bDrawHeightMap = false;
  ForceMulFactor = 1.f;
  ParticleForceMulFactor = 1.f;
  FloorHeight = 0.0;
  bDrawLoadedTiles = false;
  bUseSoilType = true;
  EffectMultiplayer = 200.0f;
  MinDisplacement = -10.0f;
  MaxDisplacement = 10.0f;
  bRemoveLandscapeColliders = false;
#endif

  int IntValue;
  if (FParse::Value(FCommandLine::Get(), TEXT("-cuda-device="), IntValue))
  {
    CUDADevice = IntValue;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-max-particles-per-wheel="), IntValue))
  {
    MaxParticlesPerWheel = IntValue;
  }
  float Value;
  if (FParse::Value(FCommandLine::Get(), TEXT("-particle-size="), Value))
  {
    ParticleDiameter = MToCM*Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-terrain-depth="), Value))
  {
    TerrainDepth = MToCM*Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-search-radius="), Value))
  {
    SearchRadius = MToCM*Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-box-search-forward="), Value))
  {
    BoxSearchForwardDistance = MToCM*Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-box-search-lateral="), Value))
  {
    BoxSearchLateralDistance = MToCM*Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-box-search-depth="), Value))
  {
    BoxSearchDepthDistance = MToCM*Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-force-mul-factor="), Value))
  {
    ForceMulFactor = Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-defor-mul="), Value))
  {
    EffectMultiplayer = Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-defor-res="), Value))
  {
    ChosenRes = static_cast<EDefResolutionType>(Value);
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-min-displacement="), Value))
  {
    MinDisplacement = Value;
  }
   if (FParse::Value(FCommandLine::Get(), TEXT("-max-displacement="), Value))
  {
    MaxDisplacement = Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-particle-force-mul-factor="), Value))
  {
    ParticleForceMulFactor = Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-max-force="), Value))
  {
    MaxForceMagnitude = MToCM*Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-floor-height="), Value))
  {
    FloorHeight = MToCM*Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-tile-size="), Value))
  {
    TileSize = Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-tile-radius="), Value))
  {
    TileRadius = MToCM*FVector(Value, Value, 0.f);
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-cache-radius="), Value))
  {
    CacheRadius = MToCM*FVector(Value, Value, 0.f);
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-update-particles")))
  {
    bUpdateParticles = true;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-draw-debug-info")))
  {
    DrawDebugInfo = true;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-use-local-frame")))
  {
    bUseLocalFrame = true;
  }
  FString Path;
  if (FParse::Value(FCommandLine::Get(), TEXT("-network-path="), Path))
  {
    NeuralModelFile = Path;
  }
#ifndef WITH_EDITOR
  // if (Path == "")
  // {
  //   NeuralModelFile = FPaths::ProjectContentDir() + NeuralModelFile;
  // }
#endif
  if (FParse::Param(FCommandLine::Get(), TEXT("-dynamic-model")))
  {
    bUseDynamicModel = true;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-disable-gravity")))
  {
    bDisableVehicleGravity = true;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-disable-nn-verbose")))
  {
    NNVerbose = false;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-use-terrain-type")))
  {
    bUseSoilType = true;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-use-impulse")))
  { 
    bUseImpulse = true;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-use-mean-acceleration")))
  {
    bUseMeanAcceleration = true;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-hide-debug-forces")))
  {
    bShowForces = false;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-benchmark")))
  {
    bBenchMark = true;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-draw-loaded-tiles")))
  {
    bDrawLoadedTiles = true;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-remove-colliders")))
  {
    bRemoveLandscapeColliders = true;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-disable-terramechanics")))
  {
    SetComponentTickEnabled(false);
    return;
  }

  if(bRemoveLandscapeColliders)
  {
    FWorldDelegates::LevelAddedToWorld.AddUObject(
        this, &UCustomTerrainPhysicsComponent::OnLevelAddedToWorld);
  }

  LargeMapManager = UCarlaStatics::GetLargeMapManager(GetWorld());
  if( TexturesRes.Contains(ChosenRes) ){
    TextureToUpdate = TexturesRes[ChosenRes];
  }

  {
    TRACE_CPUPROFILER_EVENT_SCOPE(InitializeDenseMap);
    SparseMap.Clear();
    UE_LOG(LogCarla, Warning, 
        TEXT("ParticleDiameter %f"), ParticleDiameter);

    SparseMap.Init(TextureToUpdate->GetSizeX(), TextureRadius, ParticleDiameter * CMToM, TerrainDepth * CMToM, FloorHeight * CMToM );
    RootComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
    if(LargeMapManager)
    {
      FIntVector NumTiles = LargeMapManager->GetNumTilesInXY();
      // WorldSize = FVector(NumTiles) * LargeMap->GetTileSize();
      // UE_LOG(LogCarla, Log, 
      //     TEXT("World Size %s"), *(WorldSize.ToString()));
    }
    // SparseMap.InitializeMap(HeightMap, UEFrameToSI(Tile0Origin), UEFrameToSI(WorldSize),
    //     1.f, MinHeight, MaxHeight, HeightMapScaleFactor.Z);
    if (DataAsset)
    {
      SparseMap.InitializeMap(DataAsset, UEFrameToSI(Tile0Origin), UEFrameToSI(WorldSize),
        TileSize, HeightMapScaleFactor.Z);
    }
  }
#ifdef WITH_PYTORCH
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(LoadNNModel);
    carla::learning::test_learning();
    TerramechanicsModel.LoadModel(TCHAR_TO_ANSI(*NeuralModelFile), CUDADevice);
  }
#endif

  FString LevelName = GetWorld()->GetMapName();
  LevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
  
  SavePath = FString(_filesBaseFolder.c_str()) + LevelName + "_Terrain/";
  SparseMap.SavePath = SavePath;
  // Creating the FileManager
  IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
  if( FileManager.CreateDirectory(*SavePath)){
    UE_LOG(LogCarla, Warning, 
        TEXT("Folder was created at %s"), *SavePath);
  }else{
    UE_LOG(LogCarla, Error, 
        TEXT("Folder was not created at %s"), *SavePath);  
  }

  if(bUseDeformationPlane){
    DeformationPlaneActor = GetWorld()->SpawnActor<AStaticMeshActor>();
    
    if( DeformationPlaneActor )
    {
      DeformationPlaneActor->GetStaticMeshComponent()->SetStaticMesh( DeformationPlaneMesh );
      DeformationPlaneActor->GetStaticMeshComponent()->SetMaterial( 0, DeformationPlaneMaterial );
      DeformationPlaneActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
    }
  }
  
  InitTexture();
  
  UE_LOG(LogCarla, Log, TEXT("MainThread Data ArraySize %d "), Data.Num());
  UE_LOG(LogCarla, Log, TEXT("Map Size %d "), SparseMap.Map.size() );

  if (TilesWorker == nullptr)
  {
    TilesWorker = new FTilesWorker(this, GetOwner()->GetActorLocation(), TileRadius.X, TileRadius.Y);
    Thread = FRunnableThread::Create(TilesWorker, TEXT("TilesWorker"));
  }

}

void UCustomTerrainPhysicsComponent::BuildLandscapeHeightMapDataAasset(ALandscapeProxy* Landscape, 
    int Resolution, FVector MapSize, FString AssetPath, FString AssetName)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::BuildLandscapeHeightMapTexture);
  TArray<float> HeightMap;
  HeightMap.Reserve(Resolution*Resolution);
  FVector Origin = Landscape->GetActorLocation();
  float DeltaX = MapSize.X / Resolution;
  float DeltaY = MapSize.Y / Resolution;
  for (size_t i = 0; i < Resolution; ++i)
  {
    float PosX = Origin.X + i*DeltaX;
    for (size_t j = 0; j < Resolution; ++j)
    {
      float PosY = Origin.Y + j*DeltaY;
      FVector Location = FVector(PosX, PosY, 0);
      float Height = Landscape->GetHeightAtLocation(Location).Get(-1);
      HeightMap.Emplace(Height);
    }
  }
  int TextureWidth = Resolution;
  int TextureHeight = Resolution;
  FString PackageName = AssetPath;
  PackageName += AssetName;
  UPackage* Package = CreatePackage(NULL, *PackageName);
  Package->FullyLoad();

  UHeightMapDataAsset* HeightMapAsset = NewObject<UHeightMapDataAsset>(Package, *AssetName, RF_Public | RF_Standalone | RF_MarkAsRootSet);
  HeightMapAsset->AddToRoot();
  HeightMapAsset->SizeX = TextureWidth;
  HeightMapAsset->SizeY = TextureHeight;
  HeightMapAsset->HeightValues = HeightMap;

  Package->MarkPackageDirty();
  // FAssetRegistryModule::AssetCreated(NewTexture);

  FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
  bool bSaved = UPackage::SavePackage(Package, HeightMapAsset, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
}


float UCustomTerrainPhysicsComponent::GetHeightAtLocation(ALandscapeProxy * Landscape, FVector Location)
{
  TOptional<float> OptionalHeight =  Landscape->GetHeightAtLocation(Location);
  if(OptionalHeight.IsSet())
    return OptionalHeight.GetValue();
  return -1;
}

void UCustomTerrainPhysicsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason){
  Super::EndPlay(EndPlayReason);
  if( Thread && TilesWorker){
    TilesWorker->bShouldContinue = false;
    Thread->WaitForCompletion();
    Thread->Kill(false);
    delete TilesWorker;
    TilesWorker = nullptr;
  }

  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::SaveMap);
  SparseMap.SaveMap();
}

void UCustomTerrainPhysicsComponent::TickComponent(float DeltaTime, 
    ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::TickComponent);
  Super::TickComponent(DeltaTime,TickType,ThisTickFunction);

  TArray<AActor*> VehiclesActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACarlaWheeledVehicle::StaticClass(), VehiclesActors);
  UCarlaEpisode* Episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
  for (AActor* VehicleActor : VehiclesActors)
  {

    ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle> (VehicleActor);
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(Vehicle);
    if (!CarlaActor)
    {
      continue;
    }
    const FActorInfo* ActorInfo = CarlaActor->GetActorInfo();
    const FActorDescription& Description = ActorInfo->Description;
    const FActorAttribute* Attribute = Description.Variations.Find("terramechanics");
    // If the vehicle has terramechanics enabled
    if(!Attribute || !UActorBlueprintFunctionLibrary::ActorAttributeToBool(*Attribute, false))
    {
      continue;
    }

    FVector GlobalLocation = Vehicle->GetActorLocation();
    if(LargeMapManager)
    {
      GlobalLocation = LargeMapManager->LocalToGlobalLocation(Vehicle->GetActorLocation());

      uint64_t TileId = LargeMapManager->GetTileID(GlobalLocation);
      FIntVector CurrentTileId = 
          LargeMapManager->GetTileVectorID(GlobalLocation);
      if(CurrentLargeMapTileId != CurrentTileId)
      {
        //load new height map
        FCarlaMapTile* LargeMapTile = LargeMapManager->GetCarlaMapTile(TileId);
        if(LargeMapTile)
        {
          CurrentLargeMapTileId = CurrentTileId;
          FString FullTileNamePath = LargeMapTile->Name;
          FString TileDirectory;
          FString TileName;
          FString Extension;
          FPaths::Split(FullTileNamePath, TileDirectory, TileName, Extension);
          FString AssetPath = TileDirectory + "/HeightMaps/" + TileName + "." + TileName;
          UE_LOG(LogCarla, Log, TEXT("Enter tile %s, %s \n %s \n %s \n %s"), *CurrentTileId.ToString(), 
              *FullTileNamePath, *TileDirectory, *TileName, *Extension);

          UObject* DataAssetObject = StaticLoadObject(UHeightMapDataAsset::StaticClass(), nullptr, *(AssetPath));
          if(DataAssetObject)
          {
            UHeightMapDataAsset* HeightMapDataAsset = Cast<UHeightMapDataAsset>(DataAssetObject);
            if (HeightMapDataAsset != nullptr)
            {
              FVector TilePosition = HeightMapOffset + LargeMapManager->GetTileLocation(CurrentLargeMapTileId) - 0.5f*FVector(LargeMapManager->GetTileSize(), -LargeMapManager->GetTileSize(), 0);
              UE_LOG(LogCarla, Log, TEXT("Updating height map to location %s in tile location %s"), 
                  *TilePosition.ToString(), *LargeMapManager->GetTileLocation(CurrentLargeMapTileId).ToString());
              TilePosition.Z += UEFrameToSI(FloorHeight) ;
              SparseMap.UpdateHeightMap(
                  HeightMapDataAsset, UEFrameToSI(TilePosition), UEFrameToSI(FVector(
                    LargeMapManager->GetTileSize(),-LargeMapManager->GetTileSize(), 0)), 
                  1.f, HeightMapScaleFactor.Z);
            }
          }
        }
        else
        {/*
          UE_LOG(LogCarla, Log, TEXT("Tile not found %s %s"), 
              *GlobalLocation.ToString(), *CurrentTileId.ToString());
          */
        }
      }
    }

    SparseMap.LockMutex();
    RunNNPhysicsSimulation(Vehicle, DeltaTime);
    LastUpdatedPosition = GlobalLocation;
    SparseMap.UnLockMutex();

    if (bDrawLoadedTiles)
    {
      DrawTiles(GetWorld(), SparseMap.GetTileIdInMap(), GlobalLocation.Z + 300, FLinearColor(0.0,0.0,1.0,0.0));
      DrawTiles(GetWorld(), SparseMap.GetTileIdInCache(), GlobalLocation.Z + 300, FLinearColor(1.0,0.0,0.0,0.0));
    }

    if( MPCInstance == nullptr )
    {
      if(MPC){
        MPCInstance = GetWorld()->GetParameterCollectionInstance( MPC );
      }
    }

    if( MPCInstance ){
      MPCInstance->SetVectorParameterValue("PositionToUpdate", GetTileCenter(LastUpdatedPosition));
      // We set texture radius in cm as is UE4 default measure unit
      MPCInstance->SetScalarParameterValue("TextureRadius", TextureRadius * 100);
      MPCInstance->SetScalarParameterValue("LargeTextureRadius", TextureRadius * 100);
      MPCInstance->SetScalarParameterValue("EffectMultiplayer", EffectMultiplayer);
      MPCInstance->SetScalarParameterValue("MinDisplacement",  MinDisplacement);
      MPCInstance->SetScalarParameterValue("MaxDisplacement",  MinDisplacement);
      if(TextureToUpdate){
        MPCInstance->SetScalarParameterValue("TexSizeX", TextureToUpdate->GetSizeX()); 
      } 
    }

    if(bUseDeformationPlane){
      if( DeformationPlaneActor ){
        DeformationPlaneActor->SetActorLocation(LastUpdatedPosition, false, nullptr);
      }else{
        DeformationPlaneActor = GetWorld()->SpawnActor<AStaticMeshActor>();
    
        if( DeformationPlaneActor )
        {
          DeformationPlaneActor->GetStaticMeshComponent()->SetStaticMesh( DeformationPlaneMesh );
          DeformationPlaneActor->GetStaticMeshComponent()->SetMaterial( 0, DeformationPlaneMaterial );
          DeformationPlaneActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
        }
      }
    }
  }

  if (bDrawHeightMap)
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(DrawHeightMap);
    float LifeTime = 0.3f;
    bool bPersistentLines = false;
    bool bDepthIsForeground = (0 == SDPG_Foreground);
    UWorld* World = GetWorld();
    ULineBatchComponent* LineBatcher = 
        (World ? (bDepthIsForeground ? World->ForegroundLineBatcher : 
        (( bPersistentLines || (LifeTime > 0.f) ) ? World->PersistentLineBatcher : World->LineBatcher)) : nullptr);
    if (!LineBatcher)
    {
      UE_LOG(LogCarla, Error, TEXT("Missing linebatcher"));
    }

    float DrawPos_X = DrawStart.X;
    float DrawPos_Y = DrawStart.Y;
    for (DrawPos_X = DrawStart.X; DrawPos_X < DrawEnd.X; DrawPos_X += DrawInterval.X)
    {
      for (DrawPos_Y = DrawStart.Y; DrawPos_Y < DrawEnd.Y; DrawPos_Y += DrawInterval.Y)
      {
        float Height = SparseMap.GetHeight(UEFrameToSI(FVector(DrawPos_X, DrawPos_Y, 0)));
        FVector Point = FVector(DrawPos_X, DrawPos_Y, SIToUEFrame(Height));
        if(LargeMapManager)
        {
          Point = LargeMapManager->GlobalToLocalLocation(Point);
        }
        LineBatcher->DrawPoint(Point, 
            FLinearColor(1.f, 0.f, 0.f), 10.0, 0, LifeTime);

      }
    }    
  }
}

void UCustomTerrainPhysicsComponent::DrawParticles(UWorld* World, std::vector<FParticle*>& Particles, 
    FLinearColor Color)
{
  float LifeTime = 0.3f;
  bool bPersistentLines = false;
  bool bDepthIsForeground = (0 == SDPG_Foreground);
  ULineBatchComponent* LineBatcher = 
      (World ? (bDepthIsForeground ? World->ForegroundLineBatcher : 
      (( bPersistentLines || (LifeTime > 0.f) ) ? World->PersistentLineBatcher : World->LineBatcher)) : nullptr);
  if (!LineBatcher)
  {
    UE_LOG(LogCarla, Error, TEXT("Missing linebatcher"));
  }
  for(FParticle* Particle : Particles)
  {
    // DrawDebugPoint(World, MToCM*Particle->Position.ToFVector(),
    //     1.0, FColor(255,0,0), false, 0.3, 0);
    FVector Point = SIToUEFrame(Particle->Position.ToFVector());
    if(LargeMapManager)
    {
      Point = LargeMapManager->GlobalToLocalLocation(Point);
    }
    LineBatcher->DrawPoint(Point, 
          Color, 1.0, 0, LifeTime);
  }
}

void UCustomTerrainPhysicsComponent::DrawParticlesArray(UWorld* World, TArray<float>& ParticlesArray, 
    FLinearColor Color)
{
  float LifeTime = 0.3f;
  bool bPersistentLines = false;
  bool bDepthIsForeground = (0 == SDPG_Foreground);
  ULineBatchComponent* LineBatcher = 
      (World ? (bDepthIsForeground ? World->ForegroundLineBatcher : 
      (( bPersistentLines || (LifeTime > 0.f) ) ? World->PersistentLineBatcher : World->LineBatcher)) : nullptr);
  if (!LineBatcher)
  {
    UE_LOG(LogCarla, Error, TEXT("Missing linebatcher"));
  }
  for(int i = 0; i < ParticlesArray.Num(); i+=3)
  {
    FVector Position = FVector(ParticlesArray[i+0], ParticlesArray[i+1], ParticlesArray[i+2]);
    // DrawDebugPoint(World, MToCM*Particle->Position.ToFVector(),
    //     1.0, FColor(255,0,0), false, 0.3, 0);
    FVector Point = SIToUEFrame(Position);
    LineBatcher->DrawPoint(Point, 
          Color, 1.0, 0, LifeTime);
  }
}

void UCustomTerrainPhysicsComponent::DrawOrientedBox(UWorld* World, const TArray<FOrientedBox>& Boxes)
{
  float LifeTime = 0.3f;
  bool bPersistentLines = false;
  bool bDepthIsForeground = (0 == SDPG_Foreground);
  float Thickness = 2.0f;
  ULineBatchComponent* LineBatcher = 
      (World ? (bDepthIsForeground ? World->ForegroundLineBatcher : 
      (( bPersistentLines || (LifeTime > 0.f) ) ? World->PersistentLineBatcher : World->LineBatcher)) : nullptr);
  if (!LineBatcher)
  {
    UE_LOG(LogCarla, Error, TEXT("Missing linebatcher"));
  }
  for (const FOrientedBox& OBox : Boxes)
  {
    TArray<FVector> Vertices;
    Vertices.SetNum(8);
    OBox.CalcVertices(&Vertices[0]);
    if(LargeMapManager)
    {
      for(FVector& Point : Vertices)
      {
        Point = LargeMapManager->GlobalToLocalLocation(Point);
      }
    }
    LineBatcher->DrawLines({
        FBatchedLine(Vertices[0], Vertices[1], 
            FLinearColor(0.0,0.0,1.0), LifeTime, Thickness, 0),
        FBatchedLine(Vertices[0], Vertices[2], 
            FLinearColor(0.0,0.0,1.0), LifeTime, Thickness, 0),
        FBatchedLine(Vertices[2], Vertices[3], 
            FLinearColor(0.0,0.0,1.0), LifeTime, Thickness, 0),
        FBatchedLine(Vertices[3], Vertices[1], 
            FLinearColor(0.0,0.0,1.0), LifeTime, Thickness, 0),

        FBatchedLine(Vertices[4], Vertices[5], 
            FLinearColor(0.0,0.0,1.0), LifeTime, Thickness, 0),
        FBatchedLine(Vertices[4], Vertices[6], 
            FLinearColor(0.0,0.0,1.0), LifeTime, Thickness, 0),
        FBatchedLine(Vertices[6], Vertices[7], 
            FLinearColor(0.0,0.0,1.0), LifeTime, Thickness, 0),
        FBatchedLine(Vertices[7], Vertices[5], 
            FLinearColor(0.0,0.0,1.0), LifeTime, Thickness, 0),

        FBatchedLine(Vertices[0], Vertices[4], 
            FLinearColor(0.0,0.0,1.0), LifeTime, Thickness, 0),
        FBatchedLine(Vertices[1], Vertices[5], 
            FLinearColor(0.0,0.0,1.0), LifeTime, Thickness, 0),
        FBatchedLine(Vertices[2], Vertices[6], 
            FLinearColor(0.0,0.0,1.0), LifeTime, Thickness, 0),
        FBatchedLine(Vertices[3], Vertices[7], 
            FLinearColor(0.0,0.0,1.0), LifeTime, Thickness, 0)});
  }
}

void UCustomTerrainPhysicsComponent::DrawTiles(UWorld* World, const std::vector<uint64_t>& TilesIds,float Height,
    FLinearColor Color)
{
  float LifeTime = 0.3f;
  bool bPersistentLines = false;
  bool bDepthIsForeground = (0 == SDPG_Foreground);
  float Thickness = 2.0f;
  ULineBatchComponent* LineBatcher = 
      (World ? (bDepthIsForeground ? World->ForegroundLineBatcher : 
      (( bPersistentLines || (LifeTime > 0.f) ) ? World->PersistentLineBatcher : World->LineBatcher)) : nullptr);
  if (!LineBatcher)
  {
    UE_LOG(LogCarla, Error, TEXT("Missing linebatcher"));
  }
  // UE_LOG(LogCarla, Log, TEXT("Drawing %d Tiles"), TilesIds.size());
  for (const uint64_t& TileId : TilesIds)
  {
    FVector TileCenter = SparseMap.GetTilePosition(TileId).ToFVector();
    FVector V1 = SIToUEFrame(TileCenter);
    FVector V2 = SIToUEFrame(TileCenter + FVector(SparseMap.GetTileSize(),0,0));
    FVector V3 = SIToUEFrame(TileCenter + FVector(0,SparseMap.GetTileSize(),0));
    FVector V4 = SIToUEFrame(TileCenter + FVector(SparseMap.GetTileSize(),SparseMap.GetTileSize(),0));
    if(LargeMapManager)
    {
      V1 = LargeMapManager->GlobalToLocalLocation(V1);
      V2 = LargeMapManager->GlobalToLocalLocation(V2);
      V3 = LargeMapManager->GlobalToLocalLocation(V3);
      V4 = LargeMapManager->GlobalToLocalLocation(V4);
    }
    V1.Z = Height;
    V2.Z = Height;
    V3.Z = Height;
    V4.Z = Height;
    LineBatcher->DrawLines({
        FBatchedLine(V1, V2, 
            Color, LifeTime, Thickness, 0),
        FBatchedLine(V2, V4, 
            Color, LifeTime, Thickness, 0),
        FBatchedLine(V1, V3, 
            Color, LifeTime, Thickness, 0),
        FBatchedLine(V3, V4, 
            Color, LifeTime, Thickness, 0)});
    // UE_LOG(LogCarla, Log, TEXT("Drawing Tile %ld with verts %s, %s, %s, %s"),
    //     TileId, *V1.ToString(), *V2.ToString(), *V3.ToString(), *V4.ToString());
  }
}

void UCustomTerrainPhysicsComponent::LimitParticlesPerWheel(std::vector<FParticle*> &Particles)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(LimitParticlesPerWheel);
  if (Particles.size() < MaxParticlesPerWheel)
  {
    return;
  }
  std::sort(Particles.begin(), Particles.end(), [&](FParticle* P1, FParticle* P2) -> bool
  {
    return P1->Position.Z > P2->Position.Z;
  });
  Particles.resize(MaxParticlesPerWheel);
}

void UCustomTerrainPhysicsComponent::GenerateBenchmarkParticles(std::vector<FParticle>& BenchParticles, 
    std::vector<FParticle*> &ParticlesWheel0, std::vector<FParticle*> &ParticlesWheel1,
    std::vector<FParticle*> &ParticlesWheel2, std::vector<FParticle*> &ParticlesWheel3,
    FOrientedBox &BboxWheel0, FOrientedBox &BboxWheel1, 
    FOrientedBox &BboxWheel2, FOrientedBox &BboxWheel3)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(GenerateBenchmarkParticles);
  BenchParticles.reserve(100000);
  auto GenerateBoxParticles = [&](FOrientedBox &BboxWheel, std::vector<FParticle*> &ParticlesWheel)
  {
    ParticlesWheel.clear();
    FVector TileOrigin = FVector(0,0,0);
    FVector BoxSize = FVector(2*BboxWheel.ExtentX, 2*BboxWheel.ExtentY, BboxWheel.ExtentZ);
    uint32_t NumParticles_X = BoxSize.X / ParticleDiameter;
    uint32_t NumParticles_Y = BoxSize.Y / ParticleDiameter;
    uint32_t NumParticles_Z = BoxSize.Z / ParticleDiameter;
    UE_LOG(LogCarla, Log, TEXT("Generating (%d,%d,%d) particles"), 
        NumParticles_X,NumParticles_Y,NumParticles_Z);
    for(uint32_t i = 0; i < NumParticles_X; i++)
    {
      float XPos = i*ParticleDiameter;
      for(uint32_t j = 0; j < NumParticles_Y; j++)
      {
        float YPos = j*ParticleDiameter;
        for(uint32_t k = 0; k < NumParticles_Z; k++)
        {
          float ZPos = k*ParticleDiameter;
          FVector ParticlePosition = TileOrigin + FVector(XPos, YPos, ZPos);
          ParticlePosition = (ParticlePosition.X - BboxWheel.ExtentX) * BboxWheel0.AxisX +
                             (ParticlePosition.Y - BboxWheel.ExtentY) * BboxWheel0.AxisY +
                             (ParticlePosition.Z - BboxWheel.ExtentZ) * BboxWheel0.AxisZ + BboxWheel.Center;
          ParticlePosition = UEFrameToSI(ParticlePosition);
          BenchParticles.emplace_back(FParticle{ParticlePosition, FVector(0), ParticleDiameter/2.f});
          ParticlesWheel.emplace_back(&BenchParticles.back());
        }
      }
    }
  };
  GenerateBoxParticles(BboxWheel0, ParticlesWheel0);
  GenerateBoxParticles(BboxWheel1, ParticlesWheel1);
  GenerateBoxParticles(BboxWheel2, ParticlesWheel2);
  GenerateBoxParticles(BboxWheel3, ParticlesWheel3);

  UE_LOG(LogCarla, Log, TEXT("Generated %d particles"), BenchParticles.size());
}

void UCustomTerrainPhysicsComponent::RunNNPhysicsSimulation(
    ACarlaWheeledVehicle *Vehicle, float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(RunNNPhysicsSimulation);
  #ifdef WITH_PYTORCH
  FTransform VehicleTransform = Vehicle->GetTransform();
  FTransform WheelTransform0 = VehicleTransform;
  FTransform WheelTransform1 = VehicleTransform;
  FTransform WheelTransform2 = VehicleTransform;
  FTransform WheelTransform3 = VehicleTransform;
  FVector WheelPosition0 = VehicleTransform.TransformPosition(FVector(140, -70, 40));
  FVector WheelPosition1 = VehicleTransform.TransformPosition(FVector(140, 70, 40));
  FVector WheelPosition2 = VehicleTransform.TransformPosition(FVector(-140, -70, 40));
  FVector WheelPosition3 = VehicleTransform.TransformPosition(FVector(-140, 70, 40));
  WheelTransform0.SetLocation(WheelPosition0);
  WheelTransform1.SetLocation(WheelPosition1);
  WheelTransform2.SetLocation(WheelPosition2);
  WheelTransform3.SetLocation(WheelPosition3);
  if(LargeMapManager)
  {
    WheelPosition0 = LargeMapManager->LocalToGlobalLocation(WheelPosition0);
    WheelPosition1 = LargeMapManager->LocalToGlobalLocation(WheelPosition1);
    WheelPosition2 = LargeMapManager->LocalToGlobalLocation(WheelPosition2);
    WheelPosition3 = LargeMapManager->LocalToGlobalLocation(WheelPosition3);
  }
  FOrientedBox BboxWheel0;
  BboxWheel0.AxisX = VehicleTransform.GetUnitAxis(EAxis::X);
  BboxWheel0.AxisY = VehicleTransform.GetUnitAxis(EAxis::Y);
  BboxWheel0.AxisZ = VehicleTransform.GetUnitAxis(EAxis::Z);
  BboxWheel0.Center = WheelPosition0 + FVector(0,0,-TireRadius);
  BboxWheel0.ExtentX = BoxSearchForwardDistance;
  BboxWheel0.ExtentY = BoxSearchLateralDistance;
  BboxWheel0.ExtentZ = BoxSearchDepthDistance;
  FOrientedBox BboxWheel1 = BboxWheel0;
  BboxWheel1.Center = WheelPosition1 + FVector(0,0,-TireRadius);
  FOrientedBox BboxWheel2 = BboxWheel0;
  BboxWheel2.Center = WheelPosition2 + FVector(0,0,-TireRadius);
  FOrientedBox BboxWheel3 = BboxWheel0;
  BboxWheel3.Center = WheelPosition3 + FVector(0,0,-TireRadius);
  if (DrawDebugInfo)
  {
    DrawOrientedBox(GetWorld(), {BboxWheel0, BboxWheel1, BboxWheel2, BboxWheel3});
  }

  std::vector<FParticle*> ParticlesWheel0, ParticlesWheel1, ParticlesWheel2, ParticlesWheel3;
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(ParticleSearch);
    auto GetAndFilterParticlesInBox = 
        [&] (FOrientedBox& OBox) -> std::vector<FParticle*>
    {
      std::vector<FParticle*> Particles;
      Particles = SparseMap.GetParticlesInBox(OBox);
      LimitParticlesPerWheel(Particles);
      return Particles;
    };
    auto FutureParticles0 = Async(EAsyncExecution::ThreadPool, 
        [&]() {return GetAndFilterParticlesInBox(BboxWheel0);});
    auto FutureParticles2 = Async(EAsyncExecution::ThreadPool, 
        [&]() {return GetAndFilterParticlesInBox(BboxWheel2);});
    auto FutureParticles1 = Async(EAsyncExecution::ThreadPool, 
        [&]() {return GetAndFilterParticlesInBox(BboxWheel1);});
    auto FutureParticles3 = Async(EAsyncExecution::ThreadPool, 
        [&]() {return GetAndFilterParticlesInBox(BboxWheel3);});
    ParticlesWheel0 = FutureParticles0.Get();
    ParticlesWheel2 = FutureParticles2.Get();
    ParticlesWheel1 = FutureParticles1.Get();
    ParticlesWheel3 = FutureParticles3.Get();
  }

  std::vector<FParticle> BenchParticles;
  if(bBenchMark)
  {
    GenerateBenchmarkParticles(BenchParticles, 
        ParticlesWheel0, ParticlesWheel1, ParticlesWheel2, ParticlesWheel3,
        BboxWheel0, BboxWheel1, BboxWheel2, BboxWheel3);
    UE_LOG(LogCarla, Log, TEXT("Generated %d particles"), BenchParticles.size());
  }

  if(DrawDebugInfo)
  {
    DrawParticles(GetWorld(), ParticlesWheel0);
    DrawParticles(GetWorld(), ParticlesWheel1);
    DrawParticles(GetWorld(), ParticlesWheel2);
    DrawParticles(GetWorld(), ParticlesWheel3);
    DrawTiles(GetWorld(), SparseMap.GetIntersectingTiles(BboxWheel0), BboxWheel0.Center.Z);
    DrawTiles(GetWorld(), SparseMap.GetIntersectingTiles(BboxWheel1), BboxWheel1.Center.Z);
    DrawTiles(GetWorld(), SparseMap.GetIntersectingTiles(BboxWheel2), BboxWheel2.Center.Z);
    DrawTiles(GetWorld(), SparseMap.GetIntersectingTiles(BboxWheel3), BboxWheel3.Center.Z);
  }

  TArray<float> ParticlePos0, ParticleVel0, ParticlePos1, ParticleVel1,
                ParticlePos2, ParticleVel2, ParticlePos3, ParticleVel3;
  TArray<float> WheelPos0, WheelOrient0, WheelLinVel0, WheelAngVel0;
  TArray<float> WheelPos1, WheelOrient1, WheelLinVel1, WheelAngVel1;
  TArray<float> WheelPos2, WheelOrient2, WheelLinVel2, WheelAngVel2;
  TArray<float> WheelPos3, WheelOrient3, WheelLinVel3, WheelAngVel3;
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(SetUpArrays);
    SetUpParticleArrays(ParticlesWheel0, ParticlePos0, ParticleVel0, WheelTransform0);
    SetUpParticleArrays(ParticlesWheel1, ParticlePos1, ParticleVel1, WheelTransform1);
    SetUpParticleArrays(ParticlesWheel2, ParticlePos2, ParticleVel2, WheelTransform2);
    SetUpParticleArrays(ParticlesWheel3, ParticlePos3, ParticleVel3, WheelTransform3);

    SetUpWheelArrays(Vehicle, 0, WheelPos0, WheelOrient0, WheelLinVel0, WheelAngVel0);
    SetUpWheelArrays(Vehicle, 1, WheelPos1, WheelOrient1, WheelLinVel1, WheelAngVel1);
    SetUpWheelArrays(Vehicle, 2, WheelPos2, WheelOrient2, WheelLinVel2, WheelAngVel2);
    SetUpWheelArrays(Vehicle, 3, WheelPos3, WheelOrient3, WheelLinVel3, WheelAngVel3);
  }

  carla::learning::WheelInput Wheel0 {
      static_cast<int>(ParticlesWheel0.size()), 
      ParticlePos0.GetData(), ParticleVel0.GetData(),
      WheelPos0.GetData(), WheelOrient0.GetData(),
      WheelLinVel0.GetData(), WheelAngVel0.GetData()};
  carla::learning::WheelInput Wheel1 {
      static_cast<int>(ParticlesWheel1.size()), 
      ParticlePos1.GetData(), ParticleVel1.GetData(),
      WheelPos1.GetData(), WheelOrient1.GetData(),
      WheelLinVel1.GetData(), WheelAngVel1.GetData()};
  carla::learning::WheelInput Wheel2 {
      static_cast<int>(ParticlesWheel2.size()), 
      ParticlePos2.GetData(), ParticleVel2.GetData(),
      WheelPos2.GetData(), WheelOrient2.GetData(),
      WheelLinVel2.GetData(), WheelAngVel2.GetData()};
  carla::learning::WheelInput Wheel3 {
      static_cast<int>(ParticlesWheel3.size()), 
      ParticlePos3.GetData(), ParticleVel3.GetData(),
      WheelPos3.GetData(), WheelOrient3.GetData(),
      WheelLinVel3.GetData(), WheelAngVel3.GetData()};

  const FVehicleControl& VehicleControl = Vehicle->GetVehicleControl();
  ASoilTypeManager* SoilTypeManagerActor =  Cast<ASoilTypeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ASoilTypeManager::StaticClass()));
  if(SoilTypeManagerActor)
  {
    FSoilTerramechanicsProperties TerramechanicsProperties = 
        SoilTypeManagerActor->GetTerrainPropertiesAtGlobalLocation(WheelPosition0);

    switch(TerramechanicsProperties.TerrainType){
      case ESoilTerramechanicsType::DESERT:
        SoilType = 0;
        break;
      case ESoilTerramechanicsType::FOREST:
        SoilType = 1;
        break;
      default:
        SoilType = 0;
    }

  }
  carla::learning::Inputs NNInput {Wheel0,Wheel1,Wheel2,Wheel3, 
      VehicleControl.Steer, VehicleControl.Throttle, VehicleControl.Brake, 
      SoilType, NNVerbose};
  if (!bUseSoilType)
  {
    NNInput.terrain_type = -1;
  }
  if (VehicleControl.bReverse)
  {
    NNInput.throttle *= -1;
  }
  TerramechanicsModel.SetInputs(NNInput);
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(RunModel);
    if(bUseCUDAModel)
    {
      TerramechanicsModel.ForwardCUDATensors();
    }
    else if(bUseDynamicModel)
    {
      TerramechanicsModel.ForwardDynamic();
    }
    else 
    {
      TerramechanicsModel.Forward();
    }
  }
  carla::learning::Outputs& Output = TerramechanicsModel.GetOutputs();

  if(bUpdateParticles)
  {
    {
      TRACE_CPUPROFILER_EVENT_SCOPE(UpdateParticles);
      FScopeLock ScopeLock(&SparseMap.Lock_Particles);
      auto UpdateFutureParticles = 
      [&] (std::vector<FParticle*>& Particles, std::vector<float>& Forces, float DeltaTime,
          const FTransform& WheelTransform)
      {
        UpdateParticles( Particles, Forces, DeltaTime, WheelTransform );
      };
      UpdateFutureParticles(
          ParticlesWheel0, Output.wheel0._particle_forces, DeltaTime, WheelTransform0);
      UpdateFutureParticles(
          ParticlesWheel1, Output.wheel1._particle_forces, DeltaTime, WheelTransform1);
      UpdateFutureParticles(
          ParticlesWheel2, Output.wheel2._particle_forces, DeltaTime, WheelTransform2);
      UpdateFutureParticles(
          ParticlesWheel3, Output.wheel3._particle_forces, DeltaTime, WheelTransform3);
    }
    if (DrawDebugInfo)
    {
      FLinearColor Color(1.0,0.0,1.0,1.0);
      DrawParticles(GetWorld(), ParticlesWheel0, Color);
      DrawParticles(GetWorld(), ParticlesWheel1, Color);
      DrawParticles(GetWorld(), ParticlesWheel2, Color);
      DrawParticles(GetWorld(), ParticlesWheel3, Color);
    }
  }

  if(bUseMeanAcceleration)
  {
    ApplyMeanAccelerationToVehicle(Vehicle, 
        ForceMulFactor*SIToUEFrame(FVector(
            Output.wheel0.wheel_forces_x,
            Output.wheel0.wheel_forces_y,
            Output.wheel0.wheel_forces_z)),
        ForceMulFactor*SIToUEFrame(FVector(
            Output.wheel1.wheel_forces_x,
            Output.wheel1.wheel_forces_y,
            Output.wheel1.wheel_forces_z)),
        ForceMulFactor*SIToUEFrame(FVector(
            Output.wheel2.wheel_forces_x,
            Output.wheel2.wheel_forces_y,
            Output.wheel2.wheel_forces_z)),
        ForceMulFactor*SIToUEFrame(FVector(
            Output.wheel3.wheel_forces_x,
            Output.wheel3.wheel_forces_y,
            Output.wheel3.wheel_forces_z)));
  }
  else
  {
    ApplyForcesToVehicle(Vehicle, 
        ForceMulFactor*SIToUEFrame(FVector(
            Output.wheel0.wheel_forces_x,
            Output.wheel0.wheel_forces_y,
            Output.wheel0.wheel_forces_z)),
        ForceMulFactor*SIToUEFrame(FVector(
            Output.wheel1.wheel_forces_x,
            Output.wheel1.wheel_forces_y,
            Output.wheel1.wheel_forces_z)),
        ForceMulFactor*SIToUEFrame(FVector(
            Output.wheel2.wheel_forces_x,
            Output.wheel2.wheel_forces_y,
            Output.wheel2.wheel_forces_z)),
        ForceMulFactor*SIToUEFrame(FVector(
            Output.wheel3.wheel_forces_x,
            Output.wheel3.wheel_forces_y,
            Output.wheel3.wheel_forces_z)),
        ForceMulFactor*MToCM*SIToUEFrame(FVector(
            Output.wheel0.wheel_torque_x,
            Output.wheel0.wheel_torque_y,
            Output.wheel0.wheel_torque_z)),
        ForceMulFactor*MToCM*SIToUEFrame(FVector(
            Output.wheel1.wheel_torque_x,
            Output.wheel1.wheel_torque_y,
            Output.wheel1.wheel_torque_z)),
        ForceMulFactor*MToCM*SIToUEFrame(FVector(
            Output.wheel2.wheel_torque_x,
            Output.wheel2.wheel_torque_y,
            Output.wheel2.wheel_torque_z)),
        ForceMulFactor*MToCM*SIToUEFrame(FVector(
            Output.wheel3.wheel_torque_x,
            Output.wheel3.wheel_torque_y,
            Output.wheel3.wheel_torque_z)));
  }
  #endif
}

void UCustomTerrainPhysicsComponent::UpdateParticles(
    std::vector<FParticle*> Particles, std::vector<float> Forces,
    float DeltaTime, const FTransform& WheelTransform)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UpdateParticles);
  //UE_LOG(LogCarla, Log, TEXT("%d vs %d"), Particles.size(), Forces.size()/3);
  if(bUseLocalFrame)
  {
    for (size_t i = 0; i < Particles.size(); i++)
    {
      FVector Force = FVector(Forces[3*i + 0], Forces[3*i + 1], Forces[3*i + 2]) * ParticleForceMulFactor;
      FVector LocalAcceleration = Force;
      FVector UELocalAcceleration = SIToUEFrame(LocalAcceleration);
      FVector UEGlobalAcceleration = WheelTransform.TransformVector(UELocalAcceleration);
      FVector Acceleration = UEFrameToSI(UEGlobalAcceleration);
      FParticle* P = Particles[i];
      P->Velocity = P->Velocity + Acceleration*DeltaTime;
      P->Position = P->Position + P->Velocity*DeltaTime;
    }
  }
  else
  {
    for (size_t i = 0; i < Particles.size(); i++)
    {
      FVector Force = FVector(Forces[3*i + 0], Forces[3*i + 1], Forces[3*i + 2]) * ParticleForceMulFactor;
      FParticle* P = Particles[i];
      FVector Acceleration = Force;
      P->Velocity = P->Velocity + Acceleration*DeltaTime;
      P->Position = P->Position + P->Velocity*DeltaTime;
    }
  }
}

void UCustomTerrainPhysicsComponent::UpdateParticlesDebug( std::vector<FParticle*> Particles)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UpdateParticles);
  float DeltaTime = GetWorld()->GetDeltaSeconds();
  for (size_t i = 0; i < Particles.size(); i++)
  {
    FVector Force = FVector( 0, 0, FMath::RandRange(-1.0f, 1.0f));
    FParticle* P = Particles[i];
    FVector Acceleration = Force;
    P->Velocity = P->Velocity + Acceleration*DeltaTime;
    P->Position = P->Position + P->Velocity*DeltaTime;
  }
}

void UCustomTerrainPhysicsComponent::OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld)
{
  if( bRemoveLandscapeColliders )
  {
    for(auto CurrentActor : InLevel->Actors)
    {
      if( ALandscape* CurrentLandscape = Cast<ALandscape>( CurrentActor )  )
      {
        CurrentLandscape->BodyInstance.ReplaceResponseToChannels(  ECollisionResponse::ECR_Block, ECollisionResponse::ECR_Ignore );
        CurrentLandscape->BodyInstance.ReplaceResponseToChannels(  ECollisionResponse::ECR_Overlap, ECollisionResponse::ECR_Ignore );
        CurrentLandscape->BodyInstance.SetCollisionEnabled( ECollisionEnabled::Type::NoCollision, true);
        
        for(auto CurrentCollision : CurrentLandscape->CollisionComponents){
          CurrentCollision->SetCollisionResponseToAllChannels( ECollisionResponse::ECR_Ignore );
          CurrentCollision->SetCollisionEnabled( ECollisionEnabled::Type::NoCollision );

        }

        for(auto CurrentComponent : CurrentLandscape->LandscapeComponents){
          CurrentComponent->SetCollisionResponseToAllChannels( ECollisionResponse::ECR_Ignore );
          CurrentComponent->SetCollisionEnabled( ECollisionEnabled::Type::NoCollision );
        
        }
      }
    }
  }
}

void UCustomTerrainPhysicsComponent::UpdateTilesHeightMaps(
    const std::vector<FParticle*>& Particles)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UpdateTilesHeightMaps);

  uint32_t PartialHeightMapSize = 
      SparseMap.GetTileSize() * TextureToUpdate->GetSizeX() / (2*TextureRadius);
  float InverseTileSize = 1.f/SparseMap.GetTileSize();
  float Transformation = InverseTileSize * PartialHeightMapSize;
  float ParticlesInARowInHeightMap = TextureToUpdate->GetSizeX() / (TextureRadius * 2);
  for (size_t i = 0; i < Particles.size(); i++)
  {
    const FParticle* P = Particles[i];
    uint64_t TileId = SparseMap.GetTileId(P->Position);
    FDenseTile& CurrentTile = SparseMap.GetTile(TileId);
    FDVector TilePosition = CurrentTile.TilePosition;
    FDVector ParticleLocalPosition = P->Position - TilePosition;
    FIntVector HeightMapCoords = FIntVector(
        ParticleLocalPosition.X * Transformation,
        ParticleLocalPosition.Y * Transformation, 0);

    HeightMapCoords.X = std::min( std::max( HeightMapCoords.X, 0) , static_cast<int>(PartialHeightMapSize-1));
    HeightMapCoords.Y = std::min( std::max( HeightMapCoords.Y, 0) , static_cast<int>(PartialHeightMapSize-1));
    uint32_t Index = HeightMapCoords.Y * PartialHeightMapSize + HeightMapCoords.X;
    // Compare to the current value, if higher replace 
    if( P->Position.Z > CurrentTile.ParticlesHeightMap[Index] )
      CurrentTile.ParticlesHeightMap[Index] = P->Position.Z;
  }
}

void UCustomTerrainPhysicsComponent::RemoveParticlesFromOrderedContainer(
    const std::vector<FParticle*>& Particles)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(RemoveParticlesFromOrderedContainer);
  
  uint32_t PartialHeightMapSize = 
      SparseMap.GetTileSize() * TextureToUpdate->GetSizeX() / (2*TextureRadius);
  float InverseTileSize = 1.f/SparseMap.GetTileSize();
  float Transformation = InverseTileSize * PartialHeightMapSize;
  float ParticlesInARowInHeightMap = TextureToUpdate->GetSizeX() / (TextureRadius * 2);
  for (size_t i = 0; i < Particles.size(); i++)
  {
    const FParticle* P = Particles[i];
    uint64_t TileId = SparseMap.GetTileId(P->Position);
    FDenseTile& CurrentTile = SparseMap.GetTile(TileId);
    CurrentTile.bHeightmapNeedToUpdate = true;
    FDVector TilePosition = CurrentTile.TilePosition;
    FDVector ParticleLocalPosition = P->Position - TilePosition;
    FIntVector HeightMapCoords = FIntVector(
        ParticleLocalPosition.X * Transformation,
        ParticleLocalPosition.Y * Transformation, 0);
    
    HeightMapCoords.X = std::min( std::max( HeightMapCoords.X, 0) , static_cast<int>(PartialHeightMapSize-1));
    HeightMapCoords.Y = std::min( std::max( HeightMapCoords.Y, 0) , static_cast<int>(PartialHeightMapSize-1));
    // uint32_t Index = std::floor(ParticleLocalPosition.Y) * PartialHeightMapSize + std::floor(ParticleLocalPosition.X);
    uint32_t Index = HeightMapCoords.Y * PartialHeightMapSize + HeightMapCoords.X;
    
    if( Index < CurrentTile.ParticlesZOrdered.size() )
    {
      if( CurrentTile.ParticlesZOrdered[Index].size() > 1 )
      {
        auto it = CurrentTile.ParticlesZOrdered[Index].find(P->Position.Z );
        if( it != CurrentTile.ParticlesZOrdered[Index].end() )
        {
          CurrentTile.ParticlesZOrdered[Index].erase(it);
        }
        else
        {
          //UE_LOG(LogCarla, Error, TEXT("Cannot find in %d, position %f"), Index, P->Position.Z  );
        }       
      }
      else
      {
        UE_LOG(LogCarla, Error, TEXT("Cannot Remove more from %d, currentsize %d Tile : %s"), Index,CurrentTile.ParticlesZOrdered[Index].size(), *(TilePosition.ToString()) );
      }
    }
    else
    {
      UE_LOG(LogCarla, Error, TEXT("RemoveParticlesFromOrderedContainer Invalid Index %d ZOrderedSize %d Tile: %s"), Index,CurrentTile.ParticlesZOrdered.size(), *(TilePosition.ToString()) );
    }
  }
}

void UCustomTerrainPhysicsComponent::AddParticlesToOrderedContainer(
    const std::vector<FParticle*>& Particles)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AddParticlesToOrderedContainer);
  
  uint32_t PartialHeightMapSize = 
    SparseMap.GetTileSize() * TextureToUpdate->GetSizeX() / (2*TextureRadius);
  float InverseTileSize = 1.f/SparseMap.GetTileSize();
  float Transformation = InverseTileSize * PartialHeightMapSize;
  float ParticlesInARowInHeightMap = TextureToUpdate->GetSizeX() / (TextureRadius * 2);
  for (size_t i = 0; i < Particles.size(); i++)
  {
    const FParticle* P = Particles[i];
    uint64_t TileId = SparseMap.GetTileId(P->Position);
    FDenseTile& CurrentTile = SparseMap.GetTile(TileId);
    CurrentTile.bHeightmapNeedToUpdate = true;
    FDVector TilePosition = CurrentTile.TilePosition;
    FDVector ParticleLocalPosition = P->Position - TilePosition;
    FIntVector HeightMapCoords = FIntVector(
        std::floor(ParticleLocalPosition.X * Transformation),
        std::floor(ParticleLocalPosition.Y * Transformation), 0);
    //HeightMapCoords.X = std::min( std::max( HeightMapCoords.X, 0) , static_cast<int>(PartialHeightMapSize-1));
    //HeightMapCoords.Y = std::min( std::max( HeightMapCoords.Y, 0) , static_cast<int>(PartialHeightMapSize-1));
    uint32_t Index = HeightMapCoords.Y * PartialHeightMapSize + HeightMapCoords.X;
    if(  Index < CurrentTile.ParticlesZOrdered.size()  )
    {
      float CurrentHeight = *( CurrentTile.ParticlesZOrdered[Index].begin() );
      if( P->Position.Z - CurrentHeight < UEFrameToSI( ParticleDiameter ) * 2.0f )
      {
        CurrentTile.ParticlesZOrdered[Index].insert(P->Position.Z);
      }
    }else{
      UE_LOG(LogCarla, Error, TEXT("RemoveParticlesFromOrderedContainer Invalid Index %d ZOrderedSize %d Tile: %s"), Index,CurrentTile.ParticlesZOrdered.size(), *(TilePosition.ToString()) );
    
    }
  }
}

void UCustomTerrainPhysicsComponent::FlagTilesToRedoOrderedContainer(
    const std::vector<FParticle*>& Particles)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AddParticlesToOrderedContainer);
  
  uint32_t PartialHeightMapSize = 
    SparseMap.GetTileSize() * TextureToUpdate->GetSizeX() / (2*TextureRadius);
  float InverseTileSize = 1.f/SparseMap.GetTileSize();
  float Transformation = InverseTileSize * PartialHeightMapSize;
  float ParticlesInARowInHeightMap = TextureToUpdate->GetSizeX() / (TextureRadius * 2);
  for (size_t i = 0; i < Particles.size(); i++)
  {
    const FParticle* P = Particles[i];
    uint64_t TileId = SparseMap.GetTileId(P->Position);
    FDenseTile& CurrentTile = SparseMap.GetTile(TileId);
    CurrentTile.bHeightmapNeedToUpdate = true;
    CurrentTile.bParticlesZOrderedInitialized = false;
  }
}


void UCustomTerrainPhysicsComponent::UpdateTilesHeightMapsInRadius(FDVector Position, uint32 Rad )
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UpdateTilesHeightMapsInRadius);

  uint64_t TileId = SparseMap.GetTileId( UEFrameToSI(Position.ToFVector() ) );
  uint32_t Tile_X = (uint32_t)(TileId >> 32);
  uint32_t Tile_Y = (uint32_t)(TileId & (uint32_t)(~0));
  uint32_t RadiusInTiles = (Rad/TileSize);

  FDVector Extension = UEFrameToSI(WorldSize);
  uint32_t MinX = Tile_X - RadiusInTiles;
  uint32_t MinY = Tile_Y - RadiusInTiles;
  uint32_t MaxX = Tile_X + RadiusInTiles;
  uint32_t MaxY = Tile_Y + RadiusInTiles;
  
  for( uint32_t X = MinX; X <= MaxX; ++X )
  {
    for( uint32_t Y = MinY; Y <= MaxY; ++Y )
    {
      uint64_t CurrentTileId = SparseMap.GetTileId(X,Y);
      if( SparseMap.Map.count(CurrentTileId) )
      {
        SparseMap.GetTile(X, Y).UpdateLocalHeightmap();
      }
    }
  }
}

void UCustomTerrainPhysicsComponent::AddForceToSingleWheel( USkeletalMeshComponent* SkeletalMeshComponent, FVector WheelPosition, FVector WheelNormalForce )
{
  FVector WheelBottomLocation = WheelPosition - FVector(0,0, 0.337);
  float OriginalHeight = SparseMap.GetHeight(WheelPosition);
  float FloorHeight = OriginalHeight - UEFrameToSI(TerrainDepth);
  
  if( WheelNormalForce.Size() == 0 ){
    WheelNormalForce = FVector::UpVector;
  }

  float ForceFactor = ( WheelBottomLocation.Z - OriginalHeight ) / ( FloorHeight - OriginalHeight );
  if( ForceFactor < 0){
    ForceFactor = 0;
  }

  SkeletalMeshComponent->AddForceAtLocationLocal(WheelPosition, SIToUEFrame(WheelNormalForce) * ( ForceFactor * NormalForceIntensity) );
}

void UCustomTerrainPhysicsComponent::ApplyForcesToVehicle(
    ACarlaWheeledVehicle *Vehicle,
    FVector ForceWheel0, FVector ForceWheel1, FVector ForceWheel2, FVector ForceWheel3,
    FVector TorqueWheel0, FVector TorqueWheel1, FVector TorqueWheel2, FVector TorqueWheel3)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::ApplyForcesToVehicle);

  FTransform VehicleTransform = Vehicle->GetTransform();
  FVector WheelPosition0 = VehicleTransform.TransformPosition(FVector(140, -70, 40));
  FVector WheelPosition1 = VehicleTransform.TransformPosition(FVector(140, 70, 40));
  FVector WheelPosition2 = VehicleTransform.TransformPosition(FVector(-140, -70, 40));
  FVector WheelPosition3 = VehicleTransform.TransformPosition(FVector(-140, 70, 40));
  UPrimitiveComponent* PrimitiveComponent = 
      Cast<UPrimitiveComponent>(Vehicle->GetRootComponent());
  if(!PrimitiveComponent)
  {
    UE_LOG(LogCarla, Error, TEXT("ApplyForcesToVehicle Vehicle does not contain UPrimitiveComponent"));
    return;
  }
  if(bDisableVehicleGravity && PrimitiveComponent->IsGravityEnabled())
  {
    PrimitiveComponent->SetEnableGravity(false);
  }
  if(bUseLocalFrame)
  {
    ForceWheel0 = VehicleTransform.TransformVector(ForceWheel0);
    ForceWheel1 = VehicleTransform.TransformVector(ForceWheel1);
    ForceWheel2 = VehicleTransform.TransformVector(ForceWheel2);
    ForceWheel3 = VehicleTransform.TransformVector(ForceWheel3);
  }
  ForceWheel0 = ForceWheel0.GetClampedToMaxSize(MaxForceMagnitude);
  ForceWheel1 = ForceWheel1.GetClampedToMaxSize(MaxForceMagnitude);
  ForceWheel2 = ForceWheel2.GetClampedToMaxSize(MaxForceMagnitude);
  ForceWheel3 = ForceWheel3.GetClampedToMaxSize(MaxForceMagnitude);

  if(bUseImpulse)
  {
    PrimitiveComponent->AddImpulseAtLocation(ForceWheel0, WheelPosition0);
    PrimitiveComponent->AddImpulseAtLocation(ForceWheel1, WheelPosition1);
    PrimitiveComponent->AddImpulseAtLocation(ForceWheel2, WheelPosition2);
    PrimitiveComponent->AddImpulseAtLocation(ForceWheel3, WheelPosition3);
  }
  else
  {
    PrimitiveComponent->AddForceAtLocationLocal(ForceWheel0, FVector(140, -70, 40));
    PrimitiveComponent->AddForceAtLocationLocal(ForceWheel1, FVector(140, 70, 40));
    PrimitiveComponent->AddForceAtLocationLocal(ForceWheel2, FVector(-140, -70, 40));
    PrimitiveComponent->AddForceAtLocationLocal(ForceWheel3, FVector(-140, 70, 40));
  }

  PrimitiveComponent->AddTorqueInRadians(TorqueWheel0);
  PrimitiveComponent->AddTorqueInRadians(TorqueWheel1);
  PrimitiveComponent->AddTorqueInRadians(TorqueWheel2);
  PrimitiveComponent->AddTorqueInRadians(TorqueWheel3);
  if (DrawDebugInfo && bShowForces)
  {
    float LifeTime = 0.3f;
    bool bPersistentLines = false;
    bool bDepthIsForeground = (0 == SDPG_Foreground);
    UWorld * World = GetWorld();
    ULineBatchComponent* LineBatcher = 
        (World ? (bDepthIsForeground ? World->ForegroundLineBatcher : 
        (( bPersistentLines || (LifeTime > 0.f) ) ? World->PersistentLineBatcher : World->LineBatcher)) : nullptr);
    if (!LineBatcher)
    {
      UE_LOG(LogCarla, Error, TEXT("Missing linebatcher"));
    }
    LineBatcher->DrawLine(WheelPosition0 + FVector(0,0,50),
        WheelPosition0 + FVector(0,0,50)+ ForceWheel0.GetSafeNormal()*(5 + ForceMulFactor*10*ForceWheel0.Size()),
        FLinearColor(0.0,1.0,0.0), 0, 3.0, 0.3);
    LineBatcher->DrawLine(WheelPosition1 + FVector(0,0,50),
        WheelPosition1 + FVector(0,0,50)+ ForceWheel1.GetSafeNormal()*(5 + ForceMulFactor*10*ForceWheel1.Size()),
        FLinearColor(0.0,1.0,0.0), 0, 3.0, 0.3);
    LineBatcher->DrawLine(WheelPosition2 + FVector(0,0,50),
        WheelPosition2 + FVector(0,0,50)+ ForceWheel2.GetSafeNormal()*(5 + ForceMulFactor*10*ForceWheel2.Size()),
        FLinearColor(0.0,1.0,0.0), 0, 3.0, 0.3);
    LineBatcher->DrawLine(WheelPosition3 + FVector(0,0,50),
        WheelPosition3 + FVector(0,0,50)+ ForceWheel3.GetSafeNormal()*(5 + ForceMulFactor*10*ForceWheel3.Size()),
        FLinearColor(0.0,1.0,0.0), 0, 3.0, 0.3);
  }
}

void UCustomTerrainPhysicsComponent::ApplyMeanAccelerationToVehicle(
      ACarlaWheeledVehicle *Vehicle,
      FVector ForceWheel0, FVector ForceWheel1, FVector ForceWheel2, FVector ForceWheel3)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::ApplyForcesToVehicle);
  FVector MeanAcceleration = (ForceWheel0 + ForceWheel1 + ForceWheel2 + ForceWheel3)/4.f;
  UPrimitiveComponent* PrimitiveComponent = 
      Cast<UPrimitiveComponent>(Vehicle->GetRootComponent());
  if (!PrimitiveComponent)
  {
    UE_LOG(LogCarla, Error, TEXT("ApplyMeanAccelerationToVehicle Vehicle does not contain UPrimitiveComponent"));
    return;
  }
  if(bDisableVehicleGravity && PrimitiveComponent->IsGravityEnabled())
  {
    PrimitiveComponent->SetEnableGravity(false);
  }
  if(bUseLocalFrame)
  {
    FTransform VehicleTransform = Vehicle->GetTransform();
    MeanAcceleration = VehicleTransform.TransformVector(MeanAcceleration);
  }
  PrimitiveComponent->AddForce(MeanAcceleration, FName(""), true);

  if (DrawDebugInfo && bShowForces)
  {
    float LifeTime = 0.3f;
    bool bPersistentLines = false;
    bool bDepthIsForeground = (0 == SDPG_Foreground);
    UWorld * World = GetWorld();
    ULineBatchComponent* LineBatcher = 
        (World ? (bDepthIsForeground ? World->ForegroundLineBatcher : 
        (( bPersistentLines || (LifeTime > 0.f) ) ? World->PersistentLineBatcher : World->LineBatcher)) : nullptr);
    if (!LineBatcher)
    {
      UE_LOG(LogCarla, Error, TEXT("Missing linebatcher"));
    }
    LineBatcher->DrawLine(PrimitiveComponent->GetComponentLocation() + FVector(0,0,200),
        PrimitiveComponent->GetComponentLocation() + FVector(0,0,200)+ MeanAcceleration.GetSafeNormal()*(1 + MeanAcceleration.Size()),
        FLinearColor(0.0,1.0,0.0), 0, 3.0, 0.3);
  }
}

TArray<FVector> UCustomTerrainPhysicsComponent::GetParticlesInRadius(FVector Position, float Radius)
{
  std::vector<FParticle*> Particles = SparseMap.GetParticlesInRadius(UEFrameToSI(Position), Radius*CMToM);
  TArray<FVector> ParticlePositions;
  for(FParticle* Particle : Particles)
  {
    ParticlePositions.Add(SIToUEFrame(Particle->Position.ToFVector()));
  }
  return ParticlePositions;
}

TArray<FVector> UCustomTerrainPhysicsComponent::GetParticlesInTileRadius(FVector Position, float Radius)
{
  std::vector<FParticle*> Particles = SparseMap.GetParticlesInTileRadius(UEFrameToSI(Position), Radius*CMToM);
  TArray<FVector> ParticlePositions;
  for(FParticle* Particle : Particles)
  {
    ParticlePositions.Add(SIToUEFrame(Particle->Position.ToFVector()));
  }
  return ParticlePositions;
}


FVector UCustomTerrainPhysicsComponent::GetTileCenter(FVector Position)
{
  return SIToUEFrame(
      SparseMap.GetTilePosition(
          SparseMap.GetTileId(
              UEFrameToSI(Position))).ToFVector());
}

void UCustomTerrainPhysicsComponent::SetUpParticleArrays(std::vector<FParticle*>& ParticlesIn, 
    TArray<float>& ParticlePosOut, 
    TArray<float>& ParticleVelOut,
    const FTransform &WheelTransform)
{
  ParticlePosOut.Empty();
  ParticleVelOut.Empty();
  ParticlePosOut.Reserve(ParticlesIn.size()*3);
  ParticleVelOut.Reserve(ParticlesIn.size()*3);
  if(bUseLocalFrame)
  {
    const FTransform InverseTransform = WheelTransform.Inverse();
    for(FParticle* Particle : ParticlesIn)
    {
      FVector UEPosition = SIToUEFrame(Particle->Position.ToFVector());
      FVector UELocalPosition = InverseTransform.TransformPosition(UEPosition);
      FVector Position = UEFrameToSI(UELocalPosition);
      ParticlePosOut.Add(static_cast<float>(Position.X));
      ParticlePosOut.Add(static_cast<float>(Position.Y));
      ParticlePosOut.Add(static_cast<float>(Position.Z));
      ParticleVelOut.Add(Particle->Velocity.X);
      ParticleVelOut.Add(Particle->Velocity.Y);
      ParticleVelOut.Add(Particle->Velocity.Z);
    }
    if(ParticlesIn.size() < MaxParticlesPerWheel)
    {
      for (int i = 0; i < (MaxParticlesPerWheel - ParticlesIn.size()); ++i)
      {
        ParticlePosOut.Add(0.f);
        ParticlePosOut.Add(0.f);
        ParticlePosOut.Add(0.f);
        ParticleVelOut.Add(0.f);
        ParticleVelOut.Add(0.f);
        ParticleVelOut.Add(0.f);
      }
    }
  }
  else
  {
    for(FParticle* Particle : ParticlesIn)
    {
      ParticlePosOut.Add(static_cast<float>(Particle->Position.X));
      ParticlePosOut.Add(static_cast<float>(Particle->Position.Y));
      ParticlePosOut.Add(static_cast<float>(Particle->Position.Z));
      ParticleVelOut.Add(Particle->Velocity.X);
      ParticleVelOut.Add(Particle->Velocity.Y);
      ParticleVelOut.Add(Particle->Velocity.Z);
    }
    if(ParticlesIn.size() < MaxParticlesPerWheel)
    {
      FVector WheelPosition = UEFrameToSI(WheelTransform.GetLocation());
      for (int i = 0; i < (MaxParticlesPerWheel - ParticlesIn.size()); ++i)
      {
        ParticlePosOut.Add(WheelPosition.X);
        ParticlePosOut.Add(WheelPosition.Y);
        ParticlePosOut.Add(WheelPosition.Z);
        ParticleVelOut.Add(0.f);
        ParticleVelOut.Add(0.f);
        ParticleVelOut.Add(0.f);
      }
    }
  }
}

void UCustomTerrainPhysicsComponent::SetUpWheelArrays(ACarlaWheeledVehicle *Vehicle, int WheelIdx,
      TArray<float>& WheelPos, 
      TArray<float>& WheelOrientation, 
      TArray<float>& WheelLinearVelocity, 
      TArray<float>& WheelAngularVelocity)
{
  FTransform VehicleTransform = Vehicle->GetTransform();
  FVector Position;
  //placeholder
  switch (WheelIdx)
  {
    case 0:
      Position = FVector(140, -70, 40);
      break;
    case 1:
      Position = FVector(140, 70, 40);
      break;
    case 2:
      Position = FVector(-140, -70, 40);
      break;
    case 3:
    default:
      Position = FVector(-140, 70, 40);
      break;
  }
  FVector PhysAngularVelocity = Vehicle->GetMesh()->GetPhysicsAngularVelocityInRadians();
  //UE_LOG(LogCarla, Log, TEXT("AngVel: %s"), *PhysAngularVelocity.ToString());
  if (bUseLocalFrame)
  {
    FTransform InverseTransform = VehicleTransform.Inverse();
    Position = FVector(0,0,0);
    FVector Velocity = UEFrameToSI(
        InverseTransform.TransformVector(Vehicle->GetVelocity()));
    WheelPos = {Position.X, Position.Y, Position.Z};
    // convert to SI
    WheelOrientation = {1.f, 0.f, 0.f, 0.f};
    WheelLinearVelocity = {Velocity.X, Velocity.Y, Velocity.Z};
    FVector LocalAngularVelocity = InverseTransform.TransformVector(PhysAngularVelocity);
    //UE_LOG(LogCarla, Log, TEXT("Local Total AngVel: %s"), *LocalAngularVelocity.ToString());
    float ForwardSpeed = Velocity.X;
    float AngularSpeed = (ForwardSpeed/(CMToM*TireRadius));
    WheelAngularVelocity = {
        LocalAngularVelocity.X, 
        AngularSpeed + LocalAngularVelocity.Y, 
        -LocalAngularVelocity.Z};
  }
  else
  {
    Position = VehicleTransform.TransformPosition(Position);
    if(LargeMapManager)
    {
      Position = LargeMapManager->LocalToGlobalLocation(Position);
    }
    Position = UEFrameToSI(Position);
    float ForwardSpeed = FVector::DotProduct(
        Vehicle->GetVelocity(),VehicleTransform.GetRotation().GetForwardVector());
    FVector Velocity = UEFrameToSI(Vehicle->GetVelocity());
    WheelPos = {Position.X, Position.Y, Position.Z};
    FQuat Quat = VehicleTransform.GetRotation();
    //UE_LOG(LogCarla, Log, TEXT("Quat: %s"), *Quat.ToString());
    // convert to SI
    WheelOrientation = {Quat.W,Quat.X,-Quat.Y,Quat.Z};
    WheelLinearVelocity = {Velocity.X, Velocity.Y, Velocity.Z};
    float AngularSpeed = (ForwardSpeed)/(TireRadius);
    FVector GlobalAngulaSpeed = VehicleTransform.TransformVector(FVector(0, AngularSpeed, 0));
    PhysAngularVelocity = PhysAngularVelocity + GlobalAngulaSpeed;
    WheelAngularVelocity = {
        PhysAngularVelocity.X, 
        PhysAngularVelocity.Y, 
        -PhysAngularVelocity.Z};
    //UE_LOG(LogCarla, Log, TEXT("Total AngVel: %s"), *PhysAngularVelocity.ToString());
  }
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

void UCustomTerrainPhysicsComponent::UpdateMaps(
    FVector Position, float RadiusX, float RadiusY, float CacheRadiusX, float CacheRadiusY)
{
  SparseMap.UpdateMaps(UEFrameToSI(Position), UEFrameToSI(RadiusX), UEFrameToSI(RadiusY), 
      UEFrameToSI(CacheRadiusX), UEFrameToSI(CacheRadiusY));
}

FTilesWorker::FTilesWorker(UCustomTerrainPhysicsComponent* TerrainComp, FVector NewPosition, float NewRadiusX, float NewRadiusY )
{
  CustomTerrainComp = TerrainComp;
  Position = NewPosition;
  RadiusX = NewRadiusX;
  RadiusY = NewRadiusY;
}

FTilesWorker::~FTilesWorker()
{
  CustomTerrainComp = nullptr;
}


uint32 FTilesWorker::Run(){

  FDateTime CacheCurrentTime = FDateTime::Now();
  FDateTime LoadTilesCurrentTime = FDateTime::Now();
  FDateTime UnloadTilesCurrentTime = FDateTime::Now();

  while(bShouldContinue){
    FVector LastPosition = CustomTerrainComp->LastUpdatedPosition;
    if(Position != LastPosition)
    {
      Position = LastPosition;
      CustomTerrainComp->UpdateMaps(CustomTerrainComp->LastUpdatedPosition,
          CustomTerrainComp->TileRadius.X, CustomTerrainComp->TileRadius.Y,
          CustomTerrainComp->CacheRadius.X, CustomTerrainComp->CacheRadius.Y);
    }
    if(!bShouldContinue)
    {
      break;
    }
  }

  return 0;
}
