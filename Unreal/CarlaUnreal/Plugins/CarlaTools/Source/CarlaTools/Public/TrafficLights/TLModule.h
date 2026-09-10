// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/StaticMeshComponent.h"
#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "Misc/Guid.h"
#include "TrafficLights/TLLightType.h"

#include "TLModule.generated.h"

USTRUCT(BlueprintType)
struct FTLModuleLight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Module")
	int32 U{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Module")
	int32 V{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Module")
	float EmissiveIntensity{25.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Module")
	FLinearColor EmissiveColor{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Module")
	ETLLightType LightType{ETLLightType::SolidColorGreen};

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* LightMID{nullptr};
};

USTRUCT(BlueprintType)
struct FTLModule
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Module")
	TArray<int32> LaneIds{};

    /** Local transform relative to parent head */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Module")
	FTransform Transform{FTransform::Identity};

	/** Offset transform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Module")
	FTransform Offset{FTransform::Identity};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Module")
	bool bHasVisor{false};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Module")
	TArray<FTLModuleLight> Lights{};

	UPROPERTY(Transient)
	UStaticMesh* ModuleMesh{nullptr};

	UPROPERTY(Transient)
	UStaticMeshComponent* ModuleMeshComponent{nullptr};
};
