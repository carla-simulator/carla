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
// #include <carla/pytorch/pytorch.h>

#include <algorithm>
#include <fstream>


constexpr float ParticleDiameter = 0.04f;
constexpr float TerrainDepth = 0.40f;
static FString SavePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());

std::string FParticle::ToString() const{
  return std::string(carla::rpc::FromFString(FString::Printf(TEXT("%.2lf %.2lf %.2lf %.2lf "), 
                                                          Position.X, Position.Y, Position.Z, Radius) + "\n") );
}  

void FParticle::ModifyDataFromString(const std::string& BaseString){
  size_t StartOfSubStr = 0;
  size_t EndOfSubStr = BaseString.find(" ");
  std::string XPosition = BaseString.substr(0, EndOfSubStr - StartOfSubStr);
  // We need to search from the next position to the last place we found delimeter
  StartOfSubStr = EndOfSubStr + 1;
  EndOfSubStr = BaseString.find(" ", StartOfSubStr);
  std::string YPosition = BaseString.substr(StartOfSubStr, EndOfSubStr - StartOfSubStr);

  StartOfSubStr = EndOfSubStr + 1;
  EndOfSubStr = BaseString.find(" ", StartOfSubStr);
  std::string ZPosition = BaseString.substr(StartOfSubStr, EndOfSubStr - StartOfSubStr);

  StartOfSubStr = EndOfSubStr + 1;
  EndOfSubStr = BaseString.find(" ", StartOfSubStr);
  std::string RadiusString = BaseString.substr(StartOfSubStr, EndOfSubStr - StartOfSubStr);

  Position.X = std::stod(XPosition);
  Position.Y = std::stod(YPosition);
  Position.Z = std::stod(ZPosition);
  Radius = std::stod(RadiusString);
}  

