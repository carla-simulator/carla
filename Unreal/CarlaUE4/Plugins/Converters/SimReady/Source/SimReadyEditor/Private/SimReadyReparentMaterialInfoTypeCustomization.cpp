// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyReparentMaterialInfoTypeCustomization.h"

#include "Framework/Application/SlateApplication.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SWidget.h"
#include "PropertyHandle.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "DesktopPlatformModule.h"
#include "PropertyEditorModule.h"
#include "SimReadyMaterialReparentSettings.h"
#include "SimReadyMDL.h"
#include "SimReadyMaterialReparentDialog.h"

#define LOCTEXT_NAMESPACE "SimReadyMaterialReparent"

TSharedRef<IPropertyTypeCustomization> FSimReadyReparentMaterialInfoTypeCustomization::MakeInstance()
{
    return MakeShareable( new FSimReadyReparentMaterialInfoTypeCustomization);
}

void FSimReadyReparentMaterialInfoTypeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    // Get names from core materials lib
    TArray<FString> MaterialNames;
    USimReadyMDL::GetLocalBaseMaterialNames(MaterialNames);

    OmniMaterialList.Add(MakeShared<FName>(USDPreviewSurfaceDisplayName));

    for (auto Name : MaterialNames)
    {
        OmniMaterialList.Add(MakeShared<FName>(*Name));
    }

    // Enable property
    EnableProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentMaterialInfo, bEnabled));
    if (!EnableProperty->IsValidHandle())
    {
        return;
    }

    bool bEnabled = false;
    EnableProperty->GetValue(bEnabled);
    FSimpleDelegate OnEnableChangedDelegate = FSimpleDelegate::CreateSP(this, &FSimReadyReparentMaterialInfoTypeCustomization::OnEnableChanged);
    EnableProperty->SetOnPropertyValueChanged(OnEnableChangedDelegate);

    UsageProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentMaterialInfo, Usage));
    if (!UsageProperty->IsValidHandle())
    {
        return;
    }
    UsagePropertyWidget = UsageProperty->CreatePropertyValueWidget();

    // Combo Box for Omniverse Material
    OmniMaterialProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentMaterialInfo, OmniMaterial));
    if (!OmniMaterialProperty->IsValidHandle())
    {
        return;
    }
    OmniMaterialProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FSimReadyReparentMaterialInfoTypeCustomization::OnMaterialChanged));

    // Set initial value for combo box
    FString OmniMaterialName;
    OmniMaterialProperty->GetValue(OmniMaterialName);
    if (!OmniMaterialName.IsEmpty())
    {
        SelectOmniMaterial = *OmniMaterialName;
    }

    SAssignNew(OmniMaterialComboBox, SComboBox<TSharedPtr<FName>>)
    .IsEnabled(bEnabled)
    .OptionsSource(&OmniMaterialList)
    .OnSelectionChanged_Lambda([&](TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo)
    {
        if (Item)
        {
            SelectOmniMaterial = *Item;
            OmniMaterialProperty->SetValue(SelectOmniMaterial.ToString());
        }
    })
    .OnGenerateWidget_Lambda([](TSharedPtr<FName> InItem)
    {
        return SNew(STextBlock).Text(FText::FromName(InItem ? *InItem : NAME_None));
    })
    .Content()
    [
        SNew(STextBlock)
        .MinDesiredWidth(100.0f)
        .Text_Lambda([&]()
        {
            return FText::FromName(SelectOmniMaterial);
        })
    ];

    //HeaderRow.ForceAutoExpansion = true;

    HeaderRow
    .NameContent()
    [
        SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .HAlign(HAlign_Left)
            [
                EnableProperty->CreatePropertyValueWidget()
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2, 0)
            [
                UsagePropertyWidget.ToSharedRef()
            ]
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .Padding(2, 0)
            [
                OmniMaterialComboBox.ToSharedRef()
            ]
    ];

    UnrealMaterialProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentMaterialInfo, UnrealMaterial));
    if (UnrealMaterialProperty->IsValidHandle() )
    {
        // Widget for unreal material
        MaterialPropertyWidget = UnrealMaterialProperty->CreatePropertyValueWidget();
        MaterialPropertyWidget->SetEnabled(bEnabled);
        UnrealMaterialProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FSimReadyReparentMaterialInfoTypeCustomization::OnMaterialChanged));

        HeaderRow
        .ValueContent()
        .MinDesiredWidth(300.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                MaterialPropertyWidget.ToSharedRef()
            ]
        ];
    }
}

void FSimReadyReparentMaterialInfoTypeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    ParametersProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentMaterialInfo, ParameterInfoList));
    if (!ParametersProperty->IsValidHandle())
    {
        return;
    }

    ChildBuilder.AddProperty(ParametersProperty.ToSharedRef())
        .IsEnabled(MakeAttributeLambda([=] { return OmniMaterialComboBox->IsEnabled(); }));

    ConstantsProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentMaterialInfo, ConstantInfoList));
    if (!ConstantsProperty->IsValidHandle())
    {
        return;
    }

    ChildBuilder.AddProperty(ConstantsProperty.ToSharedRef())
        .IsEnabled(MakeAttributeLambda([=] { return OmniMaterialComboBox->IsEnabled(); }))
        .Visibility(MakeAttributeLambda([=]
        { 
            uint8 UsageValue;
            if (UsageProperty->GetValue(UsageValue) == FPropertyAccess::Result::Success)
            {
                if (UsageValue == static_cast<uint8>(ESimReadyReparentUsage::Import))
                {
                    return EVisibility::Collapsed;
                }
                else
                {
                    return EVisibility::Visible;
                }
            }
            return EVisibility::Collapsed;
        }));
}

void FSimReadyReparentMaterialInfoTypeCustomization::OnEnableChanged()
{
    bool bEnabled = false;
    EnableProperty->GetValue(bEnabled);
    UsagePropertyWidget->SetEnabled(bEnabled);
    OmniMaterialComboBox->SetEnabled(bEnabled);
    MaterialPropertyWidget->SetEnabled(bEnabled);

    if (bEnabled)
    {
        MaterialValidate();
    }
}

void FSimReadyReparentMaterialInfoTypeCustomization::OnMaterialChanged()
{
    // Reset parameters list
    auto Array = ParametersProperty->AsArray();
    if (Array)
    {
        Array->EmptyArray();
    }

    auto Constants = ConstantsProperty->AsArray();
    if (Constants)
    {
        Constants->EmptyArray();
    }

    MaterialValidate();
}

void FSimReadyReparentMaterialInfoTypeCustomization::MaterialValidate()
{
    if (!SelectOmniMaterial.IsNone())
    {
        // Get unreal material
        auto Property = UnrealMaterialProperty->GetProperty();
        if (Property->IsA<FSoftObjectProperty>())
        {
            void* Data;
            UnrealMaterialProperty->GetValueData(Data);
            auto SoftObjectPtr = CastField<FSoftObjectProperty>(Property)->GetPropertyValuePtr(Data);

            if (SoftObjectPtr)
            {
                UMaterialInterface* UnrealMaterial = Cast<UMaterialInterface>(SoftObjectPtr->Get());
                if (UnrealMaterial == nullptr)
                {
                    UnrealMaterial = Cast<UMaterialInterface>(SoftObjectPtr->LoadSynchronous());
                }

                SSimReadyMaterialReparentDialog::ClearLogs();
                SSimReadyMaterialReparentDialog::ValidateSameNameParameters(SelectOmniMaterial.ToString(), UnrealMaterial);
            }
        }
    }
}
#undef LOCTEXT_NAMESPACE
