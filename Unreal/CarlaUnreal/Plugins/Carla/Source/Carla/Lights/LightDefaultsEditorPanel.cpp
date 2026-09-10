// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "LightDefaultsEditorPanel.h"

#if WITH_EDITOR

#include <util/ue-header-guard-begin.h>
#include "Editor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Styling/AppStyle.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Colors/SColorPicker.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"
#include <util/ue-header-guard-end.h>

#define LOCTEXT_NAMESPACE "SLightDefaultsPanel"

namespace
{
    // One row: a class summary plus its editable fields (Count, Intensity,
    // Emissive, Color) -- LightGroup itself isn't repeated per row since the
    // panel's group tabs already filter to one group at a time. Every edit
    // pushes live onto the level immediately (OnLiveChanged); nothing here
    // writes Defaults.json -- that's the panel's single "Save All" button,
    // once an artist is happy with what they see.
    class SLightDefaultsRow : public STableRow<TSharedPtr<FLightAssetSummary>>
    {
    public:
        SLATE_BEGIN_ARGS(SLightDefaultsRow) {}
        SLATE_END_ARGS()

        void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable, TSharedPtr<FLightAssetSummary> InItem,
            TFunction<void(TSharedPtr<FLightAssetSummary>)> InOnLiveChanged, TFunction<void(const FString&, bool)> InOnToggleSelect,
            TFunction<void(const FString&, bool)> InOnTogglePreviewOn = nullptr)
        {
            Item = InItem;
            OnLiveChanged = MoveTemp(InOnLiveChanged);
            OnToggleSelect = MoveTemp(InOnToggleSelect);
            OnTogglePreviewOn = MoveTemp(InOnTogglePreviewOn);
            // Slate's SNew required-args plumbing caps at 5 positional
            // arguments (RequiredArgs::MakeRequiredArgs), already spent on
            // (OwnerTable, Item, OnLiveChanged, OnToggleSelect,
            // OnTogglePreviewOn) -- so a row's display mode is detected from
            // data already in hand instead of extra constructor arguments.
            // Traffic lights (OnTogglePreviewOn set) hide Intensity and Color
            // entirely -- neither is meaningful for a signal lens. Buildings
            // (fixed synthetic ClassName) hide Color only: their Intensity
            // slot is repurposed as "Percent Lit" (see
            // ULightDefaultsJsonUtils::ApplyProceduralBuildingsEmissiveLive).
            const bool bIsTrafficRow = static_cast<bool>(OnTogglePreviewOn);
            const bool bIsBuildingsRow = Item.IsValid() && Item->ClassName == TEXT("ProceduralBuildings");
            const bool bIsLegacyBuildingsRow = Item.IsValid() && Item->ClassName == TEXT("LegacyBuildings");
            const bool bIsAnyBuildingsRow = bIsBuildingsRow || bIsLegacyBuildingsRow;
            const bool bShowIntensity = !bIsTrafficRow;
            const bool bShowColor = !bIsTrafficRow && !bIsAnyBuildingsRow;

            TSharedRef<SHorizontalBox> RowBox = SNew(SHorizontalBox)

                + SHorizontalBox::Slot().AutoWidth().Padding(2.0f).VAlign(VAlign_Center)
                [
                    SNew(SBox).WidthOverride(240.0f)
                    [
                        SNew(STextBlock).Text(FText::FromString(Item->ClassName))
                    ]
                ]

                + SHorizontalBox::Slot().AutoWidth().Padding(2.0f).VAlign(VAlign_Center)
                [
                    SNew(SBox).WidthOverride(50.0f)
                    [
                        SNew(STextBlock).Text(FText::FromString(FString::FromInt(Item->InstanceCount)))
                    ]
                ];

            if (bShowIntensity)
            {
                const TCHAR* IntensityHint = bIsAnyBuildingsRow ? TEXT("Percent Lit") : TEXT("Intensity");
                RowBox->AddSlot().AutoWidth().Padding(2.0f)
                [ MakeNumberBox(IntensityHint, Item->Current.Intensity, [this](float V) { Item->Current.Intensity = V; NotifyLiveChanged(); }) ];
            }

            RowBox->AddSlot().AutoWidth().Padding(2.0f)
            [ MakeNumberBox(TEXT("Emissive"), Item->Current.EmissiveIntensity, [this](float V) { Item->Current.EmissiveIntensity = V; NotifyLiveChanged(); }) ];

            if (bShowColor)
            {
                RowBox->AddSlot().AutoWidth().Padding(2.0f).VAlign(VAlign_Center)
                [ MakeColorSwatch(this) ];
            }

            RowBox->AddSlot().AutoWidth().Padding(6.0f, 2.0f).VAlign(VAlign_Center)
            [
                SNew(SCheckBox)
                .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
                .Padding(FMargin(8.0f, 2.0f))
                .IsChecked_Lambda([this]() { return bIsSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
                .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
                {
                    bIsSelected = (NewState == ECheckBoxState::Checked);
                    if (OnToggleSelect)
                    {
                        OnToggleSelect(Item->ClassName, bIsSelected);
                    }
                })
                [
                    SNew(STextBlock).Text(LOCTEXT("SelectButton", "Select"))
                ]
            ];

            if (OnTogglePreviewOn)
            {
                RowBox->AddSlot().AutoWidth().Padding(2.0f, 2.0f).VAlign(VAlign_Center)
                [
                    SNew(SCheckBox)
                    .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
                    .Padding(FMargin(8.0f, 2.0f))
                    .IsChecked_Lambda([this]() { return bIsPreviewOn ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
                    .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
                    {
                        bIsPreviewOn = (NewState == ECheckBoxState::Checked);
                        if (OnTogglePreviewOn)
                        {
                            OnTogglePreviewOn(Item->ClassName, bIsPreviewOn);
                        }
                    })
                    [
                        SNew(STextBlock).Text(LOCTEXT("PreviewOnButton", "Preview On"))
                    ]
                ];
            }

            STableRow<TSharedPtr<FLightAssetSummary>>::Construct(
                STableRow<TSharedPtr<FLightAssetSummary>>::FArguments()
                .Padding(FMargin(4.0f, 2.0f))
                [ RowBox ],
                OwnerTable);
        }

    private:
        TSharedPtr<FLightAssetSummary> Item;
        TFunction<void(TSharedPtr<FLightAssetSummary>)> OnLiveChanged;
        TFunction<void(const FString&, bool)> OnToggleSelect;
        TFunction<void(const FString&, bool)> OnTogglePreviewOn;
        bool bIsSelected = false;
        bool bIsPreviewOn = false;

        void NotifyLiveChanged()
        {
            if (OnLiveChanged)
            {
                OnLiveChanged(Item);
            }
        }

        static TSharedRef<SWidget> MakeNumberBox(const TCHAR* Hint, float InitialValue, TFunction<void(float)> OnChanged)
        {
            return SNew(SBox).WidthOverride(70.0f)
            [
                SNew(SEditableTextBox)
                .HintText(FText::FromString(Hint))
                .ToolTipText(FText::FromString(Hint))
                .Text(FText::FromString(FString::SanitizeFloat(InitialValue)))
                .OnTextCommitted_Lambda([OnChanged](const FText& NewText, ETextCommit::Type)
                {
                    OnChanged(FCString::Atof(*NewText.ToString()));
                })
            ];
        }

        // A clickable color swatch that opens UE's own color picker -- no
        // separate R/G/B text fields.
        static TSharedRef<SWidget> MakeColorSwatch(SLightDefaultsRow* Row)
        {
            TSharedPtr<FLightAssetSummary> InItem = Row->Item;
            return SNew(SBox).WidthOverride(40.0f).HeightOverride(20.0f)
            [
                SNew(SColorBlock)
                .Color_Lambda([InItem]() { return InItem->Current.Color; })
                .ShowBackgroundForAlpha(false)
                .OnMouseButtonDown_Lambda([InItem, Row](const FGeometry&, const FPointerEvent&) -> FReply
                {
                    FColorPickerArgs PickerArgs;
                    PickerArgs.bUseAlpha = false;
                    PickerArgs.InitialColor = InItem->Current.Color;
                    PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateLambda(
                        [InItem, Row](FLinearColor NewColor) { InItem->Current.Color = NewColor; Row->NotifyLiveChanged(); });
                    OpenColorPicker(PickerArgs);
                    return FReply::Handled();
                })
            ];
        }
    };

    // Vehicles row: a fixed global "Intensity" field plus one narrow field
    // per canonical light group (ULightDefaultsJsonUtils::GetVehicleLightGroupNames),
    // disabled/greyed when Item->GroupPresent says this particular vehicle's
    // material doesn't expose that parameter -- rather than showing a
    // control that would silently do nothing. No Color (vehicles' Emissive
    // material has no such concept) and no Emissive column (Intensity here
    // IS that column, see FVehicleLightSummary). "Preview" replaces Select:
    // most rows have no placed instance in the level to jump to, since this
    // tab lists the full catalog, not a level scan.
    class SVehicleLightRow : public STableRow<TSharedPtr<FVehicleLightSummary>>
    {
    public:
        SLATE_BEGIN_ARGS(SVehicleLightRow) {}
        SLATE_END_ARGS()

        void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable, TSharedPtr<FVehicleLightSummary> InItem,
            TFunction<void(TSharedPtr<FVehicleLightSummary>)> InOnLiveChanged, TFunction<void(const FString&, bool)> InOnTogglePreview)
        {
            Item = InItem;
            OnLiveChanged = MoveTemp(InOnLiveChanged);
            OnTogglePreview = MoveTemp(InOnTogglePreview);

            TSharedRef<SHorizontalBox> RowBox = SNew(SHorizontalBox)

                + SHorizontalBox::Slot().AutoWidth().Padding(2.0f).VAlign(VAlign_Center)
                [
                    SNew(SBox).WidthOverride(200.0f)
                    [
                        SNew(STextBlock).Text(FText::FromString(Item->DisplayName)).ToolTipText(FText::FromString(Item->ClassName))
                    ]
                ]

                + SHorizontalBox::Slot().AutoWidth().Padding(2.0f).VAlign(VAlign_Center)
                [
                    SNew(SBox).WidthOverride(40.0f)
                    [
                        SNew(STextBlock).Text(FText::FromString(FString::FromInt(Item->InstanceCount)))
                    ]
                ]

                + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
                [ MakeNumberBox(TEXT("Intensity"), 55.0f, Item->Intensity, [this](float V) { Item->Intensity = V; NotifyLiveChanged(); }) ];

            for (const FString& GroupName : ULightDefaultsJsonUtils::GetVehicleLightGroupNames())
            {
                const bool bPresent = Item->GroupPresent.Contains(GroupName) && Item->GroupPresent[GroupName];
                const float InitialValue = Item->GroupIntensity.Contains(GroupName) ? Item->GroupIntensity[GroupName] : 0.0f;
                TSharedRef<SWidget> Field = MakeNumberBox(*GroupName, 55.0f, InitialValue, [this, GroupName](float V)
                {
                    Item->GroupIntensity.Add(GroupName, V);
                    NotifyLiveChanged();
                });
                Field->SetEnabled(bPresent);
                RowBox->AddSlot().AutoWidth().Padding(1.0f)[ Field ];
            }

            RowBox->AddSlot().AutoWidth().Padding(6.0f, 2.0f).VAlign(VAlign_Center)
            [
                SNew(SCheckBox)
                .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
                .Padding(FMargin(8.0f, 2.0f))
                .IsChecked_Lambda([this]() { return bIsPreviewOn ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
                .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
                {
                    bIsPreviewOn = (NewState == ECheckBoxState::Checked);
                    if (OnTogglePreview)
                    {
                        OnTogglePreview(Item->ClassName, bIsPreviewOn);
                    }
                    if (bIsPreviewOn)
                    {
                        // The freshly spawned actor still has the asset's raw
                        // authored material values -- push whatever's
                        // currently in this row onto it immediately, instead
                        // of waiting for the next field edit.
                        NotifyLiveChanged();
                    }
                })
                [
                    SNew(STextBlock).Text(LOCTEXT("PreviewVehicleButton", "Preview"))
                ]
            ];

            STableRow<TSharedPtr<FVehicleLightSummary>>::Construct(
                STableRow<TSharedPtr<FVehicleLightSummary>>::FArguments()
                .Padding(FMargin(4.0f, 2.0f))
                [ RowBox ],
                OwnerTable);
        }

    private:
        TSharedPtr<FVehicleLightSummary> Item;
        TFunction<void(TSharedPtr<FVehicleLightSummary>)> OnLiveChanged;
        TFunction<void(const FString&, bool)> OnTogglePreview;
        bool bIsPreviewOn = false;

        void NotifyLiveChanged()
        {
            if (OnLiveChanged)
            {
                OnLiveChanged(Item);
            }
        }

        static TSharedRef<SWidget> MakeNumberBox(const TCHAR* Hint, float Width, float InitialValue, TFunction<void(float)> OnChanged)
        {
            return SNew(SBox).WidthOverride(Width)
            [
                SNew(SEditableTextBox)
                .HintText(FText::FromString(Hint))
                .ToolTipText(FText::FromString(Hint))
                .Text(FText::FromString(FString::SanitizeFloat(InitialValue)))
                .OnTextCommitted_Lambda([OnChanged](const FText& NewText, ETextCommit::Type)
                {
                    OnChanged(FCString::Atof(*NewText.ToString()));
                })
            ];
        }
    };
}

TSharedRef<SWidget> SLightDefaultsPanel::MakeGroupTab(ELightType Group, const FText& Label)
{
    return SNew(SCheckBox)
        .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
        .Padding(FMargin(10.0f, 4.0f))
        .IsChecked_Lambda([this, Group]()
        {
            return (!bShowingTrafficLights && !bShowingBuildings && !bShowingVehicles && ActiveFilter == Group) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
        })
        .OnCheckStateChanged_Lambda([this, Group](ECheckBoxState)
        {
            bShowingTrafficLights = false;
            bShowingBuildings = false;
            bShowingVehicles = false;
            ActiveFilter = Group;
            ApplyFilter();
        })
        [
            SNew(STextBlock).Text(Label)
        ];
}

TSharedRef<SWidget> SLightDefaultsPanel::MakeTrafficTab()
{
    return SNew(SCheckBox)
        .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
        .Padding(FMargin(10.0f, 4.0f))
        .IsChecked_Lambda([this]()
        {
            return bShowingTrafficLights ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
        })
        .OnCheckStateChanged_Lambda([this](ECheckBoxState)
        {
            bShowingTrafficLights = true;
            bShowingBuildings = false;
            bShowingVehicles = false;
            ApplyFilter();
        })
        [
            SNew(STextBlock).Text(LOCTEXT("TabTrafficLight", "Traffic Lights"))
        ];
}

TSharedRef<SWidget> SLightDefaultsPanel::MakeBuildingsTab()
{
    return SNew(SCheckBox)
        .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
        .Padding(FMargin(10.0f, 4.0f))
        .IsChecked_Lambda([this]()
        {
            return bShowingBuildings ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
        })
        .OnCheckStateChanged_Lambda([this](ECheckBoxState)
        {
            bShowingBuildings = true;
            bShowingTrafficLights = false;
            bShowingVehicles = false;
            ApplyFilter();
        })
        [
            SNew(STextBlock).Text(LOCTEXT("TabBuildings", "Buildings"))
        ];
}

TSharedRef<SWidget> SLightDefaultsPanel::MakeVehiclesTab()
{
    return SNew(SCheckBox)
        .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
        .Padding(FMargin(10.0f, 4.0f))
        .IsChecked_Lambda([this]()
        {
            return bShowingVehicles ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
        })
        .OnCheckStateChanged_Lambda([this](ECheckBoxState)
        {
            bShowingVehicles = true;
            bShowingTrafficLights = false;
            bShowingBuildings = false;
            ApplyFilter();
        })
        [
            SNew(STextBlock).Text(LOCTEXT("TabVehicles", "Vehicles"))
        ];
}

void SLightDefaultsPanel::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SNew(SVerticalBox)

