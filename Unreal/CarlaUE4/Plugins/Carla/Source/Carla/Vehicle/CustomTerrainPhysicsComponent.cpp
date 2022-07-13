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
#include "Engine/Texture2D.h"
#include "Rendering/Texture2DResource.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CommandLine.h"
#include "Components/LineBatchComponent.h"
#include "Math/OrientedBox.h"

#include <algorithm>

// constexpr float ParticleDiameter = 0.013f;
// constexpr float TerrainDepth = 0.40f;
constexpr float MToCM = 100.f;
constexpr float CMToM = 1.f/100.f;
 
void FHeightMapData::InitializeHeightmap(UTexture2D* Texture, FDVector Size, FDVector Origin)
{
  WorldSize = Size;
  Offset = Origin;
  Pixels.clear();
  // const FColor* FormatedImageData = 
  //     static_cast<const FColor*>(
  //       Texture->PlatformData->Mips[0].BulkData.LockReadOnly());
  // const FColor* FormatedImageData = 
  //     (const FColor*)(Texture->PlatformData->Mips[0].BulkData.LockReadOnly());
  // Size_X = Texture->GetSizeX();
  // Size_Y = Texture->GetSizeY();
  // for(uint32_t j = 0; j < Size_Y; j++)
  // {
  //   for(uint32_t i = 0; i < Size_X; i++)
  //   {
  //     uint32_t idx = j*Size_X + i;
  //     float HeightLevel = FormatedImageData[idx].R/255.f;
  //     // float HeightLevel = FormatedImageData[idx].R.GetFloat();
  //     Pixels.emplace_back(HeightLevel);
  //   }
  // }
  Size_X = static_cast<uint32_t>(WorldSize.X);
  Size_Y = static_cast<uint32_t>(WorldSize.Y);
  for(uint32_t j = 0; j < Size_Y; j++)
  {
    for(uint32_t i = 0; i < Size_X; i++)
    {
      uint32_t idx = j*Size_X + i;
      float HeightLevel = 0.f;
      // float HeightLevel = FormatedImageData[idx].R.GetFloat();
      Pixels.emplace_back(HeightLevel);
    }
  }
  // Texture->PlatformData->Mips[0].BulkData.Unlock();
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
            {ParticlePosition, FVector(0), ParticleSize/2.f};
      }
    }
  }
}

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
    FVector PToCenter = MToCM*Particle.Position.ToFVector() - OBox.Center;
    if((FMath::Abs(FVector::DotProduct(PToCenter, OBox.AxisX)) < OBox.ExtentX) &&
       (FMath::Abs(FVector::DotProduct(PToCenter, OBox.AxisY)) < OBox.ExtentY) &&
       (FMath::Abs(FVector::DotProduct(PToCenter, OBox.AxisZ)) < OBox.ExtentZ))
    {
      ParticlesInRadius.emplace_back(&Particle);
    }
  }
}

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
  GetTile(Tile_X, Tile_Y).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X-1, Tile_Y-1).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X, Tile_Y-1).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X-1, Tile_Y).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X+1, Tile_Y).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X, Tile_Y+1).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  GetTile(Tile_X+1, Tile_Y+1).GetParticlesInRadius(Position, Radius, ParticlesInRadius);
  return ParticlesInRadius;
}

