// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyMaterialReparentDialog.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
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
#include "UserInterface/SMessageLogListing.h"
#include "SimReadyMDL.h"
#include "SimReadyPathHelper.h"
#include "MaterialEditorUtilities.h"


#define LOCTEXT_NAMESPACE "SimReadyMaterialReparent"

TMap<FString, TMap<FString, EMaterialParameterType>> SSimReadyMaterialReparentDialog::GlobalOmniParameterMap;
TMap<FString, TMap<FName, EMaterialParameterType>> SSimReadyMaterialReparentDialog::GlobalUnrealParameterMap;
FSimpleMulticastDelegate SSimReadyMaterialReparentDialog::RefreshEditor;

SSimReadyMaterialReparentDialog::SSimReadyMaterialReparentDialog()
    : ListingModel(FMessageLogListingModel::Create(TEXT("ValidationLog")))
    , ListingView(FMessageLogListingViewModel::Create(ListingModel, LOCTEXT("ValidationLog", "Validation Log")))
{
    FEditorDelegates::RefreshEditor.AddRaw(this, &SSimReadyMaterialReparentDialog::OnRefreshEditor);
}

void SSimReadyMaterialReparentDialog::ConstructDialog()
{
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bAllowSearch = false;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    DetailsView->SetObject(ReparentSettings);
    DetailsView->OnFinishedChangingProperties().AddSP(this, &SSimReadyMaterialReparentDialog::OnFinishedChangingProperties);


    const int32 ClientSizeX = 500;
    const int32 ClientSizeY = 400;
    const int32 LogViewSize = 80;
    TSharedPtr<SScrollBox> ScrollBox;
    const FText TitleText = LOCTEXT("SimReadyMaterialReparentDialogTitle", "SimReady Material Reparent");
    
    SWindow::Construct(SWindow::FArguments()
        .Title(TitleText)
        .SizingRule(ESizingRule::Autosized)
        .SupportsMinimize(false)
        .SupportsMaximize(false)
        .bDragAnywhere(false)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SScrollBox)
                + SScrollBox::Slot()
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .MaxHeight(ClientSizeY)
                    .Padding(2)
                    [
                        DetailsView.ToSharedRef()
                    ]
                ]
            ]
            + SVerticalBox::Slot()
            .Padding(0, 1, 0, 1)
            [
                SNew(SBox)
                .MinDesiredWidth(ClientSizeX)
                .MaxDesiredWidth(ClientSizeX)
                .MinDesiredHeight(LogViewSize)
                .MaxDesiredHeight(LogViewSize)
                [
                    SNew(SBorder)
                    .Visibility(EVisibility::Visible)
                    .BorderImage(FEditorStyle::GetBrush("Menu.Background"))
                    [
                        SNew(SScrollBox)
                        + SScrollBox::Slot()
                        [
                            SNew(SMessageLogListing, ListingView)
                        ]
                    ]
                ]
            ]
        ]
    );
}

void SSimReadyMaterialReparentDialog::Construct(const FArguments& InArgs)
{
    ReparentSettings = InArgs._ReparentSettings;
    ConstructDialog();
}

SSimReadyMaterialReparentDialog::~SSimReadyMaterialReparentDialog()
{
    DialogPtr = nullptr;
    FEditorDelegates::RefreshEditor.RemoveAll(this);
}

TWeakPtr<SSimReadyMaterialReparentDialog> SSimReadyMaterialReparentDialog::DialogPtr = nullptr;

void SSimReadyMaterialReparentDialog::ShowDialog()
{
    // If there's an reparent diaplog, show it.
    if (DialogPtr.IsValid())
    {
        DialogPtr.Pin()->BringToFront();
        return;
    }

    auto DefaultReparentSettings = GetMutableDefault<USimReadyMaterialReparentSettings>();

    // Create the window to pick the class
    TSharedRef<SSimReadyMaterialReparentDialog> SimReadyMaterialReparentDialog =
        SNew(SSimReadyMaterialReparentDialog)
        .ReparentSettings(DefaultReparentSettings);

    DialogPtr = SimReadyMaterialReparentDialog;

    // Check materials
    for (auto& MaterialInfo : DefaultReparentSettings->MaterialInfoList)
    {
        if (MaterialInfo.bEnabled && !MaterialInfo.OmniMaterial.IsEmpty() && !MaterialInfo.UnrealMaterial.IsNull())
        {
            auto Material = MaterialInfo.UnrealMaterial.Get();

            // failed to get material, try load
            if (Material == nullptr)
            {
                Material = MaterialInfo.UnrealMaterial.LoadSynchronous();
            }

            ValidateSameNameParameters(MaterialInfo.OmniMaterial, Material);
        }
    }

    IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));

    if (MainFrameModule.GetParentWindow().IsValid())
    {
        FSlateApplication::Get().AddWindowAsNativeChild(SimReadyMaterialReparentDialog, MainFrameModule.GetParentWindow().ToSharedRef());
    }
    else
    {
        FSlateApplication::Get().AddWindow(SimReadyMaterialReparentDialog);
    }
}