        + SVerticalBox::Slot().AutoHeight().Padding(4.0f)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("ScanButton", "Scan Level"))
                .OnClicked(this, &SLightDefaultsPanel::OnScanClicked)
            ]

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("ReapplyButton", "Reapply To Level"))
                .ToolTipText(LOCTEXT("ReapplyTooltip", "Reverts any live, unsaved preview edits back to the last saved catalog on disk."))
                .OnClicked(this, &SLightDefaultsPanel::OnReapplyClicked)
            ]

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("SaveAllButton", "Save All"))
                .ToolTipText(LOCTEXT("SaveAllTooltip", "Commits every row's current (already live-previewed) values to Config/Lights/Defaults.json."))
                .OnClicked(this, &SLightDefaultsPanel::OnSaveAllClicked)
            ]
        ]

        + SVerticalBox::Slot().AutoHeight().Padding(4.0f, 0.0f)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ MakeGroupTab(ELightType::Street, LOCTEXT("TabStreet", "Street")) ]

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ MakeTrafficTab() ]

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ MakeBuildingsTab() ]

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ MakeGroupTab(ELightType::Other, LOCTEXT("TabOther", "Other")) ]

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ MakeVehiclesTab() ]
        ]

        + SVerticalBox::Slot().AutoHeight().Padding(6.0f, 4.0f, 4.0f, 0.0f)
        [
            SNew(SHorizontalBox)
            .Visibility_Lambda([this]() { return bShowingVehicles ? EVisibility::Collapsed : EVisibility::Visible; })

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SBox).WidthOverride(240.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderClass", "Class")) ] ]

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SBox).WidthOverride(50.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderCount", "Count")) ] ]

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [
                SNew(SBox).WidthOverride(70.0f)
                [
                    SNew(STextBlock).Text_Lambda([this]() -> FText
                    {
                        return bShowingBuildings
                            ? LOCTEXT("HeaderPercentLit", "Percent Lit")
                            : LOCTEXT("HeaderIntensity", "Intensity");
                    })
                ]
            ]

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SBox).WidthOverride(70.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderEmissive", "Emissive")) ] ]

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SBox).WidthOverride(40.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderColor", "Color")) ] ]

            + SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 2.0f)
            [ SNew(STextBlock).Text(LOCTEXT("HeaderSelect", "Select")) ]
        ]

        + SVerticalBox::Slot().FillHeight(1.0f).Padding(4.0f)
        [
            SAssignNew(RowsListView, SListView<TSharedPtr<FLightAssetSummary>>)
            .Visibility_Lambda([this]() { return bShowingVehicles ? EVisibility::Collapsed : EVisibility::Visible; })
            .ListItemsSource(&Rows)
            .SelectionMode(ESelectionMode::None)
            .OnGenerateRow(this, &SLightDefaultsPanel::OnGenerateRow)
        ]

        // Vehicles: a wider, differently-shaped header/row (see
        // SVehicleLightRow) -- a second header row + ListView, occupying the
        // same area but Collapsed (not just hidden) whenever this tab isn't
        // active, same trick as above in reverse.
        + SVerticalBox::Slot().AutoHeight().Padding(6.0f, 4.0f, 4.0f, 0.0f)
        [
            SNew(SHorizontalBox)
            .Visibility_Lambda([this]() { return bShowingVehicles ? EVisibility::Visible : EVisibility::Collapsed; })

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SBox).WidthOverride(200.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderVehicleClass", "Vehicle")) ] ]

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SBox).WidthOverride(40.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderCount2", "Count")) ] ]

            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SBox).WidthOverride(55.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderVehicleIntensity", "Intensity")) ] ]

            + SHorizontalBox::Slot().AutoWidth().Padding(1.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().Padding(1.0f)
                [ SNew(SBox).WidthOverride(55.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderPos", "Position")).ToolTipText(LOCTEXT("HeaderPosTT", "Position")) ] ]
                + SHorizontalBox::Slot().AutoWidth().Padding(1.0f)
                [ SNew(SBox).WidthOverride(55.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderLow", "Low Beam")).ToolTipText(LOCTEXT("HeaderLowTT", "Low Beam")) ] ]
                + SHorizontalBox::Slot().AutoWidth().Padding(1.0f)
                [ SNew(SBox).WidthOverride(55.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderHigh", "High Beam")).ToolTipText(LOCTEXT("HeaderHighTT", "High Beam")) ] ]
                + SHorizontalBox::Slot().AutoWidth().Padding(1.0f)
                [ SNew(SBox).WidthOverride(55.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderFog", "Fog")).ToolTipText(LOCTEXT("HeaderFogTT", "Fog")) ] ]
                + SHorizontalBox::Slot().AutoWidth().Padding(1.0f)
                [ SNew(SBox).WidthOverride(55.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderBrake", "Brake")).ToolTipText(LOCTEXT("HeaderBrakeTT", "Brake")) ] ]
                + SHorizontalBox::Slot().AutoWidth().Padding(1.0f)
                [ SNew(SBox).WidthOverride(55.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderReverse", "Reverse")).ToolTipText(LOCTEXT("HeaderReverseTT", "Reverse")) ] ]
                + SHorizontalBox::Slot().AutoWidth().Padding(1.0f)
                [ SNew(SBox).WidthOverride(55.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderLBlink", "L Blink")).ToolTipText(LOCTEXT("HeaderLBlinkTT", "Left Blinker")) ] ]
                + SHorizontalBox::Slot().AutoWidth().Padding(1.0f)
                [ SNew(SBox).WidthOverride(55.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderRBlink", "R Blink")).ToolTipText(LOCTEXT("HeaderRBlinkTT", "Right Blinker")) ] ]
                + SHorizontalBox::Slot().AutoWidth().Padding(1.0f)
                [ SNew(SBox).WidthOverride(55.0f) [ SNew(STextBlock).Text(LOCTEXT("HeaderSpecial1", "Special1")).ToolTipText(LOCTEXT("HeaderSpecial1TT", "Siren/lightbar")) ] ]
            ]

            + SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 2.0f)
            [ SNew(STextBlock).Text(LOCTEXT("HeaderPreview", "Preview")) ]
        ]

        + SVerticalBox::Slot().FillHeight(1.0f).Padding(4.0f)
        [
            SAssignNew(VehicleRowsListView, SListView<TSharedPtr<FVehicleLightSummary>>)
            .Visibility_Lambda([this]() { return bShowingVehicles ? EVisibility::Visible : EVisibility::Collapsed; })
            .ListItemsSource(&AllVehicleRows)
            .SelectionMode(ESelectionMode::None)
            .OnGenerateRow(this, &SLightDefaultsPanel::OnGenerateVehicleRow)
        ]
    ];
}

FReply SLightDefaultsPanel::OnScanClicked()
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;

    AllRows.Reset();
    for (const FLightAssetSummary& Summary : ULightDefaultsJsonUtils::ScanLightAssetsInLevel(World))
    {
        AllRows.Add(MakeShared<FLightAssetSummary>(Summary));
    }

    AllTrafficRows.Reset();
    for (const FLightAssetSummary& Summary : ULightDefaultsJsonUtils::ScanTrafficLightAssetsInLevel(World))
    {
        AllTrafficRows.Add(MakeShared<FLightAssetSummary>(Summary));
    }

    AllBuildingRows.Reset();
    for (const FLightAssetSummary& Summary : ULightDefaultsJsonUtils::ScanProceduralBuildingsInLevel(World))
    {
        AllBuildingRows.Add(MakeShared<FLightAssetSummary>(Summary));
    }

    AllLegacyBuildingRows.Reset();
    for (const FLightAssetSummary& Summary : ULightDefaultsJsonUtils::ScanLegacyBuildingsInLevel(World))
    {
        AllLegacyBuildingRows.Add(MakeShared<FLightAssetSummary>(Summary));
    }

    AllVehicleRows.Reset();
    for (const FVehicleLightSummary& Summary : ULightDefaultsJsonUtils::ScanVehicleCatalog(World))
    {
        AllVehicleRows.Add(MakeShared<FVehicleLightSummary>(Summary));
    }
    if (VehicleRowsListView.IsValid())
    {
        VehicleRowsListView->RequestListRefresh();
    }

    ApplyFilter();
    return FReply::Handled();
}

void SLightDefaultsPanel::ApplyFilter()
{
    if (bShowingVehicles)
    {
        // Vehicles has its own array/ListView (AllVehicleRows/VehicleRowsListView),
        // not the Rows subset this function otherwise builds -- see OnScanClicked.
        return;
    }

    Rows.Reset();
    if (bShowingTrafficLights)
    {
        Rows = AllTrafficRows;
    }
    else if (bShowingBuildings)
    {
        // Baked (HISM, per-window) and legacy (non-HISM, per-building) rows
        // use Percent Lit -- see SLightDefaultsRow's bIsAnyBuildingsRow.
        // AllRows items with bIsBuildingClass are real UCarlaLight-driven
        // buildings (BP_Apt*, BP_Factory*, ...), routed here regardless of
        // their artist-authored LightGroup because they live under
        // Static/Building/ -- they use the normal Intensity/Color UI, not
        // Percent Lit, same as any Street/Vehicle/Other row.
        Rows = AllBuildingRows;
        Rows.Append(AllLegacyBuildingRows);
        for (const TSharedPtr<FLightAssetSummary>& Item : AllRows)
        {
            if (Item->bIsBuildingClass)
            {
                Rows.Add(Item);
            }
        }
    }
    else
    {
        for (const TSharedPtr<FLightAssetSummary>& Item : AllRows)
        {
            if (Item->bIsBuildingClass)
            {
                continue;
            }
            // No dedicated tab for ELightType::Building -- folded into
            // Other, since with baked/legacy buildings covered by their own
            // material-based tab, this group saw little standalone use.
            const bool bMatchesGroup = (Item->LightGroup == ActiveFilter) ||
                (ActiveFilter == ELightType::Other && Item->LightGroup == ELightType::Building);
            if (bMatchesGroup)
            {
                Rows.Add(Item);
            }
        }
    }

    if (RowsListView.IsValid())
    {
        RowsListView->RequestListRefresh();
    }
}

FReply SLightDefaultsPanel::OnReapplyClicked()
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    ULightDefaultsJsonUtils::ReapplyLightDefaultsToLevel(World);
    ULightDefaultsJsonUtils::ReapplyTrafficLightDefaultsToLevel(World);
    ULightDefaultsJsonUtils::ReapplyProceduralBuildingsToLevel(World);
    ULightDefaultsJsonUtils::ReapplyLegacyBuildingsToLevel(World);
    ULightDefaultsJsonUtils::ReapplyVehicleLightsToLevel(World);
    return FReply::Handled();
}

FReply SLightDefaultsPanel::OnSaveAllClicked()
{
    // Saves every scanned row (AllRows/AllTrafficRows, not just the currently
    // filtered group's Rows) so switching tabs doesn't lose an edit made
    // before Save All was clicked. Same FLightAssetDefault/JSON format for
    // both -- traffic rows just leave Intensity/Color at their unused
    // defaults.
    for (const TSharedPtr<FLightAssetSummary>& Item : AllRows)
    {
        if (Item.IsValid())
        {
            ULightDefaultsJsonUtils::SaveClassDefault(Item->ClassName, Item->Current);
        }
    }
    for (const TSharedPtr<FLightAssetSummary>& Item : AllTrafficRows)
    {
        if (Item.IsValid())
        {
            ULightDefaultsJsonUtils::SaveClassDefault(Item->ClassName, Item->Current);
        }
    }
    for (const TSharedPtr<FLightAssetSummary>& Item : AllBuildingRows)
    {
        if (Item.IsValid())
        {
            ULightDefaultsJsonUtils::SaveClassDefault(Item->ClassName, Item->Current);
        }
    }
    for (const TSharedPtr<FLightAssetSummary>& Item : AllLegacyBuildingRows)
    {
        if (Item.IsValid())
        {
            ULightDefaultsJsonUtils::SaveClassDefault(Item->ClassName, Item->Current);
        }
    }
    for (const TSharedPtr<FVehicleLightSummary>& Item : AllVehicleRows)
    {
        if (Item.IsValid())
        {
            ULightDefaultsJsonUtils::SaveVehicleLightDefault(Item->ClassName, Item->Intensity, Item->GroupIntensity);
        }
    }
    return FReply::Handled();
}

void SLightDefaultsPanel::ApplyRowLive(TSharedPtr<FLightAssetSummary> Item)
{
    if (!Item.IsValid())
    {
        return;
    }
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    ULightDefaultsJsonUtils::ApplyLightDefaultToClassLive(World, Item->ClassName, Item->Current);
}

void SLightDefaultsPanel::SetClassSelected(const FString& ClassName, bool bSelected)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    ULightDefaultsJsonUtils::SetClassActorsSelected(World, ClassName, bSelected);
}

void SLightDefaultsPanel::ApplyTrafficRowLive(TSharedPtr<FLightAssetSummary> Item)
{
    if (!Item.IsValid())
    {
        return;
    }
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    ULightDefaultsJsonUtils::ApplyTrafficLightDefaultToClassLive(World, Item->ClassName, Item->Current.EmissiveIntensity);
}

void SLightDefaultsPanel::SetTrafficClassSelected(const FString& ClassName, bool bSelected)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    ULightDefaultsJsonUtils::SetTrafficLightClassActorsSelected(World, ClassName, bSelected);
}

void SLightDefaultsPanel::SetTrafficClassPreviewOn(const FString& ClassName, bool bOn)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    ULightDefaultsJsonUtils::SetTrafficLightClassPreviewOn(World, ClassName, bOn);
}

void SLightDefaultsPanel::ApplyBuildingRowLive(TSharedPtr<FLightAssetSummary> Item)
{
    if (!Item.IsValid())
    {
        return;
    }
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    ULightDefaultsJsonUtils::ApplyProceduralBuildingsEmissiveLive(World, Item->Current.EmissiveIntensity, Item->Current.Intensity);
}

void SLightDefaultsPanel::SetBuildingSelected(bool bSelected)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    ULightDefaultsJsonUtils::SetProceduralBuildingsSelected(World, bSelected);
}

void SLightDefaultsPanel::ApplyLegacyBuildingRowLive(TSharedPtr<FLightAssetSummary> Item)
{
    if (!Item.IsValid())
    {
        return;
    }
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    ULightDefaultsJsonUtils::ApplyLegacyBuildingsEmissiveLive(World, Item->Current.EmissiveIntensity, Item->Current.Intensity);
}

void SLightDefaultsPanel::SetLegacyBuildingSelected(bool bSelected)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    ULightDefaultsJsonUtils::SetLegacyBuildingsSelected(World, bSelected);
}

void SLightDefaultsPanel::ApplyVehicleRowLive(TSharedPtr<FVehicleLightSummary> Item)
{
    if (!Item.IsValid())
    {
        return;
    }
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    ULightDefaultsJsonUtils::ApplyVehicleLightsLive(World, Item->ClassName, Item->Intensity, Item->GroupIntensity);
}

void SLightDefaultsPanel::ToggleVehiclePreview(const FString& ClassName, bool bPreviewOn)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (bPreviewOn)
    {
        ULightDefaultsJsonUtils::SpawnVehiclePreview(World, ClassName);
        // Blinkers are gated by Sine('VisualTime') in the vehicle lights
        // material -- that clock only advances during Play, so it can sit on
        // a dark half of the wave in a static viewport. Pin it to a
        // confirmed-visible value so Left/Right Blinker actually shows on
        // the freshly spawned preview without running the simulation.
        ULightDefaultsJsonUtils::PreviewVehicleBlinkers(World);
    }
    else
    {
        ULightDefaultsJsonUtils::ClearVehiclePreview(World);
    }
}

TSharedRef<ITableRow> SLightDefaultsPanel::OnGenerateRow(
    TSharedPtr<FLightAssetSummary> Item,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    // bShowingTrafficLights/bShowingBuildings are stable for the whole
    // refresh that triggers this (set before ApplyFilter runs), so it's safe
    // to branch once here rather than re-checking per row at click time.
    if (bShowingTrafficLights)
    {
        return SNew(SLightDefaultsRow, OwnerTable, Item,
            [this](TSharedPtr<FLightAssetSummary> RowItem) { ApplyTrafficRowLive(RowItem); },
            [this](const FString& ClassName, bool bSelected) { SetTrafficClassSelected(ClassName, bSelected); },
            [this](const FString& ClassName, bool bOn) { SetTrafficClassPreviewOn(ClassName, bOn); });
    }
    if (bShowingBuildings)
    {
        // Baked and legacy rows share this tab now -- tell them apart by
        // ClassName so each wires to its own apply/select target. A row
        // that's neither is a real UCarlaLight-driven building
        // (bIsBuildingClass, e.g. an Apt-family row) -- wire it exactly
        // like a normal Street/Vehicle/Other row.
        const bool bIsLegacyRow = Item.IsValid() && Item->ClassName == TEXT("LegacyBuildings");
        const bool bIsBakedRow = Item.IsValid() && Item->ClassName == TEXT("ProceduralBuildings");
        if (bIsLegacyRow)
        {
            return SNew(SLightDefaultsRow, OwnerTable, Item,
                [this](TSharedPtr<FLightAssetSummary> RowItem) { ApplyLegacyBuildingRowLive(RowItem); },
                [this](const FString&, bool bSelected) { SetLegacyBuildingSelected(bSelected); });
        }
        if (bIsBakedRow)
        {
            return SNew(SLightDefaultsRow, OwnerTable, Item,
                [this](TSharedPtr<FLightAssetSummary> RowItem) { ApplyBuildingRowLive(RowItem); },
                [this](const FString&, bool bSelected) { SetBuildingSelected(bSelected); });
        }
        return SNew(SLightDefaultsRow, OwnerTable, Item,
            [this](TSharedPtr<FLightAssetSummary> RowItem) { ApplyRowLive(RowItem); },
            [this](const FString& ClassName, bool bSelected) { SetClassSelected(ClassName, bSelected); });
    }
    return SNew(SLightDefaultsRow, OwnerTable, Item,
        [this](TSharedPtr<FLightAssetSummary> RowItem) { ApplyRowLive(RowItem); },
        [this](const FString& ClassName, bool bSelected) { SetClassSelected(ClassName, bSelected); });
}

TSharedRef<ITableRow> SLightDefaultsPanel::OnGenerateVehicleRow(
    TSharedPtr<FVehicleLightSummary> Item,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(SVehicleLightRow, OwnerTable, Item,
        [this](TSharedPtr<FVehicleLightSummary> RowItem) { ApplyVehicleRowLive(RowItem); },
        [this](const FString& ClassName, bool bPreviewOn) { ToggleVehiclePreview(ClassName, bPreviewOn); });
}

namespace
{
    const FName LightDefaultsTabId(TEXT("CarlaLightDefaultsPanel"));

    TSharedRef<SDockTab> SpawnLightDefaultsTab(const FSpawnTabArgs&)
    {
        return SNew(SDockTab)
            .TabRole(ETabRole::NomadTab)
            [
                SNew(SLightDefaultsPanel)
            ];
    }
}

void RegisterLightDefaultsEditorTab()
{
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LightDefaultsTabId, FOnSpawnTab::CreateStatic(&SpawnLightDefaultsTab))
        .SetDisplayName(LOCTEXT("LightDefaultsTabTitle", "Light Defaults"))
        .SetTooltipText(LOCTEXT("LightDefaultsTabTooltip", "Edit per-class light color/intensity/emissive defaults"))
        .SetMenuType(ETabSpawnerMenuType::Enabled);
}

void UnregisterLightDefaultsEditorTab()
{
    if (FSlateApplication::IsInitialized())
    {
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LightDefaultsTabId);
    }
}

namespace
{
    FDelegateHandle MapOpenedHandle;
}

void RegisterLightDefaultsMapOpenedHook()
{
    MapOpenedHandle = FEditorDelegates::OnMapOpened.AddLambda([](const FString&, bool)
    {
        UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
        if (World == nullptr)
        {
            return;
        }
        TWeakObjectPtr<UWorld> WeakWorld(World);
        FTimerHandle Unused;
        World->GetTimerManager().SetTimer(Unused, FTimerDelegate::CreateLambda([WeakWorld]()
        {
            if (WeakWorld.IsValid())
            {
                ULightDefaultsJsonUtils::ReapplyLightDefaultsToLevel(WeakWorld.Get());
                ULightDefaultsJsonUtils::ReapplyTrafficLightDefaultsToLevel(WeakWorld.Get());
                ULightDefaultsJsonUtils::ReapplyProceduralBuildingsToLevel(WeakWorld.Get());
                ULightDefaultsJsonUtils::ReapplyLegacyBuildingsToLevel(WeakWorld.Get());
                ULightDefaultsJsonUtils::ReapplyVehicleLightsToLevel(WeakWorld.Get());
            }
        }), 1.0f, false);
    });
}

void UnregisterLightDefaultsMapOpenedHook()
{
    FEditorDelegates::OnMapOpened.Remove(MapOpenedHandle);
}

#undef LOCTEXT_NAMESPACE

#endif // WITH_EDITOR
