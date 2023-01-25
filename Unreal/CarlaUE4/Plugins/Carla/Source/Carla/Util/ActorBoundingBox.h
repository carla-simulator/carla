// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Util/BoundingBox.h"
#include "ActorBoundingBox.generated.h"

USTRUCT(BlueprintType)
struct FActorBoundingBox
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	int32 Id;

	UPROPERTY(BlueprintReadWrite)
	FBoundingBox Box;
};