// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CarlaSunSky.generated.h"

UCLASS()
class CARLATOOLS_API ACarlaSunSky : public AActor
{
	GENERATED_BODY()
	
public:	

	UFUNCTION(BlueprintCallable, Category = "Sun Sky Utilities", meta = (KeyWords = "SunSky, Utils"))
	void SetRealTimeCapture(USkyLightComponent* SkyLightComp, bool NewRTC);
};
