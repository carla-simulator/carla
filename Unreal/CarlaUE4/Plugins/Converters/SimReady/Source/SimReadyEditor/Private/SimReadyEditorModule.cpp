// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyEditorModule.h"
#include "SimReadyEditorPrivate.h"
#include "ISettingsModule.h"
#include "AssetToolsModule.h"
#include "ISettingsSection.h"
#include "Editor/EditorPerformanceSettings.h"
#include "Settings/ContentBrowserSettings.h"
#include "Logging/LogMacros.h"
#include "SimReadySettings.h"
#include "SimReadyMainButton.h"
#include "SimReadyImporterDetails.h"
#include "SimReadyReparentMaterialInfoTypeCustomization.h"
#include "SimReadyReparentParameterInfoTypeCustomization.h"
#include "SimReadyReparentConstantInfoTypeCustomization.h"
#include "SimReadyImporterUI.h"
#include "SimReadyMaterialReparentSettings.h"
#include "PropertyEditorModule.h"

DEFINE_LOG_CATEGORY(LogSimReadyEditor);
#define LOCTEXT_NAMESPACE "SimReadyEditor"

void FSimReadyEditorModule::StartupModule()
{
    // Register settings
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings(
            "Project", "Plugins", "SimReadySettings",
            FText::FromString("SimReady"),
            FText::FromString("Configure SimReady settings"),
            GetMutableDefault<USimReadySettings>());
    }

    // Editor menu extension
    extern void InitializeLevelEditorMenu();
    extern void InitializeContenBrowserMenu();
    extern TFunction<void()> InitializeViewportMenu();
    InitializeLevelEditorMenu();
    InitializeContenBrowserMenu();
    ShutdownViewportExtension = InitializeViewportMenu();

    // Main button UI
    MainButton = MakeShared<FSimReadyMainButton>();
    MainButton->Initialize();

    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout(USimReadyImporterUI::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FSimReadyImporterDetails::MakeInstance));
    PropertyModule.RegisterCustomPropertyTypeLayout(FSimReadyReparentMaterialInfo::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSimReadyReparentMaterialInfoTypeCustomization::MakeInstance));
    PropertyModule.RegisterCustomPropertyTypeLayout(FSimReadyReparentParameterInfo::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSimReadyReparentParameterInfoTypeCustomization::MakeInstance));
    PropertyModule.RegisterCustomPropertyTypeLayout(FSimReadyReparentConstantInfo::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSimReadyReparentConstantInfoTypeCustomization::MakeInstance));
    PropertyModule.NotifyCustomizationModuleChanged();
}

void FSimReadyEditorModule::ShutdownModule()
{
    // Remove level viewport context menu extenders
    if(ShutdownViewportExtension)
    {
        ShutdownViewportExtension();
    }

    if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

        // Unregister all classes customized by name
        PropertyModule.UnregisterCustomClassLayout(USimReadyImporterUI::StaticClass()->GetFName());
        PropertyModule.UnregisterCustomPropertyTypeLayout(FSimReadyReparentMaterialInfo::StaticStruct()->GetFName());
        PropertyModule.UnregisterCustomPropertyTypeLayout(FSimReadyReparentParameterInfo::StaticStruct()->GetFName());
        PropertyModule.UnregisterCustomPropertyTypeLayout(FSimReadyReparentConstantInfo::StaticStruct()->GetFName());
        PropertyModule.NotifyCustomizationModuleChanged();
    }
}

IMPLEMENT_MODULE(FSimReadyEditorModule, SimReadyEditor)

#undef LOCTEXT_NAMESPACE