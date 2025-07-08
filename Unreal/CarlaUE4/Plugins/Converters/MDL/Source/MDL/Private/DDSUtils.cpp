// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "DDSUtils.h"
#include "HAL/Platform.h"

#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "Engine/Texture2DDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "MDLModule.h"
#include "DynamicMipsLoader.h"
#include "RenderUtils.h"

#if PLATFORM_WINDOWS
#include <DXGIFormat.h> // This is a Windows ThirdParty dependency
#endif // #if PLATFORM_WINDOWS

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)\
    ((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |\
    ((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))
#endif

enum EDDSFlags
{
    DDSF_Caps			= 0x00000001,
    DDSF_Height			= 0x00000002,
    DDSF_Width			= 0x00000004,
    DDSF_PITCH			= 0x00000008,
    DDSF_PixelFormat	= 0x00001000,
    DDSF_MipMapCount	= 0x00020000,
    DDSF_LINEARSIZE		= 0x00080000,
    DDSF_DEPTH			= 0x00800000,
};

enum EDDSCaps
{
    DDSC_Complex			= 0x8,
    DDSC_CubeMap			= 0x00000200,
    DDSC_CubeMap_AllFaces	= 0x00000400 | 0x00000800 | 0x00001000 | 0x00002000 | 0x00004000 | 0x00008000,
    DDSC_Texture			= 0x00001000,
    DDSC_Volume				= 0x00200000,
    DDSC_MipMap				= 0x00400000
};

enum EDDSPixelFormat
{
    DDSPF_AlphaPixels	= 0x00000001,
    DDSPF_Alpha			= 0x00000002,
    DDSPF_FourCC		= 0x00000004,
    DDSPF_RGB			= 0x00000040,
    DDSPF_Luminance		= 0x00020000,
    DDSPF_DXT1			= MAKEFOURCC('D','X','T','1'),
    DDSPF_DXT3			= MAKEFOURCC('D','X','T','3'),
    DDSPF_DXT5			= MAKEFOURCC('D','X','T','5'),
    DDSPF_BC4			= MAKEFOURCC('B','C','4','U'),
    DDSPF_BC5			= MAKEFOURCC('B','C','5','U'),
    DDSPF_BC6H_7		= MAKEFOURCC('D','X','1','0'), 
    DDSPF_RGBAFloat		= 0x00000071,
};

enum EDDSD3D10ResourceDimension {
    D3D10_RESOURCE_DIMENSION_UNKNOWN = 0,
    D3D10_RESOURCE_DIMENSION_BUFFER = 1,
    D3D10_RESOURCE_DIMENSION_TEXTURE1D = 2,
    D3D10_RESOURCE_DIMENSION_TEXTURE2D = 3,
    D3D10_RESOURCE_DIMENSION_TEXTURE3D = 4,
};

// .DDS subheader.
#pragma pack(push,1)
struct FDDSPixelFormatHeader
{
    uint32 dwSize;
    uint32 dwFlags;
    uint32 dwFourCC;
    uint32 dwRGBBitCount;
    uint32 dwRBitMask;
    uint32 dwGBitMask;
    uint32 dwBBitMask;
    uint32 dwABitMask;
};
#pragma pack(pop)

// .DDS header.
#pragma pack(push,1)
struct FDDSFileHeader
{
    uint32 dwSize;
    uint32 dwFlags;
    uint32 dwHeight;
    uint32 dwWidth;
    uint32 dwLinearSize;
    uint32 dwDepth;
    uint32 dwMipMapCount;
    uint32 dwReserved1[11];
    FDDSPixelFormatHeader ddpf;
    uint32 dwCaps;
    uint32 dwCaps2;
    uint32 dwCaps3;
    uint32 dwCaps4;
    uint32 dwReserved2;
};
#pragma pack(pop)

#if PLATFORM_WINDOWS
#pragma pack(push,1)
struct FDDSFileHeaderDXT10
{
    DXGI_FORMAT dxgiFormat;
    EDDSD3D10ResourceDimension  resourceDimension;
    uint32 miscFlag;
    uint32 arraySize;
    uint32 miscFlags2;
};
#pragma pack(pop)
#endif // #if PLATFORM_WINDOWS

int32 FDDSUtils::GetDDSHeaderSize()
{
    return sizeof(FDDSFileHeader) + 4;
}

bool FDDSUtils::ExportToDDS(UTexture* Tex, const FCubeFacesComposite& CompositeSetting, FArchive& Ar)
{
    if (!Tex->IsA<UTexture2D>()
        && !Tex->IsA<UTextureCube>()
        && !Tex->IsA<UTexture2DDynamic>()
        && !Tex->IsA<UTextureRenderTarget2D>())
    {
        return false;
    }

    // Virtual texture can't be exported to DDS now
    if (Tex->IsCurrentlyVirtualTextured())
    {
        return false;
    }

    auto MDLModule = FModuleManager::GetModulePtr<IMDLModule>("MDL");

    UTexture2D* Tex2D = Cast<UTexture2D>(Tex);
    UTextureCube* TexCube = Cast<UTextureCube>(Tex);
    UTexture2DDynamic* Tex2DDynamic = Cast<UTexture2DDynamic>(Tex);
    UTextureRenderTarget2D* TexRenderTarget2D = Cast<UTextureRenderTarget2D>(Tex);
    static uint32 DDSMagic = 0x20534444;

    check(Tex != nullptr);

    EPixelFormat pf = PF_Unknown;
    if (TexCube)
    {
        pf = TexCube->GetPixelFormat();
    }
    else if (Tex2D)
    {
        pf = Tex2D->GetPixelFormat();
    }
    else if (Tex2DDynamic)
    {
        pf = Tex2DDynamic->Format;
    }
    else if (TexRenderTarget2D)
    {
        pf = TexRenderTarget2D->GetFormat();
    }

    FDDSFileHeader DDSFileHeader = {};
    DDSFileHeader.dwSize = sizeof(DDSFileHeader);
    DDSFileHeader.dwFlags = DDSF_Height | DDSF_Width | DDSF_MipMapCount | DDSF_PixelFormat | DDSF_Caps;

    if (TexCube)
    {
        DDSFileHeader.dwHeight = TexCube->GetSizeY();
        DDSFileHeader.dwWidth = TexCube->GetSizeX();
        DDSFileHeader.dwMipMapCount = TexCube->GetNumMips();

        if (CompositeSetting.FaceA == INDEX_NONE)
        {
            DDSFileHeader.dwCaps2 = DDSC_CubeMap | DDSC_CubeMap_AllFaces;
            DDSFileHeader.dwCaps = DDSC_Complex | DDSC_Texture | ((DDSFileHeader.dwMipMapCount > 0) ? DDSC_MipMap : 0);
        }
        else
        {
            DDSFileHeader.dwCaps = DDSC_Texture | ((DDSFileHeader.dwMipMapCount > 0) ? DDSC_MipMap : 0) | ((DDSFileHeader.dwMipMapCount > 1) ? DDSC_Complex : 0);
        }
    }
    else if (Tex2D)
    {	
        DDSFileHeader.dwHeight = Tex2D->GetSizeY();
        DDSFileHeader.dwWidth = Tex2D->GetSizeX();
        DDSFileHeader.dwMipMapCount = Tex2D->GetNumMips();
        DDSFileHeader.dwCaps = DDSC_Texture | ((DDSFileHeader.dwMipMapCount > 0) ? DDSC_MipMap : 0) | ((DDSFileHeader.dwMipMapCount > 1) ? DDSC_Complex : 0);
    }
    else if (Tex2DDynamic)
    {
        DDSFileHeader.dwHeight = Tex2DDynamic->SizeX;
        DDSFileHeader.dwWidth = Tex2DDynamic->SizeY;
        DDSFileHeader.dwMipMapCount = Tex2DDynamic->NumMips;
        DDSFileHeader.dwCaps = DDSC_Texture | ((DDSFileHeader.dwMipMapCount > 0) ? DDSC_MipMap : 0) | ((DDSFileHeader.dwMipMapCount > 1) ? DDSC_Complex : 0);
    }
    else if (TexRenderTarget2D)
    {
        DDSFileHeader.dwHeight = TexRenderTarget2D->SizeX;
        DDSFileHeader.dwWidth = TexRenderTarget2D->SizeY;
        DDSFileHeader.dwMipMapCount = 1;
        DDSFileHeader.dwCaps = DDSC_Texture | DDSC_MipMap;
    }

    FDDSPixelFormatHeader& DDSPixelFormat = DDSFileHeader.ddpf;
    DDSPixelFormat.dwSize = sizeof(DDSPixelFormat);

    #if PLATFORM_WINDOWS
    bool DX10Header = false;
    FDDSFileHeaderDXT10 DDSFileHeaderDXT10 = {};
    #endif

    switch (pf)
    {
        // R8
        case PF_R8_UINT:
        // G8
        case PF_G8:
            DDSPixelFormat.dwFlags = DDSPF_Luminance;
            DDSPixelFormat.dwRGBBitCount = 8;
            DDSPixelFormat.dwRBitMask = 0xff;
            break;

        // RG8
        case PF_R8G8:
            DDSPixelFormat.dwFlags = DDSPF_Luminance | DDSPF_AlphaPixels;
            DDSPixelFormat.dwRGBBitCount = 16;
            DDSPixelFormat.dwRBitMask = 0x00ff;
            DDSPixelFormat.dwABitMask = 0xff00;
            break;

        // RGBA8
        case PF_R8G8B8A8:
        case PF_R8G8B8A8_UINT:
        case PF_R8G8B8A8_SNORM:
            DDSPixelFormat.dwFlags = DDSPF_RGB | DDSPF_AlphaPixels;
            DDSPixelFormat.dwRGBBitCount = 32;
            DDSPixelFormat.dwRBitMask = 0x000000ff;
            DDSPixelFormat.dwGBitMask = 0x0000ff00;
            DDSPixelFormat.dwBBitMask = 0x00ff0000;
            DDSPixelFormat.dwABitMask = 0xff000000;
            break;

        // BGRA8
        case PF_B8G8R8A8:
            DDSPixelFormat.dwFlags = DDSPF_RGB | DDSPF_AlphaPixels;
            DDSPixelFormat.dwRGBBitCount = 32;
            DDSPixelFormat.dwBBitMask = 0x000000ff;
            DDSPixelFormat.dwGBitMask = 0x0000ff00;
            DDSPixelFormat.dwRBitMask = 0x00ff0000;
            DDSPixelFormat.dwABitMask = 0xff000000;
            break;

        case PF_FloatRGBA:
            DDSPixelFormat.dwFlags = DDSPF_FourCC;
            DDSPixelFormat.dwFourCC = DDSPF_RGBAFloat;
            break;

        // DXT1 (BC1)
        case PF_DXT1:
            DDSPixelFormat.dwFlags = DDSPF_FourCC;
            DDSPixelFormat.dwFourCC = DDSPF_DXT1;
            break;

        // DXT3 (BC2)
        case PF_DXT3:
            DDSPixelFormat.dwFlags = DDSPF_FourCC;
            DDSPixelFormat.dwFourCC = DDSPF_DXT3;
            break;

        // DXT5 (BC3)
        case PF_DXT5:
            DDSPixelFormat.dwFlags = DDSPF_FourCC;
            DDSPixelFormat.dwFourCC = DDSPF_DXT5;
            break;

        case PF_BC5:
            DDSPixelFormat.dwFlags = DDSPF_FourCC;
            DDSPixelFormat.dwFourCC = DDSPF_BC5;
            break;

        case PF_BC4:
            DDSPixelFormat.dwFlags = DDSPF_FourCC;
            DDSPixelFormat.dwFourCC = DDSPF_BC4;
            break;

        case PF_BC6H:

#if PLATFORM_WINDOWS
        case PF_BC7:
            DDSPixelFormat.dwFlags = DDSPF_FourCC;
            DDSPixelFormat.dwFourCC = DDSPF_BC6H_7;
            DX10Header = true;
            if (pf == PF_BC7)
            {
                DDSFileHeaderDXT10.dxgiFormat = Tex->SRGB ? DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
            }
            else
            {
                DDSFileHeaderDXT10.dxgiFormat = DXGI_FORMAT_BC6H_UF16;
            }
            DDSFileHeaderDXT10.resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
            DDSFileHeaderDXT10.arraySize = 1; // NOTE: cube-map should also be 1, not 6
            if (TexCube && CompositeSetting.FaceA == INDEX_NONE)
            {	
                DDSFileHeaderDXT10.miscFlag = 4;
            }
            break;
#endif // #if PLATFORM_WINDOWS

        default:
            /* not supported */
            return false;
    }

    // export header
    Ar.Serialize(&DDSMagic, sizeof(DDSMagic));
    Ar.Serialize(&DDSFileHeader, sizeof(DDSFileHeader));

#if PLATFORM_WINDOWS
    if (DX10Header)
    {
        Ar.Serialize(&DDSFileHeaderDXT10, sizeof(DDSFileHeaderDXT10));
    }
#endif

    // copy the mipmap data
    TArray<uint8*> RawData;
    RawData.AddZeroed(DDSFileHeader.dwMipMapCount);

    if (TexCube)
    {
        TexCube->GetMipData(0, (void**)RawData.GetData());
    }
    else if (Tex2D)
    {
        Tex2D->GetMipData(0, (void**)RawData.GetData());
    }
    else if (Tex2DDynamic && MDLModule->GetDynamicMipsLoader().IsValid())
    {
        MDLModule->GetDynamicMipsLoader()->GetMipData(Tex2DDynamic, 0, (void**)RawData.GetData());
    }
    else if (TexRenderTarget2D)
    {
        uint32 RawSize = CalcTextureMipMapSize(DDSFileHeader.dwWidth, DDSFileHeader.dwHeight, pf, 0);
        RawData[0] = (uint8*)FMemory::Malloc(RawSize);
        FRenderTarget* RenderTarget = TexRenderTarget2D->GameThread_GetRenderTargetResource();
        bool bReadSuccess = false;
        switch (pf)
        {
        case PF_FloatRGBA:
        {
            TArray<FFloat16Color> FloatColors;
            bReadSuccess = RenderTarget->ReadFloat16Pixels(FloatColors);
            FMemory::Memcpy(RawData[0], FloatColors.GetData(), RawSize);
        }
        break;
        case PF_B8G8R8A8:
            bReadSuccess = RenderTarget->ReadPixelsPtr((FColor*)RawData[0]);
            break;
        }
        if (!bReadSuccess)
        {
            FMemory::Free(RawData[0]);
            return false;
        }
    }

    if (TexCube)
    {
        if (CompositeSetting.FaceA != INDEX_NONE)
        {
            for (uint32 MipMapIndex = 0; MipMapIndex < DDSFileHeader.dwMipMapCount; ++MipMapIndex)
            {
                uint8* FaceA = nullptr;
                uint8* FaceB = nullptr;
                int64 RawSize = CalcTextureMipMapSize(DDSFileHeader.dwWidth, DDSFileHeader.dwHeight, pf, MipMapIndex);
                if (CompositeSetting.FaceA < 6)
                {
                    FaceA = RawData[MipMapIndex] + RawSize * CompositeSetting.FaceA;
                }
                if (CompositeSetting.FaceB != INDEX_NONE && CompositeSetting.FaceB < 6)
                {
                    FaceB = RawData[MipMapIndex] + RawSize * CompositeSetting.FaceB;
                }

                uint8* MipData = (uint8*)FMemory::Malloc(RawSize);

                if (!FaceB)
                {
                    FMemory::Memcpy(MipData, FaceA, RawSize);
                }
                else
                {
                    int64 BlockY = CalcTextureMipHeightInBlocks(DDSFileHeader.dwHeight, pf, MipMapIndex);
                    uint32 WidthB = (1.0f - CompositeSetting.Ratio) * DDSFileHeader.dwWidth;
                    int64 WidthSize = CalcTextureMipMapSize(DDSFileHeader.dwWidth, 1, pf, MipMapIndex);
                    int64 WidthBSize = CalcTextureMipMapSize(WidthB, 1, pf, MipMapIndex);
                    int64 WidthASize = WidthSize - WidthBSize;
                    int64 Offset = 0;

                    for (uint32 BlockIndex = 0; BlockIndex < BlockY; BlockIndex++)
                    {
                        FMemory::Memcpy(MipData + Offset, FaceB + Offset + WidthASize, WidthBSize);
                        FMemory::Memcpy(MipData + Offset + WidthBSize, FaceA + Offset, WidthASize);
                        Offset += WidthSize;						
                    }
                }

                Ar.Serialize(MipData, RawSize);
                FMemory::Free(MipData);
            }
        }
        else
        {
            for (int32 SliceIndex = 0; SliceIndex < 6; ++SliceIndex)
            {
                for (uint32 MipMapIndex = 0; MipMapIndex < DDSFileHeader.dwMipMapCount; ++MipMapIndex)
                {
                    if (RawData[MipMapIndex])
                    {
                        int64 RawSize = CalcTextureMipMapSize(DDSFileHeader.dwWidth, DDSFileHeader.dwHeight, pf, MipMapIndex);
                        Ar.Serialize(RawData[MipMapIndex] + RawSize * SliceIndex, RawSize);
                    }
                }
            }
        }
    }
    else
    {
        // export mipmaps
        for (uint32 MipMapIndex = 0; MipMapIndex < DDSFileHeader.dwMipMapCount; ++MipMapIndex)
        {
            if (RawData[MipMapIndex])
            {
                int64 RawSize = CalcTextureMipMapSize(DDSFileHeader.dwWidth, DDSFileHeader.dwHeight, pf, MipMapIndex);
                Ar.Serialize(RawData[MipMapIndex], RawSize);
            }
        }
    }

    // deallocate the mipmap data
    for (uint32 MipMapIndex = 0; MipMapIndex < DDSFileHeader.dwMipMapCount; ++MipMapIndex)
    {
        FMemory::Free(RawData[MipMapIndex]);
    }

    return true;
}
