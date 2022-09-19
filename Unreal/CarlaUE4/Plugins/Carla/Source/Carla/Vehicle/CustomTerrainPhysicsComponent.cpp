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
#include "carla/rpc/String.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/RunnableThread.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

#include "RHICommandList.h"
#include "TextureResource.h"
#include "Rendering/Texture2DResource.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
// #include <carla/pytorch/pytorch.h>

#include "GenericPlatform/GenericPlatformFile.h"
#include "Async/Async.h"
#include "Async/Future.h"


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
 
static bool bDebugLoadingTiles = false;

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
    UTexture2D* Texture, FDVector Size, FDVector Origin,
    float Min, float Max, FDVector Tile0, float ScaleZ)
{
  Tile0Position = Tile0;
  MinHeight = Min;
  MaxHeight = Max;
  WorldSize = Size;
  Offset = Origin;
  Pixels.clear();
  
  // setup required parameters
  Texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
  Texture->SRGB = false;
  Texture->UpdateResource();

  FTexture2DMipMap* TileMipMap = &Texture->PlatformData->Mips[0];
  FByteBulkData* TileRawImageData = &TileMipMap->BulkData;
  FColor* FormatedImageData = (FColor*)(TileRawImageData->Lock(LOCK_READ_ONLY));
    Size_X = Texture->GetSizeX();
  Size_Y = Texture->GetSizeY();
  for(uint32_t j = 0; j < Size_Y; j++)
  {
    for(uint32_t i = 0; i < Size_X; i++)
    {
      uint32_t idx = j*Size_X + i;
      float HeightLevel = Scale_Z*(MinHeight + (MaxHeight - MinHeight) * FormatedImageData[idx].R/255.f);
      Pixels.emplace_back(HeightLevel);
    }
  }
  TileRawImageData->Unlock();
  UE_LOG(LogCarla, Log, 
      TEXT("Height Map initialized with %d pixels"), Pixels.size());
}
float FHeightMapData::GetHeight(FDVector Position) const
{
  Position = Position - Tile0Position;
  uint32_t Coord_X = (Position.X / WorldSize.X) * Size_X;
  uint32_t Coord_Y = (1.f - Position.Y / WorldSize.Y) * Size_Y;
  Coord_X = std::min(Coord_X, Size_X-1);
  Coord_Y = std::min(Coord_Y, Size_Y-1);
  return Pixels[Coord_Y*Size_X + Coord_X];
}

void FHeightMapData::Clear()
{
  Pixels.clear();
}

FDenseTile::FDenseTile(){
  Particles.empty();
  ParticlesHeightMap.empty();
  TilePosition = FDVector(0.0,0.0,0.0);
  SavePath = FString("NotValidPath");
  bHeightmapNeedToUpdate = false;
}

FDenseTile::~FDenseTile(){
  Particles.empty();
  ParticlesHeightMap.empty();
  ParticlesZOrdered.empty();
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


  uint32_t TileSize = (TileEnd.X - TileOrigin.X );
  uint32_t PartialHeightMapSize = TextureSize / ( (AffectedRadius * 2) / TileSize);
  std::string FileName = std::string(TCHAR_TO_UTF8(*( SavePath + TileOrigin.ToString() + ".tile" ) ) );
  
  if( FPaths::FileExists(FString(FileName.c_str())) )
  {
    
    TRACE_CPUPROFILER_EVENT_SCOPE(DenseTile::InitializeTile::Read);
    std::ifstream ReadStream(FileName);
    FVector VectorToRead;
    ReadFVector(ReadStream, VectorToRead );
    TilePosition = FDVector(VectorToRead);
    ReadStdVector<FParticle> (ReadStream, Particles);
    ReadStdVector<float> (ReadStream, ParticlesHeightMap);
    //UE_LOG(LogCarla, Log, TEXT("Reading data, got %d particles"), Particles.size());
  }
  else
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(DenseTile::InitializeTile::Create);

    TilePosition = TileOrigin;
    uint32_t NumParticles_X = (TileEnd.X - TileOrigin.X) / ParticleSize;
    uint32_t NumParticles_Y = FMath::Abs(TileEnd.Y - TileOrigin.Y) / ParticleSize;
    uint32_t NumParticles_Z = (Depth) / ParticleSize;
    ParticlesHeightMap.resize( PartialHeightMapSize*PartialHeightMapSize );
    Particles = std::vector<FParticle>(NumParticles_X*NumParticles_Y*NumParticles_Z);

    UE_LOG(LogCarla, Log, TEXT("Initializing Tile with (%d,%d,%d) particles at location %s, size %f, depth %f, HeightMap at tile origin %f"), 
       NumParticles_X,NumParticles_Y,NumParticles_Z, *TileOrigin.ToString(), ParticleSize, Depth,  HeightMap.GetHeight(TileOrigin)  );

    for(uint32_t i = 0; i < NumParticles_X; i++)
    {
      for(uint32_t j = 0; j < NumParticles_Y; j++)
      {
        FDVector ParticleLocalPosition = FDVector(i*ParticleSize, j*ParticleSize, 0.0f);
        FDVector ParticlePosition = TileOrigin + ParticleLocalPosition;
        // UE_LOG(LogCarla, Log, TEXT("  Particle position %s"), *ParticlePosition.ToString());
        float Height = HeightMap.GetHeight(ParticlePosition);
        for(uint32_t k = 0; k < NumParticles_Z; k++)
        {
          ParticlePosition.Z = TileOrigin.Z + Height - k*ParticleSize;
          Particles[k*NumParticles_X*NumParticles_Y + j*NumParticles_X + i] = 
            {ParticlePosition, FVector(0), ParticleSize/2.f};
        }
      }
    }

    for(uint32_t i = 0; i < PartialHeightMapSize; i++)
    {
      for(uint32_t j = 0; j < PartialHeightMapSize; j++)
      {
        ParticlesHeightMap[PartialHeightMapSize * i + j] = 0;
      }
    }

    UE_LOG(LogCarla, Log, TEXT("Initialized, got %d particles"), Particles.size());
    UE_LOG(LogCarla, Log, TEXT("Initialized ParticlesHeightMap, got %d particles"), ParticlesHeightMap.size());
  }
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(DenseTile::InitializeTile::ParticlesZOrdered);
    ParticlesZOrdered.resize( PartialHeightMapSize*PartialHeightMapSize );
    float InverseTileSize = 1/TileSize;
    float Transformation = InverseTileSize * PartialHeightMapSize;
    for (size_t i = 0; i < Particles.size(); i++)
    {
      const FParticle& P = Particles[i];
      FDVector ParticleLocalPosition = P.Position - TilePosition;
      // Recalculate position to get it into heightmap coords
      ParticleLocalPosition.X *= ( Transformation );
      ParticleLocalPosition.Y *= ( Transformation );

      uint32_t Index = std::floor(ParticleLocalPosition.Y) * PartialHeightMapSize + std::floor(ParticleLocalPosition.X);
      // Compare to the current value, if higher replace 
      if( Index < ParticlesZOrdered.size() ){
        ParticlesZOrdered[Index].insert(P.Position.Z);
      }else{
        /*
        UE_LOG(LogCarla, Log, TEXT("Invalid Index on Z order %d, Size %d"), Index,ParticlesZOrdered.size() );
        UE_LOG(LogCarla, Log, TEXT("ParticleLocalPosition X: %f, Y %f"), ParticleLocalPosition.X, ParticleLocalPosition.Y);
        UE_LOG(LogCarla, Log, TEXT("FParticle X: %f, Y %f"), P.Position.X, P.Position.Y);
        UE_LOG(LogCarla, Log, TEXT("TilePosition X: %f, Y %f"), TilePosition.X, TilePosition.Y);
        */
      }
    }
  }
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
  TRACE_CPUPROFILER_EVENT_SCOPE(FDenseTile::UpdateLocalHeightmap);
  if( bHeightmapNeedToUpdate ){
    for( uint32_t i = 0; i < ParticlesHeightMap.size() ; ++i ){
      auto it = ParticlesZOrdered[i].begin();
      ParticlesHeightMap[i] = *it;
    }
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

std::vector<float> FSparseHighDetailMap::
    GetParticlesHeightMapInTileRadius(FDVector Position, float Radius)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::GetParticlesHeightMapInTileRadius);
 
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
  UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::GetParticlesHeightMapInTileRadius MinX %zu MaxX: %zu, MinY %zu MaxY %zu"),
      MinX, MaxX, MinY, MaxY);
  UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::GetParticlesHeightMapInTileRadius TileId %lld TileX: %d, TileY %d"),
      TileId, Tile_X, Tile_Y);
  UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::GetParticlesHeightMapInTileRadius RadiusInTiles %d"), RadiusInTiles);
  UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::GetParticlesHeightMapInTileRadius Extension X: %f, Y %f"), Extension.X, Extension.Y);
  UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::GetParticlesHeightMapInTileRadius Position X: %f, Y %f"), Position.X, Position.Y);
  */
  std::vector<float> ParticlesHeightMap;
  for( uint32_t X = MinX; X < MaxX; ++X )
  {
    for( uint32_t Y = MinY; Y < MaxY; ++Y )
    {
      uint64_t CurrentTileId = GetTileId(X,Y);
      if( Map.find(CurrentTileId) != Map.end() )
      {
        std::vector<float>& CurrentHeightMap = GetTile(X,Y).ParticlesHeightMap;
        // UE_LOG(LogCarla, Log, TEXT("Pre ParticlesHeightMap %d particlesheightmap"), ParticlesHeightMap.size());

        ParticlesHeightMap.insert(ParticlesHeightMap.end(), 
                                    CurrentHeightMap.begin() ,
                                    CurrentHeightMap.end() ); 
        // UE_LOG(LogCarla, Log, TEXT("New %d particlesheightmap"), CurrentHeightMap.size());
      }else{
        //UE_LOG(LogCarla, Log, TEXT("Requested Tile not found TileId %lld TileX: %d, TileY %d"), CurrentTileId, X, Y);
      }
    }
  }
  return ParticlesHeightMap;
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

  FDVector Position = FDVector(Tile_X*TileSize, Tile_Y*TileSize, 0);
  Position = Position + Tile0Position;
  //UE_LOG(LogCarla, Log, TEXT("Getting location from id (%lu, %lu) %s"),
  //      (unsigned long)Tile_X, (unsigned long)Tile_Y, *(Position).ToString());
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
    auto CacheIterator = CacheMap.find(TileId);
    if (Iterator == CacheMap.end())
    {
      return InitializeRegionInCache(TileId);
    }
    return CacheIterator->second;
  }
  return Iterator->second;
}

