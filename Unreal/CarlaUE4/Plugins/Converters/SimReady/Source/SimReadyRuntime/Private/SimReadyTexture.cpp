// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyTexture.h"
#include "ISimReadyRuntimeModule.h"
#include "Async/Async.h"
#include "Async/ParallelFor.h"
#include "SimReadySettings.h"
#include "UObject/UObjectHash.h"
#include "Misc/CoreDelegates.h"
#include "Misc/ScopedSlowTask.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "Engine/TextureLightProfile.h"
#include "ContentStreaming.h"
#include "Stats/Stats2.h"
#include "DDSUtils.h"
#if WITH_EDITOR
#include "Editor.h"
#include "Factories/TextureFactory.h"
#include "Subsystems/ImportSubsystem.h"
#include "Editor/EditorEngine.h"
#endif
#include "DDSLoader.h"
#include "HDRLoader.h"
#include "SimReadyDDSLoader.h"
#include "SimReadyAssetImportHelper.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

// Trick here: 1x1 texture represents the null texture in mdl, creating the unloaded texture with 4x4
const int32 UnLoadedTextureSize = 4;

bool IsDDSTextureCubemap(const uint8* TextureBuffer, uint64 Length)
{
#if PLATFORM_WINDOWS
    if (Length < FDDSUtils::GetDDSHeaderSize())
    {
        return false;
    }

    FDDSLoadHelper DDSLoadHelper(TextureBuffer, Length);
    return (DDSLoadHelper.IsValidCubemapTexture());
#else
    return false;
#endif
}

bool IsDDSTexture2D(const uint8* TextureBuffer, uint64 Length)
{
#if PLATFORM_WINDOWS
    if (Length < FDDSUtils::GetDDSHeaderSize())
    {
        return false;
    }

    FDDSLoadHelper DDSLoadHelper(TextureBuffer, Length);
    return (DDSLoadHelper.IsValid2DTexture());
#else
    return false;
#endif
}



UTexture* USimReadyTexture::CreateTextureFromFile(const FString& File, UObject* Parent, FName Name, EObjectFlags Flags)
{
    TArray64<uint8> Content;
    if (FFileHelper::LoadFileToArray(Content, *File, EFileRead::FILEREAD_Silent))
    {
        return CreateTextureFromBuffer(Content.GetData(), Content.Num(), Parent, Name, Flags, File);
    }

    return nullptr;
}

UTexture* USimReadyTexture::CreateTextureFromBuffer(const uint8* Buffer, uint64 Size, UObject* Parent, FName Name, EObjectFlags Flags, const FString& SourceFile)
{
    // Force virtual texture off.
    // TODO: UDIM texture will be enabled as virtual texture
    static IConsoleVariable* CVarVirtualTexturesEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("r.VirtualTextures"));
    int32 VirtualTexturesEnabledBackup = 0;
    if (CVarVirtualTexturesEnabled)
    {
         VirtualTexturesEnabledBackup = CVarVirtualTexturesEnabled->GetInt();
         CVarVirtualTexturesEnabled->Set(0);
    }

    UTexture* Texture = nullptr;
#if WITH_EDITOR
    auto TextureFactory = NewObject<UTextureFactory>();
    TextureFactory->AddToRoot();
    TextureFactory->SuppressImportOverwriteDialog(true);
    FString Extension = FPaths::GetExtension(SourceFile);

#if PLATFORM_WINDOWS
    FSimReadyDDSLoadHelper DDSLoadHelper(Buffer, Size);
    if (DDSLoadHelper.IsValid2DTexture())
    {
        TArray64<uint8> RawData;
        if (DDSLoadHelper.LoadRawData(RawData))
        {
            TSharedPtr<IImageWrapper> ImageWrapper;
            IImageWrapperModule* ImageWrapperModule = FModuleManager::GetModulePtr<IImageWrapperModule>(FName("ImageWrapper"));
            ImageWrapper = ImageWrapperModule->CreateImageWrapper(EImageFormat::PNG);
            ImageWrapper->SetRaw(RawData.GetData(), RawData.GetAllocatedSize(), DDSLoadHelper.GetSizeX(), DDSLoadHelper.GetSizeY(), ERGBFormat::BGRA, 8);
            const TArray64<uint8>& CompressedData = ImageWrapper->GetCompressed();

            const uint8* CompressedBuffer = CompressedData.GetData();
            Texture = (UTexture*)TextureFactory->FactoryCreateBinary(
                    UTexture::StaticClass(),
                    Parent,
                    Name,
                    Flags,
                    nullptr,
                    *Extension,
                    CompressedBuffer,
                    CompressedBuffer + CompressedData.Num(),
                    GWarn
                );
        }
    }
