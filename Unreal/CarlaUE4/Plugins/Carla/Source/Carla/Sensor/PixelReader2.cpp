// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/PixelReader2.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Async/Async.h"
#include "HighResScreenshot.h"
#include "Runtime/ImageWriteQueue/Public/ImageWriteQueue.h"

// =============================================================================
// -- Local variables and types ------------------------------------------------
// =============================================================================

struct LockTexture2
{
  LockTexture2(FRHITexture2D *InTexture, uint32 &Stride)
    : Texture(InTexture),
      Source(reinterpret_cast<const uint8 *>(
            RHILockTexture2D(Texture, 0, RLM_ReadOnly, Stride, false))) {}

  ~LockTexture2()
  {
    RHIUnlockTexture2D(Texture, 0, false);
  }

  FRHITexture2D *Texture;

  const uint8 *Source;
};

// =============================================================================
// -- Static local functions ---------------------------------------------------
// =============================================================================

// Temporal; this avoid allocating the array each time and also avoids checking
// for a bigger texture, ReadSurfaceData will allocate the space needed.
TArray<FColor> gPixels2;

static void WritePixelsToBuffer_Vulkan2(
    const UTextureRenderTarget2D &RenderTarget,
    uint32 Offset,
    FRHICommandListImmediate &RHICmdList,
    std::function<void(void *, uint32, uint32)> FuncForSending)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  check(IsInRenderingThread());

  auto RenderResource = static_cast<const FTextureRenderTarget2DResource *>(RenderTarget.Resource);
  FTexture2DRHIRef Texture = RenderResource->GetRenderTargetTexture();
  if (!Texture)
  {
    return;
  }

  std::unique_ptr<FRHIGPUTextureReadback> BackBufferReadback = std::make_unique<FRHIGPUTextureReadback>(TEXT("CameraBufferReadback"));
  FIntPoint BackBufferSize = Texture->GetSizeXY();
  EPixelFormat BackBufferPixelFormat = Texture->GetFormat();
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("EnqueueCopy");
    BackBufferReadback->EnqueueCopy(RHICmdList, Texture, FResolveRect(0, 0, BackBufferSize.X, BackBufferSize.Y));
  }
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Transition");
    RHICmdList.Transition(FRHITransitionInfo(Texture, ERHIAccess::CopySrc, ERHIAccess::Present));
  }

  TRACE_CPUPROFILER_EVENT_SCOPE_STR("Wait GPU transfer");

  FPixelFormatInfo PixelFormat = GPixelFormats[BackBufferPixelFormat];
  int32 Count = (BackBufferSize.Y * (PixelFormat.BlockBytes * BackBufferSize.X));
  while (1)
  {
    void* LockedData = BackBufferReadback->Lock(Count);
    if (LockedData)
    {
      FuncForSending(LockedData, Count, Offset);
      break;
    }
  }
  
  BackBufferReadback->Unlock();       
  BackBufferReadback.reset();
}

// Temporal; this avoid allocating the array each time
TArray<FFloat16Color> gFloatPixels2;

static void WriteFloatPixelsToBuffer_Vulkan2(
    const UTextureRenderTarget2D &RenderTarget,
    carla::Buffer &Buffer,
    uint32 Offset,
    FRHICommandListImmediate &InRHICmdList)
    
{
  check(IsInRenderingThread());
  gFloatPixels2.Empty();
  auto RenderResource =
      static_cast<const FTextureRenderTarget2DResource *>(RenderTarget.Resource);
  FTexture2DRHIRef Texture = RenderResource->GetRenderTargetTexture();
  if (!Texture)
  {
    return;
  }

  FIntPoint Rect = RenderResource->GetSizeXY();

  // NS: Extra copy here, don't know how to avoid it.
  InRHICmdList.ReadSurfaceFloatData(
      Texture,
      FIntRect(0, 0, Rect.X, Rect.Y),
      gFloatPixels2,
      CubeFace_PosX,0,0);

  TArray<float> IntermediateBuffer;
  IntermediateBuffer.Reserve(gFloatPixels2.Num() * 2);
  for (FFloat16Color& color : gFloatPixels2) {
    float x = (color.R.GetFloat() - 0.5f)*4.f;
    float y = (color.G.GetFloat() - 0.5f)*4.f;
    IntermediateBuffer.Add(x);
    IntermediateBuffer.Add(y);
  }
  Buffer.copy_from(Offset, IntermediateBuffer);
}

// =============================================================================
// -- FPixelReader -------------------------------------------------------------
// =============================================================================

bool FPixelReader2::WritePixelsToArray(
    UTextureRenderTarget2D &RenderTarget,
    TArray<FColor> &BitMap)
{
  check(IsInGameThread());
  FTextureRenderTargetResource *RTResource =
      RenderTarget.GameThread_GetRenderTargetResource();
  if (RTResource == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("FPixelReader2: UTextureRenderTarget2D missing render target"));
    return false;
  }
  FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
  ReadPixelFlags.SetLinearToGamma(true);
  return RTResource->ReadPixels(BitMap, ReadPixelFlags);
}

TUniquePtr<TImagePixelData<FColor>> FPixelReader2::DumpPixels(
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

TFuture<bool> FPixelReader2::SavePixelsToDisk(
    UTextureRenderTarget2D &RenderTarget,
    const FString &FilePath)
{
  return SavePixelsToDisk(DumpPixels(RenderTarget), FilePath);
}

TFuture<bool> FPixelReader2::SavePixelsToDisk(
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

void FPixelReader2::WritePixelsToBuffer(
    UTextureRenderTarget2D &RenderTarget,
    uint32 Offset,
    FRHICommandListImmediate &InRHICmdList,
    std::function<void(void *, uint32, uint32)> FuncForSending,
    bool use16BitFormat)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  check(IsInRenderingThread());

  if (IsVulkanPlatform(GMaxRHIShaderPlatform) || IsD3DPlatform(GMaxRHIShaderPlatform, false))
  {
    if (use16BitFormat)
    {
      // WriteFloatPixelsToBuffer_Vulkan2(RenderTarget, Buffer, Offset, InRHICmdList);
    }
    else
    {
      WritePixelsToBuffer_Vulkan2(RenderTarget, Offset, InRHICmdList, std::move(FuncForSending));
    }
  }
}
