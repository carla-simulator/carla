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

#include "SimReadyExportSettings.h"

#include "SimReadyExportCommandlet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSimReadyExportCommandlet, Log, All);

UCLASS()
class USimReadyExportCommandlet : public UCommandlet
{
    GENERATED_UCLASS_BODY()

public:

    USimReadyExportCommandlet();

    //~ Begin UCommandlet Interface
    int32 Main(const FString& Params) override;
    void CreateCustomEngine(const FString& Params) override;
    //~ End UCommandlet Interface

private:

    /** The list of switches that were passed on the commandline */
    TArray<FString> Switches;

    /** The USD path of the export. */
    FString USDPath{};

    /** The USD file name. */
    FString USDFile{};

    /** Commandlet USD settings. */
    FSimReadyExportSettings Settings{};

    /** Convert actors to StaticMeshActors. */
    bool bConvertToStaticMeshActors{ false };

    /** Emit log messages. */
    bool bVerboseLogMessages{ !(UE_BUILD_SHIPPING || UE_BUILD_TEST) };

    /** Call BeginPlay. */
    bool bUseBeginPlay{ true };

    /** Test only. */
    bool bTestOnly{ false };

    /** Maps Package Names. */
    TArray<FString> MapNames;

    /** Initialize Commandlet. */
    void InitializeExportParameters();

    /** Create/Reset USD file before exporting. */
    void CreateUSD();

    /** Export Level */
    bool ExportWorldToLocalDisk(UWorld* World);

    /** Export Actors. */
    bool ExportActorsAsUSDToPath(const TArray<class AActor*>& Actors, const FString& ExportUSDPath);
    bool ExportActorsToLocalDisk(const TArray<AActor*>& SelectedActors);
};


UCLASS(config = Engine, transient)
class USimReadyEngine : public UUnrealEdEngine
{
    GENERATED_UCLASS_BODY()

public:

    USimReadyEngine();

    //~ Begin UEditorEngine Interface
    bool ShouldThrottleCPUUsage() const override;
    //~ End UEditorEngine Interface
};
