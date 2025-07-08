// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyImportCommandlet.h"

#include "CoreGlobals.h"
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEdTypes.h"
#include "EngineGlobals.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "Logging/LogMacros.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "SourceControlHelpers.h"
#include "UnrealEdGlobals.h"
#include "UnrealEngine.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY(LogSimReadyImportCommandlet);

USimReadyImportCommandlet::USimReadyImportCommandlet()
    : Super()
{
}

void USimReadyImportCommandlet::PrintUsage()
{
    UE_LOG(LogSimReadyImportCommandlet, Display, TEXT("Usage: LogSimReadyImportCommandlet {arglist}"));
    UE_LOG(LogSimReadyImportCommandlet, Display, TEXT("Arglist:"));

    UE_LOG(LogSimReadyImportCommandlet, Display, TEXT("-help or -?"));
    UE_LOG(LogSimReadyImportCommandlet, Display, TEXT("\tDisplays this help"));

    UE_LOG(LogSimReadyImportCommandlet, Display, TEXT("-source=\"path\""));
    UE_LOG(LogSimReadyImportCommandlet, Display, TEXT("\tThe absolute path for a source USD file to import (required)."));

    UE_LOG(LogSimReadyImportCommandlet, Display, TEXT("-dest=\"path\""));
    UE_LOG(LogSimReadyImportCommandlet, Display, TEXT("\tThe destination path in the project's content directory to import to (required - /Game/)."));
}

bool USimReadyImportCommandlet::InitializeImportParameters()
{
    // Prevent dialogs.
    GIsRunningUnattendedScript = true;

    for (auto const& CurrentSwitch : Switches)
    {
        FString StringValue;
        bool bBoolValue;

        if (FParse::Value(*CurrentSwitch, TEXT("Dest="), StringValue))
        {
            DestContentPath = StringValue.TrimQuotes();
            FPaths::RemoveDuplicateSlashes(DestContentPath);
            FPaths::NormalizeDirectoryName(DestContentPath);

            if (!DestContentPath.StartsWith(TEXT("/Game/")))
            {
                UE_LOG(LogSimReadyImportCommandlet, Error, TEXT("The content folder path must be under the /Game content folder: %s"), *DestContentPath);
                PrintUsage();
                return false;
            }
        }
        else if (FParse::Value(*CurrentSwitch, TEXT("SOURCE="), StringValue))
        {
            SourceUsdPath = StringValue.TrimQuotes();
            FPaths::RemoveDuplicateSlashes(SourceUsdPath);
            FPaths::NormalizeDirectoryName(SourceUsdPath);

            // Split into filename and path if path contains a filename.
            if (!SourceUsdPath.Contains(".usd") || 
                !FPaths::FileExists(SourceUsdPath) || 
                FPaths::IsRelative(SourceUsdPath)
            )
            {
                UE_LOG(LogSimReadyImportCommandlet, Error, TEXT("The USD file source path must be a absolute and a valid .usd[a,c] file: %s"), *SourceUsdPath);
                PrintUsage();
                return false;
            }
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("SAVE="), bBoolValue))
        {
            bSaveImportedPackages = bBoolValue;
        }
    }

    // Error conditions
    if (DestContentPath.IsEmpty())
    {
        UE_LOG(LogSimReadyImportCommandlet, Error, TEXT("\tThe content folder path under the /Game content folder must be specified"));
        PrintUsage();
        return false;
    }
    else if (SourceUsdPath.IsEmpty())
    {
        UE_LOG(LogSimReadyImportCommandlet, Error, TEXT("\tThe absolute USD file source path must be specified"));
        PrintUsage();
        return false;
    }	

    return true;
}

USimReadyImportCommandlet::USimReadyImportCommandlet(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    HelpDescription = TEXT("SimReady Import Utility");

    HelpParamNames = TArray<FString>{	"SOURCE", 
                                        "DEST", 
                                        "HELP"
    };

    HelpParamDescriptions = TArray<FString>{"The absolute path for a source USD file to import (required).", 
                                            "The destination path in the project's content directory to import to (required - /Game/).",
                                            "Print usage directions."
    };
}

int32 USimReadyImportCommandlet::Main(const FString& Params)
{
    /* We need Engine, Editor, and UnEd defined for this commandlet to work properly. */
    if (!GEngine || !GEditor || !GUnrealEd)
    {
        UE_LOG(LogSimReadyImportCommandlet, Display, TEXT(" We need Engine, Editor, and UnEd defined for this commandlet to work properly."));
        return 0;
    }

    /* Parse commandline. */
    const auto Parms = *Params;
    TArray<FString> Tokens;
    ParseCommandLine(Parms, Tokens, Switches);

    if( Params.Contains(TEXT("?")) || Params.Contains(TEXT("help") ) )
    {
        PrintUsage();
        return 0;
    }	

    /* Get the parameters required for import. */
    if (!InitializeImportParameters())
    {
        return -1;
    }

    // Several paths check the UI, but it is not fatal if the UI is initialized.
    // This crashes trying to display the Normal Map notification from NormalMapIdentification::HandleAssetPostImport()
    // UI not required if we don't open the blueprint asset from the SimReadyStageActor
    // FSlateApplication::Create();

    UE_LOG(LogSimReadyImportCommandlet, Display, TEXT("SimReady Import Commandlet starting..."));

    Settings.bImportAsBlueprint = true;

    if (!FSimReadyUSDImporter::LoadUSD(SourceUsdPath, DestContentPath, Settings))
    {
        UE_LOG(LogSimReadyImportCommandlet, Error, TEXT("Error importing: %s"), *SourceUsdPath);
        return -1;
    }
    else
    {
        UE_LOG(LogSimReadyImportCommandlet, Display, TEXT("Imported %s"), *SourceUsdPath);

        if (bSaveImportedPackages)
        {
            TArray<UPackage*> DirtyPackages;
            FEditorFileUtils::GetDirtyContentPackages(DirtyPackages);
            FEditorFileUtils::GetDirtyWorldPackages(DirtyPackages);

            FSavePackageArgs SaveArgs;
            SaveArgs.TopLevelFlags = RF_Standalone;
            SaveArgs.Error = GWarn;
            for (int32 PackageIndex = 0; PackageIndex < DirtyPackages.Num(); ++PackageIndex)
            {
                UPackage* PackageToSave = DirtyPackages[PackageIndex];
                FString PackageFilename = SourceControlHelpers::PackageFilename(PackageToSave);
                UE_LOG(LogSimReadyImportCommandlet, Display, TEXT("PackageFilename %s"), *PackageFilename);
                GEditor->SavePackage(PackageToSave, nullptr, RF_Standalone, *PackageFilename, GWarn);
            }
        }
    }

    CollectGarbage(RF_NoFlags);

    return 0;
}

/* Create a custom editor object to override certain runtime behavior. */
void USimReadyImportCommandlet::CreateCustomEngine(const FString& Params)
{
    // Various settings to allow editor behavior in the Commandlet.
    GIsRunningUnattendedScript = true;
    PRIVATE_GAllowCommandletRendering = true;
    PRIVATE_GAllowCommandletAudio = true;

    IsEditor = true;
    IsClient = false;
    IsServer = false;

    // Custom editor engine.
    GEngine = GEditor = GUnrealEd = NewObject<UUnrealEdEngine>(GetTransientPackage(), USimReadyEngine::StaticClass());
    GEngine->ParseCommandline();
    GEditor->InitEditor(nullptr);
}
