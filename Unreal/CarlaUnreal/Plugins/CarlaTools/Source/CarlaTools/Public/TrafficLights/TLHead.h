// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "TrafficLights/TLHeadAttachment.h"
#include "TrafficLights/TLModule.h"
#include "TrafficLights/TLOrientation.h"
#include "TrafficLights/TLStyle.h"

#include "TLHead.generated.h"

USTRUCT(BlueprintType)
struct FTLHead
{
	GENERATED_BODY()

	/** Local transform relative to parent pole */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Head")
	FTransform Transform{FTransform::Identity};

	/** Offset transform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Head")
	FTransform Offset{FTransform::Identity};

	/** Head style */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Head")
	ETLStyle Style{ETLStyle::NorthAmerican};

	/** Attachment type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Head")
	ETLHeadAttachment Attachment{ETLHeadAttachment::Ring};

	/** Orientation of the head */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Head")
	ETLOrientation Orientation{ETLOrientation::Vertical};

	//** Has backplate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Head")
	bool bHasBackplate{false};

	/** Modules (one or more) visible on this head */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Head")
	TArray<FTLModule> Modules{};

	/**
	 * OpenDRIVE signal this head shows the state of. Empty = the actor's own SignalID.
	 *
	 * One head of a mast arm can be a protected-turn arrow while the head beside it is the
	 * through movement, and the two are red and green at the same instant. A CARLA traffic
	 * light cannot express that: ATrafficLightBase holds exactly one UTrafficLightComponent
	 * and one ETrafficLightState, and every client and Traffic Manager call is per actor. So
	 * a rig whose heads name different signals is split into one ADigitalTwinsTrafficLight
	 * per distinct SignalID at load time (UMapLogicParser::ApplyLaneIdsFromMapLogic), by the
	 * "Heads" array ExportLogicToJSON writes into map_logic.json -- explicitly, not by
	 * guessing from positions.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Head")
	FString SignalID{};

};
