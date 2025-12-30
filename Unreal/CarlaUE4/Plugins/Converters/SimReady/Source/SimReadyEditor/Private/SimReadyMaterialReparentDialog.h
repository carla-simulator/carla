// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once 

#include "Widgets/SWindow.h"
#include "SimReadyMaterialReparentSettings.h"
#include "MessageLog/Private/Model/MessageLogListingModel.h"
#include "MessageLog/Private/Presentation/MessageLogListingViewModel.h"


class SSimReadyMaterialReparentDialog : public SWindow
{
public:
    SLATE_BEGIN_ARGS(SSimReadyMaterialReparentDialog)
    {}

    SLATE_ARGUMENT(USimReadyMaterialReparentSettings*, ReparentSettings)

    SLATE_END_ARGS()

    /** 
     * Constructs a new reparent dialog. 
     *
     * @param InArgs Slate arguments. 
     */
    void Construct(const FArguments& InArgs);
    SSimReadyMaterialReparentDialog();
    ~SSimReadyMaterialReparentDialog();

    void AddMessage(class UObject* Object, const FString& MessageStr);
    void ClearMessages();
    void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent) const;

    // Show the dialog
    static void ShowDialog();
    static void ValidateSameNameParameters(const FString& OmniMaterialName, class UMaterialInterface* UnrealMaterial);
    static void UpdateUnrealMaterialMap(class UMaterialInterface* Material, TMap<FName, EMaterialParameterType>& ParameterMap);
    static void UpdatePreviewSurfaceMap(TMap<FString, EMaterialParameterType>& ParameterMap);
    static void ClearLogs();

    // Shared with all parameters
    static TMap<FString, TMap<FString, EMaterialParameterType>> GlobalOmniParameterMap;
    static TMap<FString, TMap<FName, EMaterialParameterType>> GlobalUnrealParameterMap;
    static FSimpleMulticastDelegate RefreshEditor;

protected:
    void OnRefreshEditor();
    static TWeakPtr<class SSimReadyMaterialReparentDialog> DialogPtr;

private:
    void ConstructDialog();
    static void GetMaterialVisibleExpressions(class UMaterialInterface* Material, TArray<struct FMaterialParameterInfo>& VisibleExpressions);

    USimReadyMaterialReparentSettings* ReparentSettings;
    TSharedPtr<class IDetailsView> DetailsView;
    TSharedRef<FMessageLogListingModel> ListingModel;
    TSharedRef<FMessageLogListingViewModel> ListingView;
};
