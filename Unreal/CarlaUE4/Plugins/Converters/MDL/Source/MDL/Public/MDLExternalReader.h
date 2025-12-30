// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"

class MDL_API IMDLExternalReader
{
public:
    /** Destructor, also the only way to close the file handle **/
    virtual ~IMDLExternalReader()
    {
    }

    virtual bool		FileExists(const FString& InPath, const FString& InMask) = 0;

    virtual class IFileHandle* OpenRead(const FString& InPath) = 0;
};

