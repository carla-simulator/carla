#pragma once

#include "CoreMinimal.h"
#include "Carla/DTC/SensorInterface.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "CameraSensor.generated.h"

USTRUCT(BlueprintType)
struct FCameraSensorData
{
	GENERATED_BODY()

	// ---------------------------------
	// --- Variables
	// ---------------------------------

	UPROPERTY(BlueprintReadWrite)
	int32 Height;

	UPROPERTY(BlueprintReadWrite)
	int32 Width;

	UPROPERTY(BlueprintReadWrite)
	FString Encoding;

	UPROPERTY(BlueprintReadWrite)
	bool IsBigEndian;

	UPROPERTY(BlueprintReadWrite)
	int32 Step;

	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> Data;

	// ---------------------------------
	// --- Constructors
	// ---------------------------------

	FCameraSensorData()
	{
		Height = 0;
		Width = 0;
		Encoding = "";
		IsBigEndian = false;
		Step = 0;
		Data.Init(0, 1);
	}

	// ---------------------------------
	// --- API
	// ---------------------------------
public:
	void SetData(const int32& inWidth, const int32& inHeight, const FString& inEncoding, const bool& inIsBigEndian, const int32& inStep, const TArray<uint8>& inData)
	{
		Height = inHeight;
		Width = inWidth;
		Encoding = inEncoding;
		IsBigEndian = inIsBigEndian;
		Step = inStep;
		Data = inData;
	}

};

UCLASS(Blueprintable, ClassGroup = (Sensors), meta = (BlueprintSpawnableComponent))
class CARLA_API UCameraSensor : public USceneCaptureComponent2D, public ISensorInterface
{
	GENERATED_BODY()
public:
	// ---------------------------------
	// --- Delegates
	// ---------------------------------
private:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCameraDispatchSignature, const FCameraSensorData&, sensorData);
public:

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FCameraDispatchSignature CameraDispatch2;
	// ---------------------------------
	// --- Variables
	// ---------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "Sensor")
	bool bCaptureWithInfrared;

	FCameraSensorData CameraData;

	// ---------------------------------
	// --- Constructors
	// ---------------------------------
public:
	UCameraSensor();
	// ---------------------------------
	// --- Variables
	// ---------------------------------


	// ---------------------------------
	// --- Inherited
	// ---------------------------------
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	
	// ---------------------------------
	// --- Implementation
	// ---------------------------------
	UFUNCTION(BlueprintCallable)
	TArray<uint8> ConvertToRGB8(UTextureRenderTarget2D* Render_Target);

	UFUNCTION(BlueprintCallable)
	void UpdatePixelData();

	// ---------------------------------
	// --- Sensor Interface
	// ---------------------------------
public:
	ESensorType GetSensorType_Implementation() const;

};
