// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyStageActor.h"
#include "SimReadyExporterDialog.h"
#include "SimReadyEditorExportUtils.h"
#include "SimReadyPathHelper.h"
#include "ISimReadyRuntimeModule.h"
#include "SimReadyNotificationHelper.h"
#include "LevelEditor.h"
#include "ContentBrowserModule.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EngineUtils.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "SimReadyImporterDialog.h"
#include "SimReadyUSDImporter.h"

#include "IContentBrowserDataModule.h"
#include "ContentBrowserDataFilter.h"
#include "ContentBrowserDataSource.h"
#include "ContentBrowserDataSubsystem.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "EditorDirectories.h"
#include "SimReadyAssetImportHelper.h"
#ifdef USE_EDITORSCRIPTINGUTILITIES
#include "EditorAssetLibrary.h"
#endif

#define IMPORT_USD_AS_UASSET_FEATURE_ENABLED 1

void InitializeLevelEditorMenu()
{
    // Add SimReady menu section
    TSharedRef< FExtender > Extender(new FExtender());

    Extender->AddMenuExtension(
        "FileLoadAndSave",
        EExtensionHook::After,
        MakeShareable(new FUICommandList),
        FMenuExtensionDelegate::CreateLambda(
            [](FMenuBuilder& MenuBuilder)
            {
                MenuBuilder.BeginSection("FileSimReady", FText::FromString(TEXT("SimReady")));
                MenuBuilder.EndSection();
            }
        )
    );

    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(Extender);
}

bool GetLocalExportPath(const TArray<UObject*>& Objects, FString& OutPath)
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform)
    {		
        if (Objects.Num() > 1)
        {
            bool Success = DesktopPlatform->OpenDirectoryDialog(
                FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
                NSLOCTEXT("SimReady", "ChooseADirectory", "Choose A Directory").ToString(),
                FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_EXPORT), OutPath);
            if (Success)
            {
                FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_EXPORT, OutPath);
                return true;
            }

        }
        else if (Objects.Num() == 1)
        {
            TArray<FString> OutFilenames;
            FString FileTypes(TEXT("USD files (*.usd; *.usda)|*.usd; *.usda"));
            bool Success = DesktopPlatform->SaveFileDialog(
                FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
                NSLOCTEXT("SimReady", "ChooseAFile", "Choose A File").ToString(),
                FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_EXPORT), Objects[0]->GetName(), FileTypes, EFileDialogFlags::None, OutFilenames);
            if (Success)
            {
                FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_EXPORT, FPaths::GetPath(OutFilenames[0]));
                OutPath = OutFilenames[0];
                return true;
            }
        }
    }							
    return false;
}

