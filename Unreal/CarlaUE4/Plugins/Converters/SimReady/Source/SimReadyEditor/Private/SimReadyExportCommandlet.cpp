// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyExportCommandlet.h"

#include "Animation/SkeletalMeshActor.h"
#include "AssetData.h"
#include "AssetRegistryModule.h"
#include "CoreGlobals.h"
#include "DirectoryWatcherModule.h"
#include "Editor/EditorEngine.h"
#include "Editor/GroupActor.h"
#include "Editor/UnrealEdTypes.h"
#include "Engine/Brush.h"
#include "Engine/DirectionalLight.h"
#include "Engine/EngineTypes.h"
#include "Engine/Font.h"
#include "Engine/Level.h"
#include "Engine/LevelStreaming.h"
#include "Engine/World.h"
#include "EngineGlobals.h"
#include "EngineUtils.h"
#include "Features/IModularFeatures.h"
#include "FileHelpers.h"
#include "Framework/Application/SlateApplication.h"
#include "IAssetTools.h"
#include "IDirectoryWatcher.h"
#include "Logging/LogMacros.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionLandscapeVisibilityMask.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MeshMergeDataTracker.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/OutputDeviceConsole.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Model.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "PackageHelperFunctions.h"
#include "Particles/ParticleSystemComponent.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Settings/EditorProjectSettings.h"
#include "ShaderCompiler.h"
#include "StaticMeshAttributes.h"
#include "Stats/Stats.h"
#include "UnrealEdGlobals.h"
#include "UnrealEdMisc.h"
#include "UnrealEngine.h"
#include "UnrealExporter.h"
#include "UObject/Class.h"
#include "UObject/GarbageCollection.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Package.h"
#include "UObject/UObjectIterator.h"

#include "ISimReadyRuntimeModule.h"
#include "MeshDescription.h"
#include "SimReadyAssetExportHelper.h"
#include "SimReadyEditorExportUtils.h"
#include "SimReadySettings.h"
#include "SimReadyMDL.h"
#include "SimReadyNotificationHelper.h"
#include "SimReadyPxr.h"
#include "SimReadySettings.h"
#include "SimReadyStageActor.h"
#include "SimReadyTexture.h"
#include "SimReadyPathHelper.h"
#include "SimReadyUSDHelper.h"
#include "SimReadyUSDModule.h"
#include "SimReadyUSDTokens.h"
#include "SimReadyUSDTranslator.h"
#include "SceneTypes.h"
#include "Runtime/Launch/Resources/Version.h"



DEFINE_LOG_CATEGORY(LogSimReadyExportCommandlet);

namespace
{
    int32 Find(FString const& Name, TArray<FString> const& Delimeters = {";", "+", ","})
    {
        int32 CurrentIndex = INDEX_NONE;

        for (const auto& Delim : Delimeters)
        {
            CurrentIndex = FMath::Max(CurrentIndex, Name.Find(Delim));
        }
        
        return CurrentIndex;
    }

} // namespace


USimReadyExportCommandlet::USimReadyExportCommandlet()
    : Super()
{
}