FIntVector FSparseHighDetailMap::GetVectorTileId(FDVector Position)
{
  uint32_t Tile_X = static_cast<uint32_t>((Position.X - Tile0Position.X) / TileSize);
  uint32_t Tile_Y = static_cast<uint32_t>((Position.Y - Tile0Position.Y) / TileSize);
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
  return Tile;
}

void FSparseHighDetailMap::InitializeMap(UTexture2D* HeightMapTexture,
      FDVector Origin, FDVector MapSize, float Size, float MinHeight, float MaxHeight, 
      float ScaleZ)
{
  Tile0Position = Origin;
  TileSize = Size;
  Extension = MapSize;
  Heightmap.InitializeHeightmap(
      HeightMapTexture, Extension, Tile0Position, 
      MinHeight, MaxHeight, Tile0Position, ScaleZ);
  UE_LOG(LogCarla, Error, 
      TEXT("Sparse Map initialized"));


  UE_LOG(LogCarla, Error, 
      TEXT("Map Extension %f %f %f"), MapSize.X, MapSize.Y, MapSize.Z );
}

void FSparseHighDetailMap::UpdateHeightMap(UTexture2D* HeightMapTexture,
      FDVector Origin, FDVector MapSize, float Size, float MinHeight, float MaxHeight,
      float ScaleZ)
{
  Heightmap.Clear();
  Heightmap.InitializeHeightmap(
      HeightMapTexture, Extension, Origin, 
      MinHeight, MaxHeight, Origin, ScaleZ);
  UE_LOG(LogCarla, Log, 
      TEXT("Height map updated"));
}

void FSparseHighDetailMap::Clear()
{
  Heightmap.Clear();
  Map.clear();
}

void FSparseHighDetailMap::LoadTilesAtPositionFromCache(FDVector Position, float RadiusX , float RadiusY )
{
  // Translate UE4 Position to our coords
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition);

  double MinX = std::min( std::max( Position.X - RadiusX, 0.0) , Extension.X - 1.0);
  double MinY = std::min( std::max( Position.Y - RadiusY, 0.0) , -Extension.Y + 1.0);
  double MaxX = std::min( std::max( Position.X + RadiusX, 0.0) , Extension.X - 1.0);
  double MaxY = std::min( std::max( Position.Y + RadiusY, 0.0) , -Extension.Y + 1.0);

  FIntVector MinVector = GetVectorTileId(FDVector(MinX, MinY, 0));
  FIntVector MaxVector = GetVectorTileId(FDVector(MaxX, MaxY, 0));

  FIntVector CacheMinVector = GetVectorTileId(FDVector(MinX - 10.0, MinY - 10.0, 0));
  FIntVector CacheMaxVector = GetVectorTileId(FDVector(MaxX + 10.0, MaxY + 10.0, 0));

  if( bDebugLoadingTiles ){
    static FDateTime CurrentTime = FDateTime::Now();
    if( ( FDateTime::Now() - CurrentTime).GetTotalSeconds() > 20.0f )
    {
      UE_LOG(LogCarla, Error, TEXT("FSparseHighDetailMap::LoadTilesAtPositionFromCache Position X: %f, Y %f"), Position.X, Position.Y);
      UE_LOG(LogCarla, Error, TEXT("FSparseHighDetailMap::LoadTilesAtPositionFromCache Loading Tiles Between X: %d %d Y: %d %d "), MinVector.X, MaxVector.X, MinVector.Y, MaxVector.Y );
      UE_LOG(LogCarla, Error, TEXT("FSparseHighDetailMap::LoadTilesAtPositionFromCache Cache Tiles Between X: %d %d Y: %d %d "), CacheMinVector.X, CacheMaxVector.X, CacheMinVector.Y, CacheMaxVector.Y );
      //UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::LoadTilesAtPositionFromCache Extension X: %f, Y %f"), Extension.X, Extension.Y);
      //UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::LoadTilesAtPositionFromCache Before MapSize : %d"), Map.size() );
      CurrentTime = FDateTime::Now();
    }
  }

  {
    TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition::Process);
    for(uint32_t CacheX = CacheMinVector.X; CacheX < CacheMaxVector.X; CacheX++ )
    {
      for(uint32_t CacheY = CacheMinVector.Y; CacheY < CacheMaxVector.Y; CacheY++ )
      {
        uint64_t CurrentTileID = GetTileId(CacheX, CacheY);
        if( Map.find(CurrentTileID) == Map.end() )
        {
          if ( MinVector.X <= CacheX  && CacheX <= MaxVector.X && MinVector.Y <= CacheY  && CacheY <= MaxVector.Y )
          {
            FDenseTile NewTile;
            TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition::Insert);
            FScopeLock ScopeLock(&Lock_Map);
            FScopeLock ScopeCacheLock(&Lock_CacheMap);
            {
              TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition::Insert::Emplace);
              Map.emplace(CurrentTileID, std::move( CacheMap[CurrentTileID] ) ); 
            }
            {
              TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition::Insert::Erase);
              CacheMap.erase(CurrentTileID);
            }
          }
        }
      }
    }    
  }
}

