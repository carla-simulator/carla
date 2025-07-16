// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"

class FSimReadyMDLHelper
{
public:
    FSimReadyMDLHelper(uint8* Data, int32 Size);
    ~FSimReadyMDLHelper();

    bool ExtractMaterial(const FString& MaterialName);
    void SetExportName(const FString& ExportName);
    void SetValue(const FString& Name, bool Value, bool bLastValue);
    void SetValue(const FString& Name, float Value, bool bLastValue);
    void SetValue(const FString& Name, int32 Value, bool bLastValue);
    void SetValue(const FString& Name, FLinearColor Value, int32 Channels, bool bColor, bool bLastValue);
    void SetValue(const FString& Name, class UTexture* Value, const FString& RelativePath, bool bCubemap, bool bLastValue);

    FString GetFinalOutput() { return FinalOutput; }
private:
    void RemoveComment(char* Start, int32 Size);
    void AppendParamToFinal(const FString& Param, bool bLastValue);

    FString OriginMDLWithoutComment;
    FString CustomExportName;
    FString FinalOutput;
    int32 SearchPosition;
};