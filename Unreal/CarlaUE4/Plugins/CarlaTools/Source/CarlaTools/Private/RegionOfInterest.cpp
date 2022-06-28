// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "RegionOfInterest.h"

URegionOfInterest::URegionOfInterest()
{
  TilesList.Empty();
}

void URegionOfInterest::AddTile(int X, int Y)
{
  FRoiTile Tile(X,Y);
  TilesList.Add(Tile);
}

TEnumAsByte<ERegionOfInterestType> URegionOfInterest::GetRegionType()
{
  return this->RegionType;
}

bool URegionOfInterest::IsTileInRegion(int X, int Y)
{
  FRoiTile TileToCheck(X, Y);
  for(FRoiTile Tile : TilesList)
  {
    if(Tile == TileToCheck)
      return true; 
  }
  return false;
}


UVegetationROI::UVegetationROI() : URegionOfInterest()
{
  this->FoliageSpawners.Empty();
}

void UVegetationROI::AddFoliageSpawner(UProceduralFoliageSpawner* Spawner)
{
  FoliageSpawners.Add(Spawner);
}

void UVegetationROI::AddFoliageSpawners(TArray<UProceduralFoliageSpawner*> Spawners)
{
  for(UProceduralFoliageSpawner* Spawner : Spawners)
  {
    AddFoliageSpawner(Spawner);
  }

}

TArray<UProceduralFoliageSpawner*> UVegetationROI::GetFoliageSpawners()
{
  return this->FoliageSpawners;
}