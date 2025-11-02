// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyImporterDetails.h"

#include "Framework/Application/SlateApplication.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "PropertyHandle.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "DesktopPlatformModule.h"
#include "PropertyEditorModule.h"
#include "EditorDirectories.h"
#include "SimReadyImporterUI.h"
#include "SimReadyImporterDialog.h"
#include "Dialogs/DlgPickPath.h"

#define LOCTEXT_NAMESPACE "SimReadyEditor"

TSharedRef<IDetailCustomization> FSimReadyImporterDetails::MakeInstance()
{
    return MakeShareable( new FSimReadyImporterDetails );
}

void FSimReadyImporterDetails::CustomizeDetails( IDetailLayoutBuilder& DetailBuilder )
{
    IDetailCategoryBuilder& PathCategory = DetailBuilder.EditCategory( "Path" );

    FDetailWidgetRow& InfoWidget = PathCategory.AddCustomRow(FText::FromString(TEXT("Info Widget")));

    InfoWidget.NameContent()
    [
        SNew(STextBlock)
        .Text(this, &FSimReadyImporterDetails::GetInfoContentName)
    ];
    InfoWidget.ValueContent()
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SEditableText)
            .Text(this, &FSimReadyImporterDetails::GetInfoPathText)
            .ToolTipText(this, &FSimReadyImporterDetails::GetInfoPathText)
            .IsReadOnly(true)
        ]
    ];

    FDetailWidgetRow& SelectWidget = PathCategory.AddCustomRow(FText::FromString(TEXT("Select Widget")));

    SelectWidget.NameContent()
    [
        SNew(STextBlock)
        .Text(this, &FSimReadyImporterDetails::GetSelectContentName)
    ];
    SelectWidget.ValueContent()
    .MinDesiredWidth(200.0f)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        [
            SNew(SEditableTextBox)
            .Text(this, &FSimReadyImporterDetails::GetSelectPathText)
            .OnTextCommitted(this, &FSimReadyImporterDetails::OnSelectPathTextCommitted)
            .ToolTipText(this, &FSimReadyImporterDetails::GetSelectPathText)
            .MinDesiredWidth(200.0f)
        ]
        + SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .AutoWidth()
        .Padding(5)
        [
            SNew(SButton)
            .IsFocusable(false)
            .OnClicked(this, &FSimReadyImporterDetails::OnSelectPathClicked)
            .ToolTipText(this, &FSimReadyImporterDetails::GetSelectButtonTipText)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("...", "..."))
            ]
        ]
    ];
}

FText FSimReadyImporterDetails::GetInfoContentName() const
{
    auto ImporterUI = GetMutableDefault<USimReadyImporterUI>();
    FString SelectName = TEXT("Asset Folder");
    return FText::FromString(SelectName);
}

FText FSimReadyImporterDetails::GetSelectContentName() const
{
    auto ImporterUI = GetMutableDefault<USimReadyImporterUI>();
    FString SelectName = TEXT("Input USD File");
    return FText::FromString(SelectName);
}

FText FSimReadyImporterDetails::GetInfoPathText() const
{
    auto ImporterUI = GetMutableDefault<USimReadyImporterUI>();
    return FText::FromString(ImporterUI->AssetPath);
}

FText FSimReadyImporterDetails::GetSelectPathText() const
{
    auto ImporterUI = GetMutableDefault<USimReadyImporterUI>();
    return FText::FromString(ImporterUI->USDPath);
}

FText FSimReadyImporterDetails::GetSelectButtonTipText() const
{
    auto ImporterUI = GetMutableDefault<USimReadyImporterUI>();
    return FText::FromString(TEXT("Browse for the USD files"));
}

void FSimReadyImporterDetails::OnSelectPathTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
    auto ImporterUI = GetMutableDefault<USimReadyImporterUI>();
    ImporterUI->USDPath = Text.ToString();
    ImporterUI->PreUSDPath[ImporterUI->ImporterType] = ImporterUI->USDPath;

}

FReply FSimReadyImporterDetails::OnSelectPathClicked()
{
    auto ImporterUI = GetMutableDefault<USimReadyImporterUI>();
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform)
    {
        TArray<FString> OpenFilenames;
        bool bOpened = false;

        const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
        const FString FileTypes = TEXT("USD Files (*.usd; *.usda; *.usdc; *.usdz)|*.usd; *.usda; *.usdc; *.usdz|USD Binary File (*.usd)|*.usd|USD Text File (*.usda)|*.usda|USD Crate File (*.usdc)|*.usdc|USD Zip File (*.usdz)|*.usdz");

        bOpened = DesktopPlatform->OpenFileDialog(
            ParentWindowWindowHandle,
            LOCTEXT("ImportDialogTitle", "Import").ToString(),
            FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT),
            TEXT(""),
            FileTypes,
            EFileDialogFlags::Multiple,
            OpenFilenames
            );

        if (bOpened && OpenFilenames.Num() > 0)
        {
            FString& OpenedFile = OpenFilenames[0];
            FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_IMPORT, FPaths::GetPath(OpenedFile));

            FString Filenames;
            for (auto OpenFile : OpenFilenames)
            {
                if (!OpenFile.IsEmpty())
                {
                    Filenames += ";";
                }
                Filenames += OpenFile;
            }
            
            ImporterUI->USDPath = Filenames;
            ImporterUI->PreUSDPath[ImporterUI->ImporterType] = ImporterUI->USDPath;
        }
    }

    if (SSimReadyImporterDialog::DialogPtr.IsValid())
    {
        auto DialogWindow = SSimReadyImporterDialog::DialogPtr.Pin();
        DialogWindow->BringToFront();
    }

    return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
