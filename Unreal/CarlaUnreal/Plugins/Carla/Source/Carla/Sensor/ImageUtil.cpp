// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <Carla/Sensor/ImageUtil.h>
#include <Carla/Sensor/ShaderBasedSensor.h>
#include <Carla/Carla.h>

#include <util/ue-header-guard-begin.h>
#include <RHISurfaceDataConversion.h>
#include <ImageWriteQueue.h>
#include <HighResScreenshot.h>
#include <RHIGPUReadback.h>
#include <Async/ParallelFor.h>
#include <util/ue-header-guard-end.h>

#include <chrono>
#include <thread>

template <typename F>
class ScopedCallback
{
    F fn;
public:

    constexpr ScopedCallback(F&& fn) : fn(fn) { }

    ~ScopedCallback() { fn(); }
};



namespace ImageUtil
{
  bool DecodePixelsByFormat(
    const void* PixelData,
    int32 SourcePitch,
    FIntPoint Extent,
    EPixelFormat Format,
    FReadSurfaceDataFlags Flags,
    TArrayView<FLinearColor> Out)
  {
    SourcePitch *= GPixelFormats[Format].BlockBytes;
    return ConvertRAWSurfaceDataToFLinearColor(
      Format,
      Extent.X,
      Extent.Y,
      (uint8*)PixelData,
      SourcePitch,
      Out.GetData(),
      Flags);
  }



