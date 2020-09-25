// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/PixelReader.h"

#include "Engine/TextureRenderTarget2D.h"
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

// Temporal; this avoid allocating the array each time and also avoids checking
// for a bigger texture, ReadSurfaceData will allocate the space needed.
TArray<FColor> gPixels;

static void WritePixelsToBuffer_Vulkan(
    const UTextureRenderTarget2D &RenderTarget,
    carla::Buffer &Buffer,
    uint32 Offset,
    FRHICommandListImmediate &InRHICmdList)
{
  check(IsInRenderingThread());
  auto RenderResource =
      static_cast<const FTextureRenderTarget2DResource *>(RenderTarget.Resource);
  FTexture2DRHIRef Texture = RenderResource->GetRenderTargetTexture();
  if (!Texture)
  {
    return;
  }

  FIntPoint Rect = RenderResource->GetSizeXY();

  // NS: Extra copy here, don't know how to avoid it.
  InRHICmdList.ReadSurfaceData(
      Texture,
      FIntRect(0, 0, Rect.X, Rect.Y),
      gPixels,
      FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));

  Buffer.copy_from(Offset, gPixels);
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
  if (!WritePixelsToArray(RenderTarget, PixelData->Pixels))
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
    carla::Buffer &Buffer,
    uint32 Offset,
    FRHICommandListImmediate &InRHICmdList
    )
{
  check(IsInRenderingThread());

  if (IsVulkanPlatform(GMaxRHIShaderPlatform) || IsD3DPlatform(GMaxRHIShaderPlatform, false))
  {
    WritePixelsToBuffer_Vulkan(RenderTarget, Buffer, Offset, InRHICmdList);
    return;
  }

  FTextureRenderTargetResource* RenderTargetResource = RenderTarget.GetRenderTargetResource();
  if(!RenderTargetResource)
  {
    return;
  }
  FRHITexture2D *Texture = RenderTargetResource->GetRenderTargetTexture();
  checkf(Texture != nullptr, TEXT("FPixelReader: UTextureRenderTarget2D missing render target texture"));

  const uint32 BytesPerPixel = 4u; // PF_R8G8B8A8
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
}
