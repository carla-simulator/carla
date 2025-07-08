// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "MDLExternalReader.h"


class FSimReadyMDLReader : public IMDLExternalReader
{

public:
    virtual bool FileExists(const FString& InPath, const FString& InMask); 

    virtual class IFileHandle* OpenRead(const FString& InPath);

    bool GetOmniDependencies(TArray<FString> & OutOmniDependencies);
    void ClearOmniDependencies();

private:
    TMap<FString, FString>	OmniDependencies;
};