// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MediaCapture.h"

#include "ImageWriteTypes.h"
#include <functional>

#include "NetMediaCapture.generated.h"

/**
 * 
 */
UCLASS()
class CARLA_API UNetMediaCapture : public UMediaCapture
{
	GENERATED_BODY()

	UNetMediaCapture();
	
public:
	void SetCallback(std::function<void(void*, int32, int32, EPixelFormat)> Func)
	{
		OnCapturedUserCallback = Func;
	}

protected:
	virtual void OnFrameCaptured_RenderingThread(const FCaptureBaseData& InBaseData, TSharedPtr<FMediaCaptureUserData, ESPMode::ThreadSafe> InUserData, void* InBuffer, int32 Width, int32 Height) override;
	virtual bool CaptureSceneViewportImpl(TSharedPtr<FSceneViewport>& InSceneViewport) override;
	virtual bool CaptureRenderTargetImpl(UTextureRenderTarget2D* InRenderTarget) override;

private:
	void CacheMediaOutputValues();

private:
	FString BaseFilePathName;
	EImageFormat ImageFormat;
	TFunction<void(bool)> OnCompleteWrapper;
	bool bOverwriteFile;
	int32 CompressionQuality;
	bool bAsync;
	std::function<void(void*, int32, int32, EPixelFormat)> OnCapturedUserCallback;
};