std::vector<FParticle*> FSparseHighDetailMap::
    GetParticlesInBox(const FOrientedBox& OBox)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::GetParticlesInBox);
  uint64_t TileId = GetTileId(OBox.Center*CMToM);
  uint32_t Tile_X = (uint32_t)(TileId >> 32);
  uint32_t Tile_Y = (uint32_t)(TileId & (uint32_t)(~0));
  // FDenseTile& Tile = GetTile(TileId);
  // return Tile.GetParticlesInRadius(Position, Radius);
  std::vector<FParticle*> ParticlesInRadius;
  GetTile(Tile_X, Tile_Y).GetParticlesInBox(OBox, ParticlesInRadius);
  GetTile(Tile_X-1, Tile_Y-1).GetParticlesInBox(OBox, ParticlesInRadius);
  GetTile(Tile_X, Tile_Y-1).GetParticlesInBox(OBox, ParticlesInRadius);
  GetTile(Tile_X-1, Tile_Y).GetParticlesInBox(OBox, ParticlesInRadius);
  GetTile(Tile_X+1, Tile_Y).GetParticlesInBox(OBox, ParticlesInRadius);
  GetTile(Tile_X, Tile_Y+1).GetParticlesInBox(OBox, ParticlesInRadius);
  GetTile(Tile_X+1, Tile_Y+1).GetParticlesInBox(OBox, ParticlesInRadius);
  return ParticlesInRadius;
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
      ParticleSize, TerrainDepth,
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
  TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::BeginPlay);
  Super::BeginPlay();
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
  if (FParse::Value(FCommandLine::Get(), TEXT("-floor-height="), Value))
  {
    if(FloorActor)
    {
      FVector Location = FloorActor->GetActorLocation();
      Location.Z = MToCM*Value;
      FloorActor->SetActorLocation(Location);
    }
    else
    {
        UE_LOG(LogCarla, Error, TEXT("Floor not set"));
    }
  }
  if (FParse::Value(FCommandLine::Get(), TEXT("-particle-size="), Value))
  {
    ForceMulFactor = Value;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-update-particles")))
  {
    bUpdateParticles = true;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-no-draw-debug-info")))
  {
    DrawDebugInfo = false;
  }
  FString Path;
  if (FParse::Value(FCommandLine::Get(), TEXT("-network-path="), Path))
  {
    NeuralModelFile = Path;
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("-dynamic-model")))
  {
    bUseDynamicModel = true;
  }
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(InitializeDenseMap);
    // torch::Tensor tensor = torch::rand({2, 3});
    // std::cout << tensor << std::endl;
    SparseMap = FSparseHighDetailMap(CMToM*ParticleDiameter, CMToM*TerrainDepth);
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
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(LoadNNModel);
    carla::learning::test_learning();
    TerramechanicsModel.LoadModel(TCHAR_TO_ANSI(*NeuralModelFile));
  }
}

UCustomTerrainPhysicsComponent::UCustomTerrainPhysicsComponent(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryComponentTick.bCanEverTick = true;
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
    RunNNPhysicsSimulation(Vehicle, DeltaTime);
  }
  // RunNNPhysicsSimulation(nullptr, DeltaTime);
  // if(!TextureToUpdate)
  // {
  //   UE_LOG(LogCarla, Log, TEXT("Missing texture"));
  //   return;
  // }

  // UE_LOG(LogCarla, Log, TEXT("Updating texture gamethread with %d Pixels"), TextureToUpdate->GetSizeX()*TextureToUpdate->GetSizeY());
  // TArray<uint8_t> Data;
  // {
  // TRACE_CPUPROFILER_EVENT_SCOPE(SetupData);
  // Data.SetNum(TextureToUpdate->GetSizeX()*TextureToUpdate->GetSizeY());
  // // for(int i = 0; i < Data.Num(); i++)
  // // {
  // //   Data[i] = static_cast<uint8_t>(FMath::RandRange(0,255));
  // // }
  // }
  // ENQUEUE_RENDER_COMMAND(UpdateDynamicTextureCode)
  // (
	// [Data, Texture=TextureToUpdate](auto &InRHICmdList) mutable
  // {
  //   TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::TickComponent Renderthread);
  //   FUpdateTextureRegion2D region;
  //   region.SrcX = 0;
  //   region.SrcY = 0;
  //   region.DestX = 0;
  //   region.DestY = 0;
  //   region.Width = Texture->GetSizeX();
  //   region.Height = Texture->GetSizeY();

  //   FTexture2DResource* resource = (FTexture2DResource*)Texture->Resource;
  //   RHIUpdateTexture2D(
  //       resource->GetTexture2DRHI(), 0, region, region.Width * sizeof(uint8_t), &Data[0]);
  //   UE_LOG(LogCarla, Log, TEXT("Updating texture renderthread with %d Pixels"), Texture->GetSizeX()*Texture->GetSizeY());
  // });
}

void DrawParticles(UWorld* World, std::vector<FParticle*>& Particles)
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
	    LineBatcher->DrawPoint(MToCM*Particle->Position.ToFVector(), 
          FLinearColor(1.f, 0.f, 0.f), 1.0, 0, LifeTime);
  }
}

void DrawOrientedBox(UWorld* World, const TArray<FOrientedBox>& Boxes)
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

