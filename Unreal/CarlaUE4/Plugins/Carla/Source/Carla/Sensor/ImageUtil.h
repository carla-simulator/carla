// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
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
      TArrayView<FColor> Out,
      TArrayView<uint8> PixelData,
      int32 SourcePitch,
      FIntPoint SourceExtent,
      FIntPoint DestinationExtent,
      EPixelFormat Format,
      FReadSurfaceDataFlags Flags);

  void DecodePixelsByFormat(
      TArrayView<FLinearColor> Out,
      TArrayView<uint8> PixelData,
      int32 SourcePitch,
      FIntPoint SourceExtent,
      FIntPoint DestinationExtent,
      EPixelFormat Format,
      FReadSurfaceDataFlags Flags);
}
