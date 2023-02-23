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

#ifdef CARLA_ENABLE_LLM

#include "HAL/LowLevelMemStats.h"

DECLARE_LLM_MEMORY_STAT(TEXT("STAT_CARLA"), STAT_CARLA, STATGROUP_LLMFULL);

static void InitTags()
{
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_00, TEXT("CARLA_LLM_TAG_00"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_01, TEXT("CARLA_LLM_TAG_01"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_02, TEXT("CARLA_LLM_TAG_02"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_03, TEXT("CARLA_LLM_TAG_03"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_04, TEXT("CARLA_LLM_TAG_04"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_05, TEXT("CARLA_LLM_TAG_05"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_06, TEXT("CARLA_LLM_TAG_06"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_07, TEXT("CARLA_LLM_TAG_07"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_08, TEXT("CARLA_LLM_TAG_08"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_09, TEXT("CARLA_LLM_TAG_09"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_10, TEXT("CARLA_LLM_TAG_10"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_11, TEXT("CARLA_LLM_TAG_11"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_12, TEXT("CARLA_LLM_TAG_12"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_13, TEXT("CARLA_LLM_TAG_13"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_14, TEXT("CARLA_LLM_TAG_14"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_15, TEXT("CARLA_LLM_TAG_15"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_16, TEXT("CARLA_LLM_TAG_16"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_17, TEXT("CARLA_LLM_TAG_17"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_18, TEXT("CARLA_LLM_TAG_18"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_19, TEXT("CARLA_LLM_TAG_19"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_20, TEXT("CARLA_LLM_TAG_20"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_21, TEXT("CARLA_LLM_TAG_21"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_22, TEXT("CARLA_LLM_TAG_22"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_23, TEXT("CARLA_LLM_TAG_23"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_24, TEXT("CARLA_LLM_TAG_24"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_25, TEXT("CARLA_LLM_TAG_25"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_26, TEXT("CARLA_LLM_TAG_26"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_27, TEXT("CARLA_LLM_TAG_27"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_28, TEXT("CARLA_LLM_TAG_28"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_29, TEXT("CARLA_LLM_TAG_29"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_30, TEXT("CARLA_LLM_TAG_30"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_31, TEXT("CARLA_LLM_TAG_31"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_32, TEXT("CARLA_LLM_TAG_32"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_33, TEXT("CARLA_LLM_TAG_33"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_34, TEXT("CARLA_LLM_TAG_34"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_35, TEXT("CARLA_LLM_TAG_35"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_36, TEXT("CARLA_LLM_TAG_36"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_37, TEXT("CARLA_LLM_TAG_37"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_38, TEXT("CARLA_LLM_TAG_38"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_39, TEXT("CARLA_LLM_TAG_39"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_40, TEXT("CARLA_LLM_TAG_40"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_41, TEXT("CARLA_LLM_TAG_41"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_42, TEXT("CARLA_LLM_TAG_42"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_43, TEXT("CARLA_LLM_TAG_43"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_44, TEXT("CARLA_LLM_TAG_44"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_45, TEXT("CARLA_LLM_TAG_45"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_46, TEXT("CARLA_LLM_TAG_46"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_47, TEXT("CARLA_LLM_TAG_47"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_48, TEXT("CARLA_LLM_TAG_48"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_49, TEXT("CARLA_LLM_TAG_49"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_50, TEXT("CARLA_LLM_TAG_50"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_51, TEXT("CARLA_LLM_TAG_51"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_52, TEXT("CARLA_LLM_TAG_52"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_53, TEXT("CARLA_LLM_TAG_53"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_54, TEXT("CARLA_LLM_TAG_54"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_55, TEXT("CARLA_LLM_TAG_55"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_56, TEXT("CARLA_LLM_TAG_56"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_57, TEXT("CARLA_LLM_TAG_57"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_58, TEXT("CARLA_LLM_TAG_58"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_59, TEXT("CARLA_LLM_TAG_59"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_60, TEXT("CARLA_LLM_TAG_60"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_61, TEXT("CARLA_LLM_TAG_61"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_62, TEXT("CARLA_LLM_TAG_62"), GET_STATFNAME(STAT_CARLA), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_63, TEXT("CARLA_LLM_TAG_63"), GET_STATFNAME(STAT_CARLA), NAME_None));
}

#endif

void FPixelReader::WritePixelsToBuffer(
    const UTextureRenderTarget2D &RenderTarget,
    uint32 Offset,
    FRHICommandListImmediate &RHICmdList,
    FPixelReader::Payload&& FuncForSending)
{
#ifdef CARLA_ENABLE_LLM
    static std::once_flag once_flag;

    std::call_once(once_flag, InitTags);
#endif

  TRACE_CPUPROFILER_EVENT_SCOPE_STR("WritePixelsToBuffer");
  check(IsInRenderingThread());
  
  auto Texture = RenderTarget.Resource->GetTexture2DRHI();

  if (!Texture)
  {
    return;
  }

  auto BackBufferReadback = MakeUnique<FRHIGPUTextureReadback>(TEXT("CameraBufferReadback"));

  FIntPoint BackBufferSize = Texture->GetSizeXY();
  EPixelFormat BackBufferPixelFormat = Texture->GetFormat();
  
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("EnqueueCopy");
    BackBufferReadback->EnqueueCopy(RHICmdList, Texture);
  }

  // workaround to force RHI with Vulkan to refresh the fences state in the middle of frame
  static thread_local auto QueryPool = RHICreateRenderQueryPool(RQT_AbsoluteTime);

  {
    auto Query = QueryPool->AllocateQuery();
    auto QueryPtr = Query.GetQuery();
    {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("create query");
        RHICmdList.EndRenderQuery(QueryPtr);
    }
    {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("Flush");
        RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
    }
    {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("query result");
        uint64 OldAbsTime = 0;
        RHICmdList.GetRenderQueryResult(QueryPtr, OldAbsTime, true);
    }
  }

  AsyncTask(ENamedThreads::ActualRenderingThread, [=, Readback = MoveTemp(BackBufferReadback), FuncForSending = std::move(FuncForSending)]() mutable
  {
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Wait GPU transfer");
      while (!Readback->IsReady())
      {
        std::this_thread::yield();
      }
    }
    
    FPixelFormatInfo PixelFormat = GPixelFormats[BackBufferPixelFormat];
    uint32 ExpectedRowBytes = BackBufferSize.X * PixelFormat.BlockBytes;
    int32 Size = (BackBufferSize.Y * (PixelFormat.BlockBytes * BackBufferSize.X));
    void* LockedData = Readback->Lock(Size);
    if (LockedData)
    {
        try
        {
            FuncForSending(LockedData, Size, Offset, ExpectedRowBytes);
        }
        catch (std::exception& e)
        {
            UE_LOG(LogCarla, Warning, TEXT("Exception thrown from PixelReader send callback: %s"), e.what());
        }
    }
    Readback->Unlock();
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
