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
// -- Local variables and types ------------------------------------------------
// =============================================================================

struct LockTexture
{
  LockTexture(FRHITexture2D *InTexture, uint32 &Stride)
    : Texture(InTexture),
      Source(reinterpret_cast<const uint8 *>(
            RHILockTexture2D(Texture, 0, RLM_ReadOnly, Stride, false))) {}

  ~LockTexture()
  {
    RHIUnlockTexture2D(Texture, 0, false);
  }

  FRHITexture2D *Texture;

  const uint8 *Source;
};

// =============================================================================
// -- Static local functions ---------------------------------------------------
// =============================================================================

static void WritePixelsToBuffer_Vulkan(
    const UTextureRenderTarget2D &RenderTarget,
    uint32 Offset,
    FRHICommandListImmediate &RHICmdList,
    FPixelReader::Payload FuncForSending)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR("WritePixelsToBuffer_Vulkan");
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
    BackBufferReadback->EnqueueCopy(RHICmdList, Texture, FResolveRect(0, 0, BackBufferSize.X, BackBufferSize.Y));
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

  AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [=, Readback=std::move(BackBufferReadback)]() mutable {
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Wait GPU transfer");
      while (!Readback->IsReady())
      {
        std::this_thread::yield();
      }
    }
    
    FPixelFormatInfo PixelFormat = GPixelFormats[BackBufferPixelFormat];
    int32 Count = (BackBufferSize.Y * (PixelFormat.BlockBytes * BackBufferSize.X));
    void* LockedData = Readback->Lock(Count);
    if (LockedData)
    {
      FuncForSending(LockedData, Count, Offset);
    }
    Readback->Unlock();
    Readback.reset();
  });
  
}

// Temporal; this avoid allocating the array each time
TArray<FFloat16Color> gFloatPixels;

static void WriteFloatPixelsToBuffer_Vulkan(
    const UTextureRenderTarget2D &RenderTarget,
    uint32 Offset,
    FRHICommandListImmediate &RHICmdList,
    FPixelReader::Payload FuncForSending)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR("WritePixelsToBuffer_Vulkan");
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
    BackBufferReadback->EnqueueCopy(RHICmdList, Texture, FResolveRect(0, 0, BackBufferSize.X, BackBufferSize.Y));
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

  AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [=, Readback=std::move(BackBufferReadback)]() mutable {
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Wait GPU transfer");
      while (!Readback->IsReady())
      {
        std::this_thread::yield();
      }
    }
    
    FPixelFormatInfo PixelFormat = GPixelFormats[BackBufferPixelFormat];
    int32 Count = (BackBufferSize.Y * (PixelFormat.BlockBytes * BackBufferSize.X));
    int32 TotalPixels = (BackBufferSize.Y * BackBufferSize.X);
    void* LockedData = Readback->Lock(Count);
    if (LockedData)
    {
      TArray<float> IntermediateBuffer;
      FFloat16Color *Data = reinterpret_cast<FFloat16Color *>(LockedData);
      IntermediateBuffer.Reserve(TotalPixels * 2);
      for (int i=0; i<TotalPixels; ++i)
      {
        float x = (Data->R.GetFloat() - 0.5f) * 4.f;
        float y = (Data->G.GetFloat() - 0.5f) * 4.f;
        IntermediateBuffer.Add(x);
        IntermediateBuffer.Add(y);
        ++Data;
      }
      FuncForSending(reinterpret_cast<void *>(IntermediateBuffer.GetData()), TotalPixels * sizeof(float) * 2 , Offset);
    }
    Readback->Unlock();
    Readback.reset();
  });
 }

// =============================================================================
// -- FPixelReader -------------------------------------------------------------
// =============================================================================

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

void FPixelReader::WritePixelsToBuffer(
    UTextureRenderTarget2D &RenderTarget,
    uint32 Offset,
    FRHICommandListImmediate &InRHICmdList,
    FPixelReader::Payload FuncForSending,
    bool use16BitFormat)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR("WritePixelsToBuffer");
  check(IsInRenderingThread());

  if (IsVulkanPlatform(GMaxRHIShaderPlatform) || IsD3DPlatform(GMaxRHIShaderPlatform, false))
  {
    if (use16BitFormat)
    {
      WriteFloatPixelsToBuffer_Vulkan(RenderTarget, Offset, InRHICmdList, std::move(FuncForSending));
    }
    else
    {
      WritePixelsToBuffer_Vulkan(RenderTarget, Offset, InRHICmdList, std::move(FuncForSending));
    }
    return;
  }
  /*
  FTextureRenderTargetResource* RenderTargetResource = RenderTarget.GetRenderTargetResource();
  if(!RenderTargetResource)
  {
    return;
  }

  FRHITexture2D *Texture = RenderTargetResource->GetRenderTargetTexture();
  checkf(Texture != nullptr, TEXT("FPixelReader: UTextureRenderTarget2D missing render target texture"));

  const uint32 BytesPerPixel = use16BitFormat ? 8u : 4u; // PF_R8G8B8A8 or PF_FloatRGBA
  const uint32 Width = Texture->GetSizeX();
  const uint32 Height = Texture->GetSizeY();
  const uint32 ExpectedStride = Width * BytesPerPixel;

  uint32 SrcStride;
  LockTexture Lock(Texture, SrcStride);

#ifdef PLATFORM_WINDOWS
  // JB: Direct 3D uses additional rows in the buffer, so we need check the
  // result stride from the lock:
  if (IsD3DPlatform(GMaxRHIShaderPlatform, false) && (ExpectedStride != SrcStride))
  {
    Buffer.reset(Offset + ExpectedStride * Height);
    auto DstRow = Buffer.begin() + Offset;
    const uint8 *SrcRow = Lock.Source;
    for (uint32 Row = 0u; Row < Height; ++Row)
    {
      FMemory::Memcpy(DstRow, SrcRow, ExpectedStride);
      DstRow += ExpectedStride;
      SrcRow += SrcStride;
    }
  }
  else
#endif // PLATFORM_WINDOWS
  {
    check(ExpectedStride == SrcStride);
    const uint8 *Source = Lock.Source;
    if(Source)
    {
      Buffer.copy_from(Offset, Source, ExpectedStride * Height);
    }
  }
  */
}
