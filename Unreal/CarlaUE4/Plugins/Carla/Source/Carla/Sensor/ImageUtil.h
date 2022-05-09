#pragma once
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/ImageWriteQueue/Public/ImagePixelData.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Buffer.h>
#include <carla/sensor/SensorRegistry.h>
#include <compiler/enable-ue4-macros.h>

#include <string_view>



namespace ImageUtil
{
    void ExportTexturePixels(FRHITexture2D* Texture, FIntPoint Extent, TArray<FColor>& OutPixels, FRenderCommandFence* Fence = nullptr);
    void ExportTexturePixels(FRHITexture2D* Texture, FIntPoint Extent, TArray<FLinearColor>& OutPixels, FRenderCommandFence* Fence = nullptr);
    void ExportTexturePixels(FRHITexture2D* Texture, FIntPoint Extent, TArray<FFloat16Color>& OutPixels, FRenderCommandFence* Fence = nullptr);



    template <typename Function>
    bool ExportAndVisit(FRHITexture2D* Texture, FIntPoint Extent, Function&& Visitor, FRenderCommandFence* Fence = nullptr)
    {
        switch (Texture->GetFormat())
        {
            // Common pixel formats:
            case PF_FloatRGBA:
            {
                TArray<FFloat16Color> Pixels;
                ExportTexturePixels(Texture, Extent, Pixels, Fence);
                Visitor(MoveTemp(Pixels));
                break;
            }
            case PF_A32B32G32R32F:
            {
                TArray<FLinearColor> Pixels;
                ExportTexturePixels(Texture, Extent, Pixels, Fence);
                Visitor(MoveTemp(Pixels));
                break;
            }
            case PF_G8:
            case PF_R8G8B8A8:
            case PF_B8G8R8A8:
            case PF_A2B10G10R10:
            case PF_A16B16G16R16:
            {
                TArray<FColor> Pixels;
                ExportTexturePixels(Texture, Extent, Pixels, Fence);
                Visitor(MoveTemp(Pixels));
                break;
            }
            /* case PF_DepthStencil:
            {
                TArray<FLinearColor> Pixels;
                ExportTexturePixels(Texture, Extent, Pixels, Fence);
                Visitor(MoveTemp(Pixels));
                break;
            } */
            default:
                return false;
        }
        return true;
    }
}