void InitializeContenBrowserMenu()
{
    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Add(FContentBrowserMenuExtender_SelectedAssets::CreateLambda([=](const TArray<FAssetData>& SelectedAssets)
        {
            TSharedRef<FExtender> Extender = MakeShared<FExtender>();
            Extender->AddMenuExtension(
                "CommonAssetActions",
                EExtensionHook::Before,
                TSharedPtr<FUICommandList>(),
                FMenuExtensionDelegate::CreateLambda(
                    [SelectedAssets](FMenuBuilder& MenuBuilder)
                    {
                        if(SelectedAssets.Num() == 0)
                        {
                            return;
                        }

                        TArray<UObject*> SelectedObjects;
                        for(auto Asset : SelectedAssets)
                        {
                            SelectedObjects.Add(Asset.GetAsset());
                        }

                        if(FSimReadyEditorExportUtils::AreObjectsFromTheSameClass(SelectedObjects))
                        {
                            MenuBuilder.BeginSection("SimReadyAssetActions", FText::FromString("SimReady"));

                            if(SelectedObjects[0]->IsA<UWorld>())
                            {
                                MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Export as USD")), FText(), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda(
                                    [SelectedObjects]()
                                    {
                                        FString OutPath;
                                        if (GetLocalExportPath(SelectedObjects, OutPath))
                                        {
                                            TArray<UObject*> WorldObjects;
                                            for (auto Object : SelectedObjects)
                                            {
                                                WorldObjects.Add(Object);
                                            }

                                            if (WorldObjects.Num() > 0)
                                            {
                                                SSimReadyExporterDialog::ShowDialog(EDialogType::Level, WorldObjects, OutPath);
                                            }
                                        }
                                    }
                                )));
                            }
                            else
                            {
                                MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Export as USD")), FText(), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda(
                                    [SelectedObjects]()
                                    {
                                        FString OutPath;
                                        if (GetLocalExportPath(SelectedObjects, OutPath))
                                        {
                                            if (SelectedObjects[0]->IsA<UTexture>())
                                            {
                                                SSimReadyExporterDialog::ShowDialog(EDialogType::Texture, SelectedObjects, OutPath);
                                            }
                                            else if (SelectedObjects[0]->IsA<UMaterialInterface>())
                                            {
                                                SSimReadyExporterDialog::ShowDialog(EDialogType::Material, SelectedObjects, OutPath);
                                            }
                                            else if (SelectedObjects[0]->IsA<UAnimSequence>() || SelectedObjects[0]->IsA<UAnimMontage>())
                                            {
                                                SSimReadyExporterDialog::ShowDialog(EDialogType::Animation, SelectedObjects, OutPath);
                                            }
                                            else
                                            {
                                                SSimReadyExporterDialog::ShowDialog(EDialogType::Object, SelectedObjects, OutPath);
                                            }
                                        }
                                    }
                                )));
                            }
                        }

                        MenuBuilder.EndSection();
                    }
                )
            );

            return Extender;
        }));

    ContentBrowserModule.GetAllAssetContextMenuExtenders().Add(FContentBrowserMenuExtender_SelectedPaths::CreateLambda([=](const TArray<FString>& SelectedPaths)
        {
            TSharedRef<FExtender> Extender = MakeShared<FExtender>();
            Extender->AddMenuExtension(
                "ContentBrowserNewFolder",
                EExtensionHook::Before,
                TSharedPtr<FUICommandList>(),
                FMenuExtensionDelegate::CreateLambda(
                    [SelectedPaths](FMenuBuilder& MenuBuilder)
                    {
                        MenuBuilder.BeginSection("ContentBrowserSimReady", FText::FromString(TEXT("SimReady")));
#if IMPORT_USD_AS_UASSET_FEATURE_ENABLED
                        MenuBuilder.AddMenuEntry(
                            FText::FromString("Import USD"),
                            FText::FromString("Import USD as Unreal asset."),
                            FSlateIcon(),
                            FUIAction(FExecuteAction::CreateLambda(
                                [SelectedPaths]()
                                {
                                    for(const FString& Path : SelectedPaths)
                                    {
                                        SSimReadyImporterDialog::ShowDialog(ESimReadyImporterType::OIT_USD, Path);
                                        break;
                                    }
                                }
                            )));
#endif

                        MenuBuilder.AddMenuEntry(
                            FText::FromString(TEXT("Import USD Animation")),
                            FText::FromString(TEXT("Import USD animation as Unreal sequence.")),
                            FSlateIcon(),
                            FUIAction(FExecuteAction::CreateLambda(
                                [SelectedPaths]()
                                {
                                    for (const FString& Path : SelectedPaths)
                                    {
                                        SSimReadyImporterDialog::ShowDialog(ESimReadyImporterType::OIT_Animation, Path);
                                        break;
                                    }
                                }
                        )));

                        MenuBuilder.EndSection();
                    }
                )
            );

            return Extender;
        }));
}

static TSharedRef<FExtender> ExtendLevelViewportContextMenuForSimReady(const TSharedRef<FUICommandList> CommandList, TArray<AActor*> SelectedActors)
{
    TSharedPtr<FExtender> Extender = MakeShareable(new FExtender);

    Extender->AddMenuExtension("LevelViewportAttach", EExtensionHook::Before, CommandList,
        FMenuExtensionDelegate::CreateLambda(
            [SelectedActors](FMenuBuilder& MenuBuilder)
            {
                MenuBuilder.BeginSection("SimReady", FText::FromString(TEXT("SimReady")));
                MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Export as USD")), FText(), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda(
                    [SelectedActors]()
                    {
                        TArray<UObject*> SelectedObjects;
                        for (AActor* SelectedActor : SelectedActors)
                        {
                            SelectedObjects.Add(Cast<UObject>(SelectedActor));
                        }
                        FString OutPath;
                        if (GetLocalExportPath(SelectedObjects, OutPath))
                        {
                            SSimReadyExporterDialog::ShowDialog(SelectedActors, OutPath);
                        }
                    }
                )));
                MenuBuilder.EndSection();
            }
        )
    );

    return Extender.ToSharedRef();
}

TFunction<void()> InitializeViewportMenu()
{
    static auto LevelViewportContextMenuSimReadyExtender = FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateStatic(&ExtendLevelViewportContextMenuForSimReady);
    FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    auto& MenuExtenders = LevelEditorModule.GetAllLevelViewportContextMenuExtenders();
    MenuExtenders.Add(LevelViewportContextMenuSimReadyExtender);
    auto LevelViewportContextMenuSimReadyExtenderDelegateHandle = MenuExtenders.Last().GetHandle();

    return [LevelViewportContextMenuSimReadyExtenderDelegateHandle]()
    {
        if(FModuleManager::Get().IsModuleLoaded("LevelEditor"))
        {
            FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
            LevelEditorModule.GetAllLevelViewportContextMenuExtenders().RemoveAll([&](const FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors& Delegate) {
                return Delegate.GetHandle() == LevelViewportContextMenuSimReadyExtenderDelegateHandle;
                });
        }
    };
}
