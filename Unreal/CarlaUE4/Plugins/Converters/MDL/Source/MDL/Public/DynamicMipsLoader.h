// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture2DDynamic.h"

class MDL_API IDynamicMipsLoader
{
public:
    virtual ~IDynamicMipsLoader()
    {
    }

    virtual void GetMipData(UTexture2DDynamic* Texture2DDynamic, int32 FirstMipToLoad, void** OutMipData) = 0;

    virtual bool GetUncompressedMipData(UTexture2DDynamic* Texture2DDynamic, int32 MipIndex, TArray64<uint8>& OutMipData, ETextureSourceFormat& OutFormat) = 0;
};

