#include "ImageUtil.h"



namespace ImageUtil
{
    void ExportTexturePixels(FRHITexture2D* Texture, FIntPoint Extent, TArray<FFloat16Color>& OutPixels, FRenderCommandFence* Fence)
    {
        ENQUEUE_RENDER_COMMAND(ExportPixels_FFloat16Color)(
            [Texture, Extent, &OutPixels, Fence](FRHICommandListImmediate& RHICmdList)
            {
                auto Rect = FIntRect(0, 0, Extent.X, Extent.Y);
                RHICmdList.ReadSurfaceFloatData(Texture, Rect, OutPixels, FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));
                if (Fence != nullptr)
                    Fence->BeginFence();
            }
        );
        
        if (Fence == nullptr)
            FlushRenderingCommands();
    }

    void ExportTexturePixels(FRHITexture2D* Texture, FIntPoint Extent, TArray<FLinearColor>& OutPixels, FRenderCommandFence* Fence)
    {
        ENQUEUE_RENDER_COMMAND(ExportPixels_FLinearColor)(
            [Texture, Extent, &OutPixels, Fence](FRHICommandListImmediate& RHICmdList)
            {
                auto Rect = FIntRect(0, 0, Extent.X, Extent.Y);
                RHICmdList.ReadSurfaceData(Texture, Rect, OutPixels, FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));

                if (Fence != nullptr)
                    Fence->BeginFence();
            }
        );

        if (Fence == nullptr)
            FlushRenderingCommands();
    }

    void ExportTexturePixels(FRHITexture2D* Texture, FIntPoint Extent, TArray<FColor>& OutPixels, FRenderCommandFence* Fence)
    {
        ENQUEUE_RENDER_COMMAND(ExportPixels_FColor)(
            [Texture, Extent, &OutPixels, Fence](FRHICommandListImmediate& RHICmdList)
            {
                auto Rect = FIntRect(0, 0, Extent.X, Extent.Y);
                RHICmdList.ReadSurfaceData(Texture, Rect, OutPixels, FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));

                if (Fence != nullptr)
                    Fence->BeginFence();
            }
        );

        if (Fence == nullptr)
            FlushRenderingCommands();
    }

}