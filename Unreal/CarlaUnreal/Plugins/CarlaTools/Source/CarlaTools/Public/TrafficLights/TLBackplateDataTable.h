// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "TrafficLights/TLStyle.h"
#include "UObject/ObjectMacros.h"

#include "TLBackplateDataTable.generated.h"

USTRUCT(BlueprintType)
struct FTLBackplateRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traffic Light|Backplate")
	ETLStyle Style{ETLStyle::NorthAmerican};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traffic Light|Backplate")
	UStaticMesh* CornerMesh{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traffic Light|Backplate")
	UStaticMesh* VerticalMesh{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traffic Light|Backplate")
	UStaticMesh* HorizontalMesh{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traffic Light|Backplate")
	UStaticMesh* MiddleMesh{nullptr};
};
