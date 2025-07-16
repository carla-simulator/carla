// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyAssetExportHelper.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/ScopedSlowTask.h"
#include "EditorFramework/AssetImportData.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "Engine/Texture2DDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "DDSUtils.h"
#include "Serialization/BufferArchive.h"
#include "SimReadyRuntimeModule.h"
#include "SimReadyPathHelper.h"
#include "SimReadySettings.h"
#include "SimReadySlowTask.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "MDLExporterUtility.h"
#include "MDLImporterUtility.h"
#include "SimReadyMessageLogContext.h"
#include "TextureCompressorModule.h"
#include "ImageCore.h"
#include "SimReadyRuntimePrivate.h"
#include "MDLModule.h"
#include "DynamicMipsLoader.h"
#include "HAL/FileManager.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Misc/FileHelper.h"

static void GetTextureBuildSettings(const UTexture& Texture, FTextureBuildSettings& TextureBuildSettings)
{
    // Texture adjustments
    TextureBuildSettings.ColorAdjustment.AdjustBrightness = Texture.AdjustBrightness;
    TextureBuildSettings.ColorAdjustment.AdjustBrightnessCurve = Texture.AdjustBrightnessCurve;
    TextureBuildSettings.ColorAdjustment.AdjustVibrance = Texture.AdjustVibrance;
    TextureBuildSettings.ColorAdjustment.AdjustSaturation = Texture.AdjustSaturation;
    TextureBuildSettings.ColorAdjustment.AdjustRGBCurve = Texture.AdjustRGBCurve;
    TextureBuildSettings.ColorAdjustment.AdjustHue = Texture.AdjustHue;
    TextureBuildSettings.ColorAdjustment.AdjustMinAlpha = Texture.AdjustMinAlpha;
    TextureBuildSettings.ColorAdjustment.AdjustMaxAlpha = Texture.AdjustMaxAlpha;

    TextureBuildSettings.bChromaKeyTexture = Texture.bChromaKeyTexture;
    TextureBuildSettings.ChromaKeyColor = Texture.ChromaKeyColor;
    TextureBuildSettings.ChromaKeyThreshold = Texture.ChromaKeyThreshold;
    TextureBuildSettings.bHDRSource = Texture.HasHDRSource();

    // NOTE: just initialize the parameter was useful for mipmap generation.
    TextureBuildSettings.bVolume = false; // we don't export volume texture;
    TextureBuildSettings.SharpenMipKernelSize = 2;
    TextureBuildSettings.MipSharpening = 0;
    TextureBuildSettings.bPreserveBorder = Texture.bPreserveBorder;
    TextureBuildSettings.bDitherMipMapAlpha = Texture.bDitherMipMapAlpha;
    TextureBuildSettings.AlphaCoverageThresholds = Texture.AlphaCoverageThresholds;
    TextureBuildSettings.bDownsampleWithAverage = true;
    TextureBuildSettings.bSharpenWithoutColorShift = !Texture.IsNormalMap();
    TextureBuildSettings.bBorderColorBlack = false;

    FTextureFormatSettings FormatSettings;
    Texture.GetLayerFormatSettings(0, FormatSettings);

    TextureBuildSettings.bSRGB = false;//FormatSettings.SRGB;
    TextureBuildSettings.bApplyYCoCgBlockScale = FormatSettings.CompressionYCoCg;

    if (FormatSettings.CompressionSettings == TC_Displacementmap || FormatSettings.CompressionSettings == TC_DistanceFieldFont)
    {
        TextureBuildSettings.bReplicateAlpha = true;
    }
    else if (FormatSettings.CompressionSettings == TC_Grayscale || FormatSettings.CompressionSettings == TC_Alpha)
    {
        TextureBuildSettings.bReplicateRed = true;
    }
}

/**
 * Flips the contents of the green channel.
 * @param InOutMipChain - The mip chain on which the green channel shall be flipped.
 */
static void FlipGreenChannel( FImage& Image )
{
    FLinearColor* FirstColor = Image.AsRGBA32F().GetData();
    FLinearColor* LastColor = FirstColor + (Image.SizeX * Image.SizeY * Image.NumSlices);
    for ( FLinearColor* Color = FirstColor; Color < LastColor; ++Color )
    {
        Color->G = 1.0f - FMath::Clamp(Color->G, 0.0f, 1.0f);
    }
}

/**
 * Replicates the contents of the red channel to the green, blue, and alpha channels.
 */
static void ReplicateRedChannel(TArray<FImage>& InOutMipChain)
{
    const uint32 MipCount = InOutMipChain.Num();
    for (uint32 MipIndex = 0; MipIndex < MipCount; ++MipIndex)
    {
        FImage& SrcMip = InOutMipChain[MipIndex];
        FLinearColor* FirstColor = SrcMip.AsRGBA32F().GetData();
        FLinearColor* LastColor = FirstColor + (SrcMip.SizeX * SrcMip.SizeY * SrcMip.NumSlices);
        for (FLinearColor* Color = FirstColor; Color < LastColor; ++Color)
        {
            *Color = FLinearColor(Color->R, Color->R, Color->R, Color->R);
        }
    }
}

