// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"

#include "Containers/Array.h"
#include "Containers/EnumAsByte.h"
#include "ProceduralFoliageSpawner.h"
#include "UObject/NoExportTypes.h"

#include "RegionOfInterest.generated.h"


UENUM(BlueprintType)
enum ERegionOfInterestType
{
  NONE,
  TERRAIN_REGION,      // Not Supported yet
  WATERBODIES_REGION,  // Not Supported yet 
  VEGETATION_REGION
};


// USTRUCT(BlueprintType)
// struct CARLATOOLS_API FRegionOfInterest
// {
//   GENERATED_USTRUCT_BODY();

  

// }

USTRUCT(BlueprintType)
struct CARLATOOLS_API FRoiTile
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY()
  int X;
  UPROPERTY()
  int Y;

public:
  FRoiTile(
  )
  {
    this->X = -1;
    this->Y = -1;
  };

  FRoiTile(int X, int Y)
  {
    this->X = X;
    this->Y = Y;
  };

  inline bool operator==(FRoiTile other)
  {
    return (this->X == other.X) || (this->Y == other.Y);
  }
};

/**
 * 
 */
UCLASS()
class CARLATOOLS_API URegionOfInterest : public UObject
{
	GENERATED_BODY()

private:
  UPROPERTY()
  TArray<FRoiTile> TilesList;

  UPROPERTY()
  TEnumAsByte<ERegionOfInterestType> RegionType = ERegionOfInterestType::NONE;

public:
  URegionOfInterest();

  UFUNCTION()
  void AddTile(int X, int Y);

  UFUNCTION()
  TEnumAsByte<ERegionOfInterestType> GetRegionType();

  UFUNCTION()
  bool IsTileInRegion(int X, int Y);

};

UCLASS(BlueprintType)
class CARLATOOLS_API UVegetationROI : public URegionOfInterest
{
  GENERATED_BODY()

private:
  UPROPERTY()
  TArray<UProceduralFoliageSpawner*> FoliageSpawners;

public:
  
  UVegetationROI();

  UFUNCTION()
  void AddFoliageSpawner(UProceduralFoliageSpawner* Spawner);

  UFUNCTION()
  void AddFoliageSpawners(TArray<UProceduralFoliageSpawner*> Spawners);

  UFUNCTION()
  TArray<UProceduralFoliageSpawner*> GetFoliageSpawners();
};