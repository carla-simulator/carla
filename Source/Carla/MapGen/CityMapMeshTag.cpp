// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CityMapMeshTag.h"

#include "Package.h"

#include <array>

ECityMapMeshTag CityMapMeshTag::GetBaseMeshTag()
{
  return ECityMapMeshTag::RoadTwoLanes_LaneLeft;
}

uint32 CityMapMeshTag::GetRoadIntersectionSize()
{
  return 5u;
}

FString CityMapMeshTag::ToString(ECityMapMeshTag Tag)
{
  const UEnum* ptr = FindObject<UEnum>(ANY_PACKAGE, TEXT("ECityMapMeshTag"), true);
  if(!ptr)
    return FString("Invalid");
  return ptr->GetEnumName(static_cast<int32>(Tag));
}
