// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once 

#include "Widgets/SWindow.h"
#include "SimReadyExportSettings.h"

namespace ETextureSizeChoices
{
    enum Type
    {
        UseTextureSetting,
        UseTextureSource
    };
}

namespace EExportMethodChoices
{
    enum Type
    {
        SharedExport,
        ModularExport
    };
}

namespace EDialogType
{
    enum Type
    {
        Material,
        Texture,
        Animation,
        Object,
        Level
    };
}

class SSimReadyExporterDialog : public SWindow
{
public:
    SLATE_BEGIN_ARGS(SSimReadyExporterDialog)
    {}

    SLATE_END_ARGS()

    //~SSimReadyExporterDialog();

    /** 
     * Constructs a new exporter dialog. 
     *
     * @param InArgs Slate arguments. 
     */
    void Construct(const FArguments& InArgs, EDialogType::Type DialogType, const TArray<UObject*>& InExportedObjects, const FString& InExportPath);

    void Construct(const FArguments& InArgs, const TArray<AActor*>& InExportedActors, const FString& InExportPath);


    // Show the dialog
    static void ShowDialog(EDialogType::Type DialogType, const TArray<UObject*>& InExportedObjects, const FString& InExportPath = FString());

    static void ShowDialog(const TArray<AActor*>& InExportedActors, const FString& InExportPath = FString());

private:
    FReply OkClicked();
    FReply CancelClicked();

    ECheckBoxState ExportMethodIsChecked(EExportMethodChoices::Type Choice) const;
    void OnExportMethodChanged(ECheckBoxState NewCheckedState, EExportMethodChoices::Type Choice);

    ECheckBoxState TextureSizeIsChecked(ETextureSizeChoices::Type Choice) const;
    void OnTextureSizeChanged(ECheckBoxState NewCheckedState, ETextureSizeChoices::Type Choice);

    ECheckBoxState IsExportDDS() const;
    void OnExportDDSChanged(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldAddExtension() const;
    void OnAddExtensionChanged(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldExportPhysics() const;
    void OnExportPhysicsChanged(ECheckBoxState NewCheckedState);

    void ConstructDialog(EDialogType::Type InDialogType);

    ECheckBoxState ShouldExportTwoSidedFoliage() const;
    void OnExportTwoSidedFoliageChanged(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldUsePreviewSurface() const;
    void OnUsePreviewSurfaceChanged(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldUseMDL() const;
    void OnUseMDLChanged(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldInstanceMesh() const;
    void OnInstanceMeshChanged(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldExportInvisibleLevel() const;
    void OnExportInvisibleLevel(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldExportPayload() const;
    void OnPayloadChanged(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldExportSublayers() const;
    void OnExportSublayers(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldUpYAxis() const;
    void OnUpYAxisChanged(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldExportDecalActors() const;
    void OnExportDecalActorsChanged(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldCreateMaterialOverSublayer() const;
    void OnCreateMaterialOverSublayer(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldExportPreviewMesh() const;
    void OnExportPreviewMesh(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldExportLandscapeGrass() const;
    void OnExportLandscapeGrass(ECheckBoxState NewCheckedState);

    ECheckBoxState ShouldRootIdentity() const;
    void OnRootIdentityChanged(ECheckBoxState NewCheckedState);

    TSharedRef<SWidget> OnGetComboBoxWidget(TSharedPtr<FName> InItem);
    void OnComboBoxChanged(TSharedPtr<FName> InItem, ESelectInfo::Type InSeletionInfo, FName* OutValue);
    FText GetComboBoxValueAsText(FName* InValue) const;
    FText GetComboBoxValueToolTipAsText(FName* InValue) const;

    bool IsIncludeDependenciesSelected() const;
    void OnDestPathTextCommitted(const FText& Text, ETextCommit::Type CommitType);
    FText GetDestPath() const;
    FReply DestPathClicked();

    void OnCheckpointTextCommitted(const FText& Text, ETextCommit::Type CommitType);

    void InitializeExportSettings();
private:
    FSimReadyExportSettings ExportSettings;
    ETextureSizeChoices::Type TextureSizeChoice;
    EExportMethodChoices::Type ExportMethodChoice;
    FString DestTemplate;
    TArray<class UObject*> ExportedObjects;
    TArray<class AActor*> ExportedActors;
    TArray<TSharedPtr<FName>> MDLOptions;
    FName MDLSelect;
    FString ExportPath;
    FString Checkpoint;
};