/**
 * Replicates the contents of the alpha channel to the red, green, and blue channels.
 */
static void ReplicateAlphaChannel(TArray<FImage>& InOutMipChain)
{
    const uint32 MipCount = InOutMipChain.Num();
    for (uint32 MipIndex = 0; MipIndex < MipCount; ++MipIndex)
    {
        FImage& SrcMip = InOutMipChain[MipIndex];
        FLinearColor* FirstColor = SrcMip.AsRGBA32F().GetData();
        FLinearColor* LastColor = FirstColor + (SrcMip.SizeX * SrcMip.SizeY * SrcMip.NumSlices);
        for (FLinearColor* Color = FirstColor; Color < LastColor; ++Color)
        {
            *Color = FLinearColor(Color->A, Color->A, Color->A, Color->A);
        }
    }
}

/** Calculate a scale per 4x4 block of each image, and apply it to the red/green channels. Store scale in the blue channel. */
static void ApplyYCoCgBlockScale(TArray<FImage>& InOutMipChain)
{
    const uint32 MipCount = InOutMipChain.Num();
    for (uint32 MipIndex = 0; MipIndex < MipCount; ++MipIndex)
    {
        FImage& SrcMip = InOutMipChain[MipIndex];
        FLinearColor* FirstColor = SrcMip.AsRGBA32F().GetData();

        int32 BlockWidthX = SrcMip.SizeX / 4;
        int32 BlockWidthY = SrcMip.SizeY / 4;

        for (int32 Slice = 0; Slice < SrcMip.NumSlices; ++Slice)
        {
            FLinearColor* SliceFirstColor = FirstColor + (SrcMip.SizeX * SrcMip.SizeY * Slice);

            for (int32 Y = 0; Y < BlockWidthY; ++Y)
            {
                FLinearColor* RowFirstColor = SliceFirstColor + (Y * 4 * SrcMip.SizeY);

                for (int32 X = 0; X < BlockWidthX; ++X)
                {
                    FLinearColor* BlockFirstColor = RowFirstColor + (X * 4);

                    // Iterate block to find MaxComponent
                    float MaxComponent = 0.f;
                    for (int32 BlockY = 0; BlockY < 4; ++BlockY)
                    {
                        FLinearColor* Color = BlockFirstColor + (BlockY * SrcMip.SizeY);
                        for (int32 BlockX = 0; BlockX < 4; ++BlockX, ++Color)
                        {
                            MaxComponent = FMath::Max(FMath::Abs(Color->R - 128.f / 255.f), MaxComponent);
                            MaxComponent = FMath::Max(FMath::Abs(Color->G - 128.f / 255.f), MaxComponent);
                        }
                    }

                    const float Scale = (MaxComponent < 32.f / 255.f) ? 4.f : (MaxComponent < 64.f / 255.f) ? 2.f : 1.f;
                    const float OutB = (Scale - 1.f) * 8.f / 255.f;

                    // Iterate block to modify for scale
                    for (int32 BlockY = 0; BlockY < 4; ++BlockY)
                    {
                        FLinearColor* Color = BlockFirstColor + (BlockY * SrcMip.SizeY);
                        for (int32 BlockX = 0; BlockX < 4; ++BlockX, ++Color)
                        {
                            const float OutR = (Color->R - 128.f / 255.f) * Scale + 128.f / 255.f;
                            const float OutG = (Color->G - 128.f / 255.f) * Scale + 128.f / 255.f;

                            *Color = FLinearColor(OutR, OutG, OutB, Color->A);
                        }
                    }
                }
            }
        }
    }
}

// For converting color map to normal map
static void RepackNormalMap( FImage& Image )
{
    FLinearColor* FirstColor = Image.AsRGBA32F().GetData();
    FLinearColor* LastColor = FirstColor + (Image.SizeX * Image.SizeY * Image.NumSlices);
    for ( FLinearColor* Color = FirstColor; Color < LastColor; ++Color )
    {
        float X = Color->R * 2.0f - 1.0f;
        float Y = Color->G * 2.0f - 1.0f;
        float Z = FMath::Sqrt(1 - (X*X + Y*Y));
        Color->R = X * 0.5f + 0.5f;
        Color->G = Y * 0.5f + 0.5f;
        Color->B = Z * 0.5f + 0.5f;
        Color->A = 1.0f;
    }

}

bool FileExists(const FString& InExportPath)
{
    return FPaths::FileExists(InExportPath);
}

bool SaveToFile(const FString& InExportPath, const FString& Data)
{
    if (Data.Len() <= 0)
    {
        return false;
    }

    UE_LOG(LogSimReady, Display, TEXT("Exporting %s"), *InExportPath);

    return FFileHelper::SaveStringToFile(Data, *InExportPath, FFileHelper::EEncodingOptions::ForceUTF8);
}

bool SaveToFile(const FString& InExportPath, TArrayView<const uint8> ArrayData)
{
    if (ArrayData.Num() <= 0)
    {
        return false;
    }

    UE_LOG(LogSimReady, Display, TEXT("Exporting %s"), *InExportPath);

    return FFileHelper::SaveArrayToFile(ArrayData, *InExportPath);
}

