// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "MDLPathUtility.h"

#define ENABLE_MANGLE 1

namespace MDLPathUtility
{
    static FString MangledPathPrefixPkg = TEXT("MANGLED");
    static FString MangledElemPrefix = TEXT("z");
    static FString MDLSuffix = TEXT(".mdl");
    static FString FileSeparator = TEXT("/");
    static FString MDLSeparator = TEXT("::");
    static const FString MangledPathPostfix = TEXT("_MANGLED");

    bool IsValidMdlPathChar(TCHAR c)
    {
        return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'));
    }

    FString MangleMdlPath(const FString& Input, bool bMDLSeparator)
    {
#if ENABLE_MANGLE
        bool bHasSuffix = Input.EndsWith(MDLSuffix);
        FString ModulePath = bHasSuffix ? Input.LeftChop(MDLSuffix.Len()) : Input;

        FString Separator = bMDLSeparator ? MDLSeparator : FileSeparator;
        FString Output;
        FString RelativeOutput;
        bool bStartWithSeparator = ModulePath.StartsWith(Separator);

        TArray<FString> Modules;
        ModulePath.ParseIntoArray(Modules, *Separator);

        for (int32 ModuleIndex = 0; ModuleIndex < Modules.Num(); ++ModuleIndex)
        {
            auto Module = Modules[ModuleIndex];

            if (Module == TEXT(".") || Module == TEXT(".."))
            {
                RelativeOutput += Module + Separator;
            }
            else
            {
                if (Output.IsEmpty())
                {
                    Output += MangledPathPrefixPkg + Separator;
                }

                Output += MangledElemPrefix;
                Output += FString::FromInt(Module.Len());
                Output += MangledElemPrefix;

                auto CharArray = Module.GetCharArray();
                for (int32 Index = 0; Index < CharArray.Num() - 1; ++Index) // Skip the last \0
                {
                    auto Char = CharArray[Index];
                    if (IsValidMdlPathChar(Char))
                    {
                        Output += Char;
                    }
                    else
                    {
                        Output += TEXT("_") + FString::FromInt(Char) + TEXT("_");
                    }
                }

                if (ModuleIndex < Modules.Num() - 1)
                {
                    Output += Separator;
                }
            }
        }

        Output = (bStartWithSeparator ? Separator : RelativeOutput) + Output;
        if (bHasSuffix)
        {
            Output += MDLSuffix;
        }

        return Output;
#else
        return Input;
#endif
    }

    FString UnmangleMdlPath(const FString& Input, bool bMDLSeparator)
    {
#if ENABLE_MANGLE
        FString Output;
        FString RelativeOutput;
        FString Separator = bMDLSeparator ? MDLSeparator : FileSeparator;
        bool bHasSuffix = Input.EndsWith(MDLSuffix);
        FString ModulePath = bHasSuffix ? Input.LeftChop(MDLSuffix.Len()) : Input;
        bool bStartWithSeparator = ModulePath.StartsWith(Separator);

        TArray<FString> Modules;
        ModulePath.ParseIntoArray(Modules, *Separator);

        bool bPrefixPkgLocated = false;
        for (int32 ModuleIndex = 0; ModuleIndex < Modules.Num(); ++ModuleIndex)
        {
            auto Module = Modules[ModuleIndex];

            if (Module == TEXT(".") || Module == TEXT(".."))
            {
                RelativeOutput += Module + Separator;
            }
            else if (Module == MangledPathPrefixPkg)
            {
                bPrefixPkgLocated = true;
            }
            else
            {
                // prefix wasn't found
                if (!bPrefixPkgLocated)
                {
                    return Input;
                }

                // element prefix wasn't found
                if (!Module.StartsWith(MangledElemPrefix))
                {
                    Output += Module;
                }
                else
                {
                    FString MangledModule = Module.RightChop(MangledElemPrefix.Len());

                    int32 PrefixIndex = MangledModule.Find(MangledElemPrefix);
                    if (PrefixIndex == INDEX_NONE)
                    {
                        Output += Module;
                    }
                    else
                    {
                        TCHAR* End = nullptr;
                        int32 CheckSize = FCString::Strtoi(*MangledModule.Left(PrefixIndex), &End, 10);
                        MangledModule = MangledModule.RightChop(PrefixIndex + MangledElemPrefix.Len());
                        auto CharArray = MangledModule.GetCharArray();
                        bool bUnmangleChar = false;
                        FString MangledChar;
                        FString UnmangledModule;
                        for (int32 Index = 0; Index < CharArray.Num() - 1; ++Index) // Skip the last \0
                        {
                            auto Char = CharArray[Index];
                            if (Char == '_')
                            {
                                if (!bUnmangleChar)
                                {
                                    MangledChar.Empty();
                                    bUnmangleChar = true;
                                }
                                else
                                {
                                    UnmangledModule += TCHAR(FCString::Strtoi(*MangledChar, &End, 10));
                                    bUnmangleChar = false;
                                }
                            }
                            else if (bUnmangleChar)
                            {
                                MangledChar += Char;
                            }
                            else
                            {
                                UnmangledModule += Char;
                            }
                        }

                        if (UnmangledModule.Len() == CheckSize)
                        {
                            Output += UnmangledModule;
                        }
                        else
                        {
                            Output += Module;
                        }
                    }
                }

                if (ModuleIndex < Modules.Num() - 1)
                {
                    Output += Separator;
                }
            }
        }

        Output = (bStartWithSeparator ? Separator : RelativeOutput) + Output;
        if (bHasSuffix)
        {
            Output += MDLSuffix;
        }
        return Output;
#else
        return Input;
#endif
    }

    bool IsValidUE4PathChar(TCHAR c)
    {
        const FString InvalidChars = INVALID_LONGPACKAGE_CHARACTERS TEXT("/[]");
        FString Char;
        Char += c;
        return !(InvalidChars.Contains(Char));
    }

    FString MangleAssetPath(const FString& UnMangledPath)
    {
        FString MangledPath;
        TArray<FString> OutFolderNames;
        UnMangledPath.ParseIntoArray(OutFolderNames, TEXT("::"));

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

            MangledPath += TEXT("::") + MangledFolderName;
        }

        return MangledPath;
    }

    FString UnmangleAssetPath(const FString& InPath, bool bStartWithSlash)
    {
        FString OutPath;

        TArray<FString> OutFolderNames;
        InPath.ParseIntoArray(OutFolderNames, TEXT("/"));

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

                OutPath += ((OutPath.IsEmpty() && !bStartWithSlash) ? TEXT("") : TEXT("/")) + MangledFolderName;
            }
            else if (FolderName.EndsWith(TEXT(":")))
            {
                OutPath += FolderName + TEXT("/");
            }
            else
            {
                OutPath += ((OutPath.IsEmpty() && !bStartWithSlash) ? TEXT("") : TEXT("/")) + FolderName;
            }
        }

        return OutPath;
    }
}