// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "SensorManager.h"

void UAtlasManager8Bit::RegisterSceneCaptureSensor(ASceneCaptureSensor* InSensor)
{
  CaptureSensors.Add(InSensor);

  uint32 ImageWidth = InSensor->GetImageWidth();
  uint32 ImageHeight = InSensor->GetImageHeight();

  if(AtlasTextureWidth < ImageWidth)
  {
    AtlasTextureWidth = ImageWidth;
  }

  InSensor->PositionInAtlas = FIntVector(0, AtlasTextureHeight, 0);
  AtlasTextureHeight += ImageHeight;
  IsAtlasTextureValid = false;

  UE_LOG(LogCarla, Log, TEXT("UAtlasManager8Bit::RegisterSceneCaptureSensor %d %dx%d"),
      CaptureSensors.Num(), AtlasTextureWidth, AtlasTextureHeight);
}

void UAtlasManager8Bit::UnregisterSceneCaptureSensor(ASceneCaptureSensor* InSensor)
{
  CaptureSensors.Remove(InSensor);
  IsAtlasTextureValid = false;

  AtlasTextureWidth = 0u;
  AtlasTextureHeight = 0u;
  for(ASceneCaptureSensor* Sensor : CaptureSensors)
  {
    uint32 ImageWidth = Sensor->GetImageWidth();
    uint32 ImageHeight = Sensor->GetImageHeight();

    Sensor->PositionInAtlas = FIntVector(0, AtlasTextureHeight, 0);

    AtlasTextureHeight += ImageHeight;
    if(AtlasTextureWidth < ImageWidth)
    {
      AtlasTextureWidth = ImageWidth;
    }
  }
  UE_LOG(LogCarla, Log, TEXT("UAtlasManager8Bit::UnregisterSceneCaptureSensor %d %dx%d"),
      CaptureSensors.Num(), AtlasTextureWidth, AtlasTextureHeight);
}

void UAtlasManager8Bit::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR("UAtlasManager8Bit::PostPhysTick GameThread");
  if(CaptureSensors.Num() == 0) return;

  for(ASceneCaptureSensor* Sensor : CaptureSensors)
  {
    Sensor->PostPhysTick(World, TickType, DeltaSeconds);
  }

  ENQUEUE_RENDER_COMMAND(UAtlasManager8Bit_PostPhysTick)
  (
    [&,
        WasAtlasTextureValid = IsAtlasTextureValid,
        AtlasTextureWidthCache = AtlasTextureWidth,
        AtlasTextureHeightCache = AtlasTextureHeight,
        SensorList = CaptureSensors
        ](FRHICommandListImmediate& RHICmdList)
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("UAtlasManager8Bit::PostPhysTick RenderThread");
      check(IsInRenderingThread());
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("UAtlasManager8Bit::PostPhysTickRenderThread ModifyAtlas");
        if(!WasAtlasTextureValid)
        {
          if(AtlasTextureWidthCache > 0 && AtlasTextureHeightCache > 0)
          {
            UE_LOG(LogCarla, Log, TEXT("UAtlasManager8Bit::PostPhysTick RenderThread generating atlas WxH=[%d, %d]"),
                AtlasTextureWidthCache, AtlasTextureHeightCache);
            FRHIResourceCreateInfo CreateInfo;
            SceneCaptureAtlasTexture =
                RHICreateTexture2D(AtlasTextureWidthCache, AtlasTextureHeightCache,
                PF_B8G8R8A8, 1, 1, TexCreate_FastVRAM & TexCreate_CPUReadback & TexCreate_RenderTargetable,
                CreateInfo);
            AtlasPixels.SetNum(AtlasTextureWidthCache * AtlasTextureHeightCache);
          }
          else
          {
            SceneCaptureAtlasTexture = nullptr;
          }
        }
      }

      ////////////////////////////////////////////////////////////////////////
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("UAtlasManager8Bit::PostPhysTick CopyTexturetoAtlas");
        for(ASceneCaptureSensor* Sensor : SensorList)
        {
          if(IsValid(Sensor))
          {
            Sensor->CopyTexturetoAtlas(RHICmdList, SceneCaptureAtlasTexture);
          }
        }
      }
      ////////////////////////////////////////////////////////////////////////
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("UAtlasManager8Bit::PostPhysTick DownloadAtlasTexture");

        if (!SceneCaptureAtlasTexture)
        {
          UE_LOG(LogCarla, Error, TEXT("DownloadAtlasTexture: Missing atlas texture"));
          return;
        }

        if(SceneCaptureAtlasTexture)
        {
          RHICmdList.ReadSurfaceData(
            SceneCaptureAtlasTexture,
            FIntRect(0, 0, AtlasTextureWidthCache, AtlasTextureHeightCache),
            AtlasPixels,
            FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));
        }
      }
      ////////////////////////////////////////////////////////////////////////
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("UAtlasManager8Bit::PostPhysTick SendPixels");
        // TODO: parallelize
        for(ASceneCaptureSensor* Sensor : SensorList)
        {
          if(IsValid(Sensor))
          {
            Sensor->SendPixelsInRenderThread(AtlasPixels, AtlasTextureWidthCache);
          }
        }
      }
      ////////////////////////////////////////////////////////////////////////
      RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
    }
  );
  IsAtlasTextureValid = true;
}


