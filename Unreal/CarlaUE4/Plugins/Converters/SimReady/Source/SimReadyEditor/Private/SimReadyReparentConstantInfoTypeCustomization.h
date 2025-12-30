// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"


class FSimReadyReparentConstantInfoTypeCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();

    /** Begin IPropertyTypeCustomization overrides */
    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
    /** End IPropertyTypeCustomization overrides */

    FSimReadyReparentConstantInfoTypeCustomization();
    ~FSimReadyReparentConstantInfoTypeCustomization();

protected:
    void UpdateOmniMaterialParametersList();
    bool GetCurrentOmniMaterial(FString& OmniMaterial) const;
    void OnTextureChanged();
    EMaterialParameterType GetCurrentMaterialParameterType() const;

    EVisibility GetTextureVisibility() const;
    EVisibility GetScalarVisibility() const;
    EVisibility GetVectorVisibility() const;
    EVisibility GetBooleanVisibility() const;

    TSharedPtr<IPropertyHandle> OmniParameterProperty;
    TSharedPtr<IPropertyHandle> OmniMaterialProperty;
    TSharedPtr<IPropertyHandle> TextureProperty;
    TSharedPtr<IPropertyHandle> ConstantProperty;
    TSharedPtr<IPropertyHandle> BooleanProperty;
    TArray<TSharedPtr<FName>> OmniParameterList;

    TSharedPtr<class SComboBox<TSharedPtr<FName>>> OmniParameterComboBox;
    TSharedPtr<class SWidget> TexturePropertyWidget;

    FName SelectOmniParameter = NAME_None;
    FVector4 CurrentConstant;
};
