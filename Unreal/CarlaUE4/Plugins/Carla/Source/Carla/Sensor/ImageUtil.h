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