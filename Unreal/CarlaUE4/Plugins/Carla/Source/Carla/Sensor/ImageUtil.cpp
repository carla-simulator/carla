// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/ImageUtil.h"
#include "Runtime/RHI/Public/RHISurfaceDataConversion.h"



namespace ImageUtil
{
  void DecodePixelsByFormat(
      TArrayView<FLinearColor> Out,
      TArrayView<uint8> PixelData,
      int32 SourcePitch,
      FIntPoint SourceExtent,
      FIntPoint DestinationExtent,
      EPixelFormat Format,
      FReadSurfaceDataFlags Flags)
  {
    SourcePitch *= GPixelFormats[Format].BlockBytes;
    auto OutPixelCount = DestinationExtent.X * DestinationExtent.Y;
    switch (Format)
    {
    case PF_G16:
    case PF_R16_UINT:
    case PF_R16_SINT:
      // Shadow maps
      ConvertRawR16DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_R8G8B8A8:
      ConvertRawR8G8B8A8DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_B8G8R8A8:
      ConvertRawB8G8R8A8DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_A2B10G10R10:
      ConvertRawA2B10G10R10DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_FloatRGBA:
    case PF_R16G16B16A16_UNORM:
    case PF_R16G16B16A16_SNORM:
      ConvertRawR16G16B16A16FDataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData(), Flags);
      break;
    case PF_FloatR11G11B10:
      ConvertRawRR11G11B10DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_A32B32G32R32F:
      ConvertRawR32G32B32A32DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData(), Flags);
      break;
    case PF_A16B16G16R16:
      ConvertRawR16G16B16A16DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_G16R16:
      ConvertRawR16G16DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_DepthStencil: // Depth Stencil
      ConvertRawD32S8DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData(), Flags);
      break;
    case PF_X24_G8: // Depth Stencil
      ConvertRawR24G8DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData(), Flags);
      break;
    case PF_R32_FLOAT: // Depth
      for (uint32 Y = 0; Y < (uint32)DestinationExtent.Y; Y++)
      {
        TArrayView<float> Row(
          (float*)(&PixelData[0] + Y * SourcePitch),
          DestinationExtent.X);
        auto DestPtr = Out.GetData() + Y * DestinationExtent.X;
        for (uint32 X = 0; X < (uint32)DestinationExtent.X; X++)
        {
          auto Depth = Row[X];
          // Depth = FMath::Min(Flags.ComputeNormalizedDepth(Row[X]), 1.0f);
          *DestPtr = FLinearColor(Depth, Depth, Depth);
          ++DestPtr;
        }
      }
      break;
    case PF_R16G16B16A16_UINT:
    case PF_R16G16B16A16_SINT:
      ConvertRawR16G16B16A16DataToFLinearColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    default:
      UE_LOG(LogCarla, Warning, TEXT("Unsupported format %llu"), (unsigned long long)Format);
      check(false);
      break;
    }
  }

  void DecodePixelsByFormat(
      TArrayView<FColor> Out,
      TArrayView<uint8> PixelData,
      int32 SourcePitch,
      FIntPoint SourceExtent,
      FIntPoint DestinationExtent,
      EPixelFormat Format,
      FReadSurfaceDataFlags Flags)
  {
    SourcePitch *= GPixelFormats[Format].BlockBytes;
    auto OutPixelCount = DestinationExtent.X * DestinationExtent.Y;
    switch (Format)
    {
    case PF_G16:
    case PF_R16_UINT:
    case PF_R16_SINT:
      // Shadow maps
      ConvertRawR16DataToFColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_R8G8B8A8:
      ConvertRawR8G8B8A8DataToFColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_B8G8R8A8:
      ConvertRawB8G8R8A8DataToFColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_A2B10G10R10:
      ConvertRawR10G10B10A2DataToFColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_FloatRGBA:
    case PF_R16G16B16A16_UNORM:
    case PF_R16G16B16A16_SNORM:
      ConvertRawR16G16B16A16FDataToFColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData(), false);
      break;
    case PF_FloatR11G11B10:
      ConvertRawR11G11B10DataToFColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData(), false);
      break;
    case PF_A32B32G32R32F:
      ConvertRawR32G32B32A32DataToFColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData(), false);
      break;
    case PF_A16B16G16R16:
      ConvertRawR16G16B16A16DataToFColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_G16R16:
      ConvertRawR16G16DataToFColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_DepthStencil: // Depth Stencil
      ConvertRawD32S8DataToFColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData(), Flags);
      break;
    case PF_X24_G8: // Depth Stencil
      ConvertRawR24G8DataToFColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData(), Flags);
      break;
    case PF_R32_FLOAT: // Depth
      for (uint32 Y = 0; Y < (uint32)DestinationExtent.Y; Y++)
      {
        TArrayView<float> Row(
          (float*)(&PixelData[0] + Y * SourcePitch),
          DestinationExtent.X);
        auto DestPtr = Out.GetData() + Y * DestinationExtent.X;
        for (uint32 X = 0; X < (uint32)DestinationExtent.X; X++)
        {
          auto Depth = Row[X];
          // Depth = FMath::Min(Flags.ComputeNormalizedDepth(Row[X]), 1.0f);
          *DestPtr = FLinearColor(Depth, Depth, Depth).ToFColor(Flags.GetLinearToGamma());
          ++DestPtr;
        }
      }
      break;
    case PF_R16G16B16A16_UINT:
    case PF_R16G16B16A16_SINT:
      ConvertRawR16G16B16A16DataToFColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    case PF_G8:
      ConvertRawR8DataToFColor(DestinationExtent.X, DestinationExtent.Y, &PixelData[0], SourcePitch, Out.GetData());
      break;
    default:
      UE_LOG(LogCarla, Warning, TEXT("Unsupported format %llu"), (unsigned long long)Format);
      check(false);
      break;
    }
  }
}
