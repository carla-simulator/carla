#include "Carla/DTC/BaseSensor.h"

// ---------------------------------
// --- Constructors
// ---------------------------------
UBaseSensor::UBaseSensor()
{
	PrimaryComponentTick.bCanEverTick = true;

}

ESensorType UBaseSensor::GetSensorType_Implementation() const
{
	return Type;
}
