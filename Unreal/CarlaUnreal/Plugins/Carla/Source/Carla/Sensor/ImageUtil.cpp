// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <Carla/Sensor/ImageUtil.h>
#include <Carla/Sensor/ShaderBasedSensor.h>
#include <Carla/Carla.h>
#include <Runtime/RHI/Public/RHISurfaceDataConversion.h>
#include <Runtime/ImageWriteQueue/Public/ImageWriteQueue.h>
#include <HighResScreenshot.h>
#include <RHIGPUReadback.h>



namespace ImageUtil
{
  bool DecodePixelsByFormat(
    const void* PixelData,
    int32 SourcePitch,
    FIntPoint SourceExtent,
    FIntPoint DestinationExtent,
    EPixelFormat Format,
    FReadSurfaceDataFlags Flags,
    TArrayView<FLinearColor> Out)
  {
    SourcePitch *= GPixelFormats[Format].BlockBytes;
    auto OutPixelCount = DestinationExtent.X * DestinationExtent.Y;
    switch (Format)
    {
    case PF_G16:
    case PF_R16_UINT:
    case PF_R16_SINT:
      // Shadow maps
      ConvertRawR16DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_R8G8B8A8:
      ConvertRawR8G8B8A8DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_B8G8R8A8:
      ConvertRawB8G8R8A8DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_A2B10G10R10:
      ConvertRawA2B10G10R10DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_FloatRGBA:
    case PF_R16G16B16A16_UNORM:
    case PF_R16G16B16A16_SNORM:
      ConvertRawR16G16B16A16FDataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
      break;
    case PF_FloatR11G11B10:
      ConvertRawRR11G11B10DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_A32B32G32R32F:
      ConvertRawR32G32B32A32DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
      break;
    case PF_A16B16G16R16:
      ConvertRawR16G16B16A16DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_G16R16:
      ConvertRawR16G16DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_X24_G8: // Depth Stencil
      ConvertRawR24G8DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
      break;
    case PF_R32_FLOAT: // Depth Stencil
      ConvertRawR32DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
      break;
    case PF_R16G16B16A16_UINT:
    case PF_R16G16B16A16_SINT:
      ConvertRawR16G16B16A16DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    default:
      UE_LOG(LogCarla, Warning, TEXT("Unsupported format %llu"), (unsigned long long)Format);
      return false;
    }
    return true;
  }