void USimReadyExportCommandlet::InitializeExportParameters()
{
    // Override Commmandlet default settings.
    for (const auto& CurrentSwitch : Switches)
    {
        bool bDefaults;
    
        if (FParse::Bool(*CurrentSwitch, TEXT("OVERRIDE_DEFAULTS="), bDefaults))
        {
            if (bDefaults)
            {
                Settings.bModular = false;
                Settings.bAddExtraExtension = false;
                Settings.bMDL = true;
                Settings.bPreviewSurface = false;
                Settings.bExportAssetToSeperateUSD = true;
                Settings.bExportPhysics = true;
                Settings.bEditLayer = true;
                Settings.MaterialSettings.bCopyTemplate = false;
                Settings.MaterialSettings.TextureSettings.bTextureSource = false;
                Settings.MaterialSettings.TextureSettings.bDDSExport = false;
            }
        }
    }

    // Prevent dialogs.
    GIsRunningUnattendedScript = true;

    // Check to see if we have an explicit list of packages
    for (const auto& CurrentSwitch : Switches)
    {
        FString MapList;
        FString Path;
        FString File;
        FString Section;

        bool bMdl;
        bool bModular;
        bool bPreviewSurface;
        bool bAddExtraExtension;
        bool bSeparateUSD;
        bool bCopyTemplate;
        bool bTextureSource;
        bool bTwoSidedBackfaces;
        bool bDDSExport;
        bool bVerbose;
        bool bMeshes;
        bool bPhysics;
        bool bMeshInstanced;
        bool bExportSublayers;
        bool bPayloads;
        bool bExportDecalActors;
        bool bBeginPlay;
        bool bTest;
        bool bEditLayer;

        if (FParse::Value(*CurrentSwitch, TEXT("MAP="), MapList) || FParse::Value(*CurrentSwitch, TEXT("MAPS="), MapList))
        {
            for (int32 Index = Find(MapList); Index != INDEX_NONE; Index = Find(MapList))
            {
                FString NextMap = MapList.Left(Index);

                FPaths::RemoveDuplicateSlashes(NextMap);
                FPaths::NormalizeDirectoryName(NextMap);

                if (NextMap.Len() > 0)
                {
                    FString MapFile;
                    if (FPackageName::IsShortPackageName(NextMap))
                    {
                        if (FPackageName::SearchForPackageOnDisk(FPaths::GetBaseFilename(NextMap), NULL, &MapFile))
                        {
                            MapNames.AddUnique(MapFile);
                        }
                    }
                    else if (FPackageName::DoesPackageExist(NextMap, NULL, &MapFile))
                    {
                        MapNames.AddUnique(MapFile);
                    }
                }

                MapList = MapList.Right(MapList.Len() - (Index + 1));
            }

            FPaths::RemoveDuplicateSlashes(MapList);
            FPaths::NormalizeDirectoryName(MapList);

            FString MapFile;
            if (FPackageName::IsShortPackageName(MapList))
            {
                if (FPackageName::SearchForPackageOnDisk(FPaths::GetBaseFilename(MapList), NULL, &MapFile))
                {
                    MapNames.AddUnique(MapFile);
                }
            }
            else if (FPackageName::DoesPackageExist(MapList, NULL, &MapFile))
            {
                MapNames.AddUnique(MapFile);
            }
        }
        else if (FParse::Value(*CurrentSwitch, TEXT("PATH="), Path))
        {
            Path = Path.TrimQuotes();
            USDPath = Path;
            FPaths::RemoveDuplicateSlashes(USDPath);
            FPaths::NormalizeDirectoryName(USDPath);

            // Split into filename and path if path contains a filename.
            if (USDPath.Contains(".usd"))
            {
                USDFile = FPaths::GetBaseFilename(File);
                USDPath = FPaths::GetPath(USDPath);
            }
        }
        else if (FParse::Value(*CurrentSwitch, TEXT("USD="), File))
        {
            USDFile = FPaths::GetBaseFilename(File);
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("MDL="), bMdl))
        {
            Settings.bMDL = bMdl;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("MODULAR="), bModular))
        {
            Settings.bModular = bModular;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("PREVIEWSURFACE="), bPreviewSurface))
        {
            Settings.bPreviewSurface = bPreviewSurface;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("ADDEXTRAEXTENSION="), bAddExtraExtension))
        {
            Settings.bAddExtraExtension = bAddExtraExtension;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("SEPARATE="), bSeparateUSD))
        {
            Settings.bExportAssetToSeperateUSD = bSeparateUSD;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("COPYTEMPLATE="), bCopyTemplate))
        {
            Settings.MaterialSettings.bCopyTemplate = bCopyTemplate;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("TEXTURESOURCE="), bTextureSource))
        {
            Settings.MaterialSettings.TextureSettings.bTextureSource = bTextureSource;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("DDSEXPORT="), bDDSExport))
        {
            Settings.MaterialSettings.TextureSettings.bDDSExport = bDDSExport;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("TWOSIDEDBACKFACES="), bTwoSidedBackfaces))
        {
            Settings.MaterialSettings.bExportTwoSidedSign = bTwoSidedBackfaces;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("VERBOSE="), bVerbose))
        {
            bVerboseLogMessages = bVerbose;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("STATICMESHES="), bMeshes))
        {
            bConvertToStaticMeshActors = bMeshes;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("COLLISION="), bPhysics))
        {
            Settings.bExportPhysics = bPhysics;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("INSTANCED="), bMeshInstanced))
        {
            Settings.bMeshInstanced = bMeshInstanced;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("SUBLEVELLAYERS="), bExportSublayers))
        {
            Settings.bExportSublayers = bExportSublayers;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("PAYLOADS="), bPayloads))
        {
            Settings.bPayloads = bPayloads;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("DECALS="), bExportDecalActors))
        {
            Settings.bExportDecalActors = bExportDecalActors;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("BEGINPLAY="), bBeginPlay))
        {
            bUseBeginPlay = bBeginPlay;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("TEST="), bTest))
        {
            bTestOnly = bTest;
        }
        else if (FParse::Bool(*CurrentSwitch, TEXT("EDITLAYER="), bEditLayer))
        {
            Settings.bEditLayer = bEditLayer;
        }
    }

    auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    auto& AssetRegistry = AssetRegistryModule.Get();
    AssetRegistry.SearchAllAssets(true);

    auto& DirectoryWatcherModule = FModuleManager::Get().LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
    DirectoryWatcherModule.Get()->Tick(-1.0F);
}