void SSimReadyMaterialReparentDialog::AddMessage(UObject* Object, const FString& MessageStr)
{
    TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Warning);
    FString OuterPackageName;
    if (!FPackageName::TryConvertFilenameToLongPackageName(Object->GetPathName(), OuterPackageName))
    {
        OuterPackageName = Object->GetPathName();
    }
    Message->AddToken(FAssetNameToken::Create(OuterPackageName));
    Message->AddToken(FTextToken::Create(FText::FromString(TEXT(":"))));
    Message->AddToken(FTextToken::Create(FText::FromString(MessageStr)));
    ListingModel->AddMessage(Message, false, true);
}

void SSimReadyMaterialReparentDialog::ClearMessages()
{
    ListingModel->ClearMessages();
}

void SSimReadyMaterialReparentDialog::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent) const
{
}

void SSimReadyMaterialReparentDialog::ClearLogs()
{
    if (DialogPtr.IsValid())
    {
        DialogPtr.Pin()->ClearMessages();
    }
}

void SSimReadyMaterialReparentDialog::ValidateSameNameParameters(const FString& OmniMaterialName, UMaterialInterface* UnrealMaterial)
{
    if (DialogPtr.IsValid() && UnrealMaterial)
    {
        TMap<FString, EMaterialParameterType> OmniParameterMap;
        USimReadyMDL::GetAllDisplayNamesFromLocalBaseMDL(OmniMaterialName, OmniParameterMap);

        TArray<FMaterialParameterInfo> VisibleExpressions;
        GetMaterialVisibleExpressions(UnrealMaterial, VisibleExpressions);

        auto Validate = [&](const TArray<FMaterialParameterInfo>& ParameterInfos, EMaterialParameterType MaterialParameterType)
        {
            for (auto& Info : ParameterInfos)
            {
                if (VisibleExpressions.Find(Info) != INDEX_NONE)
                {
                    FString ParameterName = Info.Name.ToString();
                    auto FindType = OmniParameterMap.Find(ParameterName);
                    if (FindType)
                    {
                        if (*FindType != MaterialParameterType)
                        {
                            FString WarningMessage = FString::Printf(TEXT("Parameter <%s> cannot convert from %s to %s"), *ParameterName, *USimReadyMDL::MaterialParameterTypeToString(*FindType), *USimReadyMDL::MaterialParameterTypeToString(MaterialParameterType));
                            DialogPtr.Pin()->AddMessage(UnrealMaterial, WarningMessage);
                        }
                    }
                }
            }
        };

        // Check the same name parameters
        {
            TArray<FMaterialParameterInfo> ParameterInfos;
            TArray<FGuid> Guids;
            //Query all base material texture parameters.
            UnrealMaterial->GetAllTextureParameterInfo(ParameterInfos, Guids);
            Validate(ParameterInfos, EMaterialParameterType::Texture);
            UnrealMaterial->GetAllScalarParameterInfo(ParameterInfos, Guids);
            Validate(ParameterInfos, EMaterialParameterType::Scalar);
            UnrealMaterial->GetAllVectorParameterInfo(ParameterInfos, Guids);
            Validate(ParameterInfos, EMaterialParameterType::Vector);
            UnrealMaterial->GetAllStaticSwitchParameterInfo(ParameterInfos, Guids);
            Validate(ParameterInfos, EMaterialParameterType::StaticSwitch);
        }
    }
}

void SSimReadyMaterialReparentDialog::GetMaterialVisibleExpressions(UMaterialInterface* Material, TArray<FMaterialParameterInfo>& VisibleExpressions)
{
    if (!Material)
    {
        return;
    }

    if (Material->IsA<UMaterialInstance>())
    {
        FMaterialEditorUtilities::GetVisibleMaterialParameters(Material->GetMaterial(), Cast<UMaterialInstance>(Material), VisibleExpressions);
    }
    else
    {
        UMaterial* OriginalMaterial = Cast<UMaterial>(Material);

        TArray<FMaterialParameterInfo> ParameterInfos;
        TArray<FGuid> Guids;
        //Query all base material texture parameters.
        OriginalMaterial->GetAllTextureParameterInfo(ParameterInfos, Guids);
        VisibleExpressions.Append(ParameterInfos);
        OriginalMaterial->GetAllScalarParameterInfo(ParameterInfos, Guids);
        VisibleExpressions.Append(ParameterInfos);
        OriginalMaterial->GetAllVectorParameterInfo(ParameterInfos, Guids);
        VisibleExpressions.Append(ParameterInfos);
        OriginalMaterial->GetAllStaticSwitchParameterInfo(ParameterInfos, Guids);
        VisibleExpressions.Append(ParameterInfos);
    }
}

