// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyDDSLoader.h"

#if PLATFORM_WINDOWS

enum EDDSPixelFormatExtend
{
    DDSPF_BC4 = MAKEFOURCC('B','C','4','U'),
    DDSPF_BC4S = MAKEFOURCC('B','C','4','S'),
    DDSPF_BC5 = MAKEFOURCC('B','C','5','U'),
    DDSPF_BC5S = MAKEFOURCC('B','C','5','S'),
};

FSimReadyDDSLoadHelper::FSimReadyDDSLoadHelper(const uint8* Buffer, uint32 Length) 
    : FDDSLoadHelper(Buffer, Length)
{

}

uint32 FSimReadyDDSLoadHelper::GetDepth() const
{
    return DDSHeader ? DDSHeader->dwDepth : 0;
}

uint32 FSimReadyDDSLoadHelper::GetBlockSize() const
{
    switch(DDSHeader->ddpf.dwFourCC)
    {
        case DDSPF_DXT1:
        case DDSPF_BC4S:
        case DDSPF_BC4:
            return 8;
        case DDSPF_DXT3:
        case DDSPF_DXT5:
        case DDSPF_BC5S:
        case DDSPF_BC5:
            return 16;
    };

    // Not a block image.
    return 0;
}

uint32 FSimReadyDDSLoadHelper::ComputeMipMapSize(uint32 MipMap) const
{
    uint32 SizeX = GetSizeX();
    uint32 SizeY = GetSizeY();
    uint32 Depth = GetDepth();
    
    for (uint32 MipMapIdx = 0; MipMapIdx < MipMap; ++MipMapIdx)
    {
        SizeX = FMath::Max(1U, SizeX / 2);
        SizeY = FMath::Max(1U, SizeY / 2);
        Depth = FMath::Max(1U, Depth / 2);
    }

    if ((DDSHeader->ddpf.dwFlags & DDSPF_FourCC) != 0)
    {
        SizeX = (SizeX + 3) / 4;
        SizeY = (SizeY + 3) / 4;
        return GetBlockSize() * SizeX * SizeY;
    }
    else if ((DDSHeader->ddpf.dwFlags & DDSPF_RGB) != 0)
    {		
        // Align pixels to bytes.
        uint32 ByteCount = (DDSHeader->ddpf.dwRGBBitCount + 7) / 8;
        
        // Align pitch to 4 bytes.
        uint32 pitch = 4 * ((SizeX * ByteCount + 3) / 4);
        
        return pitch * SizeY * Depth;
    }

    return 0;
}

nvtt::Format FSimReadyDDSLoadHelper::GetNVTTFormat() const
{
    nvtt::Format Format = nvtt::Format::Format_Count;

    if((DDSHeader->ddpf.dwFlags & DDSPF_FourCC) != 0)
    {
        if(DDSHeader->ddpf.dwFourCC == DDSPF_DXT1)
        {
            Format = nvtt::Format::Format_DXT1;
        }
        else if(DDSHeader->ddpf.dwFourCC == DDSPF_DXT3)
        {
            Format = nvtt::Format::Format_DXT3;
        }
        else if(DDSHeader->ddpf.dwFourCC == DDSPF_DXT5)
        {
            Format = nvtt::Format::Format_DXT5;
        }
        else if(DDSHeader->ddpf.dwFourCC == DDSPF_BC5S)
        {
            Format = nvtt::Format::Format_BC5S;
        }
        else if(DDSHeader->ddpf.dwFourCC == DDSPF_BC5)
        {
            Format = nvtt::Format::Format_BC5;
        }
        else if(DDSHeader->ddpf.dwFourCC == DDSPF_BC4)
        {
            Format = nvtt::Format::Format_BC4;
        }
        else if(DDSHeader->ddpf.dwFourCC == DDSPF_BC4S)
        {
            Format = nvtt::Format::Format_BC4S;
        }
        else if (DDSHeader->ddpf.dwFourCC == DDSPF_DX10 && DDS10Header)
        {
            switch (DDS10Header->format)
            {
                case(94): // DXGI_FORMAT_BC6H_TYPELESS
                case(95): Format = nvtt::Format::Format_BC6U; break; // DXGI_FORMAT_BC6H_UF16 
                case(96): Format = nvtt::Format::Format_BC6S; break; // DXGI_FORMAT_BC6H_SF16
                case(97): // DXGI_FORMAT_BC7_TYPELESS
                case(98): // DXGI_FORMAT_BC7_UNORM
                case(99): Format = nvtt::Format::Format_BC7; break; // DXGI_FORMAT_BC7_UNORM_SRGB
            }
        }
    }

    return Format;
}

const uint8* FSimReadyDDSLoadHelper::GetData() const
{
    const uint8* Ptr = (const uint8*)DDSHeader + sizeof(FDDSFileHeader);

    // jump over dx10 header if available
    if (DDS10Header)
    {
        Ptr += sizeof(FDDS10FileHeader);
    }

    return Ptr;
}

bool FSimReadyDDSLoadHelper::LoadRawData(TArray64<uint8>& RawData)
{
    uint32 Width = GetSizeX();
    uint32 Height = GetSizeY();
    uint64 RawSize = Width * Height * 4;
    RawData.Empty(RawSize);
    RawData.AddUninitialized(RawSize);

    nvtt::Format NvttFormat = GetNVTTFormat();
    if (NvttFormat != nvtt::Format::Format_Count)
    {
        nvtt::Surface Surface;
        if (Surface.setImage2D(NvttFormat, Width, Height, GetData()))
        {
            float* R = Surface.channel(0);
            float* G = Surface.channel(1);
            float* B = Surface.channel(2);
            float* A = Surface.channel(3);

            //(z * height() + y) * width() + x
            for (uint32 Y = 0; Y < Height; ++Y)
            {
                for (uint32 X = 0; X < Width; ++X)
                {
                    uint32 Index = Y * Width + X;				
                    if (NvttFormat == nvtt::Format::Format_BC5S || NvttFormat == nvtt::Format::Format_BC5)
                    {
                        float TX = R[Index] * 2.0f - 1.0f;
                        float TY = G[Index] * 2.0f - 1.0f;
                        float TZ = FMath::Sqrt(1.0f - (TX*TX + TY*TY));
                        float ColorB = TZ * 0.5f + 0.5f;
                        RawData[Index * 4] = (uint8)FMath::FloorToInt(ColorB * 255.999f);
                        RawData[Index * 4 + 1] = (uint8)FMath::FloorToInt(G[Index] * 255.999f);
                        RawData[Index * 4 + 2] = (uint8)FMath::FloorToInt(R[Index] * 255.999f);
                        RawData[Index * 4 + 3] = (uint8)FMath::FloorToInt(A[Index] * 255.999f);
                    }
                    else
                    {
                        RawData[Index * 4] = (uint8)FMath::FloorToInt(B[Index] * 255.999f);
                        RawData[Index * 4 + 1] = (uint8)FMath::FloorToInt(G[Index] * 255.999f);
                        RawData[Index * 4 + 2] = (uint8)FMath::FloorToInt(R[Index] * 255.999f);
                        RawData[Index * 4 + 3] = (uint8)FMath::FloorToInt(A[Index] * 255.999f);
                    }
                }
            }

            return true;
        }
    }

    return false;
}

#endif // PLATFORM_WINDOWS