#include "Carla/DTC/CameraSensor.h"

#include "Game_GI_DARPA.h"

// ---------------------------------
// --- Constructors
// ---------------------------------
UCameraSensor::UCameraSensor()
{
	bCaptureEveryFrame = false;
	bAlwaysPersistRenderingState = true;
}

// ---------------------------------
// --- Implementation
// ---------------------------------

void UCameraSensor::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdatePixelData();
}

TArray<uint8> UCameraSensor::ConvertToRGB8(UTextureRenderTarget2D* Render_Target)
{
	TArray<FColor> Data;
	auto RenderTargetResource = Render_Target->GameThread_GetRenderTargetResource();
	RenderTargetResource->ReadPixels(Data);
	TArray<uint8> output;
	for (auto& pixel : Data)
	{
		output.Add(pixel.R);
		output.Add(pixel.G);
		output.Add(pixel.B);
	}
	return output;
}

void UCameraSensor::UpdatePixelData()
{
	if (TextureTarget == nullptr)
	{
		return;
	}
	if (bCaptureWithInfrared)
	{
		if (UGame_GI_DARPA* gameInstance = Cast<UGame_GI_DARPA>(GetWorld()->GetGameInstance()))
		{
			gameInstance->SetIREnabled(true);
			CaptureScene();
			gameInstance->SetIREnabled(false);
		}
		
	}
	else
	{
		CaptureScene();
	}

	TArray<FColor> Data;
	auto RenderTargetResource = TextureTarget->GameThread_GetRenderTargetResource();
	FReadSurfaceDataFlags readPixelFlags;
	readPixelFlags.SetLinearToGamma(false);
	RenderTargetResource->ReadPixels(Data, readPixelFlags);
	TArray<uint8> output;
	for (auto& pixel : Data)
	{
		output.Add(pixel.R);
		output.Add(pixel.G);
		output.Add(pixel.B);
	}
	
	AActor* owner = GetOwner();
	if (owner == nullptr)
	{
		return;
	}

	CameraData.SetData(TextureTarget->SizeX, TextureTarget->SizeY, "rgb8", false, TextureTarget->SizeX * 3, output);

	CameraDispatch2.Broadcast(CameraData);
}

ESensorType UCameraSensor::GetSensorType_Implementation() const
{
	return ESensorType::ST_RGBCam;
}