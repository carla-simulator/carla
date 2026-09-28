// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "TrafficLights/TLOrientation.h"
#include "TrafficLights/TLStyle.h"
#include "UObject/ObjectMacros.h"

#include "TLModuleDataTable.generated.h"

USTRUCT(BlueprintType)
struct FTLModuleRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traffic Light|Module")
	ETLStyle Style{ETLStyle::NorthAmerican};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traffic Light|Module")
	ETLOrientation Orientation{ETLOrientation::Vertical};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traffic Light|Module")
	bool bHasVisor{false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traffic Light|Module")
	UStaticMesh* Mesh{nullptr};
};
