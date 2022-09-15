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

		if(Car != nullptr && LargeMapManager->IsTileLoaded(0))
		{
			FVector CarPos = Car->GetActorLocation();

			CarPos -= FVector(0.0f, LargeMapManager->GetTileSize() , 0.0f);
			// CarPos -= FVector(0.0f, 1.0f * 1000.0f * 100.0f , 0.0f);
			// FIntVector TileVector = LargeMapManager->GetTileVectorID(CarPos);
			uint64 TileIndex = LargeMapManager->GetTileID(CarPos);
			
			// int TileX = (int)CarPos.X % (int)LargeMapManager->GetTileSize(); 
			// int TileY = (int)(-1*CarPos.Y) % (int)LargeMapManager->GetTileSize();
			FVector Tile0Offset = LargeMapManager->GetTile0Offset();
			int TileX = (int)(CarPos.X - Tile0Offset.X) / (int)LargeMapManager->GetTileSize(); 
			int TileY = (int)(-1*CarPos.Y - Tile0Offset.Y) / (int)LargeMapManager->GetTileSize();
			FIntVector TileVector(TileX, TileY,0);
			
			UE_LOG(LogCarla, Log, TEXT("Current Tile Index %d ----> (%d, %d) with position [%f, %f, %f] %f"),
				TileIndex, TileVector.X, TileVector.Y, CarPos.X, CarPos.Y, CarPos.Z, LargeMapManager->GetTileSize() );
		}
	}
}

FSoilTerramechanicsProperties ASoilTypeManager::GetGeneralTerrainProperties()
{
	return GeneralTerrainProperties;
}

FSoilTerramechanicsProperties ASoilTypeManager::GetTerrainPropertiesAtLocation(FVector VehicleLocation)
{
	// Get Indexes from location
	// TODO
	int TileX = 0;
	int TileY = 0;

	// Get ID from indexes
	uint16 TileID = LargeMapManager->GetTileID(FIntVector(TileX, TileY, 0));

	// Query the map, if not in map, return general
	if(TilesTerrainProperties.Contains(TileID))
		return TilesTerrainProperties[TileID];	// Tile properties
	else
		return GeneralTerrainProperties;		// General properties
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
	uint64 TileID = LargeMapManager->GetTileID(FIntVector(TileX, TileY, 0));

	// Add to map
  if(TerrainProperties.TerrainType == ESoilTerramechanicsType::NONE_SOIL)
	  TilesTerrainProperties.Add(TileID, GeneralTerrainProperties);
  else
	  TilesTerrainProperties.Add(TileID, TerrainProperties);
}

void ASoilTypeManager::ClearTerrainPropertiesMap()
{
	TilesTerrainProperties.Empty(TilesTerrainProperties.Num());
}
