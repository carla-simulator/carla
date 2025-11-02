// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "SimReadyAsset.h"
#include "Async/Future.h"
#include "Engine/Texture.h"
#include "SimReadyTexture.generated.h"

UCLASS()
class SIMREADYRUNTIME_API USimReadyTexture: public USimReadyAsset
{
    GENERATED_BODY()

public:
    static class UTexture* CreateTextureFromBuffer(const uint8* Buffer, uint64 Size, UObject* Parent, FName Name, EObjectFlags Flags, const FString& SourceFile = TEXT(""));
    static class UTexture* CreateTextureFromFile(const FString& File, UObject* Parent, FName Name, EObjectFlags Flags);

protected:
    static void UpdatePixels(UTexture& Texture, int32 SizeX, int32 SizeY, int32 BitDepth, const TArray<uint8>& Pixles, bool bCallPostEditChange = true);
};