bool SaveToFile(const FString& InExportPath, const TArray64<uint8>& ArrayData)
{
    if (ArrayData.Num() <= 0)
    {
        return false;
    }

    UE_LOG(LogSimReady, Display, TEXT("Exporting %s"), *InExportPath);

    return FFileHelper::SaveArrayToFile(ArrayData, *InExportPath);
}

static bool GetRenderTarget2DRawData(UTextureRenderTarget2D* TexRT, TArray64<uint8>& RawData)
{
    FRenderTarget* RenderTarget = TexRT->GameThread_GetRenderTargetResource();
    EPixelFormat Format = TexRT->GetFormat();

    int32 ImageBytes = CalculateImageBytes(TexRT->SizeX, TexRT->SizeY, 0, Format);
    RawData.AddUninitialized(ImageBytes);
    bool bReadSuccess = false;
    switch (Format)
    {
    case PF_FloatRGBA:
    {
        TArray<FFloat16Color> FloatColors;
        bReadSuccess = RenderTarget->ReadFloat16Pixels(FloatColors);
        FMemory::Memcpy(RawData.GetData(), FloatColors.GetData(), ImageBytes);
    }
    break;
    case PF_B8G8R8A8:
        bReadSuccess = RenderTarget->ReadPixelsPtr((FColor*)RawData.GetData());
        break;
    }
    if (bReadSuccess == false)
    {
        RawData.Empty();
    }
    return bReadSuccess;
}

bool FSimReadyAssetExportHelper::ExportRawDataToPath(const void* Data, int32 DataSize, const FString& InPath, const int32 Width, const int32 Height, const ERGBFormat Format, const int32 BitDepth)
{
    bool bSuccess = false;
    
    if (FSimReadySlowTask::Get().IsProgressCancelled())
    {
        return bSuccess;
    }

    FString ExportPath = InPath;
    if (DataSize > 0)
    {
        IImageWrapperModule* ImageWrapperModule = FModuleManager::GetModulePtr<IImageWrapperModule>(FName("ImageWrapper"));

        if (ImageWrapperModule != nullptr)
        {
            TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule->CreateImageWrapper(EImageFormat::PNG);

            if (ImageWrapper->SetRaw(Data, DataSize, Width, Height, Format, BitDepth))
            {
                // There's an Omniverse function (omniReferenceContent) buried in CreateSync that only supports 32-bit sizes
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 24
                const TArray<uint8>& TempData = ImageWrapper->GetCompressed();
#else
                const TArray64<uint8>& TempData = ImageWrapper->GetCompressed();
#endif							
                bSuccess = SaveToFile(ExportPath, TempData);
            }
        }
    }

    return bSuccess;
}

bool ShouldBuildTexture(const FTextureBuildSettings& TextureBuildSettings, const UTexture* InTexture)
{
    const FTextureBuildSettings DefaultTextureBuildSettings;
    
    if (TextureBuildSettings.ColorAdjustment.AdjustBrightness != DefaultTextureBuildSettings.ColorAdjustment.AdjustBrightness
    || TextureBuildSettings.ColorAdjustment.AdjustBrightnessCurve != DefaultTextureBuildSettings.ColorAdjustment.AdjustBrightnessCurve
    || TextureBuildSettings.ColorAdjustment.AdjustVibrance != DefaultTextureBuildSettings.ColorAdjustment.AdjustVibrance
    || TextureBuildSettings.ColorAdjustment.AdjustSaturation != DefaultTextureBuildSettings.ColorAdjustment.AdjustSaturation
    || TextureBuildSettings.ColorAdjustment.AdjustRGBCurve != DefaultTextureBuildSettings.ColorAdjustment.AdjustRGBCurve
    || TextureBuildSettings.ColorAdjustment.AdjustHue != DefaultTextureBuildSettings.ColorAdjustment.AdjustHue
    || TextureBuildSettings.ColorAdjustment.AdjustMinAlpha != DefaultTextureBuildSettings.ColorAdjustment.AdjustMinAlpha
    || TextureBuildSettings.ColorAdjustment.AdjustMaxAlpha != DefaultTextureBuildSettings.ColorAdjustment.AdjustMaxAlpha
    || TextureBuildSettings.bChromaKeyTexture != DefaultTextureBuildSettings.bChromaKeyTexture
    || TextureBuildSettings.ChromaKeyColor != DefaultTextureBuildSettings.ChromaKeyColor
    || TextureBuildSettings.ChromaKeyThreshold != DefaultTextureBuildSettings.ChromaKeyThreshold
    || TextureBuildSettings.bHDRSource != DefaultTextureBuildSettings.bHDRSource
    || TextureBuildSettings.bPreserveBorder != DefaultTextureBuildSettings.bPreserveBorder
    || TextureBuildSettings.bDitherMipMapAlpha != DefaultTextureBuildSettings.bDitherMipMapAlpha
    || TextureBuildSettings.bApplyYCoCgBlockScale != DefaultTextureBuildSettings.bApplyYCoCgBlockScale
    || TextureBuildSettings.bReplicateAlpha != DefaultTextureBuildSettings.bReplicateAlpha
    || TextureBuildSettings.bReplicateRed != DefaultTextureBuildSettings.bReplicateRed)
    {
        return true;
    }

    // normalmap should always rebuild
    if (InTexture->CompressionSettings == TC_Normalmap)
    {
        return true;
    }

    if (InTexture->bFlipGreenChannel)
    {
        return true;
    }

    int32 MaxSourceSize = FMath::Max(InTexture->Source.GetSizeX(), InTexture->Source.GetSizeY());
    if (InTexture->MaxTextureSize != 0 && InTexture->MaxTextureSize < MaxSourceSize)
    {
        return true;
    }

    return false;
}

