#pragma once
#include "CoreMinimal.h"



class FRHIGPUTextureReadback;

namespace ImageUtil
{
    TArray<FColor> ExtractTexturePixelsFromReadback(
        FRHIGPUTextureReadback* Readback,
        FIntPoint SourceExtent,
        FIntPoint DestinationExtent,
        EPixelFormat Format);
}