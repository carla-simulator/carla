#include "GBufferStream.h"
#include "Runtime/Renderer/Public/GBufferView.h"



struct ScopedTextureLock
{
    ScopedTextureLock(FRHITexture2D *InTexture, uint32 &Stride) :
        Texture(InTexture),
        Source(reinterpret_cast<const uint8 *>(RHILockTexture2D(Texture, 0, RLM_ReadOnly, Stride, false)))
    {
    }

    ~ScopedTextureLock()
    {
        RHIUnlockTexture2D(Texture, 0, false);
    }

    FRHITexture2D *Texture; 
    const uint8 *Source;
};



static size_t GetTotalMemorySizeForTexture(FRHITexture2D* Texture)
{
    auto Extent = Texture->GetSizeXYZ();
    return Extent.X * Extent.Y * Extent.Z;
}

static size_t GetTotalMemorySizeForTextureList(FRHITexture2D** Textures, size_t Count)
{
    size_t r = 0;
    for (size_t i = 0; i != Count; ++i)
        r += GetTotalMemorySizeForTexture(Textures[i]);
    return r;
}

static FRHITexture2D* GetPooledTexture(TRefCountPtr<IPooledRenderTarget>& PRT)
{
    return PRT->GetRenderTargetItem().ShaderResourceTexture.GetReference()->GetTexture2D();
}



namespace GBuffer
{
    namespace detail
    {
        size_t FGBufferStreamBase::GetTotalMemorySize(GBufferView::FGBufferData& Contents)
        {
            FRHITexture2D* Textures[] =
            {
                GetPooledTexture(Contents.SceneColor),
                GetPooledTexture(Contents.SceneDepth),
                GetPooledTexture(Contents.GBufferA),
                GetPooledTexture(Contents.GBufferB),
                GetPooledTexture(Contents.GBufferC),
                GetPooledTexture(Contents.GBufferD),
                GetPooledTexture(Contents.GBufferE),
                GetPooledTexture(Contents.GBufferF),
                GetPooledTexture(Contents.Velocity),
                GetPooledTexture(Contents.SSAO),
                GetPooledTexture(Contents.CustomDepth),
                GetPooledTexture(Contents.CustomStencil),
            };

            const auto Count = sizeof(Textures) / sizeof(Textures[0]);

            return GetTotalMemorySizeForTextureList(Textures, Count);
        }

        carla::Buffer FGBufferStreamBase::SerializeGBuffer(GBufferView::FGBufferData& Contents)
        {
            carla::Buffer Output;

            FRHITexture2D* const Textures[] =
            {
                GetPooledTexture(Contents.SceneColor),
                GetPooledTexture(Contents.SceneDepth),
                GetPooledTexture(Contents.GBufferA),
                GetPooledTexture(Contents.GBufferB),
                GetPooledTexture(Contents.GBufferC),
                GetPooledTexture(Contents.GBufferD),
                GetPooledTexture(Contents.GBufferE),
                GetPooledTexture(Contents.GBufferF),
                GetPooledTexture(Contents.Velocity),
                GetPooledTexture(Contents.SSAO),
                GetPooledTexture(Contents.CustomDepth),
                GetPooledTexture(Contents.CustomStencil),
            };

            const auto Count = sizeof(Textures) / sizeof(Textures[0]);

            size_t Size = 0;
            size_t TextureSizes[Count];
            for (size_t i = 0; i != Count; ++i)
            {
                TextureSizes[i] = GetTotalMemorySizeForTexture(Textures[i]);
                Size += TextureSizes[i];
            }

            Output.resize(Size);

            size_t Offset = 0;
            for (size_t i = 0; i != Count; ++i)
            {
                auto Texture = Textures[i];
                auto Size = TextureSizes[i];
                uint32 Stride;
                ScopedTextureLock Lock(Texture, Stride);
                Output.copy_from(Offset, Lock.Source, Size);
                Offset += Size;
            }

            return Output;
        }
    }
}