bool FSimReadyAssetExportHelper::ExportTextureToPath(UTexture* InTexture, const FString& InPath, const FSimReadyExportTextureSettings& TextureSettings)
{
    bool bSuccess = false;

    if (FSimReadySlowTask::Get().IsProgressCancelled())
    {
        return bSuccess;
    }

    FString NewPath, NewFile, Ext;
    FPaths::Split(InPath, NewPath, NewFile, Ext);

    //TODO: Virtual Texture to DDS
    if (TextureSettings.bDDSExport && InTexture->IsCurrentlyVirtualTextured())
    {
        UE_LOG(LogSimReady, Warning, TEXT("%s is virtual texture, exporting to DDS isn't supported yet, replacing with PNG."), *InTexture->GetName());
    }

    bool bDDSOnly = TextureSettings.bDDSExport && !InTexture->IsCurrentlyVirtualTextured();
    
    if (bDDSOnly || InTexture->IsA<UTextureCube>())
    {
        FString ExportPath = NewPath / NewFile + TEXT(".dds");

        FBufferArchive Ar;
        FDDSUtils::ExportToDDS(InTexture, {}, Ar);
        bSuccess = SaveToFile(ExportPath, Ar);

        if (TextureSettings.bReplaceTextureCube && InTexture->IsA<UTextureCube>())
        {
            FString LayerArExportPath = NewPath / NewFile + TEXT("_f.dds");
            FBufferArchive LayerAr;
            FDDSUtils::ExportToDDS(InTexture, { 0, 4, 0.3f }, LayerAr);
            bSuccess = SaveToFile(LayerArExportPath, LayerAr);
        }
    }
    
    if (bDDSOnly)
    {
        return bSuccess;
    }

    struct FMipBlocks
    {
        TArray64<uint8> Data;
        int32 BlockX = 0;
        int32 BlockY = 0;
        int32 SizeX = 0;
        int32 SizeY = 0;
    };

    TArray<FMipBlocks> OutMipBlocks;
    ETextureSourceFormat Format = TSF_Invalid;
    EPixelFormat PixelFormat = PF_Unknown;
    bool bGray16 = false;
    bool bUDIM = false;
    if (InTexture->IsA<UTexture2D>())
    {
        // Get source mipmap data.
        auto Texture = Cast<UTexture2D>(InTexture);
        bUDIM = Texture->IsCurrentlyVirtualTextured();
        Format = Texture->Source.GetFormat();
        bGray16 = (Format == TSF_G16);

        for (int32 Block = 0; Block < Texture->Source.GetNumBlocks(); ++Block)
        {
            FMipBlocks OutBlockData;
            Texture->Source.GetMipData(OutBlockData.Data, Block, 0, 0);
            FTextureSourceBlock SourceBlock;
            Texture->Source.GetBlock(Block, SourceBlock);
            OutBlockData.BlockX = SourceBlock.BlockX; // Only UDIM use
            OutBlockData.BlockY = SourceBlock.BlockY; // Only UDIM use
            OutBlockData.SizeX = SourceBlock.SizeX;
            OutBlockData.SizeY = SourceBlock.SizeY;
            if (OutBlockData.Data.Num() > 0)
            {
                bGray16 |= (Format == TSF_RGBA16) && (OutBlockData.Data.Num() == (SourceBlock.SizeX * SourceBlock.SizeY * 2));
                OutMipBlocks.Add(OutBlockData);
            }

            if (!bUDIM)
            {
                break;
            }
        }
    }
    else if (InTexture->IsA<UTexture2DDynamic>())
    {
        auto MDLModule = FModuleManager::GetModulePtr<IMDLModule>("MDL");

        if (MDLModule->GetDynamicMipsLoader().IsValid())
        {
            auto DynamicTexture = Cast<UTexture2DDynamic>(InTexture);
            FMipBlocks OutBlockData;
            if (!MDLModule->GetDynamicMipsLoader()->GetUncompressedMipData(DynamicTexture, 0, OutBlockData.Data, Format))
            {
                return bSuccess;
            }

            bGray16 = (Format == TSF_G16);
            if (OutBlockData.Data.Num() > 0)
            {
                OutBlockData.SizeX = DynamicTexture->SizeX;
                OutBlockData.SizeY = DynamicTexture->SizeY;
                bGray16 |= (Format == TSF_RGBA16) && (OutBlockData.Data.Num() == (DynamicTexture->SizeX * DynamicTexture->SizeY * 2));
                OutMipBlocks.Add(OutBlockData);
            }
        }		
    }
    else if (InTexture->IsA<UTextureRenderTarget2D>())
    {
        auto RenderTarget = Cast<UTextureRenderTarget2D>(InTexture);
        PixelFormat = RenderTarget->GetFormat();
        FMipBlocks OutBlockData;
        GetRenderTarget2DRawData(RenderTarget, OutBlockData.Data);
        if (OutBlockData.Data.Num() > 0)
        {
            OutBlockData.SizeX = RenderTarget->GetSurfaceWidth();
            OutBlockData.SizeY = RenderTarget->GetSurfaceHeight();
            OutMipBlocks.Add(OutBlockData);
        }
    }
    else
    {
        return bSuccess; // Texture Cube was exported as DDS above.
    }

    IImageWrapperModule* ImageWrapperModule = FModuleManager::GetModulePtr<IImageWrapperModule>(FName("ImageWrapper"));

    if (ImageWrapperModule != nullptr)
    {
        FString ExportPath;
        for (int32 BlockIndex = 0; BlockIndex < OutMipBlocks.Num(); ++BlockIndex)
        {
            auto OutMipData = OutMipBlocks[BlockIndex].Data;
            int32 Width = OutMipBlocks[BlockIndex].SizeX;
            int32 Height = OutMipBlocks[BlockIndex].SizeY;

            FTextureBuildSettings TextureBuildSettings;
            GetTextureBuildSettings(*InTexture, TextureBuildSettings);

            if (InTexture->IsA<UTexture2D>() && !TextureSettings.bTextureSource
            && ShouldBuildTexture(TextureBuildSettings, InTexture)) // apply texture settings, including adjustments and texture maximum size
            {
                auto Texture = Cast<UTexture2D>(InTexture);
                // check texture maximum size
                int32 MaxSourceSize = FMath::Max(Texture->Source.GetSizeX(), Texture->Source.GetSizeY());
                {
                    ERawImageFormat::Type ImageFormat;
                    switch (Texture->Source.GetFormat())
                    {
                    case TSF_G8:		ImageFormat = ERawImageFormat::G8;		break;
                    case TSF_G16:		ImageFormat = ERawImageFormat::G16;		break;
                    case TSF_BGRA8:		ImageFormat = ERawImageFormat::BGRA8;	break;
                    case TSF_BGRE8:		ImageFormat = ERawImageFormat::BGRE8;	break;
                    case TSF_RGBA16:	ImageFormat = ERawImageFormat::RGBA16;	break;
                    case TSF_RGBA16F:	ImageFormat = ERawImageFormat::RGBA16F; break;
                    default: 
                        return bSuccess;
                    }
                    //EGammaSpace GammaSpace = Texture->SRGB ? (Texture->bUseLegacyGamma ? EGammaSpace::Pow22 : EGammaSpace::sRGB) : EGammaSpace::Linear;
                    EGammaSpace GammaSpace = EGammaSpace::Linear;

                    FImage BaseImage(Width, Height, ImageFormat, GammaSpace);
                    BaseImage.RawData = OutMipData;
                    FImage SourceImage(1, 1, ERawImageFormat::RGBA32F);
                    BaseImage.CopyTo(SourceImage, ERawImageFormat::RGBA32F, EGammaSpace::Linear);
                    
                    ITextureCompressorModule::AdjustImageColors(SourceImage, TextureBuildSettings);

                    if (Texture->CompressionSettings == TC_Normalmap)
                    {
                        RepackNormalMap(SourceImage);
                    }

                    if (Texture->bFlipGreenChannel)
                    {
                        FlipGreenChannel(SourceImage);
                    }

                    TArray<FImage> OutMipChain;
                    int32 OutMipDepth = 0;
                    if (Texture->MaxTextureSize != 0 && Texture->MaxTextureSize < MaxSourceSize)
                    {
                        int32 MipDepth = FMath::CeilLogTwo(MaxSourceSize / Texture->MaxTextureSize);
                        check(MipDepth != 0);

                        ITextureCompressorModule::GenerateMipChain(TextureBuildSettings, SourceImage, OutMipChain, MipDepth);
                        OutMipDepth = MipDepth - 1;
                    }
                    else
                    {
                        OutMipChain.Add(SourceImage);
                    }

                    if (TextureBuildSettings.bReplicateRed)
                    {
                        ReplicateRedChannel(OutMipChain);
                    }
                    else if (TextureBuildSettings.bReplicateAlpha)
                    {
                        ReplicateAlphaChannel(OutMipChain);
                    }
                    if (TextureBuildSettings.bApplyYCoCgBlockScale)
                    {
                        ApplyYCoCgBlockScale(OutMipChain);
                    }

#if 0
                    // NOTE: Workaround for OM-30912
                    // Exporting 8-bit PNG normalmap with higher precision
                    if (Texture->CompressionSettings == TC_Normalmap 
                        && ImageFormat != ERawImageFormat::RGBA16
                        && ImageFormat != ERawImageFormat::RGBA16F)
                    {
                        ImageFormat = ERawImageFormat::RGBA16;
                        Format = TSF_RGBA16;
                    }
#endif

                    OutMipChain[OutMipDepth].CopyTo(BaseImage, ImageFormat, GammaSpace);
                    OutMipData = BaseImage.RawData;
                    Width = BaseImage.SizeX;
                    Height = BaseImage.SizeY;
                }
            }

            auto GetUDIM = [](int32 BlockX, int32 BlockY) -> FString
            {
                FString UDIM = TEXT(".");	
                int32 Number = 1001 + BlockX % 10 + BlockY * 10;
                UDIM += FString::FromInt(Number);
                return UDIM;
            };

            TSharedPtr<IImageWrapper> ImageWrapper;
            FString UDIMFormat = bUDIM ? GetUDIM(OutMipBlocks[BlockIndex].BlockX, OutMipBlocks[BlockIndex].BlockY) : TEXT("");
            if (Format == TSF_RGBA16F || PixelFormat == PF_FloatRGBA)
            {
                ImageWrapper = ImageWrapperModule->CreateImageWrapper(EImageFormat::EXR);
                ExportPath = NewPath / NewFile + UDIMFormat + TEXT(".exr");
            }
            else
            {
                ImageWrapper = ImageWrapperModule->CreateImageWrapper(EImageFormat::PNG);
                ExportPath = NewPath / NewFile + UDIMFormat + TEXT(".png");
            }

            if (bGray16)
            {
                ImageWrapper->SetRaw(OutMipData.GetData(), OutMipData.GetAllocatedSize(), Width, Height, ERGBFormat::Gray, 16);
            }
            else
            {
                bool bBit16 = Format == TSF_RGBA16F || Format == TSF_RGBA16 || PixelFormat == PF_FloatRGBA;
                ERGBFormat RawFormat = (Format == TSF_G8) ? ERGBFormat::Gray : (bBit16 ? ERGBFormat::RGBA : ERGBFormat::BGRA);
                ImageWrapper->SetRaw(OutMipData.GetData(), OutMipData.GetAllocatedSize(), Width, Height, RawFormat, bBit16 ? 16 : 8);
            }

            // There's an Omniverse function (omniReferenceContent) buried in CreateSync that only supports 32-bit sizes
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 24
            const TArray<uint8>& CompressedData = ImageWrapper->GetCompressed();
#else
            const TArray64<uint8>& CompressedData = ImageWrapper->GetCompressed();
#endif
            bSuccess = SaveToFile(ExportPath, CompressedData);
        }
    }

    return bSuccess;
}

