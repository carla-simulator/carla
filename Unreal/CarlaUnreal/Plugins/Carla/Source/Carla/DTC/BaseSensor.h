#pragma once

#include "CoreMinimal.h"
#include "SensorInterface.h"
#include "DarpaEnums.h"
#include "Components/SceneComponent.h"
#include "BaseSensor.generated.h"



UCLASS(ClassGroup=(Sensors), Abstract)
class CARLA_API UBaseSensor : public USceneComponent, public ISensorInterface
{
	GENERATED_BODY()

	// ---------------------------------
	// --- Variables
	// ---------------------------------
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESensorType Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* Host;

	// ---------------------------------
	// --- Constructors
	// ---------------------------------
	UBaseSensor();

	// ---------------------------------
	// --- Sensor Interface
	// ---------------------------------
public:
	ESensorType GetSensorType_Implementation() const;
};
