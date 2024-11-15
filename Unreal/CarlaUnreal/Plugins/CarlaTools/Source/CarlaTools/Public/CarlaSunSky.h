// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <util/ue-header-guard-end.h>

#include "CarlaSunSky.generated.h"

/*
Class created to hold functionality of the "BP_CarlaSky" that is innaccesible for the blueprint editor to access
*/

UCLASS()
class CARLATOOLS_API ACarlaSunSky : public AActor
{
	GENERATED_BODY()
	
public:	
	//Controls wether the skylight component has "bRealTimeCapture" ON or OFF
	UFUNCTION(BlueprintCallable, Category = "Sun Sky Utilities", meta = (KeyWords = "SunSky, Utils"))
	void SetRealTimeCapture(USkyLightComponent* SkyLightComp, bool NewRTC);
};
