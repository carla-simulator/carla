// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"

#include "Containers/Array.h"
#include "Containers/EnumAsByte.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralFoliageSpawner.h"
#include "Templates/UnrealTypeTraits.h"
#include "UObject/NoExportTypes.h"

#include "RegionOfInterest.generated.h"


UENUM(BlueprintType)
enum ERegionOfInterestType
{
  NONE,
  TERRAIN_REGION,      
  WATERBODIES_REGION,  // Not Supported yet 
  VEGETATION_REGION
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FRoiTile
{
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite)
  int X;
  UPROPERTY(BlueprintReadWrite)
  int Y;

public:
  FRoiTile() : X(-1), Y(-1) 
  {};

  FRoiTile(int X, int Y)
  {
    this->X = X;
    this->Y = Y;
  };

  FRoiTile(const FRoiTile& Other)
    : FRoiTile(Other.X, Other.Y)
  {}

  bool operator==(const FRoiTile& Other) const
  {
    return Equals(Other);
  }

  bool Equals(const FRoiTile& Other) const
  {
    return (this->X == Other.X) && (this->Y == Other.Y);
  }
};

FORCEINLINE uint32 GetTypeHash(const FRoiTile& Thing)
{
  uint32 Hash = FCrc::MemCrc32(&Thing, sizeof(FRoiTile));
  return Hash;
}

/**
 * 
 */
USTRUCT(BlueprintType)
struct CARLATOOLS_API FRegionOfInterest
{
	GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite)
  TArray<FRoiTile> TilesList;

  UPROPERTY(BlueprintReadWrite)
  TEnumAsByte<ERegionOfInterestType> RegionType = ERegionOfInterestType::NONE;

  FRegionOfInterest()
  {
    TilesList.Empty();
  }

  void AddTile(int X, int Y)
  {
    FRoiTile Tile(X,Y);
    TilesList.Add(Tile);
  }

  TEnumAsByte<ERegionOfInterestType> GetRegionType()
  {
    return this->RegionType;
  }

  template <typename R>
  static FORCEINLINE bool IsTileInRegionsSet(FRoiTile RoiTile, TMap<FRoiTile, R> RoisMap)
  {
    static_assert(TIsDerivedFrom<R, FRegionOfInterest>::IsDerived, 
        "ROIs Map Value type is not an URegionOfInterest derived type.");   
    return RoisMap.Contains(RoiTile);
  }

};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FVegetationROI : public FRegionOfInterest
{
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite)
  TArray<UProceduralFoliageSpawner*> FoliageSpawners;
  
  FVegetationROI() : FRegionOfInterest()
  {
    this->FoliageSpawners.Empty();
  }

  void AddFoliageSpawner(UProceduralFoliageSpawner* Spawner)
  {
    FoliageSpawners.Add(Spawner);
  }

  void AddFoliageSpawners(TArray<UProceduralFoliageSpawner*> Spawners)
  {
    for(UProceduralFoliageSpawner* Spawner : Spawners)
  {
    AddFoliageSpawner(Spawner);
  }
  }

  TArray<UProceduralFoliageSpawner*> GetFoliageSpawners()
  {
    return this->FoliageSpawners;
  }
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FTerrainROI : public FRegionOfInterest
{
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite)
  UMaterialInstanceDynamic* RoiMaterialInstance;

  UPROPERTY(BlueprintReadWrite)
  UTextureRenderTarget2D* RoiHeightmapRenderTarget;

  FTerrainROI() : FRegionOfInterest(), RoiMaterialInstance()
  {}

  // TODO: IsEdge() funtion to avoid transition between tiles that belongs to the same ROI
};
