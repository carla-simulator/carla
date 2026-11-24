// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapGen/SoilTypeManager.h"

#include <util/ue-header-guard-begin.h>
#include "Kismet/GameplayStatics.h"
#include <util/ue-header-guard-end.h>

// Sets default values
ASoilTypeManager::ASoilTypeManager()
{
   // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ASoilTypeManager::BeginPlay()
{
  Super::BeginPlay();
  
}

void ASoilTypeManager::Tick(float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASoilTypeManager::Tick);
  Super::Tick(DeltaTime);
#if WITH_EDITOR // Only for debugging purposes. Requires to activate tick in contructor
  if((int)DeltaTime % 2000 == 0)
  {
    AActor* Car = UGameplayStatics::GetActorOfClass(GetWorld(), CarClass);

    if(Car != nullptr)
    {
      FVector CarPos = Car->GetActorLocation();

      FIntVector TileVector = GetTileVectorID(CarPos);
      FString TypeStr = GetTerrainPropertiesAtGlobalLocation(CarPos).ToString();

      UE_LOG(LogCarla, Log, TEXT("Current Tile (%d, %d, %d) with position [%f, %f, %f] Terrain Type: %s"),
        TileVector.X, TileVector.Y, TileVector.Z, CarPos.X, CarPos.Y, CarPos.Z, *TypeStr);
    }
  }
#endif
}

FSoilTerramechanicsProperties ASoilTypeManager::GetGeneralTerrainProperties()
{
  return GeneralTerrainProperties;
}

FIntVector ASoilTypeManager::GetTileVectorID(const FVector &Location) const
{
  // Tile (0, 0) is centred on Tile0Offset; Y grows the other way in tile
  // indices, as in the tiled maps this grid comes from.
  FIntVector VectorId = FIntVector(
      (Location - (Tile0Offset - FVector(0.5f * TileSize, -0.5f * TileSize, 0)))
      / TileSize);
  VectorId.Y *= -1;
  return VectorId;
}

FSoilTerramechanicsProperties ASoilTypeManager::GetTerrainPropertiesAtGlobalLocation(FVector VehicleLocation)
{
  // Get Indexes from location
  FIntVector TileVectorID = GetTileVectorID(VehicleLocation);

  // Query the map, if not in map, return general
  if(TilesTerrainProperties.Contains(TileVectorID))
    return TilesTerrainProperties[TileVectorID];  // Tile properties
  else
    return GeneralTerrainProperties;    // General properties
}

FSoilTerramechanicsProperties ASoilTypeManager::GetTerrainPropertiesAtLocalLocation(FVector VehicleLocation)
{
  return GetTerrainPropertiesAtGlobalLocation(VehicleLocation);
}

void ASoilTypeManager::SetGeneralTerrainProperties(FSoilTerramechanicsProperties TerrainProperties)
{
  const FString TerrainPropertiesStr = TerrainProperties.ToString();
  UE_LOG(LogCarla, Log, TEXT("Setting General Terrain Settings %s"), *TerrainPropertiesStr);
  GeneralTerrainProperties = TerrainProperties;
}

void ASoilTypeManager::AddTerrainPropertiesToTile(int TileX, int TileY, FSoilTerramechanicsProperties TerrainProperties)
{
  FIntVector TileVectorID(TileX, TileY, 0);

  // Add to map
  if(TerrainProperties.TerrainType == ESoilTerramechanicsType::NONE_SOIL)
    TilesTerrainProperties.Add(TileVectorID, GeneralTerrainProperties);
  else
    TilesTerrainProperties.Add(TileVectorID, TerrainProperties);
}

void ASoilTypeManager::ClearTerrainPropertiesMap()
{
  TilesTerrainProperties.Empty(TilesTerrainProperties.Num());
}
