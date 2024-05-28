#pragma once
#include "Carla/DTC/BaseSensor.h"

#include "GPSSensor.generated.h"


USTRUCT(BlueprintType)
struct FGPSSensorData
{
	GENERATED_BODY()
	// ---------------------------------
	// --- Variables
	// ---------------------------------
	/*int COVARIANCE_TYPE_UNKNOWN = 0;
	int COVARIANCE_TYPE_APPROXIMATED = 1;
	int COVARIANCE_TYPE_DIAGONAL_KNOWN = 2;
	int COVARIANCE_TYPE_KNOWN = 3;*/
	
	float latitude;
	float longitude;
	float altitude;

	TArray<float> position_covariance;

	int position_covariance_type;

	// ---------------------------------
	// --- Constructors
	// ---------------------------------

	FGPSSensorData()
	{
		latitude = 0;
		longitude = 0;
		altitude = 0;
		position_covariance.Init(0, 9);
		position_covariance_type = 0;
	}

	// ---------------------------------
	// --- API
	// ---------------------------------

public:
	void SetData(const float &inLongitude, const float &inLatitude, const float &inAltitude,
		const TArray<float> &inPositionCovariance, const int inCovarianceType)
	{
		longitude = inLongitude;
		latitude = inLatitude;
		altitude = inAltitude;
		position_covariance = inPositionCovariance;
		position_covariance_type = inCovarianceType;
	}

	void SetDataLight(const float &inLongitude, const float &inLatitude, const float &inAltitude)
	{
		longitude = inLongitude;
		latitude = inLatitude;
		altitude = inAltitude;
	}
};


UCLASS(Blueprintable, ClassGroup = (Sensors), meta = (BlueprintSpawnableComponent))
class CARLA_API UGPSSensor : public UBaseSensor
{
	GENERATED_BODY()
public:
	// ---------------------------------
	// --- Delegates
	// ---------------------------------
private:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGPSDispatchSignature, FGPSSensorData&, sensorData);

	// ---------------------------------
	// --- Variables
	// ---------------------------------
	
	AActor* VehicleActor;

	FGPSSensorData GPSData;
	
public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FGPSDispatchSignature GPSDispatch;

	// ---------------------------------
	// --- Constructors
	// ---------------------------------
public:
	UGPSSensor();

	// ---------------------------------
	// --- Inherited
	// ---------------------------------
	
	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ---------------------------------
	// --- API
	// ---------------------------------
	
	FGPSSensorData GetData() const {return GPSData; };

	// ---------------------------------
	// --- Implementation
	// ---------------------------------

	void UpdateGPSData();
};