void SSimReadyMaterialReparentDialog::UpdatePreviewSurfaceMap(TMap<FString, EMaterialParameterType>& ParameterMap)
{
    auto PreviewSurfaceMaterial = FSimReadyPathHelper::GetPreviewSurfaceMaterial(true/*bNative*/);

    auto Update = [&](const TArray<FMaterialParameterInfo>& ParameterInfos, EMaterialParameterType MaterialParameterType)
    {
        for (auto& Info : ParameterInfos)
        {
            ParameterMap.Add(Info.Name.ToString(), MaterialParameterType);
        }
    };

    {
        TArray<FMaterialParameterInfo> ParameterInfos;
        TArray<FGuid> ParameterGuids;
        PreviewSurfaceMaterial->GetAllTextureParameterInfo(ParameterInfos, ParameterGuids);
        Update(ParameterInfos, EMaterialParameterType::Texture);
        PreviewSurfaceMaterial->GetAllScalarParameterInfo(ParameterInfos, ParameterGuids);
        Update(ParameterInfos, EMaterialParameterType::Scalar);
        PreviewSurfaceMaterial->GetAllVectorParameterInfo(ParameterInfos, ParameterGuids);
        Update(ParameterInfos, EMaterialParameterType::Vector);
        PreviewSurfaceMaterial->GetAllStaticSwitchParameterInfo(ParameterInfos, ParameterGuids);
        Update(ParameterInfos, EMaterialParameterType::StaticSwitch);
    }

    GlobalOmniParameterMap.Add(USDPreviewSurfaceDisplayName, ParameterMap);
}

void SSimReadyMaterialReparentDialog::UpdateUnrealMaterialMap(UMaterialInterface* Material, TMap<FName, EMaterialParameterType>& ParameterMap)
{
    if (Material == nullptr)
    {
        return;
    }

    TArray<FMaterialParameterInfo> VisibleExpressions;
    GetMaterialVisibleExpressions(Material, VisibleExpressions);

    auto Update = [&](const TArray<FMaterialParameterInfo>& ParameterInfos, EMaterialParameterType MaterialParameterType)
    {
        for (auto& Info : ParameterInfos)
        {
            if (VisibleExpressions.Find(Info) != INDEX_NONE)
            {
                ParameterMap.Add(Info.Name, MaterialParameterType);
            }
        }
    };

    {
        TArray<FMaterialParameterInfo> ParameterInfos;
        TArray<FGuid> ParameterGuids;
        Material->GetAllTextureParameterInfo(ParameterInfos, ParameterGuids);
        Update(ParameterInfos, EMaterialParameterType::Texture);
        Material->GetAllScalarParameterInfo(ParameterInfos, ParameterGuids);
        Update(ParameterInfos, EMaterialParameterType::Scalar);
        Material->GetAllVectorParameterInfo(ParameterInfos, ParameterGuids);
        Update(ParameterInfos, EMaterialParameterType::Vector);
        Material->GetAllStaticSwitchParameterInfo(ParameterInfos, ParameterGuids);
        Update(ParameterInfos, EMaterialParameterType::StaticSwitch);
    }

    GlobalUnrealParameterMap.Add(Material->GetFullName(), ParameterMap);
}

void SSimReadyMaterialReparentDialog::OnRefreshEditor()
{
    auto DefaultReparentSettings = GetMutableDefault<USimReadyMaterialReparentSettings>();

    ClearMessages();

    for (auto& MaterialInfo : DefaultReparentSettings->MaterialInfoList)
    {
        if (!MaterialInfo.UnrealMaterial.IsNull())
        {
            auto Material = MaterialInfo.UnrealMaterial.Get();

            // failed to get material, try load
            if (Material == nullptr)
            {
                Material = MaterialInfo.UnrealMaterial.LoadSynchronous();
            }

            TMap<FName, EMaterialParameterType> ParameterMap;
            UpdateUnrealMaterialMap(Material, ParameterMap);
            ValidateSameNameParameters(MaterialInfo.OmniMaterial, Material);
        }
    }

    // broadcast delegates to parameters
    RefreshEditor.Broadcast();
}
#undef LOCTEXT_NAMESPACE
