// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyPathHelper.h"

#include "Misc/Paths.h"
#include "Misc/SecureHash.h"

#include "SimReadyRuntimePrivate.h"
#include "SimReadyMaterialReparentSettings.h"
#include "SimReadyMaterialReparentUtility.h"

#include <string>

static const FString MangledPathPostfix = TEXT("_MANGLED");
static const FString KeySeparator = TEXT("@@");

class FCharAllocator
{
public:
    FCharAllocator(SIZE_T InSize)
    {
        Data = (char*)FMemory::Malloc(InSize);
    }

    ~FCharAllocator()
    {
        FMemory::Free(Data);
    }

    char* GetData() { return Data; }

    char* Data;
};

static bool IsValidUE4PathChar(TCHAR c)
{
    return (c != '\'' && c != ' ' && c != ',' && c != '.' && c != '&' && c != '!' && c != '~' && c != '@' && c != '#');
}

void FSimReadyPathHelper::FixAssetName(FString& AssetName)
{
    // Asset name convert ' ,.&!~@# to _
    AssetName.ReplaceInline(TEXT("'"), TEXT("_"));
    AssetName.ReplaceInline(TEXT(" "), TEXT("_"));
    AssetName.ReplaceInline(TEXT(","), TEXT("_"));
    AssetName.ReplaceInline(TEXT("."), TEXT("_"));
    AssetName.ReplaceInline(TEXT("&"), TEXT("_"));
    AssetName.ReplaceInline(TEXT("!"), TEXT("_"));
    AssetName.ReplaceInline(TEXT("~"), TEXT("_"));
    AssetName.ReplaceInline(TEXT("@"), TEXT("_"));
    AssetName.ReplaceInline(TEXT("#"), TEXT("_"));
}

FString FSimReadyPathHelper::ComputeRelativePath(const FString& TargetPath, const FString& SourcePath, bool TargetIsDirectory, bool SourceIsDirectory)
{
    FString SourcePathCopy = SourcePath;
    if (SourceIsDirectory && !SourcePathCopy.EndsWith("/"))
    {
        SourcePathCopy += "/";
    }
    FString TargetPathCopy = TargetPath;
    if (TargetIsDirectory && !TargetPathCopy.EndsWith("/"))
    {
        TargetPathCopy += "/";
    }

    FString OutRelativePath(TargetPathCopy);
    FPaths::MakePathRelativeTo(OutRelativePath, *SourcePathCopy);
    if (OutRelativePath.IsEmpty())
    {
        OutRelativePath = TEXT("./");
    }
    if (TargetIsDirectory && !OutRelativePath.EndsWith("/"))
    {
        OutRelativePath += "/";
    }

    return OutRelativePath;
}

void FSimReadyPathHelper::ManglePath(const FString& OmniPath, FString& OutUE4Path, bool bStartWithSlash)
{
    OutUE4Path.Empty();

    FString OmniPathWithoutPrefix = OmniPath;
    OmniPathWithoutPrefix.RemoveFromStart("omniverse:/");
    OmniPathWithoutPrefix.RemoveFromStart("omni:/");

    TArray<FString> OutFolderNames;
    OmniPathWithoutPrefix.ParseIntoArray(OutFolderNames, TEXT("/"));

    for (auto FolderName : OutFolderNames)
    {
        bool bMangledChar = false;
        auto CharArray = FolderName.GetCharArray();
        FString MangledFolderName;
        for (int32 Index = 0; Index < CharArray.Num() - 1; ++Index) // Skip the last \0
        {
            auto Char = CharArray[Index];
            if (IsValidUE4PathChar(Char))
            {
                MangledFolderName += Char;
            }
            else
            {
                MangledFolderName += TEXT("_") + FString::FromInt(Char) + TEXT("_");
                bMangledChar = true;
            }
        }

        if (bMangledChar)
        {
            MangledFolderName += MangledPathPostfix;
        }

        OutUE4Path += ((OutUE4Path.IsEmpty() && !bStartWithSlash) ? TEXT("") : TEXT("/")) + MangledFolderName;
    }
}

