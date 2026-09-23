// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/StaticMeshComponent.h"
#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "TrafficLights/TLHead.h"
#include "TrafficLights/TLOrientation.h"
#include "TrafficLights/TLStyle.h"

#include "TLPole.generated.h"

USTRUCT(BlueprintType)
struct FTLPole
{
	GENERATED_BODY()

	/** Local transform relative to parent head */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Pole")
	FTransform Transform{FTransform::Identity};

	/** Offset transform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Pole")
	FTransform Offset{FTransform::Identity};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Pole")
	ETLStyle Style{ETLStyle::NorthAmerican};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Pole")
	ETLOrientation Orientation{ETLOrientation::Vertical};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Pole")
	float PoleHeight{500.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Pole")
	TArray<FTLHead> Heads;

	UPROPERTY(Transient)
	UStaticMesh* BasePoleMesh{nullptr};

	UPROPERTY(Transient)
	UStaticMeshComponent* BasePoleMeshComponent{nullptr};

	UPROPERTY(Transient)
	UStaticMesh* ExtensiblePoleMesh{nullptr};

	UPROPERTY(Transient)
	UStaticMeshComponent* ExtensiblePoleMeshComponent{nullptr};

	UPROPERTY(Transient)
	UStaticMesh* CapPoleMesh{nullptr};

	UPROPERTY(Transient)
	UStaticMeshComponent* CapPoleMeshComponent{nullptr};

	UPROPERTY(Transient)
	UStaticMesh* ConnectorPoleMesh{nullptr};

	UPROPERTY(Transient)
	UStaticMeshComponent* ConnectorPoleMeshComponent{nullptr};
};
