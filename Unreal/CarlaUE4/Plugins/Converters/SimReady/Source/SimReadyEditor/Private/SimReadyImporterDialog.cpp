// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyImporterDialog.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"

#include "Editor.h"
#include "EditorStyleSet.h"
#include "EditorDirectories.h"

#include "DesktopPlatformModule.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IMainFrameModule.h"
#include "Misc/MessageDialog.h"

#include "SimReadyAnimationImporter.h"
#include "SimReadyUSDImporter.h"

#define LOCTEXT_NAMESPACE "SimReadyEditor"

void SSimReadyImporterDialog::ConstructDialog()
{
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bAllowSearch = false;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    DetailsView->SetObject(ImporterUI);

    TSharedPtr<SUniformGridPanel> ConfirmWidget =
            SNew(SUniformGridPanel)
            .SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
            .MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
            .MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
            + SUniformGridPanel::Slot(0, 0)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
                .Text(LOCTEXT("OK", "OK"))
                .OnClicked(this, &SSimReadyImporterDialog::OkClicked)
                .IsEnabled(this, &SSimReadyImporterDialog::IsImporterReady)
            ]
            + SUniformGridPanel::Slot(1, 0)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
                .Text(LOCTEXT("Cancel", "Cancel"))
                .OnClicked(this, &SSimReadyImporterDialog::CancelClicked)
            ];

    const int32 ClientSizeX = 500;
    const int32 ClientSizeY = ImporterUI->ImporterType == OIT_Animation ? 400 : 400;
    TSharedPtr<SBox> Box;
    const FText TitleText = ImporterUI->ImporterType == OIT_Animation ? LOCTEXT("SimReadyImporterDialogTitle", "SimReady USD Animation Importer") : LOCTEXT("SimReadyImporterDialogTitle", "SimReady USD Importer");
    
    SWindow::Construct(SWindow::FArguments()
        .Title(TitleText)
        .SizingRule(ESizingRule::UserSized)
        .ClientSize(FVector2D(ClientSizeX, ClientSizeY))
        .SupportsMinimize(false)
        .SupportsMaximize(false)
        .bDragAnywhere(false)
        .ScreenPosition(FVector2D((float)(GEditor->GetActiveViewport()->GetSizeXY().X) / 2.0, (float)(GEditor->GetActiveViewport()->GetSizeXY().Y) / 2.0))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(2)
            [
                SAssignNew(Box, SBox)
                .MaxDesiredHeight(ClientSizeY)
                .WidthOverride(ClientSizeX)
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            .HAlign(HAlign_Center)
            .Padding(30)
            [
                ConfirmWidget.ToSharedRef()
            ]
        ]
    );

    Box->SetContent(DetailsView->AsShared());
}

void SSimReadyImporterDialog::Construct(const FArguments& InArgs)
{
    ImporterUI = InArgs._ImporterUI;
    ConstructDialog();
}

bool SSimReadyImporterDialog::IsImporterReady() const
{
    if (ImporterUI->ImporterType == OIT_Animation)
    {
        return ImporterUI->Skeleton && !ImporterUI->USDPath.IsEmpty() && !ImporterUI->AssetPath.IsEmpty();
    }
    else
    {
        return !ImporterUI->USDPath.IsEmpty() && !ImporterUI->AssetPath.IsEmpty();
    }
}

FReply SSimReadyImporterDialog::OkClicked()
{
    RequestDestroyWindow();

    TArray<FString> Files;
    ImporterUI->USDPath.ParseIntoArray(Files, TEXT(";"), true);

    FString FailedFiles;

    if (ImporterUI->ImporterType == OIT_Animation)
    {
        auto FailedFilesList = FSimReadyAnimationImporter::LoadUSDAnimation(Files, ImporterUI->AssetPath, ImporterUI->Skeleton, ImporterUI->bUseFileName ? TEXT("__FILENAME__") : ImporterUI->Name, ImporterUI->AnimationSource);
        for (auto File : FailedFilesList)
        {
            if (!FailedFiles.IsEmpty())
            {
                FailedFiles += ";";
            }
            FailedFiles += File;
        }
    }
    else
    {
        FSimReadyImportSettings ImportSettings;
        ImportSettings.bImportUnusedReferences = ImporterUI->bImportUnusedReferences;
        ImportSettings.bImportAsBlueprint = ImporterUI->bImportAsBlueprint;
        ImportSettings.bImportLights = ImporterUI->bImportLights;
        ImportSettings.PathSubstringsToIgnore = ImporterUI->PathSubstringsToIgnore;

        for (auto File : Files)
        {
            if (!FSimReadyUSDImporter::LoadUSD(File, ImporterUI->AssetPath, ImportSettings))
            {
                if (!FailedFiles.IsEmpty())
                {
                    FailedFiles += ";";
                }
                FailedFiles += File;
            }
        }
    }

    if (!FailedFiles.IsEmpty())
    {
        const FText Message = FText::Format(LOCTEXT("ImportFailed_Generic", "Failed to import '{0}'.\nPlease see Output Log for details."), FText::FromString(FailedFiles));
        FMessageDialog::Open(EAppMsgType::Ok, Message);
    }

    return FReply::Handled();
}

FReply SSimReadyImporterDialog::CancelClicked()
{
    RequestDestroyWindow();
    return FReply::Handled();
}

TWeakPtr<SSimReadyImporterDialog> SSimReadyImporterDialog::DialogPtr;
void SSimReadyImporterDialog::ShowDialog(ESimReadyImporterType DialogType, const FString& FullPath)
{
    // If there's an importer diaplog, show it.
    if (DialogPtr.IsValid())
    {
        DialogPtr.Pin()->BringToFront();
        return;
    }

    auto DefaultImporterUI = GetMutableDefault<USimReadyImporterUI>();
    DefaultImporterUI->USDPath = DefaultImporterUI->PreUSDPath[DialogType];
    DefaultImporterUI->AssetPath = DefaultImporterUI->PreAssetPath[DialogType];
    DefaultImporterUI->ImporterType = DialogType;
    DefaultImporterUI->AssetPath = FullPath;

    // Create the window to pick the class
    TSharedRef<SSimReadyImporterDialog> SimReadyImporterDialog = 
        SNew(SSimReadyImporterDialog)
        .ImporterUI(DefaultImporterUI);

    DialogPtr = SimReadyImporterDialog;
    IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));

    if (MainFrameModule.GetParentWindow().IsValid())
    {
        FSlateApplication::Get().AddWindowAsNativeChild(SimReadyImporterDialog, MainFrameModule.GetParentWindow().ToSharedRef());
    }
    else
    {
        FSlateApplication::Get().AddWindow(SimReadyImporterDialog);
    }
}
#undef LOCTEXT_NAMESPACE