void UAtlasManager16Bit::RegisterSceneCaptureSensor(ASceneCaptureSensor* InSensor)
{
  CaptureSensors.Add(InSensor);

  uint32 ImageWidth = InSensor->GetImageWidth();
  uint32 ImageHeight = InSensor->GetImageHeight();

  if(AtlasTextureWidth < ImageWidth)
  {
    AtlasTextureWidth = ImageWidth;
  }

  InSensor->PositionInAtlas = FIntVector(0, AtlasTextureHeight, 0);
  AtlasTextureHeight += ImageHeight;
  IsAtlasTextureValid = false;

  UE_LOG(LogCarla, Log, TEXT("UAtlasManager16Bit::RegisterSceneCaptureSensor %d %dx%d"),
      CaptureSensors.Num(), AtlasTextureWidth, AtlasTextureHeight);
}

void UAtlasManager16Bit::UnregisterSceneCaptureSensor(ASceneCaptureSensor* InSensor)
{
  CaptureSensors.Remove(InSensor);
  IsAtlasTextureValid = false;

  AtlasTextureWidth = 0u;
  AtlasTextureHeight = 0u;
  for(ASceneCaptureSensor* Sensor : CaptureSensors)
  {
    uint32 ImageWidth = Sensor->GetImageWidth();
    uint32 ImageHeight = Sensor->GetImageHeight();

    Sensor->PositionInAtlas = FIntVector(0, AtlasTextureHeight, 0);

    AtlasTextureHeight += ImageHeight;
    if(AtlasTextureWidth < ImageWidth)
    {
      AtlasTextureWidth = ImageWidth;
    }
  }
  UE_LOG(LogCarla, Log, TEXT("UAtlasManager16Bit::UnregisterSceneCaptureSensor %d %dx%d"),
      CaptureSensors.Num(), AtlasTextureWidth, AtlasTextureHeight);
}