USimReadyExportCommandlet::USimReadyExportCommandlet(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    HelpDescription = TEXT("SimReady Export Utilities");

    HelpParamNames = TArray<FString>{	"MAP", 
                                        "PATH", 
                                        "USD",
                                        "MDL",
                                        "MODULAR",
                                        "PREVIEWSURFACE",
                                        "ADDEXTRAEXTENSION",
                                        "COPYTEMPLATE",
                                        "TEXTURESOURCE",
                                        "VERBOSE",
                                        "STATICMESHES",
                                        "COLLISION"
    };

    HelpParamDescriptions = TArray<FString>{"The map name (with or without) extension.", 
                                            "The absolute directory where the USD file will be saved.",
                                            "The name of the USD file. Defaults to the Map name.",
                                            "Include MDL in export. Default true.",
                                            "Materials are exported for each asset. Default true.",
                                            "Generate preview surfaces. Default true.",
                                            "Add extra extension to exported file, ie, .stage.usd, .prop.usd. Default false.",
                                            "Copy templates to export directory. Default true.",
                                            "Use the source texture for the texture size. Default true.",
                                            "Emit some messages along with the export for debugging. Default true iff non-shipping.",
                                            "Convert Actors to StaticMeshActors. Default false.",
                                            "Export collision components. Default true."
    };
}

int32 USimReadyExportCommandlet::Main(const FString& Params)
{
    /* We need Engine, Editor, and UnEd defined for this commandlet to work properly. */
    if (!GEngine || !GEditor || !GUnrealEd)
    {
        UE_LOG(LogSimReadyExportCommandlet, Display, TEXT(" We need Engine, Editor, and UnEd defined for this commandlet to work properly."));
        return 0;
    }

    /* Parse commandline. */
    const auto Parms = *Params;
    TArray<FString> Tokens;
    ParseCommandLine(Parms, Tokens, Switches);

    /* Get the parameters required for export. */
    InitializeExportParameters();

    /* No package?  We are done. */
    if (MapNames.Num() == 0)
    {
        UE_LOG(LogSimReadyExportCommandlet, Display, TEXT("No Maps or packages loaded for export!"));
        return 0;
    }

    auto& DirectoryWatcherModule = FModuleManager::Get().LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
    auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    auto& AssetRegistry = AssetRegistryModule.Get();

    // Several paths check the UI, but it is not fatal if the UI is initialized.
    FSlateApplication::Create();

    /* Iterate over all maps */
    for (const auto& Map : MapNames)
    {
        const auto& MapBaseName = FPaths::GetBaseFilename(Map);

        if (MapNames.Num() == 1)
        {
            if (USDFile.IsEmpty())
            {
                Settings.CommandletSettings.CommandletExportPath = FPaths::Combine(USDPath, MapBaseName);
            }
            else
            {
                Settings.CommandletSettings.CommandletExportPath = FPaths::Combine(USDPath, FPaths::GetBaseFilename(USDFile));
            }
        }
        else
        {
            if (USDFile.IsEmpty())
            {
                Settings.CommandletSettings.CommandletExportPath = FPaths::Combine(USDPath, MapBaseName, MapBaseName);
            }
            else
            {
                Settings.CommandletSettings.CommandletExportPath = FPaths::Combine(USDPath, MapBaseName, FPaths::GetBaseFilename(USDFile));
            }
        }

        FPaths::RemoveDuplicateSlashes(Settings.CommandletSettings.CommandletExportPath);
        FPaths::NormalizeDirectoryName(Settings.CommandletSettings.CommandletExportPath);

        if (!USDPath.Contains(".usd"))
        {
            Settings.CommandletSettings.CommandletExportPath += FString(".usd");
        }

        if (bVerboseLogMessages)
        {
            UE_LOG(LogSimReadyExportCommandlet, Display, TEXT("Exporting map %s.usd ..."), *Settings.CommandletSettings.CommandletExportPath);
        }

        FString LoadCommand = FString::Printf(TEXT("MAP LOAD FILE=\"%s\" TEMPLATE=%d SHOWPROGRESS=%d FEATURELEVEL=%d"), *Map, false, false, (int32)GEditor->DefaultWorldFeatureLevel);
        const bool bResult = GEditor->Exec( NULL, *LoadCommand );

        UWorld* World = GWorld;
        if (World)
        {
            // Recreate the USD file
            if (bTestOnly == false)
            {
                if (ExportWorldToLocalDisk(World))
                {
                    UE_LOG(LogSimReadyExportCommandlet, Display, TEXT("%s exported to USD!"), *MapBaseName);
                }
            }
        }
        else
        {
            UE_LOG(LogSimReadyExportCommandlet, Display, TEXT("Failed to load map %s..."), *MapBaseName);
        }
    }

    CollectGarbage(RF_NoFlags);
    
    return 0;
}

