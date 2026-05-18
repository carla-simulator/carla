// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/PixelReader.h"
#include "Carla.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/TextureRenderTarget2D.h"
#include "Async/Async.h"
#include "HighResScreenshot.h"
#include "ImageWriteQueue.h"
#include "RHIGPUReadback.h"
#include <util/ue-header-guard-end.h>

#include <chrono>
#include <thread>

// =============================================================================
// -- FPixelReader -------------------------------------------------------------
// =============================================================================

static TAutoConsoleVariable<int32> CVarPixelReaderLegacyVulkanFenceFlush(
    TEXT("carla.PixelReader.LegacyVulkanFenceFlush"),
    0,
    TEXT("UE4-era Vulkan fence-flush workaround inside FPixelReader::WritePixelsToBuffer.\n")
    TEXT("Creates an RQT_AbsoluteTime render query, flushes the RHI thread, and\n")
    TEXT("synchronously waits for the result after every EnqueueCopy. UE 5.5 Vulkan\n")
    TEXT("rewrote fence handling; the workaround is believed unnecessary.\n")
    TEXT("  0: Skip the workaround (default).\n")
    TEXT("  1: Run the legacy fence-flush block (rollback)."),
    ECVF_Default);

void FPixelReader::WritePixelsToBuffer(
    UTextureRenderTarget2D &RenderTarget,
    uint32 Offset,
    FRHICommandListImmediate &RHICmdList,
    FPixelReader::Payload FuncForSending,
    FRHIGPUReadbackPoolPtr Pool)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR("WritePixelsToBuffer");
  check(IsInRenderingThread());

  auto RenderResource =
      static_cast<const FTextureRenderTarget2DResource *>(RenderTarget.GetResource());
  auto Texture = RenderResource->GetRenderTargetTexture();
  if (!Texture)
  {
    return;
  }

  // Acquire from per-sensor pool when available; fall back to per-call alloc
  // if the pool is missing or every slot is in use.
  FRHIGPUTextureReadback *Readback = nullptr;
  int32 SlotIndex = INDEX_NONE;
  TUniquePtr<FRHIGPUTextureReadback> FallbackReadback;
  if (Pool)
  {
    Readback = Pool->Acquire(SlotIndex);
  }
  if (Readback == nullptr)
  {
    FallbackReadback = MakeUnique<FRHIGPUTextureReadback>(TEXT("CameraBufferReadback"));
    Readback = FallbackReadback.Get();
  }

  FIntPoint BackBufferSize = Texture->GetSizeXY();
  EPixelFormat BackBufferPixelFormat = Texture->GetFormat();
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("EnqueueCopy");
    Readback->EnqueueCopy(RHICmdList,
                          Texture,
                          FResolveRect(0, 0, BackBufferSize.X, BackBufferSize.Y));
  }

  if (CVarPixelReaderLegacyVulkanFenceFlush.GetValueOnRenderThread() != 0)
  {
    // workaround to force RHI with Vulkan to refresh the fences state in the middle of frame
    FRenderQueryRHIRef Query = RHICreateRenderQuery(RQT_AbsoluteTime);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("create query");
    RHICmdList.EndRenderQuery(Query);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Flush");
    RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("query result");
    uint64 OldAbsTime = 0;
    RHIGetRenderQueryResult(Query, OldAbsTime, true);
  }

  AsyncTask(ENamedThreads::HighTaskPriority,
    [=, Pool = std::move(Pool),
        Fallback = std::move(FallbackReadback)]() mutable {
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Wait GPU transfer");
      while (!Readback->IsReady())
      {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
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
    }

    if (Pool && SlotIndex != INDEX_NONE)
    {
      Pool->Release(SlotIndex);
    }
    // Fallback (if any) destructs here, freeing its staging buffer.
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
