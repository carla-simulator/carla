// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyMDLHelper.h"
#include "MDLExporterUtility.h"
#include "Engine/Texture.h"

void RemoveBlankTabReturn(char*& Ptr)
{
    while (*Ptr == ' ' || *Ptr == '\t' || *Ptr == '\n' || *Ptr == '\r')
    {
        Ptr++;
    }
}

void RemoveBlankTabReturnFromEnd(char*& Ptr)
{
    while (*(Ptr-1) == ' ' || *(Ptr - 1) == '\t' || *(Ptr - 1) == '\n' || *(Ptr - 1) == '\r')
    {
        Ptr--;
    }
}

char* AllocInitialBuffer(int32 Size)
{
    char* data = (char*)FMemory::Malloc(Size+1);
    FMemory::Memzero(data, Size + 1);
    return data;
}

FSimReadyMDLHelper::FSimReadyMDLHelper(uint8* Data, int32 Size)
    : SearchPosition(0)
{
    char* BuffWithoutComment = AllocInitialBuffer(Size);
    FMemory::Memcpy(BuffWithoutComment, (char*)Data, Size);
    RemoveComment(BuffWithoutComment, Size);
    OriginMDLWithoutComment = BuffWithoutComment;
    FMemory::Free(BuffWithoutComment);
}

FSimReadyMDLHelper::~FSimReadyMDLHelper()
{
    
}

bool FSimReadyMDLHelper::ExtractMaterial(const FString& InMaterialName)
{
    int32 Ret = OriginMDLWithoutComment.Find(InMaterialName, ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchPosition);
    if (Ret == INDEX_NONE)
    {
        return false;
    }
    
    FString Output;
    Output.Append(OriginMDLWithoutComment.Mid(SearchPosition, Ret - SearchPosition));
    SearchPosition = Ret + InMaterialName.Len();

    if (!CustomExportName.IsEmpty())
    {
        Output += CustomExportName;
    }
    else
    {
        Output += InMaterialName;
    }

    Ret = OriginMDLWithoutComment.Find(TEXT("="), ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchPosition);
    if (Ret == INDEX_NONE)
    {
        return false;
    }
    SearchPosition = Ret;

    Ret = OriginMDLWithoutComment.Find(TEXT("("), ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchPosition);
    if (Ret == INDEX_NONE)
    {
        return false;
    }
    Output += TEXT("(*)\n");
    Output += TEXT(" ");
    Output.Append(OriginMDLWithoutComment.Mid(SearchPosition, Ret - SearchPosition));
    Output += TEXT("(\n");
    Output += TEXT(");\n");

    Ret = OriginMDLWithoutComment.Find(TEXT(";"), ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchPosition);
    if (Ret == INDEX_NONE)
    {
        return false;
    }
    SearchPosition = Ret + 1;

    FinalOutput = Output;
    return true;
}

void FSimReadyMDLHelper::SetExportName(const FString& ExportName)
{
    CustomExportName = ExportName;
}