void PostMDLExporting(FString& MDL, bool bAbsolute, const FString& TemplatePath, const FString& MDLPath, TArray<FString>& UsingTemplates)
{
    if (bAbsolute)
    {
        FString NewImport = TemplatePath;
        NewImport.ReplaceInline(TEXT("/"), TEXT("::"));
        for(int32 TemplateIndex = 0; TemplateIndex < FMDLExporterUtility::MDL_MAX_Templates; ++TemplateIndex)
        {
            auto Template = FMDLExporterUtility::GetTemplateName((FMDLExporterUtility::EMDL_Templates)TemplateIndex).ToString();
            FString Search = TEXT("using ") + Template;
            FString Replace = TEXT("using ") + NewImport + TEXT("::") + Template;
            MDL.ReplaceInline(*Search, *Replace, ESearchCase::CaseSensitive);
        }
    }
    else
    {
        //
        if (TemplatePath != FMDLExporterUtility::GetFullPathOfUE4Templates())
        {
            FString RelativePath = FSimReadyPathHelper::ComputeRelativePath(TemplatePath, MDLPath, true, true);
            if (!RelativePath.StartsWith(TEXT(".")) && !RelativePath.StartsWith(TEXT("..")))
            {
                RelativePath = RelativePath.IsEmpty() ? TEXT(".") : TEXT(".") / RelativePath;
            }
            if (!RelativePath.EndsWith(TEXT("/")))
            {
                RelativePath += TEXT("/");
            }		
            RelativePath.ReplaceInline(TEXT("/"), TEXT("::"));
            for(int32 TemplateIndex = 0; TemplateIndex < FMDLExporterUtility::MDL_MAX_Templates; ++TemplateIndex)
            {
                auto Template = FMDLExporterUtility::GetTemplateName((FMDLExporterUtility::EMDL_Templates)TemplateIndex).ToString();
                FString Search = TEXT("using ::") + Template + TEXT(" ");
                FString Replace = TEXT("using ") + RelativePath + Template + TEXT(" ");
                if (MDL.ReplaceInline(*Search, *Replace, ESearchCase::CaseSensitive) > 0)
                {
                    UsingTemplates.Add(Template);
                }
            }
        }
    }
}