void UCustomTerrainPhysicsComponent::RunNNPhysicsSimulation(
    ACarlaWheeledVehicle *Vehicle, float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(RunNNPhysicsSimulation);
//   FVector Position = Vehicle->GetActorLocation();
//   //wheel location from actor center FVector(140, 70, 40)
//   FVector WheelPosition0 = Position + FVector(140, 70, 40);
//   FVector WheelPosition1 = Position + FVector(140, -70, 40);
//   FVector WheelPosition2 = Position + FVector(-140, 70, 40);
//   FVector WheelPosition3 = Position + FVector(-140, -70, 40);
  FTransform VehicleTransform = Vehicle->GetTransform();
  FVector WheelPosition0 = VehicleTransform.TransformPosition(FVector(140, 70, 40));
  FVector WheelPosition1 = VehicleTransform.TransformPosition(FVector(140, -70, 40));
  FVector WheelPosition2 = VehicleTransform.TransformPosition(FVector(-140, 70, 40));
  FVector WheelPosition3 = VehicleTransform.TransformPosition(FVector(-140, -70, 40));
  FOrientedBox BboxWheel0;
  BboxWheel0.AxisX = VehicleTransform.GetUnitAxis(EAxis::X);
  BboxWheel0.AxisY = VehicleTransform.GetUnitAxis(EAxis::Y);
  BboxWheel0.AxisZ = VehicleTransform.GetUnitAxis(EAxis::Z);
  BboxWheel0.Center = WheelPosition0 + FVector(0,0,-TireRadius);
  BboxWheel0.ExtentX = 2.f*FMath::Sqrt(3.f)*TireRadius;
  BboxWheel0.ExtentY = 1.5f * TireWidth;
  BboxWheel0.ExtentZ = SearchDepth;
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
  std::vector<FParticle*> ParticlesWheel0 = 
      SparseMap.GetParticlesInBox(BboxWheel0);
  std::vector<FParticle*> ParticlesWheel1 = 
      SparseMap.GetParticlesInBox(BboxWheel1);
  std::vector<FParticle*> ParticlesWheel2 = 
      SparseMap.GetParticlesInBox(BboxWheel2);
  std::vector<FParticle*> ParticlesWheel3 = 
      SparseMap.GetParticlesInBox(BboxWheel3);
  // std::vector<FParticle*> ParticlesWheel0 = 
  //     SparseMap.GetParticlesInRadius(
  //       WheelPosition0*CMToM, SearchRadius*CMToM);
  // std::vector<FParticle*> ParticlesWheel1 = 
  //     SparseMap.GetParticlesInRadius(
  //       WheelPosition1*CMToM, SearchRadius*CMToM);
  // std::vector<FParticle*> ParticlesWheel2 = 
  //     SparseMap.GetParticlesInRadius(
  //       WheelPosition2*CMToM, SearchRadius*CMToM);
  // std::vector<FParticle*> ParticlesWheel3 = 
  //     SparseMap.GetParticlesInRadius(
  //       WheelPosition3*CMToM, SearchRadius*CMToM);
  
  if(DrawDebugInfo)
  {
    DrawParticles(GetWorld(), ParticlesWheel0);
    DrawParticles(GetWorld(), ParticlesWheel1);
    DrawParticles(GetWorld(), ParticlesWheel2);
    DrawParticles(GetWorld(), ParticlesWheel3);
  }
  UE_LOG(LogCarla, Log, TEXT("Found %d particles in wheel 0"), ParticlesWheel0.size());
  UE_LOG(LogCarla, Log, TEXT("Found %d particles in wheel 1"), ParticlesWheel1.size());
  UE_LOG(LogCarla, Log, TEXT("Found %d particles in wheel 2"), ParticlesWheel2.size());
  UE_LOG(LogCarla, Log, TEXT("Found %d particles in wheel 3"), ParticlesWheel3.size());
  if(ParticlesWheel0.size())
    UE_LOG(LogCarla, Log, 
        TEXT("Wheel0 pos %s particle pos %s"), *(WheelPosition0*CMToM).ToString(), *(ParticlesWheel0[0]->Position.ToString()));
  
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

  carla::learning::Inputs NNInput {Wheel0,Wheel1,Wheel2,Wheel3, 0,0,0, NNVerbose};
  TerramechanicsModel.SetInputs(NNInput);
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(RunModel);
    if(bUseDynamicModel)
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
    UpdateParticles(ParticlesWheel0, Output.wheel0._particle_forces, DeltaTime);
    UpdateParticles(ParticlesWheel1, Output.wheel1._particle_forces, DeltaTime);
    UpdateParticles(ParticlesWheel2, Output.wheel2._particle_forces, DeltaTime);
    UpdateParticles(ParticlesWheel3, Output.wheel3._particle_forces, DeltaTime);
  }

  ApplyForcesToVehicle(Vehicle, 
      ForceMulFactor*MToCM*FVector(
          Output.wheel0.wheel_forces_x,
          Output.wheel0.wheel_forces_y,
          Output.wheel0.wheel_forces_z),
      ForceMulFactor*MToCM*FVector(
          Output.wheel1.wheel_forces_x,
          Output.wheel1.wheel_forces_y,
          Output.wheel1.wheel_forces_z),
      ForceMulFactor*MToCM*FVector(
          Output.wheel2.wheel_forces_x,
          Output.wheel2.wheel_forces_y,
          Output.wheel2.wheel_forces_z),
      ForceMulFactor*MToCM*FVector(
          Output.wheel3.wheel_forces_x,
          Output.wheel3.wheel_forces_y,
          Output.wheel3.wheel_forces_z),
      ForceMulFactor*MToCM*MToCM*FVector(
          Output.wheel0.wheel_torque_x,
          Output.wheel0.wheel_torque_y,
          Output.wheel0.wheel_torque_z),
      ForceMulFactor*MToCM*MToCM*FVector(
          Output.wheel1.wheel_torque_x,
          Output.wheel1.wheel_torque_y,
          Output.wheel1.wheel_torque_z),
      ForceMulFactor*MToCM*MToCM*FVector(
          Output.wheel2.wheel_torque_x,
          Output.wheel2.wheel_torque_y,
          Output.wheel2.wheel_torque_z),
      ForceMulFactor*MToCM*MToCM*FVector(
          Output.wheel3.wheel_torque_x,
          Output.wheel3.wheel_torque_y,
          Output.wheel3.wheel_torque_z));
}

