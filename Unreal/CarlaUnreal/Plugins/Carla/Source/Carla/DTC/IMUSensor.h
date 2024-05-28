#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Carla/DTC/SensorInterface.h"
#include "IMUSensor.generated.h"

USTRUCT(BlueprintType)
struct FIMUSensorData
{
	GENERATED_BODY()

	// ---------------------------------
	// --- Variables
	// ---------------------------------

	UPROPERTY(BlueprintReadWrite)
	FQuat Orientation;

	UPROPERTY(BlueprintReadWrite)
	TArray<double> OrientationCovariance;

	UPROPERTY(BlueprintReadWrite)
	FVector AngularVelocity;

	UPROPERTY(BlueprintReadWrite)
	TArray<double> AngularVelocityCovariance;

	UPROPERTY(BlueprintReadWrite)
	FVector LinearAcceleration;

	UPROPERTY(BlueprintReadWrite)
	TArray<double> AngularAccelerationCovariance;

	// ---------------------------------
	// --- Constructors
	// ---------------------------------

	FIMUSensorData()
	{
		Orientation = FQuat();
		OrientationCovariance.Init(0, 9);
		AngularVelocity = FVector();
		AngularVelocityCovariance.Init(0, 9);
		LinearAcceleration = FVector();
		AngularAccelerationCovariance.Init(0, 9);
	}

	// ---------------------------------
	// --- API
	// ---------------------------------
public:
	void SetData(const FQuat& inOrientation, const TArray<double>& inOrientationCovariance, const FVector& inAngularVelocity, const TArray<double>& inAngularVelocityCovariance, const FVector& inAngularAcceleration, const TArray<double>& inAngularAccelerationCovariance)
	{
		Orientation = inOrientation;
		OrientationCovariance = inOrientationCovariance;
		AngularVelocity = inAngularVelocity;
		AngularVelocityCovariance = inAngularAccelerationCovariance;
		LinearAcceleration = inAngularAcceleration;
		AngularAccelerationCovariance = inAngularAccelerationCovariance;
	}

};

UCLASS(Blueprintable, ClassGroup = (Sensors), meta = (BlueprintSpawnableComponent))
class CARLA_API UIMUSensor : public USphereComponent, public ISensorInterface
{
	GENERATED_BODY()
public:
	// ---------------------------------
	// --- Delegates
	// ---------------------------------
private:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIMUDispatchSignature, const FIMUSensorData&, sensorData);
	// ---------------------------------
	// --- Variables
	// ---------------------------------

	FVector OldLinearVelocity;

	TArray<double> Identity;

	FIMUSensorData IMUData;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FIMUDispatchSignature IMUDispatch;

	// ---------------------------------
	// --- Constructors
	// ---------------------------------
public:
	UIMUSensor();

	// ---------------------------------
	// --- Variables
	// ---------------------------------
protected:
	

	// ---------------------------------
	// --- Inherited
	// ---------------------------------
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	// ---------------------------------
	// --- API
	// ---------------------------------
public:
	FIMUSensorData GetData() const {return IMUData;};

	// ---------------------------------
	// --- Implementation
	// ---------------------------------
	UFUNCTION(BlueprintCallable)
	void DoMathAndCallEventAndSetOldVelocity(float dt);

	// ---------------------------------
	// --- Sensor Interface
	// ---------------------------------
public:
	ESensorType GetSensorType_Implementation() const;
};
