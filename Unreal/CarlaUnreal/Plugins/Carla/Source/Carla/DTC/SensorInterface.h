#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DarpaEnums.h"
#include "SensorInterface.generated.h"

UINTERFACE(MinimalAPI)
class USensorInterface : public UInterface
{
	GENERATED_BODY()
};

class CARLA_API ISensorInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	ESensorType GetSensorType() const;
};