/* Create a custom editor object to override certain runtime behavior. */
void USimReadyExportCommandlet::CreateCustomEngine(const FString& Params)
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

bool USimReadyExportCommandlet::ExportActorsAsUSDToPath(const TArray<class AActor*>& Actors, const FString& ExportUSDPath)
{
    // export world to USD
    bool const bCreateOrClearUSDFile = true;

    pxr::UsdStageRefPtr USDStage = FSimReadyUSDHelper::CreateUSDStageFromPath(ExportUSDPath, bCreateOrClearUSDFile, !Settings.bUpYAxis);
    if (!USDStage)
    {
        return false;
    }

    FSimReadyAssetExportHelper::ResetGlobalCaches();

    pxr::SdfPath RootPrimPath = FSimReadyUSDHelper::InitDefaultPrim(USDStage);

    TMap<FString, FString> ExportMeshUSD;
    TMap<FString, FString> ExportMDLs;
    TMap<FString, FString> ExportAnimeUSD;
    for (auto Actor : Actors)
    {
        if (Actor->IsHidden())
        {
            continue;
        }

        if (!FSimReadyUSDHelper::IsActorSupported(Actor, Settings))
        {
            continue;
        }
        USceneComponent* SceneComp = Cast<USceneComponent>(Actor->GetRootComponent());
        const bool bIsRootComp = SceneComp && !SceneComp->GetAttachParent();
        if (bIsRootComp)
        {
            FSimReadyUSDHelper::ExportComponentToUSD(SceneComp, USDStage, nullptr, RootPrimPath, Settings, ExportMeshUSD, ExportMDLs, ExportAnimeUSD);
        }

        TArray<UStaticMeshComponent*> StaticMeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

        if (StaticMeshComponents.Num() > 0)
        {
            for (auto StaticMeshComponent : StaticMeshComponents)
            {
                if (StaticMeshComponent == Actor->GetRootComponent() || StaticMeshComponent->GetAttachParent())
                {
                    continue;
                }

                FSimReadyUSDHelper::ExportComponentToUSD(StaticMeshComponent, USDStage, nullptr, RootPrimPath, Settings, ExportMeshUSD, ExportMDLs, ExportAnimeUSD);
            }
        }
    }

    USDStage->Save();

    return true;
}

bool USimReadyExportCommandlet::ExportWorldToLocalDisk(UWorld* World)
{
    if (Settings.CommandletSettings.CommandletExportPath.Len() == 0) // Save path should not be empty
    {
        return false;
    }

    bool bSuccess = FSimReadyUSDHelper::ExportUWorldAsUSDToPath(World, Settings.CommandletSettings.CommandletExportPath, Settings);
    return bSuccess;
}

bool USimReadyExportCommandlet::ExportActorsToLocalDisk(const TArray<AActor*>& SelectedActors)
{
    if (SelectedActors.Num() == 0)
    {
        return true;
    }

    if (Settings.CommandletSettings.CommandletExportPath.Len() == 0) // Save path should not be empty
    {
        return false;
    }

    bool bSuccess = ExportActorsAsUSDToPath(SelectedActors, Settings.CommandletSettings.CommandletExportPath);
    return bSuccess;
}

/*
 *	USimReadyEngine implementation.
 */

USimReadyEngine::USimReadyEngine()
{
}

USimReadyEngine::USimReadyEngine(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

/* Stubs for potentially crashy behavior using editor components with a commandlet. */
bool USimReadyEngine::ShouldThrottleCPUUsage() const 
{ 
    return false; 
}