void FSimReadyMDLHelper::AppendParamToFinal(const FString& Param, bool bLastValue)
{
    int32 Ret = FinalOutput.Find(TEXT(");"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
    if (Ret != INDEX_NONE)
    {
        FinalOutput.InsertAt(Ret, bLastValue ? Param : (Param + TEXT(",\n")));
    }
}

void FSimReadyMDLHelper::SetValue(const FString& Name, bool Value, bool bLastValue)
{
    FString Param = TEXT("\t") + Name + TEXT(": ") + (Value ? TEXT("true") : TEXT("false"));
    AppendParamToFinal(Param, bLastValue);
}

void FSimReadyMDLHelper::SetValue(const FString& Name, float Value, bool bLastValue)
{
    FString Param = TEXT("\t") + Name + TEXT(": ") + FString::SanitizeFloat(Value);
    AppendParamToFinal(Param, bLastValue);
}

void FSimReadyMDLHelper::SetValue(const FString& Name, int32 Value, bool bLastValue)
{
    FString Param = TEXT("\t") + Name + TEXT(": ") + FString::FromInt(Value);
    AppendParamToFinal(Param, bLastValue);
}

void FSimReadyMDLHelper::SetValue(const FString& Name, FLinearColor Value, int32 Channels, bool bColor, bool bLastValue)
{
    FString Param;
    switch(Channels)
    {
    case 2:
        Param = TEXT("\t") + Name + TEXT(": float2(") + FString::SanitizeFloat(Value.R) + TEXT(", ") + FString::SanitizeFloat(Value.G) + TEXT(")");
        break;
    case 3:
        Param = TEXT("\t") + Name + (bColor ? TEXT(": color(") : TEXT(": float3(")) + FString::SanitizeFloat(Value.R) + TEXT(", ") + FString::SanitizeFloat(Value.G) + TEXT(", ") + FString::SanitizeFloat(Value.B) + TEXT(")");
        break;
    case 4:
    default:
        Param = TEXT("\t") + Name + TEXT(": float4(") + FString::SanitizeFloat(Value.R) + TEXT(", ") + FString::SanitizeFloat(Value.G) + TEXT(", ") + FString::SanitizeFloat(Value.B) + TEXT(", ") + FString::SanitizeFloat(Value.A) + TEXT(")");
        break;
    }

    AppendParamToFinal(Param, bLastValue);
}

void FSimReadyMDLHelper::SetValue(const FString& Name, UTexture* Value, const FString& RelativePath, bool bCubemap, bool bLastValue)
{
    FString Param = TEXT("\t") + Name + TEXT(": ");
    Param += (bCubemap ? TEXT("texture_cube(") : TEXT("texture_2d("));
    
    if (Value)
    {
        TCHAR FormattedCode[MAX_SPRINTF] = TEXT("");
        FCString::Sprintf(FormattedCode, TEXT("\"%s/%s%s.%s\", %s"),
            *RelativePath,
            *Value->GetName(), 
            Value->IsCurrentlyVirtualTextured() ? TEXT(".<UDIM>") : TEXT(""),
            *FMDLExporterUtility::GetFileExtension(Value),
            Value->Source.GetFormat() == TSF_RGBA16F ?
                TEXT("::tex::gamma_linear") :
                (Value->SRGB ? TEXT("::tex::gamma_srgb") : TEXT("::tex::gamma_linear")));

        Param += FormattedCode;
    }
    Param += TEXT(")");

    AppendParamToFinal(Param, bLastValue);
}

bool IsLineCommentStart(char* Str)
{
    return (*Str == '/' && *(Str + 1) == '/');
}

bool IsBlockCommentStart(char* Str)
{
    return (*Str == '/' && *(Str + 1) == '*');
}

bool IsBlockCommentEnd(char* Str)
{
    return (*Str == '*' && *(Str + 1) == '/');
}

bool IsCommentStart(char* Str)
{
    return IsLineCommentStart(Str) || IsBlockCommentStart(Str);
}

void FSimReadyMDLHelper::RemoveComment(char* InStart, int32 Size)
{
    char* Start = InStart;
    char* End = Start + Size;
    // check /* and //

    while (*Start != '\0')
    {
        while (Start != (End - 1) && !IsCommentStart(Start))
        {
            Start++;
        }

        if (Start >= End - 1)
        {
            break;
        }
        else
        {
            if (IsLineCommentStart(Start))
            {
                char* LineEnd = Start;

                while (*LineEnd != '\n' && LineEnd != End)
                {
                    LineEnd++;
                }

                int Count = End - LineEnd;
                for (int32 i = 0; i < Count; ++i)
                {
                    Start[i] = LineEnd[i];
                }

                FMemory::Memzero(Start + Count, End - (Start + Count));
            }
            else
            {
                char* BlockEnd = Start;

                while (BlockEnd != (End - 1) && !IsBlockCommentEnd(BlockEnd))
                {
                    BlockEnd++;
                }

                if (BlockEnd != (End - 1))
                {
                    BlockEnd += 2; // Skip */
                }
                else
                {
                    BlockEnd = End;
                }

                int Count = End - BlockEnd;
                for (int32 i = 0; i < Count; ++i)
                {
                    Start[i] = BlockEnd[i];
                }

                FMemory::Memzero(Start + Count, End - (Start + Count));
            }
        }
    }
}