// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetMediaCapture.h"

#include "NetMediaOutput.h"


UNetMediaCapture::UNetMediaCapture()
{
}

void UNetMediaCapture::OnFrameCaptured_RenderingThread(const FCaptureBaseData& InBaseData, TSharedPtr<FMediaCaptureUserData, ESPMode::ThreadSafe> InUserData, void* InBuffer, int32 Width, int32 Height)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR("OnFrameCaptured");

	// call the user callback
	if (OnCapturedUserCallback)
	{

		EPixelFormat PixelFormat = GetDesiredPixelFormat();
		std::vector<uint8_t> Pixels;
		uint32_t Size;
		switch (PixelFormat)
		{
			case PF_B8G8R8A8:  Size = Width * Height * sizeof(FColor); 				break;
			case PF_FloatRGBA: Size = Width * Height * sizeof(FFloat16Color); break;
			default: 					 Size = 0;																			break;
		}
		Pixels.resize(Size);
		memcpy(Pixels.data(), InBuffer, Size);
		OnCapturedUserCallback(std::move(Pixels), Width, Height, PixelFormat);
	}
	SetState(EMediaCaptureState::Capturing);
}


bool UNetMediaCapture::CaptureSceneViewportImpl(TSharedPtr<FSceneViewport>& InSceneViewport)
{
	SetState(EMediaCaptureState::Capturing);
	return true;
}


bool UNetMediaCapture::CaptureRenderTargetImpl(UTextureRenderTarget2D* InRenderTarget)
{
	SetState(EMediaCaptureState::Capturing);
	return true;
}
