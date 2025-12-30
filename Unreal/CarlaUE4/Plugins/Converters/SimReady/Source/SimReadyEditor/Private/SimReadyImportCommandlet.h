// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "Commandlets/Commandlet.h"

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Logging/LogMacros.h"
#include "UObject/ObjectMacros.h"
#include "Editor/UnrealEdEngine.h"

#include "SimReadyImportSettings.h"

#include "SimReadyImportCommandlet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSimReadyImportCommandlet, Log, All);

UCLASS()
class USimReadyImportCommandlet : public UCommandlet
{
    GENERATED_UCLASS_BODY()

public:

    USimReadyImportCommandlet();

    //~ Begin UCommandlet Interface
    int32 Main(const FString& Params) override;
    void CreateCustomEngine(const FString& Params) override;
    //~ End UCommandlet Interface

private:

    /** Prints command line arguments */
    void PrintUsage();

    /** Initialize Commandlet. */
    bool InitializeImportParameters();

    /** The list of switches that were passed on the commandline */
    TArray<FString> Switches;

    /** The source USD path of the import. */
    FString SourceUsdPath{};

    /** The destination content folder path for the imported assets */
    FString DestContentPath{};

    /** Commandlet USD settings. */
    FSimReadyImportSettings Settings{};

    /** Save imported packages */
    bool bSaveImportedPackages{ true };
};
