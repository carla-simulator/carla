// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/MapGen/LargeMapManager.h"

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <util/ue-header-guard-end.h>

#include "SoilTypeManager.generated.h"

UENUM(BlueprintType)
enum ESoilTerramechanicsType
{
  NONE_SOIL = 0,
  DESERT   = 1,
  FOREST   = 2
};

USTRUCT(BlueprintType)
struct CARLA_API FSoilTerramechanicsProperties
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TEnumAsByte<ESoilTerramechanicsType> TerrainType = ESoilTerramechanicsType::NONE_SOIL;

  const FString ToString() const
  {
    switch(TerrainType)
    {
      case ESoilTerramechanicsType::NONE_SOIL:
        return "None";
      case ESoilTerramechanicsType::DESERT:
        return "Desert";
      case ESoilTerramechanicsType::FOREST:
        return "Forest";
    }
    return "";
  };
};

UCLASS()
class CARLA_API ASoilTypeManager : public AActor
{
  GENERATED_BODY()

private:
  UPROPERTY(EditAnywhere)
  FSoilTerramechanicsProperties GeneralTerrainProperties;

  UPROPERTY(EditAnywhere)
  TMap<FIntVector, FSoilTerramechanicsProperties> TilesTerrainProperties;

  UPROPERTY(EditAnywhere)
  ALargeMapManager* LargeMapManager;
  
public:  
  // Sets default values for this actor's properties
  ASoilTypeManager();

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AActor> CarClass;

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

public:  
  UFUNCTION(Category="MapGen|Soil Manager")
  FSoilTerramechanicsProperties GetGeneralTerrainProperties();

  UFUNCTION(Category="MapGen|Soil Manager")
  FSoilTerramechanicsProperties GetTerrainPropertiesAtGlobalLocation(FVector VehicleLocation);

  UFUNCTION(Category="MapGen|Soil Manager")
  FSoilTerramechanicsProperties GetTerrainPropertiesAtLocalLocation(FVector VehicleLocation);

  UFUNCTION(Category="MapGen|Soil Manager")
  void SetGeneralTerrainProperties(FSoilTerramechanicsProperties TerrainProperties);

  UFUNCTION(Category="MapGen|Soil Manager")
  void AddTerrainPropertiesToTile(int TileX, int TileY, FSoilTerramechanicsProperties TerrainProperties);

  UFUNCTION(Category="MapGen|Soil Manager")
  void ClearTerrainPropertiesMap();

  virtual void Tick(float DeltaSeconds) override;

};
