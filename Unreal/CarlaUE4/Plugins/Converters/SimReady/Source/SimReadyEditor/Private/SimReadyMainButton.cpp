// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyMainButton.h"
#include "SimReadySettings.h"
#include "ISimReadyRuntimeModule.h"
#include "LevelEditor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Interfaces/IMainFrameModule.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SUserWidget.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "SimReadyEditorExportUtils.h"
#include "SimReadyExporterDialog.h"
#include "SimReadyStageActor.h"
#include "SimReadyNotificationHelper.h"
#include "SimReadyMaterialReparentDialog.h"

static TSharedPtr<SHorizontalBox> NotificationUsersBox = nullptr;

TSharedPtr<SHorizontalBox> FSimReadyMainButton::GetUserBox()
{
    return NotificationUsersBox;
}

void FSimReadyMainButton::Initialize()
{
    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    TSharedPtr<FExtender> Extender = MakeShared<FExtender>();
    LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(Extender);

    Extender->AddToolBarExtension("File", EExtensionHook::After, MakeShared<FUICommandList>(), FToolBarExtensionDelegate::CreateLambda(
        [this](FToolBarBuilder& ToolBarBuilder)
        {
            ToolBarBuilder.AddComboButton(
                FUIAction(),
                FOnGetContent::CreateSP(this, &FSimReadyMainButton::GetMenu),
                TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &FSimReadyMainButton::GetButtonText)),
                TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &FSimReadyMainButton::GetTooltip)),
                TAttribute<FSlateIcon>::Create(TAttribute<FSlateIcon>::FGetter::CreateSP(this, &FSimReadyMainButton::GetButtonIcon))
            );
        }
    ));

    TSharedPtr<FExtender> NotificationExtender = MakeShared<FExtender>();
    NotificationExtender->AddToolBarExtension("Start", EExtensionHook::After, MakeShared<FUICommandList>(), FToolBarExtensionDelegate::CreateLambda(
        [this](FToolBarBuilder& ToolBarBuilder)
        {
            NotificationUsersBox = SNew(SHorizontalBox);
            ToolBarBuilder.AddWidget(
                NotificationUsersBox.ToSharedRef()
            );
        }
    ));
    LevelEditorModule.GetNotificationBarExtensibilityManager()->AddExtender(NotificationExtender);
    LevelEditorModule.BroadcastNotificationBarChanged();
}

FText FSimReadyMainButton::GetTooltip()
{
    auto Tooltip = FString(TEXT("SimReady Converter Plugin"));
    return FText::FromString(Tooltip);
}

FText FSimReadyMainButton::GetButtonText()
{
    return FText::FromString("SimReady");
}

FSlateIcon FSimReadyMainButton::GetButtonIcon()
{
    return FSlateIcon("SimReadyEditorStyle", "MainButton.SimReady");
}

TSharedRef<SWidget> FSimReadyMainButton::GetMenu()
{
    FMenuBuilder MenuBuilder(true, nullptr);

    MenuBuilder.AddMenuEntry(
        FText::FromString("Material Reparent"),
        FText::FromString(""),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateSP(this, &FSimReadyMainButton::ShowMaterialReparent))
    );

    return MenuBuilder.MakeWidget();
}

void FSimReadyMainButton::ShowMaterialReparent()
{
    SSimReadyMaterialReparentDialog::ShowDialog();
}
