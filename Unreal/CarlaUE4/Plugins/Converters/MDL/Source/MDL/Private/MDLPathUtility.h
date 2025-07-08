// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"

namespace MDLPathUtility
{
    FString MangleMdlPath(const FString& Input, bool bMDLSeparator = true);
    FString UnmangleMdlPath(const FString& Input, bool bMDLSeparator = true);

    FString MangleAssetPath(const FString& UnMangledPath);
    FString UnmangleAssetPath(const FString& InPath, bool bStartWithSlash);
}