void UCustomTerrainPhysicsComponent::UpdateParticles(
    std::vector<FParticle*> Particles, std::vector<float> Forces,
    float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UpdateParticles);
  for (size_t i = 0; i < Particles.size(); i++)
  {
    FVector Force = FVector(Forces[3*i + 0], Forces[3*i + 1], Forces[3*i + 2]);
    FParticle* P = Particles[i];
    FVector Acceleration = Force;
    P->Velocity = P->Velocity + Acceleration*DeltaTime;
    P->Position = P->Position + P->Velocity*DeltaTime;
  }
}

void UCustomTerrainPhysicsComponent::ApplyForcesToVehicle(
    ACarlaWheeledVehicle *Vehicle,
    FVector ForceWheel0, FVector ForceWheel1, FVector ForceWheel2, FVector ForceWheel3,
    FVector TorqueWheel0, FVector TorqueWheel1, FVector TorqueWheel2, FVector TorqueWheel3)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::ApplyForcesToVehicle);
  // FVector Position = Vehicle->GetActorLocation();
  // //wheel location from actor center FVector(140, 70, 40)
  // FVector WheelPosition0 = Position + FVector(140, 70, 40) + FVector(0,0,50);
  // FVector WheelPosition1 = Position + FVector(140, -70, 40)+ FVector(0,0,50);
  // FVector WheelPosition2 = Position + FVector(-140, 70, 40)+ FVector(0,0,50);
  // FVector WheelPosition3 = Position + FVector(-140, -70, 40)+ FVector(0,0,50);
  FTransform VehicleTransform = Vehicle->GetTransform();
  FVector WheelPosition0 = VehicleTransform.TransformPosition(FVector(140, 70, 40));
  FVector WheelPosition1 = VehicleTransform.TransformPosition(FVector(140, -70, 40));
  FVector WheelPosition2 = VehicleTransform.TransformPosition(FVector(-140, 70, 40));
  FVector WheelPosition3 = VehicleTransform.TransformPosition(FVector(-140, -70, 40));
  UPrimitiveComponent* PrimitiveComponent = 
      Cast<UPrimitiveComponent>(Vehicle->GetRootComponent());
  if (!PrimitiveComponent)
  {
    UE_LOG(LogCarla, Error, TEXT("ApplyForcesToVehicle Vehicle does not contain UPrimitiveComponent"));
    return;
  }
  PrimitiveComponent->AddForceAtLocationLocal(ForceWheel0, FVector(140, 70, 40));
  PrimitiveComponent->AddForceAtLocationLocal(ForceWheel1, FVector(140, -70, 40));
  PrimitiveComponent->AddForceAtLocationLocal(ForceWheel2, FVector(-140, 70, 40));
  PrimitiveComponent->AddForceAtLocationLocal(ForceWheel3, FVector(-140, -70, 40));
  // PrimitiveComponent->AddForceAtLocationLocal(FVector(0,0,ForceWheel0.Z), FVector(140, 70, 40));
  // PrimitiveComponent->AddForceAtLocationLocal(FVector(0,0,ForceWheel1.Z), FVector(140, -70, 40));
  // PrimitiveComponent->AddForceAtLocationLocal(FVector(0,0,ForceWheel2.Z), FVector(-140, 70, 40));
  // PrimitiveComponent->AddForceAtLocationLocal(FVector(0,0,ForceWheel3.Z), FVector(-140, -70, 40));
  PrimitiveComponent->AddTorqueInRadians(TorqueWheel0);
  PrimitiveComponent->AddTorqueInRadians(TorqueWheel1);
  PrimitiveComponent->AddTorqueInRadians(TorqueWheel2);
  PrimitiveComponent->AddTorqueInRadians(TorqueWheel3);
  if (DrawDebugInfo)
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
        WheelPosition0 + FVector(0,0,50)+ ForceWheel0.GetSafeNormal()*ForceMulFactor*10,
        FLinearColor(0.0,1.0,0.0), 0, 3.0, 0.3);
    LineBatcher->DrawLine(WheelPosition1 + FVector(0,0,50),
        WheelPosition1 + FVector(0,0,50)+ ForceWheel1.GetSafeNormal()*ForceMulFactor*10,
        FLinearColor(0.0,1.0,0.0), 0, 3.0, 0.3);
    LineBatcher->DrawLine(WheelPosition2 + FVector(0,0,50),
        WheelPosition2 + FVector(0,0,50)+ ForceWheel2.GetSafeNormal()*ForceMulFactor*10,
        FLinearColor(0.0,1.0,0.0), 0, 3.0, 0.3);
    LineBatcher->DrawLine(WheelPosition3 + FVector(0,0,50),
        WheelPosition3 + FVector(0,0,50)+ ForceWheel3.GetSafeNormal()*ForceMulFactor*10,
        FLinearColor(0.0,1.0,0.0), 0, 3.0, 0.3);
    // UKismetSystemLibrary::DrawDebugLine(
    //     GetWorld(), WheelPosition0 + FVector(0,0,50), 
    //     WheelPosition0 + FVector(0,0,50)+ ForceWheel0.GetSafeNormal()*ForceMulFactor*10,
    //     FLinearColor(0.0,1.0,0.0), 0.3, 3.0);
    // UKismetSystemLibrary::DrawDebugLine(
    //     GetWorld(), WheelPosition1 + FVector(0,0,50), 
    //     WheelPosition1 + FVector(0,0,50) + ForceWheel1.GetSafeNormal()*ForceMulFactor*10,
    //     FLinearColor(0.0,1.0,0.0), 0.3, 3.0);
    // UKismetSystemLibrary::DrawDebugLine(
    //     GetWorld(), WheelPosition2 + FVector(0,0,50), 
    //     WheelPosition2 + FVector(0,0,50) + ForceWheel2.GetSafeNormal()*ForceMulFactor*10,
    //     FLinearColor(0.0,1.0,0.0), 0.3, 3.0);
    // UKismetSystemLibrary::DrawDebugLine(
    //     GetWorld(), WheelPosition3 + FVector(0,0,50), 
    //     WheelPosition3 + FVector(0,0,50) + ForceWheel3.GetSafeNormal()*ForceMulFactor*10,
    //     FLinearColor(0.0,1.0,0.0), 0.3, 3.0);
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

