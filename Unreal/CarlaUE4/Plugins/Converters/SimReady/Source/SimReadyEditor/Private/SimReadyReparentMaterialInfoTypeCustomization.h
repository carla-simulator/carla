// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"


class FSimReadyReparentMaterialInfoTypeCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();

    /** Begin IPropertyTypeCustomization overrides */
    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
    /** End IPropertyTypeCustomization overrides */

protected:
    void OnEnableChanged();
    void OnMaterialChanged();
    void MaterialValidate();

    TArray<TSharedPtr<FName>> OmniMaterialList;
    FName SelectOmniMaterial = NAME_None;
    TSharedPtr<IPropertyHandle> EnableProperty;
    TSharedPtr<IPropertyHandle> UsageProperty;
    TSharedPtr<IPropertyHandle> OmniMaterialProperty;
    TSharedPtr<IPropertyHandle> UnrealMaterialProperty;
    TSharedPtr<IPropertyHandle> ParametersProperty;
    TSharedPtr<IPropertyHandle> ConstantsProperty;
    TSharedPtr<class SComboBox<TSharedPtr<FName>>> OmniMaterialComboBox;
    TSharedPtr<class SWidget> MaterialPropertyWidget;
    TSharedPtr<class SWidget> UsagePropertyWidget;
};