void FHeightMapData::InitializeHeightmap(UTexture2D* Texture, FDVector Size, FDVector Origin)
{
  WorldSize = Size;
  Offset = Origin;
  /*
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
      UE_LOG(LogCarla, Log, TEXT("j %d i %d"), j, i);
      uint32_t idx = j*Size_X + i;
      float HeightLevel = FormatedImageData[idx].R/255.f;
      // float HeightLevel = FormatedImageData[idx].R.GetFloat();
      Pixels.emplace_back(HeightLevel);

    }
  }
  Texture->PlatformData->Mips[0].BulkData.Unlock();
  UE_LOG(LogCarla, Log, 
      TEXT("Height Map initialized with %d pixels"), Pixels.size());
  */
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
  std::string FileName = std::string(TCHAR_TO_UTF8(*( SavePath + TileOrigin.ToString() + ".tile" ) ) );

  if( FPaths::FileExists(FString(FileName.c_str())) )
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(DenseTile::InitializeTile);
    
    std::ifstream ReadStream(FileName);
    FVector VectorToRead;
    ReadFVector(ReadStream, VectorToRead );
    TilePosition = FDVector(VectorToRead);
    ReadStdVector<FParticle> (ReadStream, Particles);
  }
  else
  {
    TilePosition = TileOrigin;
    uint32_t NumParticles_X = (TileEnd.X - TileOrigin.X) / ParticleSize;
    uint32_t NumParticles_Y = (TileEnd.Y - TileOrigin.Y) / ParticleSize;
    uint32_t NumParticles_Z = (Depth) / ParticleSize;

    Particles = std::vector<FParticle>(NumParticles_X*NumParticles_Y*NumParticles_Z);
    for(uint32_t i = 0; i < NumParticles_X; i++)
    {
      for(uint32_t j = 0; j < NumParticles_Y; j++)
      {
        FDVector ParticlePosition = TileOrigin + FDVector(i*ParticleSize,j*ParticleSize, 0.f);
        float Height = 0;
        for(uint32_t k = 0; k < NumParticles_Z; k++)
        {
          ParticlePosition.Z = Height - k*ParticleSize;
          Particles[k*NumParticles_X*NumParticles_Y + j*NumParticles_X + i] = 
              {ParticlePosition, ParticleSize/2.f};
        }
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

std::string FDenseTile::ToString() const{
  std::string StringToReturn = std::string(TCHAR_TO_UTF8(*(FString::Printf(TEXT("%.2lf %.2lf %.2lf"), 
                                                          TilePosition.X, TilePosition.Y, TilePosition.Z) + "\n")));
  for( uint32_t i = 0; i < Particles.size(); ++i ){
    StringToReturn += Particles[i].ToString();
  }

  return StringToReturn;
}

void FDenseTile::ModifyDataFromString(const std::string& BaseString){
  size_t StartOfSubStr = 0;
  size_t EndOfSubStr = BaseString.find(" ");
  std::string XPosition = BaseString.substr(0, EndOfSubStr);
  // We need to search from the next position to the last place we found delimeter
  StartOfSubStr = EndOfSubStr + 1;
  EndOfSubStr = BaseString.find(" ", StartOfSubStr);
  std::string YPosition = BaseString.substr(StartOfSubStr, EndOfSubStr - StartOfSubStr);

  StartOfSubStr = EndOfSubStr + 1;
  EndOfSubStr = BaseString.find(" ", StartOfSubStr);
  std::string ZPosition = BaseString.substr(StartOfSubStr, EndOfSubStr - StartOfSubStr);

  TilePosition.X = std::stod(XPosition);
  TilePosition.Y = std::stod(YPosition);
  TilePosition.Z = std::stod(ZPosition);
  
  StartOfSubStr = 0;
  EndOfSubStr = BaseString.find("\n");

  int i = 0;
  while( EndOfSubStr != std::string::npos ){
    std::string NewParticleString = BaseString.substr(StartOfSubStr, EndOfSubStr - StartOfSubStr);
    FParticle NewParticle;
    NewParticle.ModifyDataFromString(NewParticleString);
    Particles.push_back(NewParticle);

    StartOfSubStr = EndOfSubStr +  1;
    EndOfSubStr = BaseString.find("\n", StartOfSubStr );
    ++i;
  }

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
  FDVector TileCenter = GetTilePosition(TileId);
  FDenseTile& Tile = Map[TileId]; 
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
  
  UE_LOG(LogCarla, Log, 
      TEXT("Sparse Map initialized"));


  UE_LOG(LogCarla, Log, 
      TEXT("Map Extension %f %f %f"), MapSize.X, MapSize.Y, MapSize.Z );
}

void FSparseHighDetailMap::Clear()
{
  Heightmap.Clear();
  Map.clear();
}

void FSparseHighDetailMap::LoadTilesAtPosition(FDVector Position, float RadiusX , float RadiusY )
{
  // Translate UE4 Position to our coords
  
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition);
  std::vector<uint64_t> KeysToRemove;
  std::vector<uint64_t> KeysToLoad;
  float MinX = std::max(Position.X - RadiusX, 0.0);
  float MinY = std::max(Position.Y - RadiusX, 0.0);
  float MaxX = std::min(Position.X + RadiusX, Extension.X - 1.0 );
  float MaxY = std::min(Position.Y - RadiusY, Extension.Y - 1.0 );

  {
    TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition::Process);
    
    for(auto it : Map) 
    {
      KeysToRemove.push_back(it.first);
    }
    {
      FScopeLock ScopeLock(&Lock_Map);
      for(float X = MinX; X < MaxX; X+=TileSize )
      {
        for(float Y = MinY; Y < MaxY; Y+=TileSize )
        {
          uint64_t CurrentTileID = GetTileId(FDVector(X,Y,0));
          if( Map.find(CurrentTileID) != Map.end() )
          {
            KeysToRemove.erase(std::remove_if( KeysToRemove.begin(), KeysToRemove.end(), [CurrentTileID](const uint64_t& x) { 
              return x == CurrentTileID; 
            }), KeysToRemove.end());    
          }
          else
          {
            KeysToLoad.push_back(CurrentTileID);
            Map.emplace(CurrentTileID, FDenseTile() ); 
          }
        }
      }
    }
  }

  {
    TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition::Load);
    for(auto it : KeysToLoad) 
    {
      InitializeRegion(it);
    }  
  }

  {
    TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition::Save);
    for(auto it : KeysToRemove) 
    {
      std::string FileToSavePath = std::string(TCHAR_TO_UTF8(*( SavePath + Map[it].TilePosition.ToString() + ".tile")));
      std::ofstream OutputStream(FileToSavePath.c_str());
      WriteFVector(OutputStream, Map[it].TilePosition.ToFVector());
      WriteStdVector<FParticle> (OutputStream, Map[it].Particles);
      OutputStream.close();
    }   
  }
  {
    FScopeLock ScopeLock(&Lock_Map);
    for(auto it : KeysToRemove) 
    {
      Map.erase(it);
    } 
  }

  UpdateTexture(RadiusX, RadiusY);
}

void FSparseHighDetailMap::Update(FVector Position, float RadiusX, float RadiusY)
{

  float MinX = std::max(Position.X - RadiusX, 0.0f);
  float MinY = std::max(Position.Y - RadiusX, 0.0f);
  float MaxX = std::min(Position.X + RadiusX, static_cast<float>( Extension.X ) - 1.0f );
  float MaxY = std::min(Position.Y - RadiusY, static_cast<float>( Extension.Y ) - 1.0f );
  for(float X = MinX; X < MaxX ; X += TileSize ) 
  {
    for(float Y = MinY ; Y < MaxY; Y += TileSize )
    {
      bool ConditionToStopWaiting = true;
      while(ConditionToStopWaiting) 
      {
        uint64_t CurrentTileID = GetTileId(FDVector(X,Y,0));
        Lock_Map.Lock();
        ConditionToStopWaiting = Map.find(CurrentTileID) == Map.end();
        Lock_Map.Unlock();
        if (ConditionToStopWaiting)
        {
          UE_LOG(LogCarla, Log, TEXT("Loop Waiting for Tile %d"), CurrentTileID);
        }
      }
    }
  }

 // Check if tiles are already loaded
 // If they are send position
 // If not wait until loaded
}