void FSimReadyPathHelper::UnmanglePath(const FString& UE4Path, FString& OmniPath, bool bStartWithSlash)
{
    OmniPath.Empty();

    TArray<FString> OutFolderNames;
    UE4Path.ParseIntoArray(OutFolderNames, TEXT("/"));

    for (auto FolderName : OutFolderNames)
    {
        if (FolderName.EndsWith(MangledPathPostfix))
        {
            FolderName.RemoveFromEnd(MangledPathPostfix);
            auto CharArray = FolderName.GetCharArray();
            FString MangledChar;
            bool bUnmangleChar = false;
            FString MangledFolderName;
            FString MangleFailed;
            for (int32 Index = 0; Index < CharArray.Num() - 1; ++Index) // Skip the last \0
            {
                auto Char = CharArray[Index];
                if (Char == '_')
                {
                    if (!bUnmangleChar)
                    {
                        MangledChar.Empty();
                        MangleFailed.Empty();
                        MangleFailed += Char; // For in case unmanlge failed
                        bUnmangleChar = true;
                    }
                    else
                    {
                        TCHAR* End = nullptr;
                        TCHAR AtoiChar = TCHAR(FCString::Strtoi(*MangledChar, &End, 10));
                        if (!IsValidUE4PathChar(AtoiChar))
                        {
                            // Found mangled
                            MangledFolderName += AtoiChar;
                            bUnmangleChar = false;
                        }
                        else
                        {
                            MangledFolderName += MangleFailed;
                            // it's a new start
                            MangledChar.Empty();
                            MangleFailed.Empty();
                            MangleFailed += Char; // For in case unmanlge failed
                        }
                    }
                }
                else if (bUnmangleChar)
                {
                    MangledChar += Char;
                    MangleFailed += Char;
                }
                else
                {
                    MangledFolderName += Char;
                }
            }

            if (bUnmangleChar)
            {
                MangledFolderName += MangleFailed;
            }

            OmniPath += ((OmniPath.IsEmpty() && !bStartWithSlash) ? TEXT("") : TEXT("/")) + MangledFolderName;
        }
        else if (FolderName.EndsWith(TEXT(":")))
        {
            OmniPath += FolderName + TEXT("/");
        }
        else
        {
            OmniPath += ((OmniPath.IsEmpty() && !bStartWithSlash) ? TEXT("") : TEXT("/")) + FolderName;
        }
    }
}

FString FSimReadyPathHelper::PrimPathToKey(const FString& PrimPath)
{
    TArray<FString> Address;
    int32 Elem = PrimPath.ParseIntoArray(Address, TEXT(":"));
    FString FixedPath;
    if (Elem > 1)
    {
        FixedPath = Address[0].ToLower() + TEXT(":") + Address[1];
    }
    else if (Elem > 0)
    {
        FixedPath = Address[0];
    }

    FSHA1 HashState;
    FSHAHash OutHash;
    HashState.UpdateWithString(*FixedPath, FixedPath.Len());
    HashState.Final();
    HashState.GetHash(&OutHash.Hash[0]);
    FString OutKey = FixedPath + KeySeparator +OutHash.ToString();
    return OutKey;
}

FString FSimReadyPathHelper::KeyToPrimPath(const FString& Key)
{
    int Index = Key.Find(KeySeparator);
    if (Index == INDEX_NONE)
    {
        return Key;
    }

    FString PrimPath = Key.Left(Index);
    FString HashString = Key.RightChop(Index + KeySeparator.Len());
    // Check PrimPath
    FSHA1 HashState;
    FSHAHash OutHash;
    HashState.UpdateWithString(*PrimPath, PrimPath.Len());
    HashState.Final();
    HashState.GetHash(&OutHash.Hash[0]);

    if (HashString == OutHash.ToString())
    {
        return PrimPath;
    }

    return Key;
}

bool FSimReadyPathHelper::IsPackagePath(const FString& Path)
{
    return (Path.Contains("[") && Path.EndsWith("]"));
}

FString FSimReadyPathHelper::GetPackageRoot(const FString& Path)
{
    if (IsPackagePath(Path))
    {
        int32 End = Path.Find("[");
        return Path.Mid(0, End);
    }

    return FString();
}

FString FSimReadyPathHelper::GetPackagedSubPath(const FString& Path)
{
    if (IsPackagePath(Path))
    {
        int32 Start = Path.Find("[") + 1;
        return Path.Mid(Start, Path.Len() - 1 - Start);
    }

    return FString();
}

bool FSimReadyPathHelper::ParseVersion(const FString& Version, int32& MajorVersion, int32& MinorVersion)
{
    MajorVersion = 0;
    MinorVersion = 0;
    TArray<FString> OutParsedArray;
    Version.ParseIntoArray(OutParsedArray, TEXT("."));
    if (OutParsedArray.Num() > 1)
    {
        // Only take the first 2 string
        MajorVersion = FCString::Atoi(*OutParsedArray[0]);
        MinorVersion = FCString::Atoi(*OutParsedArray[1]);
        return true;
    }

    return false;
}

UMaterialInterface* FSimReadyPathHelper::GetPreviewSurfaceMaterial(bool bNative)
{
    UMaterialInterface* PreviewSurfaceMaterial = nullptr;
    if (!bNative)
    {
        PreviewSurfaceMaterial = FSimReadyMaterialReparentUtility::FindUnrealMaterial(USDPreviewSurfaceDisplayName, ESimReadyReparentUsage::Import);
    }

    if (PreviewSurfaceMaterial == nullptr)
    {
        return LoadObject<UMaterial>(nullptr, TEXT("/SimReady/PreviewSurfaceMaterial"));
    }

    return PreviewSurfaceMaterial;
}