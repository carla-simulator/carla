// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "CoreMinimal.h"



class FRHIGPUTextureReadback;

namespace ImageUtil
{
  void DecodePixelsByFormat(
      void* PixelData,
      int32 SourcePitch,
      FIntPoint SourceExtent,
      FIntPoint DestinationExtent,
      EPixelFormat Format,
      FReadSurfaceDataFlags Flags,
      TArrayView<FColor> Out);

  void DecodePixelsByFormat(
      void* PixelData,
      int32 SourcePitch,
      FIntPoint SourceExtent,
      FIntPoint DestinationExtent,
      EPixelFormat Format,
      FReadSurfaceDataFlags Flags,
      TArrayView<FLinearColor> Out);
}
