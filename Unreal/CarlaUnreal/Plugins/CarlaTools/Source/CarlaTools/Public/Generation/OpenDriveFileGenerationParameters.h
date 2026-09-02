// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "OpenDriveFileGenerationParameters.generated.h"

USTRUCT(BlueprintType)
struct FOpenDriveFileGenerationParameters
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	double DefaultLaneWidth = 3.35;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	double DefaultOSMLayerHeight = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	double DefaultSidewalkWidth = 2;
};