void FSparseHighDetailMap::UnLoadTilesAtPositionToCache(FDVector Position, float RadiusX , float RadiusY )
{
  // Translate UE4 Position to our coords
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition);
  std::set<uint64_t> KeysToRemove;

  double MinX = std::min( std::max( Position.X - RadiusX, 0.0) , Extension.X - 1.0);
  double MinY = std::min( std::max( Position.Y - RadiusY, 0.0) , -Extension.Y + 1.0);
  double MaxX = std::min( std::max( Position.X + RadiusX, 0.0) , Extension.X - 1.0);
  double MaxY = std::min( std::max( Position.Y + RadiusY, 0.0) , -Extension.Y + 1.0);

  FIntVector MinVector = GetVectorTileId(FDVector(MinX, MinY, 0));
  FIntVector MaxVector = GetVectorTileId(FDVector(MaxX, MaxY, 0));

  FIntVector CacheMinVector = GetVectorTileId(FDVector(MinX - 10.0, MinY - 10.0, 0));
  FIntVector CacheMaxVector = GetVectorTileId(FDVector(MaxX + 10.0, MaxY + 10.0, 0));

  if( bDebugLoadingTiles ){
    static FDateTime CurrentTime = FDateTime::Now();
    if( ( FDateTime::Now() - CurrentTime).GetTotalSeconds() > 20.0f )
    {
      UE_LOG(LogCarla, Error, TEXT("FSparseHighDetailMap::UnLoadTilesAtPositionToCache Position X: %f, Y %f"), Position.X, Position.Y);
      UE_LOG(LogCarla, Error, TEXT("FSparseHighDetailMap::UnLoadTilesAtPositionToCache Loading Tiles Between X: %d %d Y: %d %d "), MinVector.X, MaxVector.X, MinVector.Y, MaxVector.Y );
      UE_LOG(LogCarla, Error, TEXT("FSparseHighDetailMap::UnLoadTilesAtPositionToCache Cache Tiles Between X: %d %d Y: %d %d "), CacheMinVector.X, CacheMaxVector.X, CacheMinVector.Y, CacheMaxVector.Y );
      //UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::UnLoadTilesAtPositionToCache Extension X: %f, Y %f"), Extension.X, Extension.Y);
      //UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::UnLoadTilesAtPositionToCache Before MapSize : %d"), Map.size() );
      CurrentTime = FDateTime::Now();
    }
  }

  {
    TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition::Process);
    for(uint32_t CacheX = CacheMinVector.X; CacheX < CacheMaxVector.X; CacheX++ )
    {
      for(uint32_t CacheY = CacheMinVector.Y; CacheY < CacheMaxVector.Y; CacheY++ )
      {
        uint64_t CurrentTileID = GetTileId(CacheX, CacheY);
        if( Map.find(CurrentTileID) != Map.end() )
        {
          if ( MinVector.X <= CacheX  && CacheX <= MaxVector.X && MinVector.Y <= CacheY  && CacheY <= MaxVector.Y )
          {
            
          }else{
            KeysToRemove.insert( CurrentTileID );
          }
        }
      }
    }    
  }

  {
    TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition::Save);
    FScopeLock ScopeLock(&Lock_Map);
    for(auto it : KeysToRemove) 
    {
      TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition::Erase);
      FScopeLock ScopeCacheLock(&Lock_CacheMap);
      CacheMap.emplace(it, std::move(Map[it]) ); 
      Map.erase(it);
    }
  }
}