void FSimReadyAssetExportHelper::CopyTemplate(FSimReadyMaterialTemplateType::Type Type, const FString& DestPath, const FString& TemplateName)
{
    IFileManager* FileManager = &IFileManager::Get();
    FString SrcPath;
    switch(Type)
    {
    case FSimReadyMaterialTemplateType::Base:
        SrcPath = FMDLExporterUtility::GetFullPathOfBaseTemplates();
        break;
    case FSimReadyMaterialTemplateType::Surface:
        SrcPath = FMDLExporterUtility::GetFullPathOfSurfaceTemplates();
        break;
    case FSimReadyMaterialTemplateType::UE4:
    default:
        SrcPath = FMDLExporterUtility::GetFullPathOfUE4Templates();
        break;
    }
    FString From = SrcPath / TemplateName;
    FString To = DestPath / TemplateName;

    if (!FileExists(To))
    {
        FArchive* Reader = FileManager->CreateFileReader(*From);
        if (Reader)
        {
            const int64 FileSize = Reader->TotalSize();
            TArray64<uint8> Buff;
            Buff.AddUninitialized(FileSize);
            Reader->Serialize(Buff.GetData(), FileSize);
            SaveToFile(To, Buff);
        }
    }
}

bool FSimReadyAssetExportHelper::ExportMaterialToPath(class UMaterialInterface* InMaterial, const FString& InPath, FString& OutLegalName, bool bUploadToServer, bool bModular, bool bUpZAxis, bool bLandscape, const FSimReadyExportMaterialSettings& MaterialSettings, FSimReadyExportMaterialResult* OutResult, FExportTexturePreCallback PreExportCallback)
{
    bool bSuccess = false;

    if (FSimReadySlowTask::Get().IsProgressCancelled())
    {
        return bSuccess;
    }

    FString MDL;
    FMDLTranslatorResult TranslatorResult;

    if (!bUploadToServer)
    {
        FMDLExporterSetting Setting = { OutLegalName, bModular, false, bUpZAxis, bLandscape, bModular, MaterialSettings.TextureSettings.bDDSExport, MaterialSettings.bRayTracingTranslucency, MaterialSettings.bRayTracingRefraction };
        bSuccess = FMDLExporterUtility::ExportMDL(InMaterial, Setting, MDL, &TranslatorResult);
    }
    else
    {
        OutLegalName = GetLegalMDLName(FPaths::GetBaseFilename(InPath));
        FString CheckedPath = FPaths::GetPath(InPath);
        FString NewMdlPath = CheckedPath / OutLegalName + ".mdl";

        FString DestTempPath = CheckedPath;
        if (!MaterialSettings.DestTemplatePath.IsEmpty())
        {
            DestTempPath = MaterialSettings.DestTemplatePath;
        }

        FString SrcTempPath = FMDLExporterUtility::GetFullPathOfUE4Templates();

        FMDLExporterSetting Setting = {OutLegalName, bModular, bModular/*reset cache*/, bUpZAxis, bLandscape, false, MaterialSettings.TextureSettings.bDDSExport, MaterialSettings.bRayTracingTranslucency, MaterialSettings.bRayTracingRefraction };
        bool bExported = FMDLExporterUtility::ExportMDL(InMaterial, Setting, MDL, &TranslatorResult, [&](UTexture* Texture, const FString& TextureName, const FString& Folder)
        {
            FString TexOmniPath = CheckedPath / (bModular ? Folder : TEXT("Textures")) / TextureName + TEXT(".png");
            if (!IsTextureExported(Texture))
            {
                if (PreExportCallback)
                {
                    PreExportCallback(Texture);
                }

                auto NewTextureSettings = MaterialSettings.TextureSettings;
                NewTextureSettings.bReplaceTextureCube = InMaterial && InMaterial->GetMaterial() && InMaterial->GetMaterial()->GetName() == CARLA_FAKE_INTERIOR_MAT;
                if (ExportTextureToPath(Texture, TexOmniPath, NewTextureSettings))
                {
                    RegisterExportedTexture(Texture);
                }
            }
        });

        if (bExported && !MDL.IsEmpty())
        {
            if (!SrcTempPath.IsEmpty())
            {
                // only copying templates if destination was same target (local/server) as the mdl, or it failed.
                bool bShouldCopyTemplate = MaterialSettings.bCopyTemplate;
                
                TArray<FString> UsingTemplates;
                PostMDLExporting(MDL, false, bShouldCopyTemplate ? DestTempPath : SrcTempPath, CheckedPath, UsingTemplates);

                if (bShouldCopyTemplate && UsingTemplates.Num() > 0)
                {
                    // local templates are copied to server		
                    for (auto Template : UsingTemplates)
                    {
                        CopyTemplate(FSimReadyMaterialTemplateType::UE4, DestTempPath, Template + ".mdl");
                    }
                }
            }

            if (SaveToFile(NewMdlPath, MDL))
            {
                bSuccess = true;

                if (TranslatorResult.IsPerlinGradientTextureUsed())
                {
                    TArray<uint32> PerlinNoiseGradientData;
                    PerlinNoiseGradientData.AddZeroed(128 * 128);

                    uint32 DestStride = 512;
                    uint8* DestBuffer = (uint8*)PerlinNoiseGradientData.GetData();
                    // seed the pseudo random stream with a good value
                    FRandomStream RandomStream(12345);
                    // Values represent float3 values in the -1..1 range.
                    // The vectors are the edge mid point of a cube from -1 .. 1
                    static uint32 gradtable[] =
                    {
                        0xFF88ffff, 0xFFff88ff, 0xFFffff88,
                        0xFF88ff00, 0xFFff8800, 0xFFff0088,
                        0xFF8800ff, 0xFF0088ff, 0xFF00ff88,
                        0xFF880000, 0xFF008800, 0xFF000088,
                    };
                    for (int32 y = 0; y < 128; ++y)
                    {
                        for (int32 x = 0; x < 128; ++x)
                        {
                            uint32* Dest = (uint32*)(DestBuffer + x * sizeof(uint32) + y * DestStride);

                            // pick a random direction (hacky way to overcome the quality issues FRandomStream has)
                            *Dest = gradtable[(uint32)(RandomStream.GetFraction() * 11.9999999f)];
                        }
                    }

                    FString TexOmniPath = CheckedPath / (bModular ? OutLegalName : TEXT("Textures")) / TEXT("PerlinNoiseGradientTexture.png");
                    FSimReadyAssetExportHelper::ExportRawDataToPath(PerlinNoiseGradientData.GetData(), PerlinNoiseGradientData.Num() * 4, TexOmniPath, 128, 128, ERGBFormat::BGRA, 8);
                }
            }
        }
        
        // From cache
        if (!bExported && !TranslatorResult.MDLName.IsEmpty())
        {
            OutLegalName = TranslatorResult.MDLName;
            bSuccess = true;
        }
        else
        {
            for (int32 MessageIndex = 0; MessageIndex < TranslatorResult.Messages.Num(); MessageIndex++)
            {
                FSimReadyMessageLogContext::AddMessage(TranslatorResult.Messages[MessageIndex]);
            }
        }
    }

    if (OutResult)
    {
        OutResult->ConstantProperties = TranslatorResult.ConstantProperties;
        OutResult->ScalarParameters = TranslatorResult.ScalarParameters;
        OutResult->VectorParameters = TranslatorResult.VectorParameters;
        OutResult->TextureParameters = TranslatorResult.TextureParameters;
        OutResult->UsageFlag = TranslatorResult.UsageFlag;
    }

    return bSuccess;
}

