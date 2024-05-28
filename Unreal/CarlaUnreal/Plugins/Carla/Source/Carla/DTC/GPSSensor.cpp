#include "Carla/DTC/GPSSensor.h"


UGPSSensor::UGPSSensor()
{
	VehicleActor = GetOwner();
}

void UGPSSensor::BeginPlay()
{
	Super::BeginPlay();

	
}

void UGPSSensor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UGPSSensor::UpdateGPSData()
{

}


