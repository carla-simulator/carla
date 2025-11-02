// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"

struct MDL_API FCubeFacesComposite
{
    int32 FaceA = INDEX_NONE;
    int32 FaceB = INDEX_NONE;
    float Ratio = 1.0f;
};

class MDL_API FDDSUtils
{
public:
    static bool ExportToDDS(class UTexture* TexRT, const FCubeFacesComposite& CompositeSetting, FArchive& Ar);
    static int32 GetDDSHeaderSize();
};
