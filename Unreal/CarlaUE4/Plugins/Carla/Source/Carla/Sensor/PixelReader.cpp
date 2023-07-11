// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/PixelReader.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Async/Async.h"
#include "HighResScreenshot.h"
#include "Runtime/ImageWriteQueue/Public/ImageWriteQueue.h"

// =============================================================================
// -- FPixelReader -------------------------------------------------------------
// =============================================================================

void FPixelReader::WritePixelsToBuffer(
    const UTextureRenderTarget2D &RenderTarget,
    uint32 Offset,
    FRHICommandListImmediate &RHICmdList,
    FPixelReader::Payload FuncForSending)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR("WritePixelsToBuffer");
  check(IsInRenderingThread());

  auto RenderResource =
      static_cast<const FTextureRenderTarget2DResource *>(RenderTarget.Resource);
  FTexture2DRHIRef Texture = RenderResource->GetRenderTargetTexture();
  if (!Texture)
  {
    return;
  }

  auto BackBufferReadback = std::make_unique<FRHIGPUTextureReadback>(TEXT("CameraBufferReadback"));
  FIntPoint BackBufferSize = Texture->GetSizeXY();
  EPixelFormat BackBufferPixelFormat = Texture->GetFormat();
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("EnqueueCopy");
    BackBufferReadback->EnqueueCopy(RHICmdList,
                                    Texture,
                                    FResolveRect(0, 0, BackBufferSize.X, BackBufferSize.Y));
  }

  // workaround to force RHI with Vulkan to refresh the fences state in the middle of frame
  {
    FRenderQueryRHIRef Query = RHICreateRenderQuery(RQT_AbsoluteTime);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("create query");
    RHICmdList.EndRenderQuery(Query);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Flush");
    RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("query result");
    uint64 OldAbsTime = 0;
    RHICmdList.GetRenderQueryResult(Query, OldAbsTime, true);
  }

  AsyncTask(ENamedThreads::HighTaskPriority, [=, Readback=std::move(BackBufferReadback)]() mutable {
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Wait GPU transfer");
      while (!Readback->IsReady())
      {
        std::this_thread::yield();
      }
    }

    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Readback data");
      FPixelFormatInfo PixelFormat = GPixelFormats[BackBufferPixelFormat];
      uint32 ExpectedRowBytes = BackBufferSize.X * PixelFormat.BlockBytes;
      int32 Size = (BackBufferSize.Y * (PixelFormat.BlockBytes * BackBufferSize.X));
      void* LockedData = Readback->Lock(Size);
      if (LockedData)
      {
        FuncForSending(LockedData, Size, Offset, ExpectedRowBytes);
      }
      Readback->Unlock();
      Readback.reset();
    }
  });
}

bool FPixelReader::WritePixelsToArray(
    UTextureRenderTarget2D &RenderTarget,
    TArray<FColor> &BitMap)
{
  check(IsInGameThread());
  FTextureRenderTargetResource *RTResource =
      RenderTarget.GameThread_GetRenderTargetResource();
  if (RTResource == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("FPixelReader: UTextureRenderTarget2D missing render target"));
    return false;
  }
  FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
  ReadPixelFlags.SetLinearToGamma(true);
  return RTResource->ReadPixels(BitMap, ReadPixelFlags);
}

TUniquePtr<TImagePixelData<FColor>> FPixelReader::DumpPixels(
    UTextureRenderTarget2D &RenderTarget)
{
  const FIntPoint DestSize(RenderTarget.GetSurfaceWidth(), RenderTarget.GetSurfaceHeight());
  TUniquePtr<TImagePixelData<FColor>> PixelData = MakeUnique<TImagePixelData<FColor>>(DestSize);
  TArray<FColor> Pixels(PixelData->Pixels.GetData(), PixelData->Pixels.Num());
  if (!WritePixelsToArray(RenderTarget, Pixels))
  {
    return nullptr;
  }
  PixelData->Pixels = Pixels;
  return PixelData;
}

TFuture<bool> FPixelReader::SavePixelsToDisk(
    UTextureRenderTarget2D &RenderTarget,
    const FString &FilePath)
{
  return SavePixelsToDisk(DumpPixels(RenderTarget), FilePath);
}

TFuture<bool> FPixelReader::SavePixelsToDisk(
    TUniquePtr<TImagePixelData<FColor>> PixelData,
    const FString &FilePath)
{
  TUniquePtr<FImageWriteTask> ImageTask = MakeUnique<FImageWriteTask>();
  ImageTask->PixelData = MoveTemp(PixelData);
  ImageTask->Filename = FilePath;
  ImageTask->Format = EImageFormat::PNG;
  ImageTask->CompressionQuality = (int32) EImageCompressionQuality::Default;
  ImageTask->bOverwriteFile = true;
  ImageTask->PixelPreProcessors.Add(TAsyncAlphaWrite<FColor>(255));

  FHighResScreenshotConfig &HighResScreenshotConfig = GetHighResScreenshotConfig();
  return HighResScreenshotConfig.ImageWriteQueue->Enqueue(MoveTemp(ImageTask));
}
