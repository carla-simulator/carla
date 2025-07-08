// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyMDLReader.h"
#include "SimReadyFileHandle.h"
#include "SimReadyRuntimePrivate.h"
#include "SimReadyPathHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"


bool FSimReadyMDLReader::FileExists(const FString& InPath, const FString& InMask)
{
    FString FullPath = InPath;
    FString* OutPath = nullptr;
    
    if (!InMask.IsEmpty())
    {
        FullPath = InPath / InMask;
        OutPath	= OmniDependencies.Find(InMask);
    }

    if (OutPath)
    {
        return (FullPath.Equals(*OutPath));
    }
    else
    {
        bool bFound = FPaths::FileExists(FullPath);
        if (bFound && !InMask.IsEmpty())
        {
            OmniDependencies.Add(InMask, FullPath);
        }

        return bFound;
    }
}

IFileHandle* FSimReadyMDLReader::OpenRead(const FString& InPath)
{
    TArray64<uint8> Content;
    if (FFileHelper::LoadFileToArray(Content, *InPath, EFileRead::FILEREAD_Silent))
    {
        return new FSimReadyFileHandle(Content);
    }

    return nullptr;
}

bool FSimReadyMDLReader::GetOmniDependencies(TArray<FString> & OutOmniDependencies)
{
    if (OmniDependencies.Num() > 0)
    {
        OmniDependencies.GenerateValueArray(OutOmniDependencies);
        return true;
    }

    return false;
}

void FSimReadyMDLReader::ClearOmniDependencies()
{
    OmniDependencies.Reset();
}