void FSparseHighDetailMap::SaveMap()
{
  std::string StringToBeWritten;

  UE_LOG(LogCarla, Warning, TEXT("Save directory %s"), *SavePath );
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::SaveMap);
  for(auto& it: Map)
  {
    StringToBeWritten += it.second.ToString();
    std::string FileToSavePath = std::string(TCHAR_TO_UTF8(*( SavePath + it.second.TilePosition.ToString() + ".tile")));
    std::ofstream OutputStream(FileToSavePath.c_str());
    WriteFVector(OutputStream, it.second.TilePosition.ToFVector());
    WriteStdVector<FParticle> (OutputStream, it.second.Particles);
    OutputStream.close();
  }
}

// In the component
void FSparseHighDetailMap::UpdateTexture(float RadiusX, float RadiusY)
{
  FDVector TextureOrigin = PositionToUpdate;
  TextureOrigin.X -= RadiusX;
  TextureOrigin.Y -= RadiusY;
  float Limit = RadiusX*2 + RadiusY*2;
  for(auto it : Map) 
  {
    for( const FParticle& CurrentParticle : it.second.Particles )
    {
      FDVector LocalPosition = CurrentParticle.Position - TextureOrigin;
      int32_t Index = LocalPosition.X * (RadiusX * 2.0f) + LocalPosition.Y;
      if ( Index > 0 && Index < Limit ){
        Data[Index] = std::max(Data[Index] , CurrentParticle.Position.Z);
      }
    }
  }

  ENQUEUE_RENDER_COMMAND(UpdateDynamicTextureCode)
  (
    // Not move this, move data
    [this, Texture=TextureToUpdate](auto &InRHICmdList) mutable
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(UCustomTerrainPhysicsComponent::TickComponent Renderthread);
    FUpdateTextureRegion2D region;
    region.SrcX = 0;
    region.SrcY = 0;
    region.DestX = 0;
    region.DestY = 0;
    region.Width = Texture->GetSizeX();
    region.Height = Texture->GetSizeY();

    FTexture2DDynamicResource* resource = (FTexture2DDynamicResource*)Texture->Resource;
    RHIUpdateTexture2D(
        resource->GetTexture2DRHI(), 0, region, region.Width * region.Height * sizeof(float), (uint8*)&(this->Data[0]) );
    UE_LOG(LogCarla, Log, TEXT("Updating texture renderthread with %d Pixels"), Texture->GetSizeX()*Texture->GetSizeY());
  });
}

UCustomTerrainPhysicsComponent::UCustomTerrainPhysicsComponent()
  : Super()
{
  PrimaryComponentTick.bCanEverTick = true;
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

  FString LevelName = GetWorld()->GetMapName();
	LevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
  SavePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + LevelName + "_Terrain/";
  // Creating the FileManager
  IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
  if( FileManager.CreateDirectory(*SavePath)){
    UE_LOG(LogCarla, Warning, 
        TEXT("Folder was created at %s"), *SavePath);
  }else{
    UE_LOG(LogCarla, Error, 
        TEXT("Folder was not created at %s"), *SavePath);  
  }
  TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::InitializeMap);
  SparseMap.InitializeMap(HeightMap, FDVector(0.f,0.f,0.f), WorldSize/100.f);

  if (TilesWorker == nullptr)
  {
    TilesWorker = new FTilesWorker(this, GetOwner()->GetActorLocation(), Radius.X, Radius.Y);
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
  Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
  
  FVector LocationVector;
  LocationVector.X = FMath::FRandRange( 0.0f, 100.0f ) * 0.1;
  LocationVector.Y = FMath::FRandRange( 0.0f, 100.0f ) * 0.1f;
  LocationVector.Z = 0.0f;
   GetOwner()->SetActorLocation(LocationVector);

  LastUpdatedPosition = GetOwner()->GetActorLocation();
  SparseMap.Update( LastUpdatedPosition, Radius.X, Radius.Y );

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

void UCustomTerrainPhysicsComponent::LoadTilesAtPosition(FVector Position, float RadiusX, float RadiusY)
{
  FDVector PositionTranslated;
  PositionTranslated.X = -Position.X;
  PositionTranslated.Y =  Position.Z;
  PositionTranslated.Z =  Position.Y;
  SparseMap.LoadTilesAtPosition(PositionTranslated, RadiusX, RadiusY);

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

  while(bShouldContinue){
    CustomTerrainComp->LoadTilesAtPosition(CustomTerrainComp->LastUpdatedPosition, RadiusX, RadiusY);
  }
  return 0;
}
