// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"


class FSimReadyReparentParameterInfoTypeCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();

    /** Begin IPropertyTypeCustomization overrides */
    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
    /** End IPropertyTypeCustomization overrides */

    FSimReadyReparentParameterInfoTypeCustomization();
    ~FSimReadyReparentParameterInfoTypeCustomization();

protected:
    void UpdateOmniMaterialParametersList();
    void UpdateUnrealMaterialParametersList();
    bool IsParametersTypeCompatible();
    void OnRefreshEditor();
    bool GetCurrentOmniMaterial(FString& OmniMaterial);
    class UMaterialInterface* GetCurrentUnrealMaterial();

    TSharedPtr<IPropertyHandle> OmniParameterProperty;
    TSharedPtr<IPropertyHandle> UnrealParameterProperty;
    TSharedPtr<IPropertyHandle> OmniMaterialProperty;
    TSharedPtr<IPropertyHandle> UnrealMaterialProperty;
    TSharedPtr<class SComboBox<TSharedPtr<FName>>> OmniParameterComboBox;
    TSharedPtr<class SComboBox<TSharedPtr<FName>>> UnrealParameterComboBox;
    TSharedPtr<class SImage> WarningIcon;

    TArray<TSharedPtr<FName>> OmniParameterList;
    TArray<TSharedPtr<FName>> UnrealParameterList;

    FName SelectOmniParameter = NAME_None;
    FName SelectUnrealParameter = NAME_None;
};