void FSparseHighDetailMap::ReloadCache(FDVector Position, float RadiusX , float RadiusY )
{
  // Translate UE4 Position to our coords
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::ReloadCache);
  std::unordered_set<uint64_t> KeysToRemove;
  std::unordered_set<uint64_t> KeysToLoad;

  double MinX = std::min( std::max( Position.X - RadiusX, 0.0) , Extension.X - 1.0);
  double MinY = std::min( std::max( Position.Y - RadiusY, 0.0) , -Extension.Y + 1.0);
  double MaxX = std::min( std::max( Position.X + RadiusX, 0.0) , Extension.X - 1.0);
  double MaxY = std::min( std::max( Position.Y + RadiusY, 0.0) , -Extension.Y + 1.0);

  FIntVector MinVector = GetVectorTileId(FDVector(MinX, MinY, 0));
  FIntVector MaxVector = GetVectorTileId(FDVector(MaxX, MaxY, 0));

  FIntVector CacheMinVector = GetVectorTileId(FDVector(MinX - RadiusX * 2, MinY - RadiusY * 2, 0));
  FIntVector CacheMaxVector = GetVectorTileId(FDVector(MaxX + RadiusX * 2, MaxY + RadiusY * 2, 0));

  UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::ReloadCache Position X: %f, Y %f"), Position.X, Position.Y);
  //UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::ReloadCache Extension X: %f, Y %f"), Extension.X, Extension.Y);
  UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::ReloadCache Loading Tiles Between X: %d %d Y: %d %d "), MinVector.X, MaxVector.X, MinVector.Y, MaxVector.Y );

  {
    TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::ReloadCache::Process);
    for(uint32_t CacheX = CacheMinVector.X; CacheX < CacheMaxVector.X; CacheX++ )
    {
      for(uint32_t CacheY = CacheMinVector.Y; CacheY < CacheMaxVector.Y; CacheY++ )
      {
          uint64_t CurrentTileID = GetTileId(CacheX, CacheY);
          if( Map.find(CurrentTileID) != Map.end() )
          {
            if ( MinVector.X <= CacheX  && CacheX <= MaxVector.X && MinVector.Y <= CacheY  && CacheY <= MaxVector.Y )
            {
              
            }else{
              KeysToRemove.insert( CurrentTileID );
            }
          }else{
            if ( MinVector.X <= CacheX  && CacheX <= MaxVector.X && MinVector.Y <= CacheY  && CacheY <= MaxVector.Y )
            {
              TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::ReloadCache::Insert);
              FScopeLock ScopeCacheLock(&Lock_CacheMap);
              KeysToLoad.insert(CurrentTileID);
              CacheMap.emplace(CurrentTileID, FDenseTile() ); 
            }
          }
      }
    }    
  }

  {
    TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::ReloadCache::Load);
    std::vector<uint64_t> KeysToBeLoaded;
    KeysToBeLoaded.insert(KeysToBeLoaded.end(), KeysToLoad.begin(), KeysToLoad.end());

    ParallelFor(KeysToBeLoaded.size(), [&](int32 Idx)
    {
      TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::ReloadCache::LoadOne);
      InitializeRegionInCache(KeysToBeLoaded[Idx]);
    });
  }

  {
    FScopeLock ScopeLock(&Lock_CacheMap);
    TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::ReloadCache::Save);
    for(auto it : KeysToRemove) 
    {
      //UE_LOG(LogCarla, Log, TEXT("Offlloading tile id %llu, (%f, %f)  (%f, %f)"), it, MinX, MinY, MaxX, MaxY);
      std::string FileToSavePath = std::string(TCHAR_TO_UTF8(*( SavePath + CacheMap[it].TilePosition.ToString() + ".tile")));
      std::ofstream OutputStream(FileToSavePath.c_str());
      WriteFVector(OutputStream, CacheMap[it].TilePosition.ToFVector());
      WriteStdVector<FParticle> (OutputStream, CacheMap[it].Particles);
      WriteStdVector<float> (OutputStream, CacheMap[it].ParticlesHeightMap);
      OutputStream.close();
      CacheMap.erase(it);
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
    for(uint32_t X = MinVector.X; X < MaxVector.X; X++ )
    {
      for(uint32_t Y = MinVector.Y; Y < MaxVector.Y; Y++ )
      {
        bool ConditionToStopWaiting = true;
        // Check if tiles are already loaded
        // If they are send position
        // If not wait until loaded
        while(ConditionToStopWaiting) 
        {
          uint64_t CurrentTileID = GetTileId(X,Y);
          LockMutex();
          {
            TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("FSparseHighDetailMap::WaitUntilProperTilesAreLoaded::TimeLocked"));
            ConditionToStopWaiting = Map.find(CurrentTileID) == Map.end();
          }
          UnLockMutex();
          
          if(ConditionToStopWaiting) {
            bDebugLoadingTiles = true; 
            FGenericPlatformProcess::Sleep(0.0001f);
            static FDateTime CurrentTime = FDateTime::Now();
            if( ( FDateTime::Now() - CurrentTime).GetTotalSeconds() > 5.0f )
            {
              UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::Update PositionToUpdate X: %f, Y %f"), PositionToUpdate.X, PositionToUpdate.Y);
              UE_LOG(LogCarla, Warning, TEXT("FSparseHighDetailMap::Update Waiting Tiles Between X: %d %d Y: %d %d "), MinVector.X, MaxVector.X, MinVector.Y, MaxVector.Y );
              /*
              UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::Update Position X: %f, Y %f"), Position.X, Position.Y);
              UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::Update PositionTranslated X: %f, Y %f"), PositionTranslated.X, PositionTranslated.Y);
              UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::Update Extension X: %f, Y %f"), Extension.X, Extension.Y);
              */
                CurrentTime = FDateTime::Now();
            }
          }
        }
      }
    }
  }
  bDebugLoadingTiles = false; 

}

void FSparseHighDetailMap::SaveMap()
{
  UE_LOG(LogCarla, Warning, TEXT("Save directory %s"), *SavePath );
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::SaveMap);
  for(auto& it: Map)
  {
    std::string FileToSavePath = std::string(TCHAR_TO_UTF8(*( SavePath + it.second.TilePosition.ToString() + ".tile")));
    std::ofstream OutputStream(FileToSavePath.c_str());
    WriteFVector(OutputStream, it.second.TilePosition.ToFVector());
    WriteStdVector<FParticle> (OutputStream, it.second.Particles);
    WriteStdVector<float> (OutputStream, it.second.ParticlesHeightMap);
    OutputStream.close();
  }
  for(auto& it: CacheMap)
  {
    std::string FileToSavePath = std::string(TCHAR_TO_UTF8(*( SavePath + it.second.TilePosition.ToString() + ".tile")));
    std::ofstream OutputStream(FileToSavePath.c_str());
    WriteFVector(OutputStream, it.second.TilePosition.ToFVector());
    WriteStdVector<FParticle> (OutputStream, it.second.Particles);
    WriteStdVector<float> (OutputStream, it.second.ParticlesHeightMap);
    
    OutputStream.close();
  }
}

void UCustomTerrainPhysicsComponent::UpdateTexture()
{  
  UpdateTextureData();

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
  float LimitX = TextureToUpdate->GetSizeX(); 
  float LimitY = TextureToUpdate->GetSizeY();
  if(Data.Num() == 0){
    Data.Init(0, LimitX * LimitY);
  }
}