  bool DecodePixelsByFormat(
    const void* PixelData,
    int32 SourcePitch,
    FIntPoint SourceExtent,
    FIntPoint DestinationExtent,
    EPixelFormat Format,
    FReadSurfaceDataFlags Flags,
    TArrayView<FColor> Out)
  {
    SourcePitch *= GPixelFormats[Format].BlockBytes;
    auto OutPixelCount = DestinationExtent.X * DestinationExtent.Y;
    switch (Format)
    {
    case PF_G16:
    case PF_R16_UINT:
    case PF_R16_SINT:
      // Shadow maps
      ConvertRawR16DataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_R8G8B8A8:
      ConvertRawR8G8B8A8DataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_B8G8R8A8:
      ConvertRawB8G8R8A8DataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_A2B10G10R10:
      ConvertRawR10G10B10A2DataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_FloatRGBA:
    case PF_R16G16B16A16_UNORM:
    case PF_R16G16B16A16_SNORM:
      ConvertRawR16G16B16A16FDataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags.GetLinearToGamma());
      break;
    case PF_FloatR11G11B10:
      ConvertRawR11G11B10DataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags.GetLinearToGamma());
      break;
    case PF_A32B32G32R32F:
      ConvertRawR32G32B32A32DataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags.GetLinearToGamma());
      break;
    case PF_A16B16G16R16:
      ConvertRawR16G16B16A16DataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_G16R16:
      ConvertRawR16G16DataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_DepthStencil: // Depth / Stencil
      ConvertRawD32S8DataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
      break;
    case PF_X24_G8: // Depth / Stencil
      ConvertRawR24G8DataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
      break;
    case PF_R32_FLOAT: // Depth
      ConvertRawR32DataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
      break;
    case PF_R16G16B16A16_UINT:
    case PF_R16G16B16A16_SINT:
      ConvertRawR16G16B16A16DataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_G8:
      ConvertRawR8DataToFColor(DestinationExtent.X, DestinationExtent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    default:
      UE_LOG(LogCarla, Warning, TEXT("Unsupported format %llu"), (unsigned long long)Format);
      return false;
    }
    return true;
  }

  bool ReadImageData(
    UTextureRenderTarget2D& RenderTarget,
    TArray<FColor>& Out)
  {
    check(IsInGameThread());
    auto Resource = RenderTarget.GameThread_GetRenderTargetResource();
    FReadSurfaceDataFlags ReadFlags(RCM_UNorm);
    ReadFlags.SetLinearToGamma(true);
    return Resource->ReadPixels(Out, ReadFlags);
  }

  bool ReadImageData(
    UTextureRenderTarget2D& RenderTarget,
    TArray64<FColor>& Out)
  {
    auto Resource = RenderTarget.GameThread_GetRenderTargetResource();
    FReadSurfaceDataFlags ReadFlags(RCM_UNorm);
    ReadFlags.SetLinearToGamma(true);
    Out.SetNum(RenderTarget.GetSurfaceWidth() * RenderTarget.GetSurfaceHeight());
    return Resource->ReadPixelsPtr(Out.GetData(), ReadFlags);
  }

  TUniquePtr<TImagePixelData<FColor>> ReadImageData(
    UTextureRenderTarget2D& RenderTarget)
  {
    const auto Size = FIntPoint(
      RenderTarget.GetSurfaceWidth(),
      RenderTarget.GetSurfaceHeight());
    auto PixelData = MakeUnique<TImagePixelData<FColor>>(Size);
    ReadImageData(RenderTarget, PixelData->Pixels);
    return TUniquePtr<TImagePixelData<FColor>>();
  }

  TFuture<bool> SaveImageData(
    UTextureRenderTarget2D& RenderTarget,
    const FStringView& Path)
  {
    return SaveImageData(ReadImageData(RenderTarget), Path);
  }

  TFuture<bool> SaveImageData(
    TUniquePtr<TImagePixelData<FColor>> Data,
    const FStringView& Path)
  {
    auto& HighResScreenshotConfig = GetHighResScreenshotConfig();
    auto ImageTask = MakeUnique<FImageWriteTask>();
    ImageTask->PixelData = MoveTemp(Data);
    ImageTask->Filename = Path;
    ImageTask->Format = EImageFormat::PNG;
    ImageTask->CompressionQuality = (int32)EImageCompressionQuality::Default;
    ImageTask->bOverwriteFile = true;
    ImageTask->PixelPreProcessors.Add(TAsyncAlphaWrite<FColor>(255));
    return HighResScreenshotConfig.ImageWriteQueue->Enqueue(MoveTemp(ImageTask));
  }

  static void ReadImageDataAsyncCommand(
    UTextureRenderTarget2D& RenderTarget,
    ReadImageDataAsyncCallback&& Callback)
  {
    static thread_local auto RenderQueryPool = RHICreateRenderQueryPool(RQT_AbsoluteTime);

    auto& CmdList = FRHICommandListImmediate::Get();
    auto Resource = static_cast<FTextureRenderTarget2DResource*>(
      RenderTarget.Resource);
    auto Texture = Resource->GetRenderTargetTexture();
    if (Texture == nullptr)
      return;
    auto Readback = MakeUnique<FRHIGPUTextureReadback>(TEXT("ReadImageData-Readback"));
    auto Size = Texture->GetSizeXY();
    auto Format = Texture->GetFormat();
    auto ResolveRect = FResolveRect();
    Readback->EnqueueCopy(CmdList, Texture, ResolveRect);

    auto Query = RenderQueryPool->AllocateQuery();
    CmdList.EndRenderQuery(Query.GetQuery());
    CmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
    uint64 DeltaTime;
    RHIGetRenderQueryResult(Query.GetQuery(), DeltaTime, true);
    Query.ReleaseQuery();

    AsyncTask(ENamedThreads::HighTaskPriority, [
      Readback = MoveTemp(Readback),
      Callback = std::move(Callback),
      Size, Format]
    {
      while (!Readback->IsReady())
        std::this_thread::yield();
      int32 RowPitch, BufferHeight;
      auto Mapping = Readback->Lock(RowPitch, &BufferHeight);
      if (Mapping != nullptr)
        Callback(Mapping, RowPitch, BufferHeight, Format, Size);
      Readback->Unlock();
    });
  }

  bool ReadImageDataAsync(
    UTextureRenderTarget2D& RenderTarget,
    ReadImageDataAsyncCallback&& Callback)
  {
    if (IsInRenderingThread())
    {
      ReadImageDataAsyncCommand(
        RenderTarget,
        std::move(Callback));
    }
    else
    {
      ENQUEUE_RENDER_COMMAND(ReadImageDataAsyncCmd)([
        &RenderTarget,
        Callback = std::move(Callback)
      ](auto& CmdList) mutable
      {
        ReadImageDataAsyncCommand(
          RenderTarget,
          std::move(Callback));
      });
    }
    return true;
  }

  bool ReadSensorImageDataAsync(
    AShaderBasedSensor& Sensor,
    ReadImageDataAsyncCallback&& Callback)
  {
    TArray<FColor> Pixels;
    auto RenderTarget = Sensor.GetCaptureRenderTarget();
    if (RenderTarget == nullptr)
      return false;
    return ReadImageDataAsync(
      *RenderTarget,
      std::move(Callback));
  }


  bool ReadImageDataAsyncFColor(
    UTextureRenderTarget2D& RenderTarget,
    ReadImageDataAsyncCallbackFColor&& Callback)
  {
    return ReadImageDataAsync(RenderTarget, [Callback = std::move(Callback)](
      const void* Mapping,
      size_t RowPitch,
      size_t BufferHeight,
      EPixelFormat Format,
      FIntPoint Size) -> bool
    {
      FReadSurfaceDataFlags Flags;
      TArray<FColor> Pixels;
      Pixels.SetNum(Size.X * Size.Y);
      if (!DecodePixelsByFormat(Mapping, RowPitch, Size, Size, Format, Flags, Pixels))
        return false;
      return Callback(Pixels, Size);
    });
  }

  bool ReadSensorImageDataAsyncFColor(
    AShaderBasedSensor& Sensor,
    ReadImageDataAsyncCallbackFColor&& Callback)
  {
    auto RenderTarget = Sensor.GetCaptureRenderTarget();
    if (RenderTarget == nullptr)
      return false;
    return ReadImageDataAsyncFColor(*RenderTarget, std::move(Callback));
  }

  bool ReadImageDataAsyncFLinearColor(
    UTextureRenderTarget2D& RenderTarget,
    ReadImageDataAsyncCallbackFLinearColor&& Callback)
  {
    return ReadImageDataAsync(RenderTarget, [Callback = std::move(Callback)](
      const void* Mapping,
      size_t RowPitch,
      size_t BufferHeight,
      EPixelFormat Format,
      FIntPoint Size) -> bool
      {
        FReadSurfaceDataFlags Flags;
        TArray<FLinearColor> Pixels;
        Pixels.SetNum(Size.X * Size.Y);
        if (!DecodePixelsByFormat(Mapping, RowPitch, Size, Size, Format, Flags, Pixels))
          return false;
        return Callback(Pixels, Size);
      });
  }

  bool ReadSensorImageDataAsyncFLinearColor(
    AShaderBasedSensor& Sensor,
    ReadImageDataAsyncCallbackFLinearColor&& Callback)
  {
    auto RenderTarget = Sensor.GetCaptureRenderTarget();
    if (RenderTarget == nullptr)
      return false;
    return ReadImageDataAsyncFLinearColor(*RenderTarget, std::move(Callback));
  }
}
