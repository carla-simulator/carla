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

#include <algorithm>
#include <fstream>

constexpr float ParticleDiameter = 0.02f;
constexpr float TerrainDepth = 0.40f;

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
      FDVector TileOrigin, FDVector TileEnd, const FHeightMapData &HeightMap,
      const FString& NewSavePath)
{
  SavePath = NewSavePath;
  std::string FileName = std::string(TCHAR_TO_UTF8(*( SavePath + TileOrigin.ToString() + ".tile" ) ) );
  if ( FMath::IsNearlyZero(TileOrigin.X) && FMath::IsNearlyZero(TileOrigin.Y) )
  {
    UE_LOG(LogCarla, Error, TEXT("Tile Origin 0 0"));
  }
  
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
  Tile.InitializeTile(
      ParticleDiameter, TerrainDepth,
      TileCenter, TileCenter + FDVector(TileSize, TileSize, 0.f),
      Heightmap, SavePath);
  return Tile;
}

void FSparseHighDetailMap::InitializeMap(UTexture2D* HeightMapTexture,
      FDVector Origin, FDVector MapSize,const FString& NewSavePath, float Size)
{
  Tile0Position = Origin;
  TileSize = Size;
  Extension = MapSize;
  SavePath = NewSavePath;
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
  std::unordered_set<uint64_t> KeysToRemove;
  std::unordered_set<uint64_t> KeysToLoad;

  double MinX = std::min( std::max( Position.X - RadiusX, 0.0) , Extension.X - 1.0);
  double MinY = std::min( std::max( Position.Y - RadiusY, 0.0) , Extension.Y - 1.0);
  double MaxX = std::min( std::max( Position.X + RadiusX, 0.0) , Extension.X - 1.0);
  double MaxY = std::min( std::max( Position.Y + RadiusY, 0.0) , Extension.Y - 1.0);

  FIntVector MinVector = GetVectorTileId(FDVector(MinX, MinY, 0));
  FIntVector MaxVector = GetVectorTileId(FDVector(MaxX, MaxY, 0));
  
  //UE_LOG(LogCarla, Log, TEXT("Loading Tiles Between X: %d %d Y: %d %d "), MinVector.X, MaxVector.X, MinVector.Y, MaxVector.Y );
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(FSparseHighDetailMap::LoadTilesAtPosition::Process);
    for(auto it : Map) 
    {
      KeysToRemove.emplace(it.first);
    }
    {
      FScopeLock ScopeLock(&Lock_Map);
      for(uint32_t X = MinVector.X; X <= MaxVector.X; X++ )
      {
        for(uint32_t Y = MinVector.Y; Y <= MaxVector.Y; Y++ )
        {
          uint64_t CurrentTileID = GetTileId(X,Y);
          if( Map.find(CurrentTileID) != Map.end() )
          {
            KeysToRemove.erase(CurrentTileID);    
          }
          else
          {
            KeysToLoad.insert(CurrentTileID);
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
}

void FSparseHighDetailMap::Update(FVector Position, float RadiusX, float RadiusY)
{
  FVector PositionTranslated;
  PositionTranslated.X = ( Position.X * 0.01 ) + (Extension.X * 0.5f);
  PositionTranslated.Y = (-Position.Y * 0.01 ) + (Extension.Y * 0.5f);
  PositionTranslated.Z = ( Position.Z * 0.01 ) + (Extension.Z * 0.5f);
  PositionToUpdate = PositionTranslated;
    
  double MinX = std::min( std::max( PositionTranslated.X - RadiusX, 0.0f) , static_cast<float>(Extension.X - 1.0f) );
  double MinY = std::min( std::max( PositionTranslated.Y - RadiusY, 0.0f) , static_cast<float>(Extension.Y - 1.0f) );
  double MaxX = std::min( std::max( PositionTranslated.X + RadiusX, 0.0f) , static_cast<float>(Extension.X - 1.0f) );
  double MaxY = std::min( std::max( PositionTranslated.Y + RadiusY, 0.0f) , static_cast<float>(Extension.Y - 1.0f) );

  FIntVector MinVector = GetVectorTileId(FDVector(MinX, MinY, 0));
  FIntVector MaxVector = GetVectorTileId(FDVector(MaxX, MaxY, 0));
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("FSparseHighDetailMap::Update"));
    for(uint32_t X = MinVector.X; X <= MaxVector.X; X++ )
    {
      for(uint32_t Y = MinVector.Y; Y <= MaxVector.Y; Y++ )
      {
        bool ConditionToStopWaiting = true;
        int32_t Counter = 0;
        while(ConditionToStopWaiting) 
        {
          uint64_t CurrentTileID = GetTileId(X,Y);
          Lock_Map.Lock();
          ConditionToStopWaiting = Map.find(CurrentTileID) == Map.end();
          Lock_Map.Unlock();
          ++Counter;
          if( Counter > 1000 ){
              UE_LOG(LogCarla, Log, TEXT("Waiting long for Tile %lld PosX %d PosY %d"), CurrentTileID, X, Y );            
              UE_LOG(LogCarla, Log, TEXT("Loadeding Tiles Between X: %d, %d, Y: %d, %d"), MinVector.X, MaxVector.X, MinVector.Y, MaxVector.Y );
              Counter = 0;
          }
        }
      }
    }
  }
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

void UCustomTerrainPhysicsComponent::UpdateTexture(float RadiusX, float RadiusY)
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
  UpdateTextureData();
}

void UCustomTerrainPhysicsComponent::UpdateTextureData()
{

  float LimitX = TextureToUpdate->GetSizeX(); 
  float LimitY = TextureToUpdate->GetSizeY();

  FDVector OriginPosition = SparseMap.PositionToUpdate;
  OriginPosition.Z = 0; 
  FScopeLock ScopeLock(&SparseMap.Lock_Map);
  
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UCustomTerrainPhysicsComponent::UpdateTextureData"));
    for(auto it : SparseMap.Map)
    {
      for( auto it2 : it.second.Particles ) 
      {
        FDVector ParticlePos = it2.Position;
        ParticlePos.Z = 0;      

        // Calculate vector between origin position and current particle position
        FDVector ParticleOriginVector = (ParticlePos - OriginPosition);


        // We need to know if the checked particles is inside of area of texture
        // Check if the position is in the desired radius
        if( ParticleOriginVector.Size() < TextureRadius )
        {
          // Create vector which is smaller than unit vector to get position in the texture
          FDVector TexturePosition = ParticleOriginVector * (1/TextureRadius);
          
          // Move Vectors Center to 0.5
          TexturePosition += 1;
          TexturePosition *= 0.5f;

          // Scale positions according to texture size
          TexturePosition.X = std::floor(TexturePosition.X * LimitX);
          TexturePosition.Y = std::floor(TexturePosition.Y * LimitY);      
          
          // Now we access to the proper position of the Data array and we update the value
          int32 Index = TexturePosition.X + (LimitX * TexturePosition.Y);
          uint8 CuurentHeightValue =  it2.Position.Z * -100;
          Data[Index] = std::max( Data[Index], CuurentHeightValue );
        }
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
  Super::BeginPlay();

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
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("FSparseHighDetailMap::InitializeMap"));
    SparseMap.InitializeMap(HeightMap, FDVector(0.f,0.f,0.f), WorldSize/100.f, SavePath);
  }

  InitTexture();

  UE_LOG(LogCarla, Log, TEXT("MainThread Data ArraySize %d "), Data.Num());
  UE_LOG(LogCarla, Log, TEXT("Map Size %d "), SparseMap.Map.size() );

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

  if( Vehicles.Num() == 0 ){
    TArray<AActor*> FoundCars;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(),  ACarlaWheeledVehicle::StaticClass(), FoundCars);
    for( auto Car : FoundCars ){
      ACarlaWheeledVehicle* CurrentCar = Cast<ACarlaWheeledVehicle>(Car);
      if( CurrentCar ){
        Vehicles.Add(CurrentCar);
      }
    }
  }
  if( Vehicles.Num() != 0 ){
    LastUpdatedPosition = Vehicles[0]->GetActorLocation();
    /*for (ACarlaWheeledVehicle* Vehicle : Vehicles)
    {
    }
    */
  }else{
    // If no cars in the map updated position is 0,0
    NextPositionToUpdate.X = 0;
    NextPositionToUpdate.Y = 0;
    LastUpdatedPosition = NextPositionToUpdate;
  }

  
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
    MPCInstance->SetScalarParameterValue("EffectMultiplayer", EffectMultiplayer); 
  }

  // This functions it the loop which waits for the proper tiles to be loaded
  SparseMap.Update( LastUpdatedPosition, Radius.X, Radius.Y );
  // This function update texture data
  UpdateTexture(Radius.X, Radius.Y);
}

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
  PositionTranslated.X = ( Position.X * 0.01 ) + (WorldSize.X * 0.005f);
  PositionTranslated.Y = (-Position.Y * 0.01 ) + (WorldSize.Y * 0.005f);
  PositionTranslated.Z = ( Position.Z * 0.01 ) + (WorldSize.Z * 0.005f);

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