void UCustomTerrainPhysicsComponent::UpdateTextureData()
{
  uint32_t NumberOfParticlesIn1AxisInHeightmap = TextureToUpdate->GetSizeX() / (TextureRadius * 2);
  std::vector<float> ParticlesPositions;
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UCustomTerrainPhysicsComponent::UpdateTextureData::GettingHeightMaps"));
    FVector OriginPosition;     
    OriginPosition.X =  LastUpdatedPosition.X + (WorldSize.X * 0.5f);
    OriginPosition.Y =  -LastUpdatedPosition.Y - (WorldSize.Y * 0.5f);
    SparseMap.LockMutex();
    ParticlesPositions = SparseMap.GetParticlesHeightMapInTileRadius(UEFrameToSI(OriginPosition), TextureRadius);
    SparseMap.UnLockMutex();
    //UE_LOG(LogCarla, Log, TEXT("FSparseHighDetailMap::UpdateTextureData OriginPosition X: %f, Y %f"), OriginPosition.X, OriginPosition.Y);

  }

  {
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UCustomTerrainPhysicsComponent::UpdateTextureData"));
    uint32_t LimitX = TextureToUpdate->GetSizeX() - 1; 
    uint32_t LimitY = TextureToUpdate->GetSizeY() - 1;
    uint32_t OffsetAbsolutX = 0;
    uint32_t OffsetAbsolutY = 0;
    uint32_t LocalOffset = 0;
    uint32_t NumberOfTexturesMiniHeightMaps = (TextureRadius * 2) / TileSize;
    //UE_LOG(LogCarla, Log, TEXT("Data : %d, ParticlesPositions %d"), Data.Num(), ParticlesPositions.size() );
    //UE_LOG(LogCarla, Log, TEXT("LimitX : %d, LimitY %d"), LimitX, LimitY  );
    
    while( OffsetAbsolutY < NumberOfTexturesMiniHeightMaps )
    {
      for(uint32_t y = 0; y < NumberOfParticlesIn1AxisInHeightmap; y++)
      {
        for(uint32_t x = 0; x < NumberOfParticlesIn1AxisInHeightmap; x++)
        {    
          uint32_t AbsolutIndex = (OffsetAbsolutX * NumberOfParticlesIn1AxisInHeightmap) + x + (LimitX * ( (OffsetAbsolutY * NumberOfParticlesIn1AxisInHeightmap)  + y) );
          uint32_t LocalIndex =   x + y * NumberOfParticlesIn1AxisInHeightmap;
          if ( Data.IsValidIndex(AbsolutIndex) && LocalIndex + LocalOffset < ParticlesPositions.size() )
          {
            Data[AbsolutIndex] = ParticlesPositions[LocalIndex + LocalOffset] + 127;
          }else{
            /*
            UE_LOG(LogCarla, Log, TEXT("Invalid Indices AbsolutIndex %d LocalIndex + LocalOffset %d"), AbsolutIndex, LocalIndex + LocalOffset);            
            UE_LOG(LogCarla, Log, TEXT("Sized Data: %d ParticlesPositions %d"), Data.Num(), ParticlesPositions.size() );            
            UE_LOG(LogCarla, Log, TEXT("OffsetAbsolutY : %d, OffsetAbsolutX %d"), OffsetAbsolutY, OffsetAbsolutX  );
            UE_LOG(LogCarla, Log, TEXT("x : %d, y %d"), x, y  );
            UE_LOG(LogCarla, Log, TEXT("NumberOfTexturesMiniHeightMaps: %d"), NumberOfTexturesMiniHeightMaps );
            */
          }
        }
      }
      LocalOffset += (TextureToUpdate->GetSizeX() / NumberOfParticlesIn1AxisInHeightmap) * (TextureToUpdate->GetSizeY() / NumberOfParticlesIn1AxisInHeightmap) ;
      OffsetAbsolutX++;
      if(OffsetAbsolutX >= NumberOfTexturesMiniHeightMaps )
      {
        OffsetAbsolutX = 0;
        OffsetAbsolutY++;
      }
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
  GEngine->Exec( GetWorld(), TEXT( "Trace.Start default,gpu" ) );

  SparseMap.Clear();
  RootComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
  if (!RootComponent)
  {
    UE_LOG(LogCarla, Error, 
        TEXT("UCustomTerrainPhysicsComponent: Root component is not a UPrimitiveComponent"));
  }

#ifndef WITH_EDITOR
  bUpdateParticles = false;
  DrawDebugInfo = false;
  bUseDynamicModel = false;
  bDisableVehicleGravity = false;
  NNVerbose = true;
  bUseImpulse = false;
  bUseMeanAcceleration = false;
  bShowForces = true;
  bBenchMark = false;
  bDrawHeightMap = false;
  ForceMulFactor = 1.0;
  FloorHeight = 0.0;
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
  if (FParse::Value(FCommandLine::Get(), TEXT("-max-force="), Value))
  {
    MaxForceMagnitude = MToCM*Value;
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-floor-height="), Value))
  {
    FloorHeight = MToCM*Value;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-update-particles")))
  {
    bUpdateParticles = true;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-draw-debug-info")))
  {
    DrawDebugInfo = true;
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
  if (FParse::Param(FCommandLine::Get(), TEXT("-disable-terramechanics")))
  {
    SetComponentTickEnabled(false);
    return;
  }
  LargeMapManager = UCarlaStatics::GetLargeMapManager(GetWorld());
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(InitializeDenseMap);
    SparseMap.Clear();
    UE_LOG(LogCarla, Warning, 
        TEXT("ParticleDiameter %f"), ParticleDiameter);

    SparseMap.Init(TextureToUpdate->GetSizeX(), TextureRadius, ParticleDiameter * CMToM, TerrainDepth * CMToM);
    RootComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
    if(LargeMapManager)
    {
      FIntVector NumTiles = LargeMapManager->GetNumTilesInXY();
      // WorldSize = FVector(NumTiles) * LargeMap->GetTileSize();
      // UE_LOG(LogCarla, Log, 
      //     TEXT("World Size %s"), *(WorldSize.ToString()));
    }
    Tile0Origin.Z += FloorHeight;
    SparseMap.InitializeMap(HeightMap, UEFrameToSI(Tile0Origin), UEFrameToSI(WorldSize),
        TileSize, MinHeight, MaxHeight, HeightMapScaleFactor.Z);
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
#ifdef WITH_EDITOR
  SavePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + LevelName + "_Terrain/";
#else
  SavePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + LevelName + "_Terrain/";
#endif
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

  ReloadCache(FVector(100,100, 0), CacheRadius.X, CacheRadius.Y );
  LoadTilesAtPosition(FVector(100,100, 0), TileRadius.X, TileRadius.Y );
  InitTexture();
  
  UE_LOG(LogCarla, Log, TEXT("MainThread Data ArraySize %d "), Data.Num());
  UE_LOG(LogCarla, Log, TEXT("Map Size %d "), SparseMap.Map.size() );

  if (TilesWorker == nullptr)
  {
    TilesWorker = new FTilesWorker(this, GetOwner()->GetActorLocation(), TileRadius.X, TileRadius.Y);
    Thread = FRunnableThread::Create(TilesWorker, TEXT("TilesWorker"));
  }

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
  
  for (AActor* VehicleActor : VehiclesActors)
  {
    ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle> (VehicleActor);
    FVector GlobalLocation = Vehicle->GetActorLocation();
    if(LargeMapManager)
    {
      GlobalLocation = 
          LargeMapManager->LocalToGlobalLocation(Vehicle->GetActorLocation());
      uint64_t TileId = LargeMapManager->GetTileID(GlobalLocation);
      FIntVector CurrentTileId = 
          LargeMapManager->GetTileVectorID(GlobalLocation);
      if(CurrentLargeMapTileId != CurrentTileId)
      {
        //load new height map
        FCarlaMapTile* LargeMapTile = LargeMapManager->GetCarlaMapTile(TileId);
        if(LargeMapTile && false)
        {
          CurrentLargeMapTileId = CurrentTileId;
          FString FullTileNamePath = LargeMapTile->Name;
          FString TileDirectory;
          FString TileName;
          FString Extension;
          FPaths::Split(FullTileNamePath, TileDirectory, TileName, Extension);
          FString TexturePath = TileDirectory + "/HeightMaps/" + TileName + "." + TileName;
          UE_LOG(LogCarla, Log, TEXT("Enter tile %s, %s \n %s \n %s \n %s"), *CurrentTileId.ToString(), 
              *FullTileNamePath, *TileDirectory, *TileName, *Extension);

          UObject* TextureObject = StaticLoadObject(UTexture2D::StaticClass(), nullptr, *(TexturePath));
          if(TextureObject)
          {
            UTexture2D* Texture = Cast<UTexture2D>(TextureObject);
            if (Texture != nullptr)
            {
              HeightMap = Texture;
              FVector TilePosition = HeightMapOffset + LargeMapManager->GetTileLocation(CurrentLargeMapTileId) - 0.5f*FVector(LargeMapManager->GetTileSize(), -LargeMapManager->GetTileSize(), 0);
              UE_LOG(LogCarla, Log, TEXT("Updating height map to location %s in tile location %s"), 
                  *TilePosition.ToString(), *LargeMapManager->GetTileLocation(CurrentLargeMapTileId).ToString());
              TilePosition.Z += FloorHeight;
              SparseMap.UpdateHeightMap(
                  HeightMap, UEFrameToSI(TilePosition), UEFrameToSI(FVector(
                    LargeMapManager->GetTileSize(),-LargeMapManager->GetTileSize(), 0)), 
                  1.f, MinHeight, MaxHeight, HeightMapScaleFactor.Z);
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
    LastUpdatedPosition = Vehicle->GetActorLocation();
    
    SparseMap.Update(LastUpdatedPosition, TextureRadius, TextureRadius );
    SparseMap.LockMutex();
    RunNNPhysicsSimulation(Vehicle, DeltaTime);
    // if(bUpdateParticles)
    // {
    //   {
    //     TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::DebugToBeRemoved);

    //     std::vector<FParticle*> ParticlesWheel0;
    //     std::vector<FParticle*> ParticlesWheel1;
    //     std::vector<FParticle*> ParticlesWheel2;
    //     std::vector<FParticle*> ParticlesWheel3;

    //     {
    //       TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::RemoveParticles);
    //       RemoveParticlesFromOrderedContainer( ParticlesWheel0 );
    //       RemoveParticlesFromOrderedContainer( ParticlesWheel1 );
    //       RemoveParticlesFromOrderedContainer( ParticlesWheel2 );
    //       RemoveParticlesFromOrderedContainer( ParticlesWheel3 );
    //     }
    //     {
    //       TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::UpdateParticles);
    //       UpdateParticlesDebug( ParticlesWheel1 );
    //       UpdateParticlesDebug( ParticlesWheel2 );
    //       UpdateParticlesDebug( ParticlesWheel0 );
    //       UpdateParticlesDebug( ParticlesWheel3 );
    //     }

    //     {
    //       TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::AddParticles);
    //       AddParticlesToOrderedContainer( ParticlesWheel0 );
    //       AddParticlesToOrderedContainer( ParticlesWheel1 );
    //       AddParticlesToOrderedContainer( ParticlesWheel2 );
    //       AddParticlesToOrderedContainer( ParticlesWheel3 );
    //     }

    //     {
    //       TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::UpdateTilesHeightMaps);
    //       UpdateTilesHeightMapsInRadius(LastUpdatedPosition, 4);
    //     }
    //   }
    // }
    SparseMap.UnLockMutex();
    UpdateTexture();

    if( MPCInstance == nullptr )
    {
      if(MPC){
        MPCInstance = GetWorld()->GetParameterCollectionInstance( MPC );
      }
    }

    if( MPCInstance ){
      MPCInstance->SetVectorParameterValue("PositionToUpdate", LastUpdatedPosition);
      // We set texture radius in cm as is UE4 default measure unit
      MPCInstance->SetScalarParameterValue("TextureRadius", TextureRadius * 100);
      MPCInstance->SetScalarParameterValue("EffectMultiplayer", EffectMultiplayer * 100); 
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

void UCustomTerrainPhysicsComponent::DrawParticles(UWorld* World, std::vector<FParticle*>& Particles)
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
          FLinearColor(1.f, 0.f, 0.f), 1.0, 0, LifeTime);
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

void UCustomTerrainPhysicsComponent::DrawTiles(UWorld* World, const std::vector<uint64_t>& TilesIds,float Height)
{
  float LifeTime = 0.3f;
  bool bPersistentLines = false;
  bool bDepthIsForeground = (0 == SDPG_Foreground);
  float Thickness = 2.0f;
  FLinearColor Color = FLinearColor(0.0,1.0,0.0);
  ULineBatchComponent* LineBatcher = 
      (World ? (bDepthIsForeground ? World->ForegroundLineBatcher : 
      (( bPersistentLines || (LifeTime > 0.f) ) ? World->PersistentLineBatcher : World->LineBatcher)) : nullptr);
  if (!LineBatcher)
  {
    UE_LOG(LogCarla, Error, TEXT("Missing linebatcher"));
  }
  UE_LOG(LogCarla, Log, TEXT("Drawing %d Tiles"), TilesIds.size());
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
    UE_LOG(LogCarla, Log, TEXT("Drawing Tile %ld with verts %s, %s, %s, %s"),
        TileId, *V1.ToString(), *V2.ToString(), *V3.ToString(), *V4.ToString());
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
    return P1->Position.Z < P2->Position.Z;
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
  FTransform VehicleTransform = Vehicle->GetTransform();
  FVector WheelPosition0 = VehicleTransform.TransformPosition(FVector(140, -70, 40));
  FVector WheelPosition1 = VehicleTransform.TransformPosition(FVector(140, 70, 40));
  FVector WheelPosition2 = VehicleTransform.TransformPosition(FVector(-140, -70, 40));
  FVector WheelPosition3 = VehicleTransform.TransformPosition(FVector(-140, 70, 40));
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
    auto FutureParticles0 = Async(EAsyncExecution::TaskGraph, 
        [&]() {return GetAndFilterParticlesInBox(BboxWheel0);});
    auto FutureParticles1 = Async(EAsyncExecution::TaskGraph, 
        [&]() {return GetAndFilterParticlesInBox(BboxWheel1);});
    auto FutureParticles2 = Async(EAsyncExecution::TaskGraph, 
        [&]() {return GetAndFilterParticlesInBox(BboxWheel2);});
    auto FutureParticles3 = Async(EAsyncExecution::TaskGraph, 
        [&]() {return GetAndFilterParticlesInBox(BboxWheel3);});
    ParticlesWheel0 = FutureParticles0.Get();
    ParticlesWheel1 = FutureParticles1.Get();
    ParticlesWheel2 = FutureParticles2.Get();
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
  UE_LOG(LogCarla, Log, TEXT("Found %d particles in wheel 0 %s"), ParticlesWheel0.size(), *WheelPosition0.ToString());
  UE_LOG(LogCarla, Log, TEXT("Found %d particles in wheel 1 %s"), ParticlesWheel1.size(), *WheelPosition1.ToString());
  UE_LOG(LogCarla, Log, TEXT("Found %d particles in wheel 2 %s"), ParticlesWheel2.size(), *WheelPosition2.ToString());
  UE_LOG(LogCarla, Log, TEXT("Found %d particles in wheel 3 %s"), ParticlesWheel3.size(), *WheelPosition3.ToString());
  if(ParticlesWheel0.size())
    UE_LOG(LogCarla, Log, 
        TEXT("Wheel0 pos %s particle pos %s"), *(UEFrameToSI(WheelPosition0)).ToString(), *(ParticlesWheel0[0]->Position.ToString()));
  
  TArray<float> ParticlePos0, ParticleVel0, ParticlePos1, ParticleVel1,
                ParticlePos2, ParticleVel2, ParticlePos3, ParticleVel3;
  TArray<float> WheelPos0, WheelOrient0, WheelLinVel0, WheelAngVel0;
  TArray<float> WheelPos1, WheelOrient1, WheelLinVel1, WheelAngVel1;
  TArray<float> WheelPos2, WheelOrient2, WheelLinVel2, WheelAngVel2;
  TArray<float> WheelPos3, WheelOrient3, WheelLinVel3, WheelAngVel3;
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(SetUpArrays);
    SetUpParticleArrays(ParticlesWheel0, ParticlePos0, ParticleVel0);
    SetUpParticleArrays(ParticlesWheel1, ParticlePos1, ParticleVel1);
    SetUpParticleArrays(ParticlesWheel2, ParticlePos2, ParticleVel2);
    SetUpParticleArrays(ParticlesWheel3, ParticlePos3, ParticleVel3);

    SetUpWheelArrays(Vehicle, 0, WheelPos0, WheelOrient0, WheelLinVel0, WheelAngVel0);
    SetUpWheelArrays(Vehicle, 1, WheelPos1, WheelOrient1, WheelLinVel1, WheelAngVel1);
    SetUpWheelArrays(Vehicle, 2, WheelPos2, WheelOrient2, WheelLinVel2, WheelAngVel2);
    SetUpWheelArrays(Vehicle, 3, WheelPos3, WheelOrient3, WheelLinVel3, WheelAngVel3);
  }

  #ifdef WITH_PYTORCH
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
  carla::learning::Inputs NNInput {Wheel0,Wheel1,Wheel2,Wheel3, 
      VehicleControl.Steer, VehicleControl.Throttle, VehicleControl.Brake, 
      NNVerbose};
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
      TRACE_CPUPROFILER_EVENT_SCOPE(RemoveParticlesFromOrderedContainer);
      RemoveParticlesFromOrderedContainer( ParticlesWheel0 );
      RemoveParticlesFromOrderedContainer( ParticlesWheel1 );
      RemoveParticlesFromOrderedContainer( ParticlesWheel2 );
      RemoveParticlesFromOrderedContainer( ParticlesWheel3 );
    }
    {
      TRACE_CPUPROFILER_EVENT_SCOPE(UpdateParticles);
      UpdateParticles(ParticlesWheel0, Output.wheel0._particle_forces, DeltaTime);
      UpdateParticles(ParticlesWheel1, Output.wheel1._particle_forces, DeltaTime);
      UpdateParticles(ParticlesWheel2, Output.wheel2._particle_forces, DeltaTime);
      UpdateParticles(ParticlesWheel3, Output.wheel3._particle_forces, DeltaTime);
    }
    {
      TRACE_CPUPROFILER_EVENT_SCOPE(AddParticles);
      AddParticlesToOrderedContainer( ParticlesWheel0 );
      AddParticlesToOrderedContainer( ParticlesWheel1 );
      AddParticlesToOrderedContainer( ParticlesWheel2 );
      AddParticlesToOrderedContainer( ParticlesWheel3 );
    }
    {
      TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::UpdateTilesHeightMaps);
      UpdateTilesHeightMapsInRadius(LastUpdatedPosition, 4);
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
    float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UpdateParticles);
  UE_LOG(LogCarla, Log, TEXT("%d vs %d"), Particles.size(), Forces.size()/3);
  for (size_t i = 0; i < Particles.size(); i++)
  {
    FVector Force = FVector(Forces[3*i + 0], Forces[3*i + 1], Forces[3*i + 2]);
    FParticle* P = Particles[i];
    FVector Acceleration = Force;
    P->Velocity = P->Velocity + Acceleration*DeltaTime;
    P->Position = P->Position + P->Velocity*DeltaTime;
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

void UCustomTerrainPhysicsComponent::UpdateTilesHeightMaps(
    const std::vector<FParticle*>& Particles)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UpdateTilesHeightMaps);

  float ParticlesInARowInHeightMap = TextureToUpdate->GetSizeX() / (TextureRadius * 2);
  for (size_t i = 0; i < Particles.size(); i++)
  {
    FParticle* P = Particles[i];
    FIntVector TilePosition;
    TilePosition.X = std::floor( P->Position.X / TileSize );
    TilePosition.Y = std::floor( P->Position.Y / TileSize );
  
    FDenseTile& CurrentTile = SparseMap.GetTile(TilePosition.X , TilePosition.Y);
    // We need to find local position of particle
    FDVector ParticleLocalPosition = P->Position - TilePosition;
      
    // Recalculate position to get it into heightmap coords
    ParticleLocalPosition.X /= ( SparseMap.GetTileSize() );
    ParticleLocalPosition.Y /= ( SparseMap.GetTileSize() );
    ParticleLocalPosition.X *= ( ParticlesInARowInHeightMap );
    ParticleLocalPosition.Y *= ( ParticlesInARowInHeightMap );

    uint32_t Index = ParticleLocalPosition.Y * ParticlesInARowInHeightMap + ParticleLocalPosition.X;
    // Compare to the current value, if higher replace 
    if( P->Position.Z > CurrentTile.ParticlesHeightMap[Index] )
      CurrentTile.ParticlesHeightMap[Index] = P->Position.Z;
  }
}

void UCustomTerrainPhysicsComponent::RemoveParticlesFromOrderedContainer(
    const std::vector<FParticle*>& Particles)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(RemoveParticlesFromOrderedContainer);
  
  float ParticlesInARowInHeightMap = TextureToUpdate->GetSizeX() / (TextureRadius * 2);
  for (size_t i = 0; i < Particles.size(); i++)
  {
    FParticle* P = Particles[i];
    FIntVector TilePosition;
    TilePosition.X = std::floor( P->Position.X / TileSize );
    TilePosition.Y = std::floor( P->Position.Y / TileSize );
  
    FDenseTile& CurrentTile = SparseMap.GetTile(TilePosition.X , TilePosition.Y);
    CurrentTile.bHeightmapNeedToUpdate = true;
    // We need to find local position of particle
    FDVector ParticleLocalPosition = P->Position - TilePosition;
    // Recalculate position to get it into heightmap coords
    ParticleLocalPosition.X /= ( SparseMap.GetTileSize() );
    ParticleLocalPosition.Y /= ( SparseMap.GetTileSize() );
    ParticleLocalPosition.X *= ( ParticlesInARowInHeightMap );
    ParticleLocalPosition.Y *= ( ParticlesInARowInHeightMap );

    uint32_t Index = ParticleLocalPosition.Y * ParticlesInARowInHeightMap + ParticleLocalPosition.X;
    CurrentTile.ParticlesZOrdered[Index].erase(P->Position.Z);
  }
}

void UCustomTerrainPhysicsComponent::AddParticlesToOrderedContainer(
    const std::vector<FParticle*>& Particles)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AddParticlesToOrderedContainer);
  
  float ParticlesInARowInHeightMap = TextureToUpdate->GetSizeX() / (TextureRadius * 2);
  for (size_t i = 0; i < Particles.size(); i++)
  {
    FParticle* P = Particles[i];
    FIntVector TilePosition;
    TilePosition.X = std::floor( P->Position.X / TileSize );
    TilePosition.Y = std::floor( P->Position.Y / TileSize );
  
    FDenseTile& CurrentTile = SparseMap.GetTile(TilePosition.X , TilePosition.Y);
    CurrentTile.bHeightmapNeedToUpdate = true;
    // We need to find local position of particle
    FDVector ParticleLocalPosition = P->Position - TilePosition;
      
    // Recalculate position to get it into heightmap coords
    ParticleLocalPosition.X /= ( SparseMap.GetTileSize() );
    ParticleLocalPosition.Y /= ( SparseMap.GetTileSize() );
    ParticleLocalPosition.X *= ( ParticlesInARowInHeightMap );
    ParticleLocalPosition.Y *= ( ParticlesInARowInHeightMap );

    uint32_t Index = ParticleLocalPosition.Y * ParticlesInARowInHeightMap + ParticleLocalPosition.X;
    CurrentTile.ParticlesZOrdered[Index].insert(P->Position.Z);
  }
}

void UCustomTerrainPhysicsComponent::UpdateTilesHeightMapsInRadius(FDVector Position, uint32 Rad )
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UpdateTilesHeightMapsInRadius);

  uint64_t TileId = SparseMap.GetTileId(Position);
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
  if (!PrimitiveComponent)
  {
    UE_LOG(LogCarla, Error, TEXT("ApplyForcesToVehicle Vehicle does not contain UPrimitiveComponent"));
    return;
  }
  if(bDisableVehicleGravity && PrimitiveComponent->IsGravityEnabled())
  {
    PrimitiveComponent->SetEnableGravity(false);
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
  UE_LOG(LogCarla, Log, TEXT("Forces0 %s"), 
      *ForceWheel0.ToString());
  UE_LOG(LogCarla, Log, TEXT("Forces1 %s"), 
      *ForceWheel1.ToString());
  UE_LOG(LogCarla, Log, TEXT("Forces2 %s"), 
      *ForceWheel2.ToString());
  UE_LOG(LogCarla, Log, TEXT("Forces3 %s"), 
      *ForceWheel3.ToString());
  UE_LOG(LogCarla, Log, TEXT("Torque0 %s"), 
      *TorqueWheel0.ToString());
  UE_LOG(LogCarla, Log, TEXT("Torque1 %s"), 
      *TorqueWheel1.ToString());
  UE_LOG(LogCarla, Log, TEXT("Torque2 %s"), 
      *TorqueWheel2.ToString());
  UE_LOG(LogCarla, Log, TEXT("Torque3 %s"), 
      *TorqueWheel3.ToString());
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
    TArray<float>& ParticleVelOut)
{
  ParticlePosOut.Empty();
  ParticleVelOut.Empty();
  ParticlePosOut.Reserve(ParticlesIn.size()*3);
  ParticleVelOut.Reserve(ParticlesIn.size()*3);
  for(FParticle* Particle : ParticlesIn)
  {
    ParticlePosOut.Add(static_cast<float>(Particle->Position.X));
    ParticlePosOut.Add(static_cast<float>(Particle->Position.Y));
    ParticlePosOut.Add(static_cast<float>(Particle->Position.Z));
    ParticleVelOut.Add(Particle->Velocity.X);
    ParticleVelOut.Add(Particle->Velocity.Y);
    ParticleVelOut.Add(Particle->Velocity.Z);
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
      Position = VehicleTransform.TransformPosition(FVector(140, -70, 40));
      break;
    case 1:
      Position = VehicleTransform.TransformPosition(FVector(140, 70, 40));
      break;
    case 2:
      Position = VehicleTransform.TransformPosition(FVector(-140, -70, 40));
      break;
    case 3:
    default:
      Position = VehicleTransform.TransformPosition(FVector(-140, 70, 40));
      break;
  }
  if(LargeMapManager)
  {
    Position = LargeMapManager->LocalToGlobalLocation(Position);
  }
  Position = UEFrameToSI(Position);
  FVector Velocity = UEFrameToSI(Vehicle->GetVelocity());
  WheelPos = {Position.X, Position.Y, Position.Z};
  FQuat Quat = VehicleTransform.GetRotation();
  float sin = FMath::Sin(Quat.W/2.f);
  WheelOrientation = {FMath::Cos(Quat.W/2.f),sin*Quat.X,sin*Quat.Y,sin*Quat.Z};
  WheelLinearVelocity = {Velocity.X, Velocity.Y, Velocity.Z};
  FVector LeftVector = -Quat.GetRightVector();
  LeftVector = LeftVector*(Velocity.Size()/(CMToM*TireRadius));
  WheelAngularVelocity = {LeftVector.X,LeftVector.Y,LeftVector.Z};
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

void UCustomTerrainPhysicsComponent::LoadTilesAtPosition(FVector Position, float RadiusX, float RadiusY)
{
  FDVector PositionTranslated;
  PositionTranslated.X = ( Position.X * 0.01 ) + (WorldSize.X * 0.005f);
  PositionTranslated.Y = ( -Position.Y * 0.01 ) + (WorldSize.Y * 0.005f);
  PositionTranslated.Z = ( Position.Z * 0.01 ) + (WorldSize.Z * 0.005f);
  SparseMap.LoadTilesAtPositionFromCache( PositionTranslated, RadiusX, RadiusY );
}

void UCustomTerrainPhysicsComponent::UnloadTilesAtPosition(FVector Position, float RadiusX, float RadiusY)
{
  FDVector PositionTranslated;
  PositionTranslated.X = ( Position.X * 0.01 ) + (WorldSize.X * 0.005f);
  PositionTranslated.Y = ( -Position.Y * 0.01 ) + (WorldSize.Y * 0.005f);
  PositionTranslated.Z = ( Position.Z * 0.01 ) + (WorldSize.Z * 0.005f);
  SparseMap.UnLoadTilesAtPositionToCache( PositionTranslated, RadiusX, RadiusY );
}

void UCustomTerrainPhysicsComponent::ReloadCache(FVector Position, float RadiusX, float RadiusY)
{
  FDVector PositionTranslated;
  PositionTranslated.X = ( Position.X * 0.01 ) + (WorldSize.X * 0.005f);
  PositionTranslated.Y = ( -Position.Y * 0.01 ) + (WorldSize.Y * 0.005f);
  PositionTranslated.Z = ( Position.Z * 0.01 ) + (WorldSize.Z * 0.005f);
  SparseMap.ReloadCache(PositionTranslated, RadiusX, RadiusY);

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

    if( ( FDateTime::Now() - LoadTilesCurrentTime ).GetTotalMilliseconds() > CustomTerrainComp->TimeToTriggerLoadTiles ){
      CustomTerrainComp->LoadTilesAtPosition( CustomTerrainComp->LastUpdatedPosition, RadiusX, RadiusY );
      LoadTilesCurrentTime = FDateTime::Now();
    }

    if( ( FDateTime::Now() - UnloadTilesCurrentTime ).GetTotalMilliseconds() > CustomTerrainComp->TimeToTriggerUnLoadTiles ){
      CustomTerrainComp->UnloadTilesAtPosition( CustomTerrainComp->LastUpdatedPosition, RadiusX, RadiusY );
      UnloadTilesCurrentTime = FDateTime::Now();
    }
    
    if( ( FDateTime::Now() - LoadTilesCurrentTime ).GetTotalSeconds() > CustomTerrainComp->TimeToTriggerCacheReload ){
      UE_LOG(LogCarla, Warning, TEXT("Tiles Cache reloaded"));
      CustomTerrainComp->ReloadCache(CustomTerrainComp->LastUpdatedPosition, 
        CustomTerrainComp->CacheRadius.X, CustomTerrainComp->CacheRadius.Y);
      CacheCurrentTime = FDateTime::Now();
    }
  }

  return 0;
}