void FSimReadyAssetExportHelper::ResetGlobalCaches()
{
    FMDLExporterUtility::ResetCaches();
}

FString FSimReadyAssetExportHelper::GetLegalMDLName(const FString& InName)
{
    return FMDLExporterUtility::GetLegalIdentifier(InName, true/*bRemoveSpaces*/);
}

FString FSimReadyAssetExportHelper::GetTextureFileName(class UTexture* InTexture, bool bDDSTexture)
{
    return (InTexture->GetName() + TEXT(".") + FMDLExporterUtility::GetFileExtension(InTexture, bDDSTexture));
}

bool FSimReadyAssetExportHelper::IsTextureExported(UTexture* InTexture)
{
    bool* bExported = FMDLExporterUtility::GlobalExportedTextures.Find(InTexture);
    return (bExported != nullptr && *bExported == true);
}

void FSimReadyAssetExportHelper::RegisterExportedTexture(UTexture* InTexture)
{
    FMDLExporterUtility::GlobalExportedTextures.Add(InTexture, true);
}

FString FSimReadyAssetExportHelper::GetUniqueTextureName(UTexture* InTexture, bool bDDSTexture)
{
    if (FMDLExporterUtility::GlobalUsedTextures.Find(InTexture) == nullptr)
    {
        FString ID = TEXT("");

        if (FMDLExporterUtility::GlobalUsedTextures.FindKey(InTexture->GetName()))
        {
            // name is not unique
            ID = TEXT("_") + FString::FromInt(FMDLExporterUtility::GlobalTextureID++);
        }

        FMDLExporterUtility::GlobalUsedTextures.Add(InTexture, InTexture->GetName() + ID);
    }

    return FMDLExporterUtility::GlobalUsedTextures[InTexture] + TEXT(".") + FMDLExporterUtility::GetFileExtension(InTexture, bDDSTexture);
}

