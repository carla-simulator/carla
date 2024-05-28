#include "Carla/DTC/IMUSensor.h"
#include "Components/PrimitiveComponent.h"
#include <iostream>
// ---------------------------------
// --- Constructors
// ---------------------------------

UIMUSensor::UIMUSensor()
{
	OldLinearVelocity = FVector(0.);
	Identity = { 1,0,0,0,1,0,0,0,1 };
	SetCollisionEnabled(ECollisionEnabled::ProbeOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereRadius = 10.f;
}

// ---------------------------------
// --- Implementation
// ---------------------------------

void UIMUSensor::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	DoMathAndCallEventAndSetOldVelocity(DeltaTime);
}

void UIMUSensor::DoMathAndCallEventAndSetOldVelocity(float dt)
{
	AActor* owner = GetOwner();
	if (owner == nullptr)
	{
		return;
	}
	
	IMUData.SetData(GetComponentRotation().Quaternion(), Identity,
			GetPhysicsAngularVelocityInRadians(), Identity,
			(GetPhysicsLinearVelocity() - OldLinearVelocity) / dt, Identity);

	IMUDispatch.Broadcast(IMUData);
	OldLinearVelocity = GetPhysicsLinearVelocity();
}

ESensorType UIMUSensor::GetSensorType_Implementation() const
{
	return ESensorType::ST_IMU;
}