  bool DecodePixelsByFormat(
    const void* PixelData,
    int32 SourcePitch,
    FIntPoint Extent,
    EPixelFormat Format,
    FReadSurfaceDataFlags Flags,
    TArrayView<FColor> Out)
  {
    SourcePitch *= GPixelFormats[Format].BlockBytes;
    auto OutPixelCount = Extent.X * Extent.Y;
    switch (Format)
    {
    case PF_G16:
    case PF_R16_UINT:
    case PF_R16_SINT:
      // Shadow maps
      ConvertRawR16DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_R8G8B8A8:
      ConvertRawR8G8B8A8DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_B8G8R8A8:
      ConvertRawB8G8R8A8DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_A2B10G10R10:
      ConvertRawR10G10B10A2DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_FloatRGBA:
    case PF_R16G16B16A16_UNORM:
    case PF_R16G16B16A16_SNORM:
      ConvertRawR16G16B16A16FDataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags.GetLinearToGamma());
      break;
    case PF_FloatR11G11B10:
      ConvertRawR11G11B10DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags.GetLinearToGamma());
      break;
    case PF_A32B32G32R32F:
      ConvertRawR32G32B32A32DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags.GetLinearToGamma());
      break;
    case PF_A16B16G16R16:
      ConvertRawR16G16B16A16DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_G16R16:
      ConvertRawR16G16DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_DepthStencil: // Depth / Stencil
      ConvertRawD32S8DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
      break;
    case PF_X24_G8: // Depth / Stencil
      ConvertRawR24G8DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
      break;
    case PF_R32_FLOAT: // Depth
		  for (uint32 Y = 0; Y < (uint32)Extent.Y; Y++)
		  {
		  	auto SrcPtr = (float*)((uint8*)PixelData + Y * SourcePitch);
		  	auto DestPtr = Out.GetData() + Y * Extent.X;
		  	for (uint32 X = 0; X < (uint32)Extent.X; X++)
		  	{
		  		*DestPtr = FLinearColor(SrcPtr[0], 0.f, 0.f, 1.f).QuantizeRound();
		  		++SrcPtr;
		  		++DestPtr;
		  	}
		  }
      break;
    case PF_R16G16B16A16_UINT:
    case PF_R16G16B16A16_SINT:
      ConvertRawR16G16B16A16DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
      break;
    case PF_G8:
      ConvertRawR8DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
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
    return PixelData;
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



  struct ReadImageDataContext
  {
    EPixelFormat Format;
    FIntPoint Size;
    ReadImageDataAsyncCallback Callback;
    // Pool-owned readback: lifetime managed by Pool/SlotIndex pair. Pointer
    // is non-owning when Pool is set, owning (via FallbackReadback) otherwise.
    FRHIGPUTextureReadback* Readback = nullptr;
    TUniquePtr<FRHIGPUTextureReadback> FallbackReadback;
    FRHIGPUReadbackPoolPtr Pool;
    int32 SlotIndex = INDEX_NONE;
  };

  static void ReadImageDataBegin(
    ReadImageDataContext& Self,
    UTextureRenderTarget2D& RenderTarget,
    FRHIGPUReadbackPoolPtr Pool,
    ReadImageDataAsyncCallback&& Callback)
  {
    auto& CmdList = FRHICommandListImmediate::Get();
    auto Resource = static_cast<FTextureRenderTarget2DResource*>(
      RenderTarget.GetResource());
    auto Texture = Resource->GetRenderTargetTexture();
    if (Texture == nullptr)
      return;
    Self.Callback = std::move(Callback);
    Self.Pool = std::move(Pool);
    if (Self.Pool)
    {
      Self.Readback = Self.Pool->Acquire(Self.SlotIndex);
    }
    if (Self.Readback == nullptr)
    {
      Self.FallbackReadback = MakeUnique<FRHIGPUTextureReadback>(
        TEXT("ReadImageData-Readback"));
      Self.Readback = Self.FallbackReadback.Get();
    }
    Self.Size = Texture->GetSizeXY();
    Self.Format = Texture->GetFormat();
    auto ResolveRect = FResolveRect();
    Self.Readback->EnqueueCopy(CmdList, Texture, ResolveRect);

    // Record the copy and return -- never flush or wait for GPU completion
    // here, on the render thread, per camera.
    //
    // Non-blocking callers (path-traced rt_lens sensor) poll completion
    // off-thread (ReadImageDataEndAsync); any synchronous wait/flush here can
    // deadlock the whole server under load (proven with gdb: the render thread
    // blocks in RHIGetRenderQueryResult(..., bWait=true) mid-render-command
    // inside ProcessInterruptQueueUntil (VulkanQuery.cpp) so it can no longer
    // drive GPU submission -- RHIThread/RHISubmission sit idle and the game
    // thread freezes behind it at FFrameEndSync).
    //
    // Blocking callers (all raster cameras) are delivered by
    // FlushBatchedReadbacks() at the end of the sensor tick: one GPU sync for
    // the whole camera batch. The historical per-camera flush + query-wait
    // right here drained the entire GPU pipeline once per camera per frame
    // (~13 ms each, resolution-independent), serializing the render thread
    // and starving the GPU (~58% SM utilization at 5 cameras).
  }

  static void ReadImageDataEnd(
    ReadImageDataContext& Self)
  {
    int32 RowPitch, BufferHeight;
    auto MappedPtr = Self.Readback->Lock(RowPitch, &BufferHeight);
    if (MappedPtr != nullptr)
    {
      ScopedCallback Unlock = [&] { Self.Readback->Unlock(); };
      Self.Callback(MappedPtr, RowPitch, BufferHeight, Self.Format, Self.Size);
    }
    if (Self.Pool && Self.SlotIndex != INDEX_NONE)
    {
      Self.Pool->Release(Self.SlotIndex);
      Self.SlotIndex = INDEX_NONE;
    }
  }

  static void ReadImageDataEndAsync(
    ReadImageDataContext&& Self)
  {
    // Poll on a BACKGROUND worker, never a named thread. ENamedThreads::
    // HighTaskPriority can be serviced by the RHIThread, and a busy IsReady()
    // poll there blocks GPU submission -- the copy then never completes (its
    // fence never signals), occlusion/other RHI fences stall, and the whole
    // server starves (proven via gdb: RHIThread stuck in this poll loop).
    AsyncTask(
      ENamedThreads::AnyBackgroundThreadNormalTask, [
      Self = std::move(Self)]() mutable
    {
      // Poll for GPU copy completion off-thread (never blocks the render thread).
      // Bounded: if the copy has not completed after a few seconds (e.g. the
      // capture stopped, so its command buffer was never submitted) drop this
      // frame and release any pool slot rather than leak a spinning task.
      uint64 Spins = 0;
      while (!Self.Readback->IsReady())
      {
        std::this_thread::sleep_for(std::chrono::microseconds(200));
        if (++Spins > 25000u)  // ~5 s
        {
          if (Self.Pool && Self.SlotIndex != INDEX_NONE)
          {
            Self.Pool->Release(Self.SlotIndex);
            Self.SlotIndex = INDEX_NONE;
          }
          return;
        }
      }
      // The copy is complete, but FRHIGPUTextureReadback::Lock() maps via the
      // immediate command list and asserts IsInRenderingThread(); it cannot run
      // on this worker thread. Hand the final Lock/decode/deliver to the render
      // thread. Because the readback is already ready, this Lock is a cheap
      // CPU-side map -- it does NOT wait on the GPU, so the render thread is not
      // stalled and cannot deadlock.
      ENQUEUE_RENDER_COMMAND(RtlReadbackDeliver)(
        [Self = MoveTemp(Self)](FRHICommandListImmediate&) mutable
        {
          ReadImageDataEnd(Self);
        });
    });
  }

  // Blocking readbacks recorded this tick, waiting for the single per-tick
  // GPU sync in FlushBatchedReadbacks(). Render-thread access only.
  static TArray<ReadImageDataContext> GBatchedReadbacks;

  static void ReadImageDataAddToBatch(ReadImageDataContext&& Context)
  {
    check(IsInRenderingThread());
    GBatchedReadbacks.Add(MoveTemp(Context));
  }

  void FlushBatchedReadbacks()
  {
    ENQUEUE_RENDER_COMMAND(FlushBatchedReadbacksCmd)(
      [](FRHICommandListImmediate& CmdList)
    {
      if (GBatchedReadbacks.IsEmpty())
        return;
      TArray<ReadImageDataContext> Batch = MoveTemp(GBatchedReadbacks);
      GBatchedReadbacks.Reset();

      // One GPU sync for the whole batch. Every camera's capture and copy is
      // already recorded ahead of this point in render-command order, so a
      // single flush + query-wait guarantees all of them completed -- the same
      // per-tick "all sensors delivered" guarantee the old per-camera wait
      // gave, at one pipeline drain per tick instead of one per camera.
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("FlushBatchedReadbacks Sync");
        static thread_local auto RenderQueryPool =
            RHICreateRenderQueryPool(RQT_AbsoluteTime);
        auto Query = RenderQueryPool->AllocateQuery();
        CmdList.EndRenderQuery(Query.GetQuery());
        CmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
        uint64 DeltaTime;
        RHIGetRenderQueryResult(Query.GetQuery(), DeltaTime, true);
        Query.ReleaseQuery();
      }

      // Lock() needs the render thread, but after the sync it is a cheap
      // CPU-side map. Decode + delivery is per-sensor independent (each
      // callback owns its pixel buffer and data stream), so it runs wide.
      struct FMappedReadback
      {
        void* Data = nullptr;
        int32 RowPitch = 0;
        int32 BufferHeight = 0;
      };
      TArray<FMappedReadback> Mapped;
      Mapped.SetNum(Batch.Num());
      for (int32 Index = 0; Index < Batch.Num(); ++Index)
      {
        Mapped[Index].Data = Batch[Index].Readback->Lock(
          Mapped[Index].RowPitch, &Mapped[Index].BufferHeight);
      }
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("FlushBatchedReadbacks Deliver");
        ParallelFor(Batch.Num(), [&](int32 Index)
        {
          if (Mapped[Index].Data != nullptr)
          {
            Batch[Index].Callback(
              Mapped[Index].Data,
              Mapped[Index].RowPitch,
              Mapped[Index].BufferHeight,
              Batch[Index].Format,
              Batch[Index].Size);
          }
        });
      }
      for (int32 Index = 0; Index < Batch.Num(); ++Index)
      {
        if (Mapped[Index].Data != nullptr)
          Batch[Index].Readback->Unlock();
        if (Batch[Index].Pool && Batch[Index].SlotIndex != INDEX_NONE)
          Batch[Index].Pool->Release(Batch[Index].SlotIndex);
      }
    });
  }

  bool ReadImageDataAsync(
    UTextureRenderTarget2D& RenderTarget,
    FRHIGPUReadbackPoolPtr Pool,
    ReadImageDataAsyncCallback&& Callback,
    bool bNonBlocking)
  {
    if (IsInRenderingThread())
    {
      ReadImageDataContext Context = { };
      ReadImageDataBegin(Context, RenderTarget, std::move(Pool), std::move(Callback));
      if (Context.Readback == nullptr)
        return false;
      // Non-blocking variant (rt_lens): completion polled off-thread.
      // Blocking variant (raster cameras): delivered by the per-tick batch.
      if (bNonBlocking)
        ReadImageDataEndAsync(std::move(Context));
      else
        ReadImageDataAddToBatch(std::move(Context));
    }
    else
    {
      ENQUEUE_RENDER_COMMAND(ReadImageDataAsyncCmd)([
        &RenderTarget, Pool = std::move(Pool), Callback = std::move(Callback), bNonBlocking
      ](auto& CmdList) mutable
      {
        ReadImageDataContext Context = { };
        ReadImageDataBegin(Context, RenderTarget, std::move(Pool), std::move(Callback));
        if (Context.Readback == nullptr)
          return;
        if (bNonBlocking)
          ReadImageDataEndAsync(std::move(Context));
        else
          ReadImageDataAddToBatch(std::move(Context));
      });

    }
    return true;
  }



  bool ReadImageDataAsync(
    UTextureRenderTarget2D& RenderTarget,
    ReadImageDataAsyncCallback&& Callback)
  {
    return ReadImageDataAsync(RenderTarget, nullptr, std::move(Callback), false);
  }



  bool ReadSensorImageDataAsync(
    AShaderBasedSensor& Sensor,
    ReadImageDataAsyncCallback&& Callback)
  {
    auto RenderTarget = Sensor.GetCaptureRenderTarget();
    if (RenderTarget == nullptr)
      return false;
    return ReadImageDataAsync(
      *RenderTarget,
      Sensor.GetReadbackPool(),
      std::move(Callback));
  }



  bool ReadImageDataAsyncFColor(
    UTextureRenderTarget2D& RenderTarget,
    ReadImageDataAsyncCallbackFColor&& Callback,
    bool bNonBlocking,
    FRHIGPUReadbackPoolPtr Pool)
  {
    return ReadImageDataAsync(RenderTarget, std::move(Pool), [Callback = std::move(Callback)](
      const void* Mapping,
      size_t RowPitch,
      size_t BufferHeight,
      EPixelFormat Format,
      FIntPoint Size) -> bool
    {
      FReadSurfaceDataFlags Flags;
      TArray<FColor> Pixels;
      Pixels.SetNum(Size.X * Size.Y);
      if (!DecodePixelsByFormat(Mapping, RowPitch, Size, Format, Flags, Pixels))
        return false;
      return Callback(Pixels, Size);
    }, bNonBlocking);
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
        if (!DecodePixelsByFormat(Mapping, RowPitch, Size, Format, Flags, Pixels))
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