void FSimReadyAssetExportHelper::LogMessage(const UObject* InObject, FSimReadyExportMessageSeverity::Type InType, FString InMessage)
{
    FString OuterPackageName;
    if (!FPackageName::TryConvertFilenameToLongPackageName(InObject->GetPathName(), OuterPackageName))
    {
        OuterPackageName = InObject->GetPathName();
    }

    EMessageSeverity::Type MessageType;
    switch (InType)
    {
    case FSimReadyExportMessageSeverity::Error:
        MessageType = EMessageSeverity::Error;
        break;
    case FSimReadyExportMessageSeverity::Warning:
        MessageType = EMessageSeverity::Warning;
        break;
    default:
        MessageType = EMessageSeverity::Warning;
        break;
    }

    TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(MessageType, FText());
    Message->AddToken(FAssetNameToken::Create(OuterPackageName));
    Message->AddToken(FTextToken::Create(FText::FromString(TEXT(":"))));
    Message->AddToken(FTextToken::Create(FText::FromString(InMessage)));
    FSimReadyMessageLogContext::AddMessage(Message);
}

TArray<FString> FSimReadyAssetExportHelper::GetImportList(const FString& Name)
{
    TSet<FString> ImportList;
    FMDLImporterUtility::GetImportModulesByModuleName(Name, ImportList);
    return ImportList.Array();
}