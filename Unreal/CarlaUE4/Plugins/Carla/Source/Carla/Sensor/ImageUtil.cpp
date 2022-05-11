#include "ImageUtil.h"
#include "Async/Async.h"
#include "CoreMinimal.h"
#include "ImageUtils.h"
#include <thread>



namespace ImageUtil
{
    void ExportTextureDataAsync(FRHITexture2D* Texture, FIntPoint Extent, std::function<void(EPixelFormat, TArray<uint8>&&)> Callback)
    {
        ENQUEUE_RENDER_COMMAND(ExportPixels_FFloat16Color)(
            [Texture, Extent, Callback = std::move(Callback)](FRHICommandListImmediate& RHICmdList)
            {
                auto Format = Texture->GetFormat();
                auto ReadBack = new FRHIGPUTextureReadback(TEXT("ExportTextureReadBack"));
                ReadBack->EnqueueCopy(RHICmdList, Texture, FResolveRect(0, 0, Extent.X, Extent.Y));
                RHICmdList.Transition(FRHITransitionInfo(Texture, ERHIAccess::CopySrc, ERHIAccess::Present));
                AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Extent, Format, ReadBack, Callback = std::move(Callback)]
                {
                    while (!ReadBack->IsReady())
                        std::this_thread::yield();
                    auto PixelFormat = GPixelFormats[Format];
                    TArray<uint8> Buffer;
                    Buffer.AddUninitialized(Extent.X * Extent.Y * PixelFormat.BlockSizeX * PixelFormat.BlockSizeY);
                    auto Data = ReadBack->Lock(Buffer.Num());
                    FMemory::Memcpy(Buffer.GetData(), Data, Buffer.Num());
                    ReadBack->Unlock();
                    Callback(Format, MoveTemp(Buffer));
                });
            }
        );
    }

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