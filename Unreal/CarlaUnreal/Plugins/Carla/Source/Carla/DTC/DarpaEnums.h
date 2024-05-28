#pragma once

#include "CoreMinimal.h"

// ---------------------------------
// --- Enums
// ---------------------------------
UENUM(BlueprintType)
enum class ESensorType : uint8
{
	ST_Base			UMETA(DisplayName = "Base"),
	ST_Location     UMETA(DisplayName = "Location"),
	ST_IMU			UMETA(DisplayName = "IMU"),
	ST_RGBCam		UMETA(DisplayName = "RGBCam")
};