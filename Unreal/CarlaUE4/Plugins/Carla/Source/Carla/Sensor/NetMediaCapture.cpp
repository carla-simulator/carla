// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetMediaCapture.h"

#include "NetMediaOutput.h"
#include "ImageWriteQueue.h"
#include "ImageWriteTask.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"


UNetMediaCapture::UNetMediaCapture()
{
}

void UNetMediaCapture::OnFrameCaptured_RenderingThread(const FCaptureBaseData& InBaseData, TSharedPtr<FMediaCaptureUserData, ESPMode::ThreadSafe> InUserData, void* InBuffer, int32 Width, int32 Height)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR("OnFrameCaptured");

	// call the user callback
	if (OnCapturedUserCallback)
	{
		OnCapturedUserCallback(InBuffer, Width, Height, GetDesiredPixelFormat());
	}
/*
  IImageWriteQueueModule* ImageWriteQueueModule = FModuleManager::Get().GetModulePtr<IImageWriteQueueModule>("ImageWriteQueue");
	if (ImageWriteQueueModule == nullptr)
	{
		SetState(EMediaCaptureState::Error);
		return;
	}

	TUniquePtr<FImageWriteTask> ImageTask = MakeUnique<FImageWriteTask>();
	ImageTask->Format = ImageFormat;
	ImageTask->Filename = FString::Printf(TEXT("%s%05d"), *BaseFilePathName, InBaseData.SourceFrameNumber);
	ImageTask->bOverwriteFile = bOverwriteFile;
	ImageTask->CompressionQuality = CompressionQuality;
	ImageTask->OnCompleted = OnCompleteWrapper;

	EPixelFormat PixelFormat = GetDesiredPixelFormat();
	if (PixelFormat == PF_B8G8R8A8)
	{
		TUniquePtr<TImagePixelData<FColor>> PixelData = MakeUnique<TImagePixelData<FColor>>(FIntPoint(Width, Height));
		PixelData->Pixels = TArray<FColor>(reinterpret_cast<FColor*>(InBuffer), Width * Height);
		ImageTask->PixelData = MoveTemp(PixelData);
	}
	else if (PixelFormat == PF_FloatRGBA)
	{
		TUniquePtr<TImagePixelData<FFloat16Color>> PixelData = MakeUnique<TImagePixelData<FFloat16Color>>(FIntPoint(Width, Height));
		PixelData->Pixels = TArray<FFloat16Color>(reinterpret_cast<FFloat16Color*>(InBuffer), Width * Height);
		ImageTask->PixelData = MoveTemp(PixelData);
	}
	else
	{
		check(false);
	}

	TFuture<bool> DispatchedTask = ImageWriteQueueModule->GetWriteQueue().Enqueue(MoveTemp(ImageTask));

	if (!bAsync)
	{
		// If not async, wait for the dispatched task to complete.
		if (DispatchedTask.IsValid())
		{
			DispatchedTask.Wait();
		}
	}
	*/
}


bool UNetMediaCapture::CaptureSceneViewportImpl(TSharedPtr<FSceneViewport>& InSceneViewport)
{
	FModuleManager::Get().LoadModuleChecked<IImageWriteQueueModule>("ImageWriteQueue");
	CacheMediaOutputValues();

	SetState(EMediaCaptureState::Capturing);
	return true;
}


bool UNetMediaCapture::CaptureRenderTargetImpl(UTextureRenderTarget2D* InRenderTarget)
{
	FModuleManager::Get().LoadModuleChecked<IImageWriteQueueModule>("ImageWriteQueue");
	CacheMediaOutputValues();

	SetState(EMediaCaptureState::Capturing);
	return true;
}


void UNetMediaCapture::CacheMediaOutputValues()
{
	UNetMediaOutput* NetMediaOutput = CastChecked<UNetMediaOutput>(MediaOutput);
	BaseFilePathName = FPaths::Combine(NetMediaOutput->FilePath.Path, NetMediaOutput->BaseFileName);
	ImageFormat = ImageFormatFromDesired(NetMediaOutput->WriteOptions.Format);
	CompressionQuality = NetMediaOutput->WriteOptions.CompressionQuality;
	bOverwriteFile = NetMediaOutput->WriteOptions.bOverwriteFile;
	bAsync = NetMediaOutput->WriteOptions.bAsync;

	OnCompleteWrapper = [NativeCB = NetMediaOutput->WriteOptions.NativeOnComplete, DynamicCB = NetMediaOutput->WriteOptions.OnComplete](bool bSuccess)
	{
		if (NativeCB)
		{
			NativeCB(bSuccess);
		}
		DynamicCB.ExecuteIfBound(bSuccess);
	};
}