void UAtlasManager16Bit::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR("UAtlasManager16Bit::PostPhysTick GameThread");
  if(CaptureSensors.Num() == 0) return;

  for(ASceneCaptureSensor* Sensor : CaptureSensors)
  {
    Sensor->PostPhysTick(World, TickType, DeltaSeconds);
  }

  ENQUEUE_RENDER_COMMAND(UAtlasManager16Bit_PostPhysTick)
  (
        [&,
        WasAtlasTextureValid = IsAtlasTextureValid,
        AtlasTextureWidthCache = AtlasTextureWidth,
        AtlasTextureHeightCache = AtlasTextureHeight,
        SensorList = CaptureSensors
        ](FRHICommandListImmediate& RHICmdList)
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("UAtlasManager16Bit::PostPhysTick RenderThread");
      check(IsInRenderingThread());
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("UAtlasManager16Bit::PostPhysTickRenderThread ModifyAtlas");
        if(!WasAtlasTextureValid)
        {
          if(AtlasTextureWidthCache > 0 && AtlasTextureHeightCache > 0)
          {
            UE_LOG(LogCarla, Log, TEXT("UAtlasManager16Bit::PostPhysTick RenderThread generating atlas WxH=[%d, %d]"),
                AtlasTextureWidthCache, AtlasTextureHeightCache);
            FRHIResourceCreateInfo CreateInfo;
            SceneCaptureAtlasTexture =
                RHICreateTexture2D(AtlasTextureWidthCache, AtlasTextureHeightCache,
                PF_FloatRGBA, 1, 1, TexCreate_FastVRAM & TexCreate_CPUReadback & TexCreate_RenderTargetable, CreateInfo);
            AtlasPixels.SetNum(AtlasTextureWidthCache * AtlasTextureHeightCache);
          }
          else
          {
            SceneCaptureAtlasTexture = nullptr;
          }
        }
      }

      ////////////////////////////////////////////////////////////////////////
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("UAtlasManager16Bit::PostPhysTick CopyTexturetoAtlas");
        for(ASceneCaptureSensor* Sensor : CaptureSensors)
        {
          if(IsValid(Sensor))
          {
            Sensor->CopyTexturetoAtlas(RHICmdList, SceneCaptureAtlasTexture);
          }
        }
      }
      ////////////////////////////////////////////////////////////////////////
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("UAtlasManager16Bit::PostPhysTick DownloadAtlasTexture");

        if (!SceneCaptureAtlasTexture)
        {
          UE_LOG(LogCarla, Error, TEXT("DownloadAtlasTexture: Missing atlas texture"));
          return;
        }

        if(SceneCaptureAtlasTexture)
        {
          //  IsAtlasTextureValid, WasAtlasTextureValid, ReadSurfaceWaitUntilIdle);
          RHICmdList.ReadSurfaceFloatData(
            SceneCaptureAtlasTexture,
            FIntRect(0, 0, AtlasTextureWidthCache, AtlasTextureHeightCache),
            AtlasPixels,
            FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));
        }
      }
      ////////////////////////////////////////////////////////////////////////
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("UAtlasManager16Bit::PostPhysTick SendPixels");
        // TODO: parallelize
        for(ASceneCaptureSensor* Sensor : CaptureSensors)
        {
          if(IsValid(Sensor))
          {
            Sensor->SendPixelsInRenderThread(AtlasPixels, AtlasTextureWidthCache);
          }
        }
      }
      ////////////////////////////////////////////////////////////////////////
      RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
    }
  );
  IsAtlasTextureValid = true;
}

ASensorManager::ASensorManager(const FObjectInitializer& ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;

  AtlasManager8Bit = NewObject<UAtlasManager8Bit>(this, FName("AtlasManager8Bit"));
  AtlasManager16Bit = NewObject<UAtlasManager16Bit>(this, FName("AtlasManager16Bit"));
}

void ASensorManager::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  // tick camera sensors
  AtlasManager8Bit->PostPhysTick(World, TickType, DeltaSeconds);
  AtlasManager16Bit->PostPhysTick(World, TickType, DeltaSeconds);

  // tick the rest of the sensors
  for(ASensor* Sensor : GenericSensors)
  {
    Sensor->PostPhysTick(World, TickType, DeltaSeconds);
  }
}

void ASensorManager::Register8BitCameraSensor(ASceneCaptureSensor* InSensor)
{
  UnregisterGenericSensor(InSensor);
  AtlasManager8Bit->RegisterSceneCaptureSensor(InSensor);
}

void ASensorManager::Unregister8BitCameraSensor(ASceneCaptureSensor* InSensor)
{
  AtlasManager8Bit->UnregisterSceneCaptureSensor(InSensor);
}

void ASensorManager::Register16BitCameraSensor(ASceneCaptureSensor* InSensor)
{
  UnregisterGenericSensor(InSensor);
  AtlasManager16Bit->RegisterSceneCaptureSensor(InSensor);
}

void ASensorManager::Unregister16BitCameraSensor(ASceneCaptureSensor* InSensor)
{
  AtlasManager16Bit->UnregisterSceneCaptureSensor(InSensor);
}

void ASensorManager::RegisterGenericSensor(ASensor* InSensor)
{
  GenericSensors.Add(InSensor);
}

void ASensorManager::UnregisterGenericSensor(ASensor* InSensor)
{
  GenericSensors.Remove(InSensor);
}
