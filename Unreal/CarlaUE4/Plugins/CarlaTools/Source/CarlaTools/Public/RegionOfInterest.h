// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RegionOfInterest.generated.h"


UENUM(BlueprintType)
enum ERegionOfInterestType
{
  NONE,
  TERRAIN_REGION,
  WATERBODIES_REGION,
  VEGETATION_REGION
};

/**
 * 
 */
UCLASS()
class CARLATOOLS_API URegionOfInterest : public UObject
{
	GENERATED_BODY()
	
};
