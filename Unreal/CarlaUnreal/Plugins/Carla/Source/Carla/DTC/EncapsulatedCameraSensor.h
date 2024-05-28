#pragma once

#include "CoreMinimal.h"
#include "CameraSensor.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/KismetRenderingLibrary.h"

#include "EncapsulatedCameraSensor.generated.h"



UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CARLA_API UEncapsulatedCameraSensor : public UCameraSensor
{
	GENERATED_BODY()
public:

	// ---------------------------------
	// --- Inherited
	// ---------------------------------
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
	// ---------------------------------
	// --- Implementation
	// ---------------------------------

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UTextureRenderTarget2D* GetRenderTarget();

};
