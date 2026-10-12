// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"
#include "BlueprintLibary/LightDefaultsJsonUtils.h"

#include <util/ue-header-guard-begin.h>
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include <util/ue-header-guard-end.h>

// Native (non-Blueprint) editor tool window: scans every light-bearing
// Blueprint class registered in the currently open level (see
// ULightDefaultsJsonUtils::ScanLightAssetsInLevel, deduplicated by class),
// lets an artist edit color/intensity/emissive per class, and saves the
// result to Config/Lights/Defaults.json. A pure C++ Slate panel instead of an
// Editor Utility Widget Blueprint -- UE's Python editor API has WidgetTree
// (a Blueprint's design-time widget tree) marked BlueprintProtected, so this
// tool cannot be authored headlessly; building it natively in C++ sidesteps
// that entirely and needs no manual Blueprint wiring.
class SLightDefaultsPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLightDefaultsPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    // Full scan results (every group); Rows is the subset matching
    // ActiveFilter, bound to the ListView -- this is what makes the group
    // buttons behave like tabs without needing four separate lists/scans.
    // AllTrafficRows is a separate, ungrouped scan (ATrafficLightBase, no
    // UCarlaLight involved) shown instead of Rows-from-AllRows when the
    // Traffic Lights tab is active -- see bShowingTrafficLights.
    // AllBuildingRows is a third such ungrouped scan (baked, non-Blueprint
    // AActor instances on HISM components, no UCarlaLight, no per-class
    // dedup possible -- see ULightDefaultsJsonUtils::ScanProceduralBuildingsInLevel),
    // and AllLegacyBuildingRows a fourth (the same FakeInterior material
    // family but on plain, non-HISM components -- pre-modular Blueprint
    // buildings and standalone placed pieces, found across Town01-07).
    // Both are shown together -- one row each -- when the Buildings tab is
    // active, see bShowingBuildings.
    TArray<TSharedPtr<FLightAssetSummary>> AllRows;
    TArray<TSharedPtr<FLightAssetSummary>> AllTrafficRows;
    TArray<TSharedPtr<FLightAssetSummary>> AllBuildingRows;
    TArray<TSharedPtr<FLightAssetSummary>> AllLegacyBuildingRows;
    TArray<TSharedPtr<FLightAssetSummary>> Rows;
    TSharedPtr<SListView<TSharedPtr<FLightAssetSummary>>> RowsListView;
    ELightType ActiveFilter = ELightType::Street;
    bool bShowingTrafficLights = false;
    bool bShowingBuildings = false;

    // Vehicles: a fundamentally different shape (see FVehicleLightSummary --
    // no Color, a global Intensity plus up to 8 per-light-group fields
    // instead of one Emissive), so it gets its own item type and ListView
    // rather than being squeezed into Rows/RowsListView above. Shown via a
    // SWidgetSwitcher slot instead of the ApplyFilter subset trick the other
    // tabs use, see bShowingVehicles.
    TArray<TSharedPtr<FVehicleLightSummary>> AllVehicleRows;
    TSharedPtr<SListView<TSharedPtr<FVehicleLightSummary>>> VehicleRowsListView;
    bool bShowingVehicles = false;

    FReply OnScanClicked();
    FReply OnReapplyClicked();
    FReply OnSaveAllClicked();
    void ApplyFilter();

    // Pushes Item->Current onto every live instance of its class in the level
    // (ULightDefaultsJsonUtils::ApplyLightDefaultToClassLive) -- does NOT
    // touch Defaults.json. Called after every in-row edit so a change is
    // visible immediately; OnSaveAllClicked is the separate, deliberate step
    // that commits every row's Current to disk.
    void ApplyRowLive(TSharedPtr<FLightAssetSummary> Item);

    // Selects/deselects every placed actor of ClassName in the level, so an
    // artist can see which actors a row is editing.
    void SetClassSelected(const FString& ClassName, bool bSelected);

    // Traffic-light equivalents of the two above (ULightDefaultsJsonUtils'
    // Traffic* functions) -- separate because traffic lights aren't found via
    // UCarlaLightSubsystem, and only Emissive (-> "lightvalue") is meaningful
    // for them; Intensity/Color stay unused for this tab.
    void ApplyTrafficRowLive(TSharedPtr<FLightAssetSummary> Item);
    void SetTrafficClassSelected(const FString& ClassName, bool bSelected);

    // Forces every traffic light of ClassName's signal MIDs' "On/Off" to 1/0,
    // live only, never saved -- lets an artist preview a lit signal without
    // starting Play (see ULightDefaultsJsonUtils::SetTrafficLightClassPreviewOn).
    void SetTrafficClassPreviewOn(const FString& ClassName, bool bOn);

    // Buildings equivalents -- a single global row (no per-class dedup, see
    // AllBuildingRows), so there's no ClassName to key by: these apply to
    // every baked building in the level unconditionally.
    void ApplyBuildingRowLive(TSharedPtr<FLightAssetSummary> Item);
    void SetBuildingSelected(bool bSelected);

    // Legacy (non-HISM) buildings equivalents -- same single-global-row idea
    // as the two above, over AllLegacyBuildingRows.
    void ApplyLegacyBuildingRowLive(TSharedPtr<FLightAssetSummary> Item);
    void SetLegacyBuildingSelected(bool bSelected);

    TSharedRef<class ITableRow> OnGenerateRow(
        TSharedPtr<FLightAssetSummary> Item,
        const TSharedRef<class STableViewBase>& OwnerTable);

    TSharedRef<SWidget> MakeGroupTab(ELightType Group, const FText& Label);
    TSharedRef<SWidget> MakeTrafficTab();
    TSharedRef<SWidget> MakeBuildingsTab();
    TSharedRef<SWidget> MakeVehiclesTab();

    // Pushes Item's Intensity/GroupIntensity onto every actor of ClassName
    // currently in the world (placed instances and/or a live Preview spawn --
    // ULightDefaultsJsonUtils::ApplyVehicleLightsLive treats both the same,
    // see there). Does NOT touch Defaults.json.
    void ApplyVehicleRowLive(TSharedPtr<FVehicleLightSummary> Item);

    // The Vehicles tab's equivalent of Select: spawns (or destroys, on
    // toggle-off) one instance of ClassName via
    // ULightDefaultsJsonUtils::SpawnVehiclePreview/ClearVehiclePreview and
    // reframes the viewport camera on it, since most rows have no placed
    // instance to jump to (this tab lists the full catalog, not a level
    // scan).
    void ToggleVehiclePreview(const FString& ClassName, bool bPreviewOn);

    TSharedRef<class ITableRow> OnGenerateVehicleRow(
        TSharedPtr<FVehicleLightSummary> Item,
        const TSharedRef<class STableViewBase>& OwnerTable);
};

// Registers/unregisters the "Light Defaults" nomad tab (Window menu). Called
// from FCarlaModule::StartupModule/ShutdownModule.
void RegisterLightDefaultsEditorTab();
void UnregisterLightDefaultsEditorTab();

// Registers/unregisters a hook on FEditorDelegates::OnMapOpened that
// reapplies the saved Light Defaults catalog (both UCarlaLight-based and
// traffic-light) shortly after a map finishes loading. Necessary because a
// light-bearing actor's own registration-time
// apply (UCarlaLight::RegisterLight, which runs this same session regardless
// of this tool) creates its emissive MID too early: confirmed via
// instrumented testing that a same-tick material change made there gets
// silently reset by something later in map load (World Partition streaming
// settling, most likely) before the component's real render proxy is built.
// Waiting for the concrete "map opened" signal, plus a short extra delay,
// reapplies once everything has actually settled.
void RegisterLightDefaultsMapOpenedHook();
void UnregisterLightDefaultsMapOpenedHook();

#endif // WITH_EDITOR
