// Copyright Epic Games, Inc. All Rights Reserved.

#include "Carla/Sensor/NetMediaOutput.h"
#include "Carla/Sensor/NetMediaCapture.h"

#include "Misc/Paths.h"
#include "UnrealEngine.h"


bool UNetMediaOutput::Validate(FString& OutFailureReason) const
{
	if (!Super::Validate(OutFailureReason))
	{
		return false;
	}

	if (GetRequestedPixelFormat() == PF_A2B10G10R10)
	{
		OutFailureReason = FString::Printf(TEXT("Can't validate MediaOutput '%s'. File media output doesn't support 10bits format."), *GetName());
		return false;
	}

	return true;
}


FIntPoint UNetMediaOutput::GetRequestedSize() const
{
	return UMediaOutput::RequestCaptureSourceSize;
}


EPixelFormat UNetMediaOutput::GetRequestedPixelFormat() const
{
	if (bOverridePixelFormat)
	{
		return DesiredPixelFormat == ENetMediaOutputPixelFormat::FloatRGBA ? PF_FloatRGBA : PF_B8G8R8A8;
	}

	static const auto CVarDefaultBackBufferPixelFormat = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.DefaultBackBufferPixelFormat"));
	EPixelFormat SceneTargetFormat = EDefaultBackBufferPixelFormat::Convert2PixelFormat(EDefaultBackBufferPixelFormat::FromInt(CVarDefaultBackBufferPixelFormat->GetValueOnAnyThread()));

	return PF_B8G8R8A8;
}


EMediaCaptureConversionOperation UNetMediaOutput::GetConversionOperation(EMediaCaptureSourceType InSourceType) const
{
	// All formats supporting alpha
	return EMediaCaptureConversionOperation::NONE;
}


UMediaCapture* UNetMediaOutput::CreateMediaCaptureImpl()
{
	MediaCapture = NewObject<UNetMediaCapture>();
	if (MediaCapture)
	{
		MediaCapture->SetMediaOutput(this);
	}
	return MediaCapture;
}
