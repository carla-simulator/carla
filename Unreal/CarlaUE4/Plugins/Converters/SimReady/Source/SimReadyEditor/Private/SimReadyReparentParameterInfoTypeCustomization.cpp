// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyReparentParameterInfoTypeCustomization.h"

#include "Framework/Application/SlateApplication.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboBox.h"
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


TSharedRef<IPropertyTypeCustomization> FSimReadyReparentParameterInfoTypeCustomization::MakeInstance()
{
    return MakeShareable( new FSimReadyReparentParameterInfoTypeCustomization);
}

FSimReadyReparentParameterInfoTypeCustomization::FSimReadyReparentParameterInfoTypeCustomization()
{
    // Don't use FEditorDelegates::RefreshEditor, this must run after SSimReadyMaterialReparentDialog::OnRefreshEditor
    SSimReadyMaterialReparentDialog::RefreshEditor.AddRaw(this, &FSimReadyReparentParameterInfoTypeCustomization::OnRefreshEditor);
}

FSimReadyReparentParameterInfoTypeCustomization::~FSimReadyReparentParameterInfoTypeCustomization()
{
    SSimReadyMaterialReparentDialog::RefreshEditor.RemoveAll(this);
}

void FSimReadyReparentParameterInfoTypeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    // Combo Box for Omniverse Material
    OmniParameterProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentParameterInfo, OmniParameter));
    if (!OmniParameterProperty->IsValidHandle())
    {
        return;
    }

    // Combo Box for Unreal Material
    UnrealParameterProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentParameterInfo, UnrealParameter));
    if (!UnrealParameterProperty->IsValidHandle())
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

    UnrealMaterialProperty = MaterialHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSimReadyReparentMaterialInfo, UnrealMaterial));
    if (!UnrealMaterialProperty->IsValidHandle())
    {
        return;
    }

    // Get mdl parameters
    UpdateOmniMaterialParametersList();
    UpdateUnrealMaterialParametersList();

    FString OmniParameter;
    OmniParameterProperty->GetValue(OmniParameter);
    if (!OmniParameter.IsEmpty())
    {
        SelectOmniParameter = *OmniParameter;
    }

    FString UnrealParameter;
    UnrealParameterProperty->GetValue(UnrealParameter);
    if (!UnrealParameter.IsEmpty())
    {
        SelectUnrealParameter = *UnrealParameter;
    }

    SAssignNew(OmniParameterComboBox, SComboBox<TSharedPtr<FName>>)
    .OptionsSource(&OmniParameterList)
    .OnSelectionChanged_Lambda([&](TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo)
    {
        if (Item)
        {
            SelectOmniParameter = *Item;
            OmniParameterProperty->SetValue(SelectOmniParameter.ToString());
            WarningIcon->SetVisibility(IsParametersTypeCompatible() ? EVisibility::Collapsed : EVisibility::Visible);
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

    SAssignNew(UnrealParameterComboBox, SComboBox<TSharedPtr<FName>>)
    .OptionsSource(&UnrealParameterList)
    .OnSelectionChanged_Lambda([&](TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo)
    {
        if (Item)
        {
            SelectUnrealParameter = *Item;
            UnrealParameterProperty->SetValue(SelectUnrealParameter.ToString());
            WarningIcon->SetVisibility(IsParametersTypeCompatible() ? EVisibility::Collapsed : EVisibility::Visible);
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
            return FText::FromName(SelectUnrealParameter);
        })
    ];

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

    HeaderRow
    .ValueContent()
    .MinDesiredWidth(200)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        [
            UnrealParameterComboBox.ToSharedRef()
        ]
        + SHorizontalBox::Slot()
        .Padding(10, 0)
        .AutoWidth()
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Center)
        [
            SAssignNew(WarningIcon, SImage)
            .ColorAndOpacity(FSlateColor::UseForeground())
            .Image(FEditorStyle::Get().GetBrush("Icons.Warning"))
            .Visibility_Lambda([&]()
            {
                if (IsParametersTypeCompatible())
                {
                    return EVisibility::Collapsed;
                }
                else
                {
                    return EVisibility::Visible;
                }
            })
            .ToolTipText_Lambda([&]()
            {
                FString SourceType;
                FString DestType;

                FString OmniMaterial;
                if (!GetCurrentOmniMaterial(OmniMaterial))
                {
                    return FText::GetEmpty();
                }
                auto OmniParameterMap = SSimReadyMaterialReparentDialog::GlobalOmniParameterMap.Find(OmniMaterial);
                if (!OmniParameterMap)
                {
                    return FText::GetEmpty();
                }

                UMaterialInterface* UnrealMaterial = GetCurrentUnrealMaterial();
                if (!UnrealMaterial)
                {
                    return FText::GetEmpty();
                }
                auto UnrealParameterMap = SSimReadyMaterialReparentDialog::GlobalUnrealParameterMap.Find(UnrealMaterial->GetFullName());
                if (!UnrealParameterMap)
                {
                    return FText::GetEmpty();
                }

                auto OmniType = OmniParameterMap->Find(SelectOmniParameter.ToString());
                if (OmniType)
                {
                    SourceType = USimReadyMDL::MaterialParameterTypeToString(*OmniType);
                }
                auto UnrealType = UnrealParameterMap->Find(SelectUnrealParameter);
                if (UnrealType)
                {
                    DestType = USimReadyMDL::MaterialParameterTypeToString(*UnrealType);
                }
                return FText::FromString(FString::Printf(TEXT("Cannot convert from %s to %s"), *SourceType, *DestType));
            })
        ]
    ];
}

void FSimReadyReparentParameterInfoTypeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{

}

bool FSimReadyReparentParameterInfoTypeCustomization::IsParametersTypeCompatible()
{
    if (SelectUnrealParameter.IsNone() || SelectOmniParameter.IsNone())
    {
        return true;
    }

    FString OmniMaterial;
    if (!GetCurrentOmniMaterial(OmniMaterial))
    {
        return true;
    }
    auto OmniParameterMap = SSimReadyMaterialReparentDialog::GlobalOmniParameterMap.Find(OmniMaterial);
    if (!OmniParameterMap)
    {
        return true;
    }

    UMaterialInterface* UnrealMaterial = GetCurrentUnrealMaterial();
    if (!UnrealMaterial)
    {
        return true;
    }
    auto UnrealParameterMap = SSimReadyMaterialReparentDialog::GlobalUnrealParameterMap.Find(UnrealMaterial->GetFullName());
    if (!UnrealParameterMap)
    {
        return true;
    }

    auto OmniType = OmniParameterMap->Find(SelectOmniParameter.ToString());
    auto UnrealType = UnrealParameterMap->Find(SelectUnrealParameter);
    if (OmniType && UnrealType)
    {
        return *OmniType == *UnrealType ? true : false;
    }

    return true;
}

bool FSimReadyReparentParameterInfoTypeCustomization::GetCurrentOmniMaterial(FString& OmniMaterial)
{
    return OmniMaterialProperty->GetValue(OmniMaterial) == FPropertyAccess::Result::Success;
}

UMaterialInterface* FSimReadyReparentParameterInfoTypeCustomization::GetCurrentUnrealMaterial()
{
    UMaterialInterface* UnrealMaterial = nullptr;
    auto Property = UnrealMaterialProperty->GetProperty();
    if (Property->IsA<FSoftObjectProperty>())
    {
        void* Data;
        UnrealMaterialProperty->GetValueData(Data);
        auto SoftObjectPtr = CastField<FSoftObjectProperty>(Property)->GetPropertyValuePtr(Data);

        if (SoftObjectPtr)
        {
            //Get Material
            UnrealMaterial = Cast<UMaterialInterface>(SoftObjectPtr->Get());
            if (UnrealMaterial == nullptr)
            {
                UnrealMaterial = Cast<UMaterialInterface>(SoftObjectPtr->LoadSynchronous());
            }
        }
    }
    return UnrealMaterial;
}

void FSimReadyReparentParameterInfoTypeCustomization::UpdateOmniMaterialParametersList()
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

void FSimReadyReparentParameterInfoTypeCustomization::UpdateUnrealMaterialParametersList()
{
    // Get unreal parameters
    UnrealParameterList.Reset();

    UMaterialInterface* UnrealMaterial = GetCurrentUnrealMaterial();
    if (UnrealMaterial)
    {
        auto UnrealParameterMap = SSimReadyMaterialReparentDialog::GlobalUnrealParameterMap.Find(UnrealMaterial->GetFullName());
        if (UnrealParameterMap)
        {
            for (auto& Iter : *UnrealParameterMap)
            {
                UnrealParameterList.Add(MakeShared<FName>(Iter.Key));
            }
        }
        else
        {
            TMap<FName, EMaterialParameterType> ParameterMap;
            SSimReadyMaterialReparentDialog::UpdateUnrealMaterialMap(UnrealMaterial, ParameterMap);

            for (auto& Iter : ParameterMap)
            {
                UnrealParameterList.Add(MakeShared<FName>(Iter.Key));
            }
        }
    }
}

void FSimReadyReparentParameterInfoTypeCustomization::OnRefreshEditor()
{
    // Material is compiled, update the parameters list
    UpdateUnrealMaterialParametersList();

    // Check if the parameter is existed
    if (!SelectUnrealParameter.IsNone() && UnrealParameterProperty)
    {
        UMaterialInterface* UnrealMaterial = GetCurrentUnrealMaterial();
        if (UnrealMaterial)
        {
            auto UnrealParameterMap = SSimReadyMaterialReparentDialog::GlobalUnrealParameterMap.Find(UnrealMaterial->GetFullName());
            if (UnrealParameterMap)
            {
                if (UnrealParameterMap->Find(SelectUnrealParameter) == nullptr)
                {
                    SelectUnrealParameter = NAME_None;
                    UnrealParameterProperty->SetValue(SelectUnrealParameter.ToString());
                }
            }	
        }
    }

    // Update visibility of warning icon
    if (WarningIcon)
    {
        WarningIcon->SetVisibility(IsParametersTypeCompatible() ? EVisibility::Collapsed : EVisibility::Visible);
    }
}

#undef LOCTEXT_NAMESPACE
