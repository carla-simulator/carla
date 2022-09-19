// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "MapGen/SoilTypeManager.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
ASoilTypeManager::ASoilTypeManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASoilTypeManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASoilTypeManager::Tick(float DeltaTime)
{
	if((int)DeltaTime % 2000 == 0)
	{
		ALargeMapManager* LargeMapManager = (ALargeMapManager*) UGameplayStatics::GetActorOfClass(GetWorld(), ALargeMapManager::StaticClass());
		AActor* Car = UGameplayStatics::GetActorOfClass(GetWorld(), PolarisClass);

		if(Car != nullptr)
		{
			FVector CarPos = Car->GetActorLocation();

			// CarPos -= FVector(0.0f, LargeMapManager->GetTileSize() , 0.0f);
			// CarPos -= FVector(0.0f, 1.0f * 1000.0f * 100.0f , 0.0f);
      FVector GlobalCarPos = LargeMapManager->LocalToGlobalLocation(CarPos);
			FIntVector TileVector = LargeMapManager->GetTileVectorID(GlobalCarPos);
			uint64 TileIndex = LargeMapManager->GetTileID(GlobalCarPos);
			
			// int TileX = (int)CarPos.X % (int)LargeMapManager->GetTileSize(); 
			// int TileY = (int)(-1*CarPos.Y) % (int)LargeMapManager->GetTileSize();
			// FVector Tile0Offset = LargeMapManager->GetTile0Offset();
			// int TileX = (int)(CarPos.X - Tile0Offset.X) / (int)LargeMapManager->GetTileSize(); 
			// int TileY = (int)(-1*CarPos.Y - Tile0Offset.Y) / (int)LargeMapManager->GetTileSize();
			// FIntVector TileVector(TileX, TileY,0);

      FString TypeStr = GetTerrainPropertiesAtGlobalLocation(GlobalCarPos).ToString();
			
			UE_LOG(LogCarla, Log, TEXT("Current Tile Index %d ----> (%d, %d, %d) with position L[%f, %f, %f] G[%f, %f, %f] Terrain Type: %s"),
				TileIndex, TileVector.X, TileVector.Y, TileVector.Z, CarPos.X, CarPos.Y, CarPos.Z, GlobalCarPos.X, GlobalCarPos.Y, GlobalCarPos.Z,
        *TypeStr);
		}
	}
}

FSoilTerramechanicsProperties ASoilTypeManager::GetGeneralTerrainProperties()
{
	return GeneralTerrainProperties;
}

FSoilTerramechanicsProperties ASoilTypeManager::GetTerrainPropertiesAtGlobalLocation(FVector VehicleLocation)
{
	// Get Indexes from location
	FIntVector TileVectorID = LargeMapManager->GetTileVectorID(VehicleLocation);

	// Query the map, if not in map, return general
	if(TilesTerrainProperties.Contains(TileVectorID))
		return TilesTerrainProperties[TileVectorID];	// Tile properties
	else
		return GeneralTerrainProperties;		// General properties
}

FSoilTerramechanicsProperties ASoilTypeManager::GetTerrainPropertiesAtLocalLocation(FVector VehicleLocation)
{
  FVector GlobalVehiclePosition = LargeMapManager->LocalToGlobalLocation(VehicleLocation);
  return GetTerrainPropertiesAtGlobalLocation(GlobalVehiclePosition);
}

void ASoilTypeManager::SetGeneralTerrainProperties(FSoilTerramechanicsProperties TerrainProperties)
{
  const FString TerrainPropertiesStr = TerrainProperties.ToString();
	UE_LOG(LogCarla, Log, TEXT("Setting General Terrain Settings %s"), *TerrainPropertiesStr);
	GeneralTerrainProperties = TerrainProperties;
}

void ASoilTypeManager::AddTerrainPropertiesToTile(int TileX, int TileY, FSoilTerramechanicsProperties TerrainProperties)
{
	// Compute ID from X,Y coords
	check(LargeMapManager != nullptr)
  
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
