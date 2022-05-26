#pragma once
#include "CoreMinimal.h"



class FRHIGPUTextureReadback;

namespace ImageUtil
{
    TArray<FColor> ExtractTexturePixelsFromReadback(
        FRHICommandListImmediate& RHICommandList,
        FRHIGPUTextureReadback* Readback,
        FIntPoint SourceExtent,
        FIntPoint DestinationExtent,
        EPixelFormat Format);
    
    inline TArray<FColor> ExtractTexturePixelsFromReadback(
        FRHIGPUTextureReadback* Readback,
        FIntPoint SourceExtent,
        FIntPoint DestinationExtent,
        EPixelFormat Format)
    {
        return ExtractTexturePixelsFromReadback(
            FRHICommandListExecutor::GetImmediateCommandList(),
            Readback, SourceExtent, DestinationExtent, Format);
    }
}