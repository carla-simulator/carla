#include "GBufferView.h"
#include "Runtime/Renderer/Public/GBufferView.h"



namespace GBuffer
{
    static size_t GetTotalMemorySizeForTexture(FRHITexture* Texture)
    {
        auto Extent = Texture->GetSizeXYZ();
        return Extent.X * Extent.Y * Extent.Z;
    }

    size_t GBufferContents::GetTotalMemorySize(GBufferView::FGBufferData& Contents)
    {
        FRHITexture* Textures[] =
        {
            Contents.SceneColor->GetRenderTargetItem().ShaderResourceTexture.GetReference(),
            Contents.SceneDepth->GetRenderTargetItem().ShaderResourceTexture.GetReference(),
            Contents.GBufferA->GetRenderTargetItem().ShaderResourceTexture.GetReference(),
            Contents.GBufferB->GetRenderTargetItem().ShaderResourceTexture.GetReference(),
            Contents.GBufferC->GetRenderTargetItem().ShaderResourceTexture.GetReference(),
            Contents.GBufferD->GetRenderTargetItem().ShaderResourceTexture.GetReference(),
            Contents.GBufferE->GetRenderTargetItem().ShaderResourceTexture.GetReference(),
            Contents.GBufferF->GetRenderTargetItem().ShaderResourceTexture.GetReference(),
            Contents.Velocity->GetRenderTargetItem().ShaderResourceTexture.GetReference(),
            Contents.SSAO->GetRenderTargetItem().ShaderResourceTexture.GetReference(),
            Contents.CustomDepth->GetRenderTargetItem().ShaderResourceTexture.GetReference(),
            Contents.CustomStencil->GetRenderTargetItem().ShaderResourceTexture.GetReference(),
        };

        size_t r = 0;
        for (auto e : Textures)
            r += GetTotalMemorySizeForTexture(e);
        return r;
    }

    void GBufferContents::SerializeGBuffer(GBufferView::FGBufferData& Contents)
    {
    }
}