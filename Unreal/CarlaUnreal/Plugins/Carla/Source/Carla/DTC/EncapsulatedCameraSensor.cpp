#include "Carla/DTC/EncapsulatedCameraSensor.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"


void UEncapsulatedCameraSensor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdatePixelData();
}

// ---------------------------------
// --- Implementation
// ---------------------------------
void UEncapsulatedCameraSensor::BeginPlay()
{
	Super::BeginPlay();
}	

UTextureRenderTarget2D* UEncapsulatedCameraSensor::GetRenderTarget()
{
	return TextureTarget;
}
