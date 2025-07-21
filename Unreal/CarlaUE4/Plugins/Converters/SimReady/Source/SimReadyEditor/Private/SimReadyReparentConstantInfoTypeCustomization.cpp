// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyReparentConstantInfoTypeCustomization.h"

#include "Framework/Application/SlateApplication.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SWidget.h"
#include "PropertyHandle.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "DesktopPlatformModule.h"
#include "PropertyEditorModule.h"
#include "IPropertyUtilities.h"
#include "SimReadyMaterialReparentSettings.h"
#include "SimReadyMDL.h"
#include "SimReadyMaterialReparentDialog.h"


#define LOCTEXT_NAMESPACE "SimReadyMaterialReparent"


TSharedRef<IPropertyTypeCustomization> FSimReadyReparentConstantInfoTypeCustomization::MakeInstance()
{
    return MakeShareable( new FSimReadyReparentConstantInfoTypeCustomization);
}

FSimReadyReparentConstantInfoTypeCustomization::FSimReadyReparentConstantInfoTypeCustomization()
{
}

FSimReadyReparentConstantInfoTypeCustomization::~FSimReadyReparentConstantInfoTypeCustomization()
{
}

void FSimReadyReparentConstantInfoTypeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    // Combo Box for SimReady Material
    OmniParameterProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentParameterInfo, OmniParameter));
    if (!OmniParameterProperty->IsValidHandle())
    {
        return;
    }

    // ParameterInfoList[] -> ParameterInfoList -> MaterialInfoList[]
    auto MaterialHandle = PropertyHandle->GetParentHandle()->GetParentHandle();

    OmniMaterialProperty = MaterialHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentMaterialInfo, OmniMaterial));
    if (!OmniMaterialProperty->IsValidHandle())
    {
        return;
    }

    // Get mdl parameters
    UpdateOmniMaterialParametersList();

    FString OmniParameter;
    OmniParameterProperty->GetValue(OmniParameter);
    if (!OmniParameter.IsEmpty())
    {
        SelectOmniParameter = *OmniParameter;
    }

    SAssignNew(OmniParameterComboBox, SComboBox<TSharedPtr<FName>>)
        .OptionsSource(&OmniParameterList)
        .OnSelectionChanged_Lambda([&](TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo)
        {
            if (Item)
            {
                SelectOmniParameter = *Item;
                OmniParameterProperty->SetValue(SelectOmniParameter.ToString());
                //WarningIcon->SetVisibility(IsParametersTypeCompatible() ? EVisibility::Collapsed : EVisibility::Visible);
            }
        })
        .OnGenerateWidget_Lambda([](TSharedPtr<FName> InItem)
        {
            return SNew(STextBlock).Text(FText::FromName(InItem ? *InItem : NAME_None));
        })
        .Content()
        [
            SNew(STextBlock)
            .Text_Lambda([&]()
                {
                    return FText::FromName(SelectOmniParameter);
                })
        ];


    TextureProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentConstantInfo, TextureValue));
    if (!TextureProperty->IsValidHandle())
    {
        return;
    }

    ConstantProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentConstantInfo, ConstantValue));
    if (!ConstantProperty->IsValidHandle())
    {
        return;
    }

    ConstantProperty->GetValue(CurrentConstant);

    BooleanProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentConstantInfo, ConstantBoolean));
    if (!BooleanProperty->IsValidHandle())
    {
        return;
    }

    HeaderRow
    .NameContent()
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .FillWidth(1.0f)
        [
            OmniParameterComboBox.ToSharedRef()
        ]
    ];

    TexturePropertyWidget = TextureProperty->CreatePropertyValueWidget();
    
    TextureProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FSimReadyReparentConstantInfoTypeCustomization::OnTextureChanged));

    HeaderRow
    .ValueContent()
    .MinDesiredWidth(250)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SBox)
            .Visibility(this, &FSimReadyReparentConstantInfoTypeCustomization::GetBooleanVisibility)
            [
                BooleanProperty->CreatePropertyValueWidget()
            ]
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SBox)
            .Visibility(this, &FSimReadyReparentConstantInfoTypeCustomization::GetTextureVisibility)
            [
                TexturePropertyWidget.ToSharedRef()
            ]
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SSpinBox<float>)
            .Visibility_Lambda([&]()
            {
                if (GetScalarVisibility() == EVisibility::Visible || GetVectorVisibility() == EVisibility::Visible)
                {
                    return EVisibility::Visible;
                }
                else
                {
                    return EVisibility::Collapsed;
                }
            })
            .Value_Lambda([&]()
            {
                return CurrentConstant.X;
            })
            .OnValueChanged_Lambda([&](float NewValue)
            {
                CurrentConstant.X = NewValue;
                ConstantProperty->SetValue(CurrentConstant);
            })
        ]

        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(2, 0)
        [
            SNew(SSpinBox<float>)
            .Visibility(this, &FSimReadyReparentConstantInfoTypeCustomization::GetVectorVisibility)
            .Value_Lambda([&]()
            {
                return CurrentConstant.Y;
            })
            .OnValueChanged_Lambda([&](float NewValue)
            {
                CurrentConstant.Y = NewValue;
                ConstantProperty->SetValue(CurrentConstant);
            })
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(2, 0)
        [
            SNew(SSpinBox<float>)
            .Visibility(this, &FSimReadyReparentConstantInfoTypeCustomization::GetVectorVisibility)
            .Value_Lambda([&]()
                {
                    return CurrentConstant.Z;
                })
            .OnValueChanged_Lambda([&](float NewValue)
                {
                    CurrentConstant.Z = NewValue;
                    ConstantProperty->SetValue(CurrentConstant);
                })
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(2, 0)
        [
            SNew(SSpinBox<float>)
            .Visibility(this, &FSimReadyReparentConstantInfoTypeCustomization::GetVectorVisibility)
            .Value_Lambda([&]()
                {
                    return CurrentConstant.W;
                })
            .OnValueChanged_Lambda([&](float NewValue)
                {
                    CurrentConstant.W = NewValue;
                    ConstantProperty->SetValue(CurrentConstant);
                })
        ]
    ];
}

void FSimReadyReparentConstantInfoTypeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{

}

void FSimReadyReparentConstantInfoTypeCustomization::OnTextureChanged()
{

}

EVisibility FSimReadyReparentConstantInfoTypeCustomization::GetTextureVisibility() const
{
    if (GetCurrentMaterialParameterType() == EMaterialParameterType::Texture)
    {
        return EVisibility::Visible;
    }

    return EVisibility::Collapsed;
}

EVisibility FSimReadyReparentConstantInfoTypeCustomization::GetVectorVisibility() const
{
    if (GetCurrentMaterialParameterType() == EMaterialParameterType::Vector)
    {
        return EVisibility::Visible;
    }

    return EVisibility::Collapsed;
}

EVisibility FSimReadyReparentConstantInfoTypeCustomization::GetScalarVisibility() const
{
    if (GetCurrentMaterialParameterType() == EMaterialParameterType::Scalar)
    {
        return EVisibility::Visible;
    }
    
    return EVisibility::Collapsed;
}

EVisibility FSimReadyReparentConstantInfoTypeCustomization::GetBooleanVisibility() const
{
    if (GetCurrentMaterialParameterType() == EMaterialParameterType::StaticSwitch)
    {
        return EVisibility::Visible;
    }

    return EVisibility::Collapsed;
}

EMaterialParameterType FSimReadyReparentConstantInfoTypeCustomization::GetCurrentMaterialParameterType() const
{
    FString OmniMaterial;
    if (GetCurrentOmniMaterial(OmniMaterial))
    {
        auto OmniParameterMap = SSimReadyMaterialReparentDialog::GlobalOmniParameterMap.Find(OmniMaterial);
        if (OmniParameterMap)
        {
            auto OmniParameter = OmniParameterMap->Find(SelectOmniParameter.ToString());
            if (OmniParameter)
            {
                return *OmniParameter;
            }
        }
    }

    return EMaterialParameterType::Count;
}

bool FSimReadyReparentConstantInfoTypeCustomization::GetCurrentOmniMaterial(FString& OmniMaterial) const
{
    return OmniMaterialProperty->GetValue(OmniMaterial) == FPropertyAccess::Result::Success;
}

void FSimReadyReparentConstantInfoTypeCustomization::UpdateOmniMaterialParametersList()
{
    OmniParameterList.Reset();

    FString OmniMaterial;
    if (GetCurrentOmniMaterial(OmniMaterial))
    {
        auto OmniParameterMap = SSimReadyMaterialReparentDialog::GlobalOmniParameterMap.Find(OmniMaterial);
        if (OmniParameterMap)
        {
            for (auto& Iter : *OmniParameterMap)
            {
                OmniParameterList.Add(MakeShared<FName>(*Iter.Key));
            }
        }
        else
        {
            TMap<FString, EMaterialParameterType> ParameterMap;
            if (OmniMaterial.Equals(USDPreviewSurfaceDisplayName))
            {
                SSimReadyMaterialReparentDialog::UpdatePreviewSurfaceMap(ParameterMap);
            }
            else
            {
                USimReadyMDL::GetAllDisplayNamesFromLocalBaseMDL(OmniMaterial, ParameterMap);
                SSimReadyMaterialReparentDialog::GlobalOmniParameterMap.Add(OmniMaterial, ParameterMap);
            }

            for (auto& Iter : ParameterMap)
            {
                OmniParameterList.Add(MakeShared<FName>(*Iter.Key));
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE
