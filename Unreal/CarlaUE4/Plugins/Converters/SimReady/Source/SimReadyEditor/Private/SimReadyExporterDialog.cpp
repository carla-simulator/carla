// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyExporterDialog.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Interfaces/IMainFrameModule.h"
#include "SimReadyEditorExportUtils.h"
#include "SimReadyPathHelper.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "SimReadyUSDHelper.h"
#include "DesktopPlatformModule.h"
#include "SimReadyMessageLogContext.h"
#include "ISimReadyRuntimeModule.h"

#define LOCTEXT_NAMESPACE "SimReadyEditor"
#define MDL_INCLUDE_DEPENDENCIES_OPTION "Include Dependencies"
#define MDL_USE_CORE_DEPENDENCIES_OPTION "Use Core Library Dependencies"

void SSimReadyExporterDialog::InitializeExportSettings()
{
    MDLOptions.Add(MakeShareable(new FName(TEXT(MDL_INCLUDE_DEPENDENCIES_OPTION))));
    MDLOptions.Add(MakeShareable(new FName(TEXT(MDL_USE_CORE_DEPENDENCIES_OPTION))));

    if (ExportSettings.MaterialSettings.bCopyTemplate)
    {
        MDLSelect = TEXT(MDL_INCLUDE_DEPENDENCIES_OPTION);
    }
    else
    {
        MDLSelect = TEXT(MDL_USE_CORE_DEPENDENCIES_OPTION);
    }

    if (ExportSettings.MaterialSettings.TextureSettings.bTextureSource)
    {
        TextureSizeChoice = ETextureSizeChoices::UseTextureSource;
    }
    else
    {
        TextureSizeChoice = ETextureSizeChoices::UseTextureSetting;
    }

    if (ExportSettings.bModular)
    {
        ExportMethodChoice = EExportMethodChoices::ModularExport;
    }
    else
    {
        ExportMethodChoice = EExportMethodChoices::SharedExport;
    }
}

void SSimReadyExporterDialog::ConstructDialog(EDialogType::Type InDialogType)
{
    InitializeExportSettings();

    if (InDialogType == EDialogType::Level || InDialogType == EDialogType::Object)
    {
        ExportSettings.bEditLayer = true;
    }

    TSharedPtr<SVerticalBox> ExportMethodWidget = 
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 10)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Export Method")))
            ]
            + SVerticalBox::Slot()
            .HAlign(HAlign_Left)
            .AutoHeight()
            [
                SNew(SCheckBox)
                .Style(FEditorStyle::Get(), "RadioButton")
                .IsChecked(this, &SSimReadyExporterDialog::ExportMethodIsChecked, EExportMethodChoices::SharedExport)
                .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnExportMethodChanged, EExportMethodChoices::SharedExport)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Shared Export")))
                    .ToolTipText(FText::FromString(TEXT("Organizes materials in a directory to be shared by many assets.")))
                ]
            ]

            + SVerticalBox::Slot()
            .HAlign(HAlign_Left)
            .AutoHeight()
            [
                SNew(SCheckBox)
                .Style(FEditorStyle::Get(), "RadioButton")
                .IsChecked(this, &SSimReadyExporterDialog::ExportMethodIsChecked, EExportMethodChoices::ModularExport)
                .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnExportMethodChanged, EExportMethodChoices::ModularExport)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Modular Export")))
                    .ToolTipText(FText::FromString(TEXT("Materials are exported for each assets. This allows an asset directory to be moved easily, as it's self contained.")))
                ]
            ];

    TSharedPtr<SVerticalBox> MaterialWidget =
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .HAlign(HAlign_Left)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                .HAlign(HAlign_Left)
                [
                    SNew(SCheckBox)
                    .IsChecked(this, &SSimReadyExporterDialog::ShouldUseMDL)
                    .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnUseMDLChanged)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Include MDL")))
                        .ToolTipText(FText::FromString(TEXT("Generate MDL materials in the export")))
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .HAlign(HAlign_Left)
                .Padding(50, 0)
                [
                    SNew(SComboBox<TSharedPtr<FName>>)
                    .OptionsSource(&MDLOptions)
                    .OnSelectionChanged(this, &SSimReadyExporterDialog::OnComboBoxChanged, &MDLSelect)
                    .OnGenerateWidget(this, &SSimReadyExporterDialog::OnGetComboBoxWidget)
                    .Content()
                    [
                        SNew(STextBlock)
                        .Text(this, &SSimReadyExporterDialog::GetComboBoxValueAsText, &MDLSelect)
                        .ToolTipText(this, &SSimReadyExporterDialog::GetComboBoxValueToolTipAsText, &MDLSelect)
                    ]
                ]
            ]
            + SVerticalBox::Slot()
            .HAlign(HAlign_Left)
            .AutoHeight()
            .Padding(0, 10)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Destination Unreal Template Path (Optional)")))
            ]
            + SVerticalBox::Slot()
            .HAlign(HAlign_Left)
            .AutoHeight()
            .MaxHeight(30)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                .MaxWidth(307)
                .HAlign(HAlign_Left)
                [
                    SNew(SEditableTextBox)
                    .IsEnabled(this, &SSimReadyExporterDialog::IsIncludeDependenciesSelected)
                    .Text(this, &SSimReadyExporterDialog::GetDestPath)
                    .OnTextCommitted(this, &SSimReadyExporterDialog::OnDestPathTextCommitted)
                    .ToolTipText(FText::FromString(TEXT("Default - Same Path as Unreal MDL")))
                    .MinDesiredWidth(307)
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                .HAlign(HAlign_Left)
                .Padding(5)
                [
                    SNew(SButton)
                    .IsEnabled(this, &SSimReadyExporterDialog::IsIncludeDependenciesSelected)
                    .IsFocusable(false)
                    .OnClicked(this, &SSimReadyExporterDialog::DestPathClicked)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("...")))
                    ]
                ]
            ];

    TSharedPtr<SVerticalBox> TextureWidget =
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 10)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Texture Size")))
            ]
            + SVerticalBox::Slot()
            .HAlign(HAlign_Left)
            .AutoHeight()
            [
                SNew(SCheckBox)
                .Style(FEditorStyle::Get(), "RadioButton")
                .IsChecked(this, &SSimReadyExporterDialog::TextureSizeIsChecked, ETextureSizeChoices::UseTextureSetting)
                .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnTextureSizeChanged, ETextureSizeChoices::UseTextureSetting)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Use texture setting as export resolution")))
                ]
            ]
            + SVerticalBox::Slot()
            .HAlign(HAlign_Left)
            .AutoHeight()
            [
                SNew(SCheckBox)
                .Style(FEditorStyle::Get(), "RadioButton")
                .IsChecked(this, &SSimReadyExporterDialog::TextureSizeIsChecked, ETextureSizeChoices::UseTextureSource)
                .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnTextureSizeChanged, ETextureSizeChoices::UseTextureSource)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Use texture source as export resolution")))
                ]
            ];
            
    TSharedPtr<SCheckBox> DDSWidget =
            SNew(SCheckBox)
            .IsChecked(this, &SSimReadyExporterDialog::IsExportDDS)
            .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnExportDDSChanged)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Export Texture as DDS")))
                .ToolTipText(FText::FromString(TEXT("Save DDS Files to destination path and reference them from materials (WARNING, these cannot be reimported in Unreal, this is to save GPU texture memory in renderers)")))
            ];

    TSharedPtr<SCheckBox> TwoSidedFoliageWidget =
            SNew(SCheckBox)
            .IsChecked(this, &SSimReadyExporterDialog::ShouldExportTwoSidedFoliage)
            .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnExportTwoSidedFoliageChanged)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Export Backfaces for Two-Sided Foliage")))
                .ToolTipText(FText::FromString(TEXT("Generate additional backface geometry and materials for meshes that use both Two-Sided Shading Model and the Two-Sided material expression.")))
            ];

    TSharedPtr<SCheckBox> PreviewSurfaceWidget =
            SNew(SCheckBox)
            .IsChecked(this, &SSimReadyExporterDialog::ShouldUsePreviewSurface)
            .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnUsePreviewSurfaceChanged)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Include USD Preview Surface")))
                .ToolTipText(FText::FromString(TEXT("Generate USD Preview Surface materials in the export")))
            ];
        
    TSharedPtr<SCheckBox> PhysicsWidget =
            SNew(SCheckBox)
            .IsChecked(this, &SSimReadyExporterDialog::ShouldExportPhysics)
            .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnExportPhysicsChanged)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Export physics data (collision)")))
                .ToolTipText(FText::FromString(TEXT("Include collision data with static meshes")))
            ];
        
    TSharedPtr<SCheckBox> ScenegraphWidget =
            SNew(SCheckBox)
            .IsChecked(this, &SSimReadyExporterDialog::ShouldInstanceMesh)
            .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnInstanceMeshChanged)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("USD Scenegraph Instancing")))
                .ToolTipText(FText::FromString(TEXT("Make every referenced mesh in a stage an instance. If the stage has duplicate meshes this can greatly reduce geometric GPU memory in another renderer, but the editing workflow may be hampered.")))
            ];

    TSharedPtr<SCheckBox> DecalActorExportWidget =
            SNew(SCheckBox)
            .IsChecked(this, &SSimReadyExporterDialog::ShouldExportDecalActors)
            .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnExportDecalActorsChanged)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Export Decal Actors as planes")))
                .ToolTipText(FText::FromString(TEXT("Convert Decal actors to plane meshes with a cutout material for downstream renderers to flatten.")))
            ];

    TSharedPtr<SCheckBox> LevelExtWidget =
            SNew(SCheckBox)
            .IsChecked(this, &SSimReadyExporterDialog::ShouldAddExtension)
            .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnAddExtensionChanged)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Add extra extension to exported file, ie, .stage.usd, .prop.usd")))
                .ToolTipText(FText::FromString(TEXT("Stage files will be named .stage.usd, props will be named .prop.usd")))
            ];

    TSharedPtr<SCheckBox> PayloadWidget =
        SNew(SCheckBox)
        .IsChecked(this, &SSimReadyExporterDialog::ShouldExportPayload)
        .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnPayloadChanged)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Export as Payloads")))
            .ToolTipText(FText::FromString(TEXT("Instead of references, each prop within a stage will be a payload.")))
        ];

    TSharedPtr<SCheckBox> SublayersWidget =
        SNew(SCheckBox)
        .IsChecked(this, &SSimReadyExporterDialog::ShouldExportSublayers)
        .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnExportSublayers)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Export Sublevels as Sublayers")))
            .ToolTipText(FText::FromString(TEXT("Each sublevel will become a sublayer in the output USD stage")))
        ];
        
    TSharedPtr<SCheckBox> UpYAxisWidget =
        SNew(SCheckBox)
        .IsChecked(this, &SSimReadyExporterDialog::ShouldUpYAxis)
        .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnUpYAxisChanged)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Export as Y-up Axis")))
            .ToolTipText(FText::FromString(TEXT("If checked, USD and MDL are exported with a Y - up stage up - axis rather than the default Z - up")))
        ];
    TSharedPtr<SCheckBox> PreviewMeshWidget =
        SNew(SCheckBox)
        .IsChecked(this, &SSimReadyExporterDialog::ShouldExportPreviewMesh)
        .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnExportPreviewMesh)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Export Preview Mesh")))
        ];

    TSharedPtr<SCheckBox> InvisibleLevelWidget =
            SNew(SCheckBox)
            .IsChecked(this, &SSimReadyExporterDialog::ShouldExportInvisibleLevel)
            .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnExportInvisibleLevel)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Include invisible levels")))
                .ToolTipText(FText::FromString(TEXT("If a sublevel is hidden it will still be exported.  The default behavior is to skip invisible sublevels.")))
            ];

    TSharedPtr<SCheckBox> MaterialOverSublayerWidget =
        SNew(SCheckBox)
        .IsChecked(this, &SSimReadyExporterDialog::ShouldCreateMaterialOverSublayer)
        .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnCreateMaterialOverSublayer)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Create a material overrides sublayer")))
            .ToolTipText(FText::FromString(TEXT("Export a sublayer that contains all of the material reference overs for the stage. Also create USD files for each material rather than including them in the prop stages.")))
        ];

    TSharedPtr<SCheckBox> LandscapeGrassWidget =
        SNew(SCheckBox)
        .IsChecked(this, &SSimReadyExporterDialog::ShouldExportLandscapeGrass)
        .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnExportLandscapeGrass)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Export Landscape Grass")))
            .ToolTipText(FText::FromString(TEXT("If checked Landscape Grass will be exported.  This option is provided because in some levels excess geometry can be exported.")))
        ];

    TSharedPtr<SCheckBox> RootIdentityWidget =
        SNew(SCheckBox)
        .IsChecked(this, &SSimReadyExporterDialog::ShouldRootIdentity)
        .OnCheckStateChanged(this, &SSimReadyExporterDialog::OnRootIdentityChanged)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Convert Skeleton root to identity transform")))
            .ToolTipText(FText::FromString(TEXT("This option will convert a Skeleton's root node to an identity transform to assist downstream tools that require this.  All animations exported during this export will be transformed properly so they'll work in downstream tools.")))
        ];

    TSharedPtr<SCheckBox> InvisibleComponentWidget =
        SNew(SCheckBox)
        .IsChecked_Lambda([this](){
            return ExportSettings.bExportInvisibleComponent ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
            })
        .OnCheckStateChanged_Lambda([this](ECheckBoxState NewCheckedState) {
                ExportSettings.bExportInvisibleComponent = (NewCheckedState == ECheckBoxState::Checked);
            })
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Include invisible components")))
            .ToolTipText(FText::FromString(TEXT("If a component is hidden or invisible it will still be exported.  The default behavior is to export invisible components.")))
        ];

    TSharedPtr<SCheckBox> EditLayerWidget =
        SNew(SCheckBox)
        .IsChecked_Lambda([&]() {
        return ExportSettings.bEditLayer ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
            })
        .OnCheckStateChanged_Lambda([&](ECheckBoxState NewCheckedState) {
                ExportSettings.bEditLayer = (NewCheckedState == ECheckBoxState::Checked);
            })
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Create a sublayer for editing")))
        .ToolTipText(FText::FromString(TEXT("The option will create a sublayer for editing that won't be overwritten by re-exporting.")))
        ];

    TSharedPtr<SUniformGridPanel> ConfirmWidget =
            SNew(SUniformGridPanel)
            .SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
            .MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
            .MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
            + SUniformGridPanel::Slot(0, 0)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
                .Text(LOCTEXT("OK", "OK"))
                .OnClicked(this, &SSimReadyExporterDialog::OkClicked)
            ]
            + SUniformGridPanel::Slot(1, 0)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
                .Text(LOCTEXT("Cancel", "Cancel"))
                .OnClicked(this, &SSimReadyExporterDialog::CancelClicked)
            ];

    const int32 BaseSizeY = 90; // Confirm window
    const int32 BaseMaterialSizeY = 110;
    const int32 BaseTextureSizeY = 80;
    const int32 BaseLevelSizeY = 90;
    const int32 ObjectNumItems = 10; // items except material and texture
    const int32 AnimationNumItems = 5;
    const int32 TextureNumItems = 1;
    const int32 MaterialNumItems = 1;
    const int32 LevelNumItems = 15;
    const int32 ItemSizeY = 30;
    const int32 LevelGroupingSizeY = 70;

    const int32 ClientSizeX = 500;
    int32 ClientSizeY = BaseSizeY;

    switch (InDialogType)
    {
    case EDialogType::Object:
        ClientSizeY += BaseMaterialSizeY + BaseTextureSizeY + ObjectNumItems * ItemSizeY;
        SWindow::Construct(SWindow::FArguments()
            .Title(LOCTEXT("SimReadyExporterDialogTitle", "SimReady Exporter"))
            .SizingRule(ESizingRule::UserSized)
            .ClientSize(FVector2D(ClientSizeX, ClientSizeY))
            .SupportsMinimize(false)
            .SupportsMaximize(false)
            .bDragAnywhere(false)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 10)
                [
                    MaterialWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 10)
                [
                    TextureWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    DDSWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    PreviewSurfaceWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    PhysicsWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    ScenegraphWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    PayloadWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    UpYAxisWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    InvisibleComponentWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    RootIdentityWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    LandscapeGrassWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    EditLayerWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(30)
                [
                    ConfirmWidget.ToSharedRef()
                ]
            ]
        );

        break;
    case EDialogType::Animation:
        ClientSizeY += AnimationNumItems * ItemSizeY;
        SWindow::Construct(SWindow::FArguments()
            .Title(LOCTEXT("SimReadyExporterDialogTitle", "SimReady Exporter"))
            .SizingRule(ESizingRule::UserSized)
            .ClientSize(FVector2D(ClientSizeX, ClientSizeY))
            .SupportsMinimize(false)
            .SupportsMaximize(false)
            .bDragAnywhere(false)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    PreviewMeshWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    ScenegraphWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    PayloadWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    UpYAxisWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    RootIdentityWidget.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(30)
                [
                    ConfirmWidget.ToSharedRef()
                ]
            ]
        );
        break;
    case EDialogType::Material:
        ClientSizeY += BaseMaterialSizeY + BaseTextureSizeY + MaterialNumItems * ItemSizeY;
        SWindow::Construct(SWindow::FArguments()
            .Title(LOCTEXT("SimReadyExporterDialogTitle", "SimReady Exporter"))
            .SizingRule(ESizingRule::UserSized)
            .ClientSize(FVector2D(ClientSizeX, ClientSizeY))
            .SupportsMinimize(false)
            .SupportsMaximize(false)
            .bDragAnywhere(false)
            [
                SNew(SVerticalBox)
                +SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 10)
                [
                    MaterialWidget.ToSharedRef()
                ]

                +SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 10)
                [
                    TextureWidget.ToSharedRef()
                ]
                +SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    DDSWidget.ToSharedRef()
                ]
                +SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(30)
                [
                    ConfirmWidget.ToSharedRef()
                ]
            ]
        );
        break;
    case EDialogType::Texture:
        ClientSizeY += BaseTextureSizeY + TextureNumItems * ItemSizeY;
        SWindow::Construct(SWindow::FArguments()
            .Title(LOCTEXT("SimReadyExporterDialogTitle", "SimReady Exporter"))
            .SizingRule(ESizingRule::UserSized)
            .ClientSize(FVector2D(ClientSizeX, ClientSizeY))
            .SupportsMinimize(false)
            .SupportsMaximize(false)
            .bDragAnywhere(false)
            [
                SNew(SVerticalBox)
                +SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 10)
                [
                    TextureWidget.ToSharedRef()
                ]
                +SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(100, 7)
                [
                    DDSWidget.ToSharedRef()
                ]
                +SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(30)
                [
                    ConfirmWidget.ToSharedRef()
                ]
            ]
        );
        break;
    case EDialogType::Level:
        ClientSizeY += BaseLevelSizeY + BaseMaterialSizeY + BaseTextureSizeY + LevelNumItems * ItemSizeY + LevelGroupingSizeY;
        SWindow::Construct(SWindow::FArguments()
            .Title(LOCTEXT("SimReadyExporterDialogTitle", "SimReady Exporter"))
            .SizingRule(ESizingRule::UserSized)
            .ClientSize(FVector2D(ClientSizeX, ClientSizeY))
            .SupportsMinimize(false)
            .SupportsMaximize(false)
            .bDragAnywhere(false)
            [
                SNew(SScrollBox)
                .Orientation(Orient_Vertical)
                +SScrollBox::Slot()
                [
                SNew(SVerticalBox)
                +SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(50, 1)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Material Settings")))
                ]
                +SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(50, 10)
                [
                    SNew(SBorder)
                    .Padding(5)
                    .Content()
                    [
                        SNew(SVerticalBox)
                        +SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            ExportMethodWidget.ToSharedRef()
                        ]
                        +SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            MaterialWidget.ToSharedRef()
                        ]
                        +SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            TextureWidget.ToSharedRef()
                        ]
                        +SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            DDSWidget.ToSharedRef()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            PreviewSurfaceWidget.ToSharedRef()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            MaterialOverSublayerWidget.ToSharedRef()
                        ]
                    ]
                ]
                +SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(50, 1)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Stage Settings")))
                ]
                +SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                .Padding(50, 10)
                [
                    SNew(SBorder)
                    .Padding(5)
                    .Content()
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            PhysicsWidget.ToSharedRef()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            ScenegraphWidget.ToSharedRef()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            PayloadWidget.ToSharedRef()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            UpYAxisWidget.ToSharedRef()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            RootIdentityWidget.ToSharedRef()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            SublayersWidget.ToSharedRef()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            InvisibleLevelWidget.ToSharedRef()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            InvisibleComponentWidget.ToSharedRef()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            DecalActorExportWidget.ToSharedRef()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            LandscapeGrassWidget.ToSharedRef()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            LevelExtWidget.ToSharedRef()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        .Padding(10, 7)
                        [
                            EditLayerWidget.ToSharedRef()
                        ]
                    ]
                ]
                +SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(30)
                [
                    ConfirmWidget.ToSharedRef()
                ]
            ]
            ]
        );

        break;
    }
}

void SSimReadyExporterDialog::Construct(const FArguments& InArgs, EDialogType::Type InDialogType, const TArray<UObject*>& InExportedObjects, const FString& InExportPath)
{
    ExportedObjects = InExportedObjects;
    ExportPath = InExportPath;

    ConstructDialog(InDialogType);
}

void SSimReadyExporterDialog::Construct(const FArguments& InArgs, const TArray<AActor*>& InExportedActors, const FString& InExportPath)
{
    ExportedActors = InExportedActors;
    ExportPath = InExportPath;

    ConstructDialog(EDialogType::Object);
}

ECheckBoxState SSimReadyExporterDialog::ExportMethodIsChecked(EExportMethodChoices::Type Choice) const
{
    if (ExportMethodChoice == Choice)
    {
        return ECheckBoxState::Checked;
    }

    return ECheckBoxState::Unchecked; 
}

void SSimReadyExporterDialog::OnExportMethodChanged(ECheckBoxState NewCheckedState, EExportMethodChoices::Type Choice)
{
    if (NewCheckedState == ECheckBoxState::Checked)
    {
        ExportMethodChoice = Choice;
    }
}

ECheckBoxState SSimReadyExporterDialog::TextureSizeIsChecked(ETextureSizeChoices::Type Choice) const
{
    if (TextureSizeChoice == Choice)
    {
        return ECheckBoxState::Checked;
    }

    return ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnTextureSizeChanged(ECheckBoxState NewCheckedState, ETextureSizeChoices::Type Choice)
{
    if (NewCheckedState == ECheckBoxState::Checked)
    {
        TextureSizeChoice = Choice;
    }
}

ECheckBoxState SSimReadyExporterDialog::IsExportDDS() const
{
    return ExportSettings.MaterialSettings.TextureSettings.bDDSExport ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnExportDDSChanged(ECheckBoxState NewCheckedState)
{
    ExportSettings.MaterialSettings.TextureSettings.bDDSExport = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldAddExtension() const
{
    return ExportSettings.bAddExtraExtension ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnAddExtensionChanged(ECheckBoxState NewCheckedState)
{
    ExportSettings.bAddExtraExtension = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldExportPhysics() const
{
    return ExportSettings.bExportPhysics ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnExportPhysicsChanged(ECheckBoxState NewCheckedState)
{
    ExportSettings.bExportPhysics = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldInstanceMesh() const
{
    return ExportSettings.bMeshInstanced ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnInstanceMeshChanged(ECheckBoxState NewCheckedState)
{
    ExportSettings.bMeshInstanced = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldExportTwoSidedFoliage() const
{
    return ExportSettings.MaterialSettings.bExportTwoSidedSign ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnExportTwoSidedFoliageChanged(ECheckBoxState NewCheckedState)
{
    ExportSettings.MaterialSettings.bExportTwoSidedSign = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldUsePreviewSurface() const
{
    return ExportSettings.bPreviewSurface ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnUsePreviewSurfaceChanged(ECheckBoxState NewCheckedState)
{
    ExportSettings.bPreviewSurface = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldExportInvisibleLevel() const
{
    return ExportSettings.bExportInvisibleLevel ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnExportInvisibleLevel(ECheckBoxState NewCheckedState)
{
    ExportSettings.bExportInvisibleLevel = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldUseMDL() const
{
    return ExportSettings.bMDL ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnUseMDLChanged(ECheckBoxState NewCheckedState)
{
    ExportSettings.bMDL = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldExportPayload() const
{
    return ExportSettings.bPayloads ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnPayloadChanged(ECheckBoxState NewCheckedState)
{
    ExportSettings.bPayloads = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldExportSublayers() const
{
    return ExportSettings.bExportSublayers ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnExportSublayers(ECheckBoxState NewCheckedState)
{
    ExportSettings.bExportSublayers = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldUpYAxis() const
{
    return ExportSettings.bUpYAxis ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnUpYAxisChanged(ECheckBoxState NewCheckedState)
{
    ExportSettings.bUpYAxis = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldExportDecalActors() const
{
    return ExportSettings.bExportDecalActors ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnExportDecalActorsChanged(ECheckBoxState NewCheckedState)
{
    ExportSettings.bExportDecalActors = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldExportPreviewMesh() const
{
    return ExportSettings.bExportPreviewMesh ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnExportPreviewMesh(ECheckBoxState NewCheckedState)
{
    ExportSettings.bExportPreviewMesh = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldExportLandscapeGrass() const
{
    return ExportSettings.bExportLandscapeGrass ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnExportLandscapeGrass(ECheckBoxState NewCheckedState)
{
    ExportSettings.bExportLandscapeGrass = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldRootIdentity() const
{
    return ExportSettings.bRootIdentity ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SSimReadyExporterDialog::OnRootIdentityChanged(ECheckBoxState NewCheckedState)
{
    ExportSettings.bRootIdentity = (NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSimReadyExporterDialog::ShouldCreateMaterialOverSublayer() const
{
    return ExportSettings.bMaterialOverSublayer ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSimReadyExporterDialog::OnCreateMaterialOverSublayer(ECheckBoxState NewCheckedState)
{
    ExportSettings.bMaterialOverSublayer = (NewCheckedState == ECheckBoxState::Checked);
}

TSharedRef<SWidget> SSimReadyExporterDialog::OnGetComboBoxWidget(TSharedPtr<FName> InItem)
{
    return SNew(STextBlock).Text(FText::FromName(InItem.IsValid() ? *InItem : NAME_None));
}

void SSimReadyExporterDialog::OnComboBoxChanged(TSharedPtr<FName> InItem, ESelectInfo::Type InSeletionInfo, FName* OutValue)
{
    *OutValue = *InItem;
}

FText SSimReadyExporterDialog::GetComboBoxValueAsText(FName* InValue) const
{
    return FText::FromName(*InValue);
}

FText SSimReadyExporterDialog::GetComboBoxValueToolTipAsText(FName* InValue) const
{
    if (InValue->Compare(FName(TEXT(MDL_USE_CORE_DEPENDENCIES_OPTION))) == 0)
    {
        return FText::FromString(TEXT("Library MDL files are referenced from the Core materials. All other material templates and collection dependencies are copied on export."));
    }
    else
    {
        return FText::FromString(TEXT("All material library items, templates and collection dependencies are copied on export."));
    }
}

bool SSimReadyExporterDialog::IsIncludeDependenciesSelected() const
{
    return (MDLSelect.Compare(TEXT(MDL_INCLUDE_DEPENDENCIES_OPTION)) == 0);
}

void SSimReadyExporterDialog::OnDestPathTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
    auto DestPath = Text.ToString();
    ExportSettings.MaterialSettings.DestTemplatePath = DestPath;
}

FText SSimReadyExporterDialog::GetDestPath() const
{
    return FText::FromString(ExportSettings.MaterialSettings.DestTemplatePath);
}

FReply SSimReadyExporterDialog::DestPathClicked()
{
    if (!ExportPath.IsEmpty())
    {
        IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
        if (DesktopPlatform)
        {
            DesktopPlatform->OpenDirectoryDialog(
                FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
                NSLOCTEXT("SimReady", "ChooseADirectory", "ChooseADirectory").ToString(),
                ExportPath, ExportSettings.MaterialSettings.DestTemplatePath);
        }
    }

    return FReply::Handled();
}

FReply SSimReadyExporterDialog::OkClicked()
{
    RequestDestroyWindow();
    ExportSettings.bModular = ExportMethodChoice == EExportMethodChoices::ModularExport;
    ExportSettings.MaterialSettings.bCopyTemplate = (MDLSelect.Compare(TEXT(MDL_INCLUDE_DEPENDENCIES_OPTION)) == 0);
    ExportSettings.MaterialSettings.TextureSettings.bTextureSource = TextureSizeChoice == ETextureSizeChoices::UseTextureSource;

    if (!ExportPath.IsEmpty() && FPaths::GetExtension(ExportPath).ToLower() == TEXT("usda"))
    {
        ExportSettings.bAsciiFormat = true;
    }

    FSimReadyMessageLogContext::ClearMessages();

    // For local export : convert to unmangled file name
    auto GetFullExportPath = [](const FString& InExportPath)
    {
        FString AbsolutePath = FPaths::ConvertRelativePathToFull(InExportPath);
        FString Path;
        FString Filename;
        FString Ext;
        FPaths::Split(AbsolutePath, Path, Filename, Ext);

        FString OutFilename;
        FSimReadyPathHelper::UnmanglePath(Filename, OutFilename, false);
        if (Ext.IsEmpty())
        {
            return FString(Path / OutFilename);
        }
        else
        {
            return FString(Path / OutFilename + TEXT(".") + Ext);
        }
    };

    if (ExportedActors.Num() > 0)
    {
        if (!ExportPath.IsEmpty())
        {
            FSimReadyUSDHelper::ExportActorsAsUSDToPath(ExportedActors, GetFullExportPath(FPaths::ConvertRelativePathToFull(ExportPath)), ExportSettings);
        }
    }
    
    if (ExportedObjects.Num() > 0)
    {
        if (!ExportPath.IsEmpty())
        {
            FString AbsoluteExportPath = GetFullExportPath(FPaths::ConvertRelativePathToFull(ExportPath));
            for (auto Object : ExportedObjects)
            {
                if (Object->IsA<UWorld>())
                {
                    FSimReadyUSDHelper::ExportUWorldAsUSDToPath(Cast<UWorld>(Object), AbsoluteExportPath, ExportSettings);
                }
                else
                {
                    FSimReadyUSDHelper::ExportObjectAsUSDToPath(Object, AbsoluteExportPath, ExportSettings);
                }

                // Only the first object
                break;
            }
        }
    }

    FSimReadyMessageLogContext::DisplayMessages();

    return FReply::Handled();
}

FReply SSimReadyExporterDialog::CancelClicked()
{
    RequestDestroyWindow();
    return FReply::Handled();
}

void SSimReadyExporterDialog::ShowDialog(EDialogType::Type DialogType, const TArray<UObject*>& InExportedObjects, const FString& InExportPath)
{
    // Create the window to pick the class
    TSharedRef<SSimReadyExporterDialog> SimReadyExporterDialog = SNew(SSimReadyExporterDialog, DialogType, InExportedObjects, InExportPath);

    TSharedPtr<SWindow> ParentWindow;
    if( FModuleManager::Get().IsModuleLoaded( "MainFrame" ) )
    {
        IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>( "MainFrame" );
        ParentWindow = MainFrame.GetParentWindow();
    }

    FSlateApplication::Get().AddModalWindow(SimReadyExporterDialog, ParentWindow, false);
}

void SSimReadyExporterDialog::ShowDialog(const TArray<AActor*>& InExportedActors, const FString& InExportPath)
{
    // Create the window to pick the class
    TSharedRef<SSimReadyExporterDialog> SimReadyExporterDialog = SNew(SSimReadyExporterDialog, InExportedActors, InExportPath);
    TSharedPtr<SWindow> ParentWindow;
    if( FModuleManager::Get().IsModuleLoaded( "MainFrame" ) )
    {
        IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>( "MainFrame" );
        ParentWindow = MainFrame.GetParentWindow();
    }

    FSlateApplication::Get().AddModalWindow(SimReadyExporterDialog, ParentWindow, false);
}
#undef LOCTEXT_NAMESPACE