#endif
    if (Texture == nullptr)
    {
        // NOTE: Omniverse treats HDR as 2D texture while it's Cube texture in UE4
        FHDRLoadHelper HDRLoadHelper(Buffer, Size);
        if (HDRLoadHelper.IsValid())
        {
            UClass* Class = UTexture::StaticClass();
            GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(TextureFactory, Class, Parent, Name, *Extension);
            UTexture* ExistingTexture = FindObject<UTexture>(Parent, *Name.ToString());

            FTextureReferenceReplacer RefReplacer(ExistingTexture);
            TArray<uint8> DDSFile;
            HDRLoadHelper.ExtractDDSInRGBE(DDSFile);
            FDDSLoadHelper HDRDDSLoadHelper(DDSFile.GetData(), DDSFile.Num());

            // create the cube texture
            UTexture2D* Texture2D = TextureFactory->CreateTexture2D(Parent, Name, Flags);
            if (Texture2D)
            {
                Texture2D->Source.Init(
                    HDRDDSLoadHelper.DDSHeader->dwWidth,
                    HDRDDSLoadHelper.DDSHeader->dwHeight,
                    /*NumSlices=*/ 1,
                    /*NumMips=*/ 1,
                    TSF_BGRE8,
                    HDRDDSLoadHelper.GetDDSDataPointer()
                );
                // the loader can suggest a compression setting
                Texture2D->CompressionSettings = TC_HDR;
                Texture2D->SRGB = false;
                Texture2D->bHasBeenPaintedInEditor = false;
            }

            RefReplacer.Replace(Texture2D);

            GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(TextureFactory, Texture2D);

            Texture2D->PostEditChange();

            Texture = Texture2D;
        }
        else
        {
            Texture = (UTexture*)TextureFactory->FactoryCreateBinary(
                UTexture::StaticClass(),
                Parent,
                Name,
                Flags,
                nullptr,
                *Extension,
                Buffer,
                Buffer + Size,
                GWarn
            );
        }
    }

    if (CVarVirtualTexturesEnabled)
    {
         CVarVirtualTexturesEnabled->Set(VirtualTexturesEnabledBackup);
    }

    if (Texture)
    {
        Texture->MarkPackageDirty();

        if (Texture->GetOutermost() != GetTransientPackage())
        {
            FAssetRegistryModule::AssetCreated(Texture);
        }
    }

    TextureFactory->RemoveFromRoot();
#endif

    return Texture;
}

void USimReadyTexture::UpdatePixels(UTexture & Texture, int32 SizeX, int32 SizeY, int32 BitDepth, const TArray<uint8>& Pixles, bool bCallPostEditChange)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("SimReadyTextureUpdatePixels"), STAT_SimReadyTextureUpdatePixels, STATGROUP_SimReady);

#if WITH_EDITOR
#if WITH_EDITORONLY_DATA
    // Cache pixels for editor
    Texture.Source.Init(SizeX, SizeY, 1, 1, BitDepth == 16 ? ETextureSourceFormat::TSF_RGBA16 : ETextureSourceFormat::TSF_BGRA8, reinterpret_cast<const uint8*>(Pixles.GetData()));
    Texture.SRGB = BitDepth < 16;
    if(bCallPostEditChange)
    {
        Texture.PostEditChange();
    }
    Texture.Modify();
#endif
#else
    // Release resource To void assertion in FTexture2DResource::ReleaseRHI()
    Texture.ReleaseResource();

    // Create platform data
    if(!Texture.PlatformData)
    {
        Texture.PlatformData = new FTexturePlatformData();
    }

    // Basic attributes
    Texture.PlatformData->SizeX = SizeX;
    Texture.PlatformData->SizeY = SizeY;
    Texture.PlatformData->NumSlices = 1;
    Texture.PlatformData->PixelFormat = EPixelFormat::PF_B8G8R8A8;

    // Mips
    Texture.PlatformData->Mips.Reset(1);
    Texture.PlatformData->Mips.Add(new FTexture2DMipMap);
    auto& Mip = Texture.PlatformData->Mips[0];
    Mip.SizeX = SizeX;
    Mip.SizeY = SizeY;

    // Write pixels
    Mip.BulkData.Lock(LOCK_READ_WRITE);

    auto Size = Pixles.Num() * sizeof(FColor);
    auto* MipData = StaticCast<FColor*>(Mip.BulkData.Realloc(Size));
    FMemory::Memcpy(MipData, Pixles.GetData(), Size);

    Mip.BulkData.Unlock();

    // Recreate render resource
    Texture.UpdateResource();
#endif
}