TArray<FVector> UCustomTerrainPhysicsComponent::GetParticlesInRadius(FVector Position, float Radius)
{
  std::vector<FParticle*> Particles = SparseMap.GetParticlesInRadius(Position*CMToM, Radius*CMToM);
  TArray<FVector> ParticlePositions;
  for(FParticle* Particle : Particles)
  {
    ParticlePositions.Add(MToCM*Particle->Position.ToFVector());
  }
  return ParticlePositions;
}

FVector UCustomTerrainPhysicsComponent::GetTileCenter(FVector Position)
{
  
  return 100.f*SparseMap.GetTilePosition(SparseMap.GetTileId(Position/100.f)).ToFVector();
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
    ParticleVelOut.Add(Particle->Velocity.X);
    ParticleVelOut.Add(Particle->Velocity.X);
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
      Position = VehicleTransform.TransformPosition(FVector(140, 70, 40));
      break;
    case 1:
      Position = VehicleTransform.TransformPosition(FVector(140, -70, 40));
      break;
    case 2:
      Position = VehicleTransform.TransformPosition(FVector(-140, 70, 40));
      break;
    case 3:
    default:
      Position = VehicleTransform.TransformPosition(FVector(-140, -70, 40));
      break;
  }
  Position = CMToM*Position;
  FVector Velocity = CMToM*Vehicle->GetVelocity();
  WheelPos = {Position.X, Position.Y, Position.Z};
  WheelOrientation = {1,0,0,0};
  WheelLinearVelocity = {Velocity.X, Velocity.Y, Velocity.Z};
  WheelAngularVelocity = {0,0,0};
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
