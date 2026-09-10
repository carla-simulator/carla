// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Carla/Lights/CarlaLight.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/Paths.h"
#include "LightDefaultsJsonUtils.generated.h"

// Everything BP_Lights' Details panel exposes per instance: the inherited
// UCarlaLight color/intensity plus its own EmissiveIntensity. One of these
// lives per light-bearing Blueprint class (e.g. "BP_StreetLight01_C") or per
// LightGroup fallback, in Config/Lights/Defaults.json.
USTRUCT(BlueprintType)
struct FLightAssetDefault
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor Color = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float Intensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float EmissiveIntensity = 0.0f;
};

// One row of the scan results a light-defaults editor widget lists: every
// unique light-bearing Blueprint class currently registered in the level,
// deduplicated (50 instances of the same streetlamp show once), pre-filled
// with its saved default if it has one, or a live sample off its first
// instance to start from otherwise.
USTRUCT(BlueprintType)
struct FLightAssetSummary
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ClassName;

    UPROPERTY(BlueprintReadOnly)
    ELightType LightGroup = ELightType::Street;

    UPROPERTY(BlueprintReadOnly)
    int32 InstanceCount = 0;

    UPROPERTY(BlueprintReadWrite)
    FLightAssetDefault Current;

    UPROPERTY(BlueprintReadOnly)
    bool bHasSavedDefault = false;

    // True when ClassName is actually a building-material family key (see
    // GetLightClassKey), not a raw Blueprint class name -- lets the panel
    // route this row into the Buildings tab regardless of LightGroup.
    UPROPERTY(BlueprintReadOnly)
    bool bIsBuildingClass = false;
};

// One row of the Vehicles tab. Unlike every other group here, vehicles have
// no single UCarlaLight to key a Color/Intensity/Emissive triad on -- a
// Blueprint like BP_LincolnMKZ places 18 separate real light components
// (front-low_beam-l-1, back-brake-l-1, ...) AND a shared "*_Emissive"
// MaterialInstance exposing 8 named scalar parameters mirroring those same
// groups (confirmed identical, same order, across Lincoln/MiniCooper/
// Ambulance) plus one standalone global "Intensity" param that turned out to
// do nothing visible on its own. Two controls, two different targets:
//   - Intensity (this struct's field): writes the SAME value into every
//     exposed group parameter on the Emissive material at once (Position,
//     Low Beam, ... all glow together) -- a "light them all up" shortcut,
//     not the material's own unused "Intensity" param.
//   - GroupIntensity (per column): the REAL Spot/Rect/Point light
//     components' physical Intensity, matched by name (see
//     CollectVehicleLightComponentsForGroup in the .cpp) -- actual scene
//     illumination, independent of the material glow above.
USTRUCT(BlueprintType)
struct FVehicleLightSummary
{
    GENERATED_BODY()

    // The Blueprint class name (e.g. "BP_LincolnMKZ_C") -- apply/select key,
    // matched against AActor::GetClass()->GetName() same as everywhere else.
    UPROPERTY(BlueprintReadOnly)
    FString ClassName;

    // "Lincoln Mkz" -- Make + Model from VehicleParameters.json, shown
    // instead of the raw class name (which the Class column still shows).
    UPROPERTY(BlueprintReadOnly)
    FString DisplayName;

    // How many instances of this class are currently placed in the level --
    // normally 0, since this tab lists the full catalog (VehicleParameters.json),
    // not a level scan. Placed instances (or a live Preview spawn) still
    // receive ApplyVehicleLightsLive same as everywhere else.
    UPROPERTY(BlueprintReadOnly)
    int32 InstanceCount = 0;

    // Global "light every exposed Emissive group up to this value at once"
    // control -- see the struct comment above. Sampled at scan time from the
    // material's own (otherwise-unused) "Intensity" param as a starting
    // value only; ApplyVehicleLightsLive gives it its real meaning.
    UPROPERTY(BlueprintReadWrite)
    float Intensity = 1.0f;

    // Canonical group name (VehicleLightGroupNames) -> that group's real
    // light components' physical Intensity, current/edited value -- scaled
    // down by VehicleLightIntensityDisplayScale (10000) from the raw UE5
    // photometric lux the component actually stores (real values run
    // ~500000; awkward to type/read directly), so this field runs roughly
    // 10-100 instead. Only groups this vehicle actually HAS matching
    // components for are meaningful to apply; see GroupPresent for which
    // those are.
    UPROPERTY(BlueprintReadWrite)
    TMap<FString, float> GroupIntensity;

    // Canonical group name -> whether this vehicle has any real light
    // component matching that group's naming convention (checked once at
    // scan time, see CollectVehicleLightComponentsForGroup). The panel greys
    // out/disables a group's field when this is false instead of showing a
    // control that would silently do nothing.
    UPROPERTY(BlueprintReadOnly)
    TMap<FString, bool> GroupPresent;

    UPROPERTY(BlueprintReadOnly)
    bool bHasSavedDefault = false;
};

UCLASS()
class CARLA_API ULightDefaultsJsonUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    // Config/Lights/Defaults.json:
    //   { "perClass": { "BP_StreetLight01_C": {...} },
    //     "perGroup": { "Street": {...}, "Building": {...}, ... } }
    // A class-specific entry wins; a LightGroup entry is the fallback for any
    // class with no entry of its own (see GetLightDefault).
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON")
    static bool SaveClassDefault(const FString& ClassName, const FLightAssetDefault& Default);

    UFUNCTION(BlueprintCallable, Category = "Lights|JSON")
    static bool LoadClassDefault(const FString& ClassName, UPARAM(ref) FLightAssetDefault& OutDefault);

    UFUNCTION(BlueprintCallable, Category = "Lights|JSON")
    static bool SaveGroupDefault(ELightType Group, const FLightAssetDefault& Default);

    UFUNCTION(BlueprintCallable, Category = "Lights|JSON")
    static bool LoadGroupDefault(ELightType Group, UPARAM(ref) FLightAssetDefault& OutDefault);

    // ClassName's own catalog entry if it has one, else Group's fallback
    // entry. False if neither exists -- caller should leave the light as
    // authored.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON")
    static bool GetLightDefault(const FString& ClassName, ELightType Group, UPARAM(ref) FLightAssetDefault& OutDefault);

    // Every UCarlaLight registered in WorldContextObject's world (editor or
    // Play), deduplicated by GetLightClassKey (Blueprint class, or a shared
    // building-material family for rows with bIsBuildingClass) -- see
    // FLightAssetSummary. Backs the light-defaults editor widget's list.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static TArray<FLightAssetSummary> ScanLightAssetsInLevel(const UObject* WorldContextObject);

    // Dedup/lookup key for a UCarlaLight owner: normally its exact Blueprint
    // class name, but if any of its StaticMeshComponent materials lives
    // under /Game/Carla/Static/Building/ (the same content path the baked/
    // legacy buildings detection uses), this is a real light-driven building
    // -- e.g. BP_Apt20_A_C..F_C, which differ only by floor count/shape but
    // share the same MI_Apt20* material family. In that case the key is the
    // family token pulled from that material's own name ("Apt20"), so all
    // the floor/shape variants collapse into one row instead of one each.
    // Falls back to the raw class name if no building material is found, or
    // if a building material doesn't follow the "Apt<N>" naming pattern.
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lights|JSON")
    static FString GetLightClassKey(AActor* Owner, bool& bOutIsBuildingClass);

    // Pushes each registered light's resolved default (class, else group)
    // back onto it. Lets the widget preview a saved change immediately,
    // without restarting Play or reloading the level.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void ReapplyLightDefaultsToLevel(const UObject* WorldContextObject);

    // Applies Value directly to every registered light whose owner class is
    // ClassName -- WITHOUT touching Config/Lights/Defaults.json. Lets the
    // editor widget preview an edit live in the level before an artist
    // decides to commit it via SaveClassDefault.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void ApplyLightDefaultToClassLive(const UObject* WorldContextObject, const FString& ClassName, const FLightAssetDefault& Value);

    // Selects (bSelected=true) or deselects (false) every placed actor in the
    // level that owns a registered light of ClassName, so an artist can see
    // at a glance which actors a row in the editor widget is editing.
    // Editor-only; a no-op in packaged builds.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void SetClassActorsSelected(const UObject* WorldContextObject, const FString& ClassName, bool bSelected);

    // ELightType's UMETA DisplayName ("Street", "Building", ...) -- the key
    // GetLightDefault/SaveGroupDefault/LoadGroupDefault use in "perGroup".
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lights|JSON")
    static FString GetLightGroupName(ELightType Group);

    // Traffic lights (ATrafficLightBase) don't own a UCarlaLight -- their
    // color/on-off cycling is driven entirely by Blueprint
    // (BP_TLOpenDrive_RHT/LHT), toggling "On/Off" at runtime on
    // MaterialInstanceDynamics its own Construction Script already created
    // (RedLights/YellowLights/GreenLights arrays, kept by reference). The
    // "lightvalue" scalar (signal brightness when lit) is set once at
    // construction and never touched again, so it's safe to override --
    // reuses FLightAssetDefault/FLightAssetSummary and the same
    // Defaults.json "perClass" catalog (EmissiveIntensity == lightvalue;
    // Color/Intensity unused for this group). Deduplicated by Blueprint
    // class, same as ScanLightAssetsInLevel.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static TArray<FLightAssetSummary> ScanTrafficLightAssetsInLevel(const UObject* WorldContextObject);

    // Applies EmissiveIntensity (-> "lightvalue") directly to every traffic
    // light of ClassName in the level, WITHOUT touching Defaults.json. Never
    // creates a new MaterialInstanceDynamic -- only writes onto the MID
    // already assigned to each signal mesh slot, since BP_TLOpenDrive keeps
    // its own array references to those exact objects for its On/Off
    // toggling; replacing the slot's material would desync that array from
    // what's actually rendered and silently break the red/yellow/green
    // cycling.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void ApplyTrafficLightDefaultToClassLive(const UObject* WorldContextObject, const FString& ClassName, float EmissiveIntensity);

    // Pushes every traffic light's saved "perClass" EmissiveIntensity (->
    // "lightvalue") back onto it, same idea as ReapplyLightDefaultsToLevel.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void ReapplyTrafficLightDefaultsToLevel(const UObject* WorldContextObject);

    // Forces "On/Off" to 1 (bOn) or 0 on every traffic light of ClassName's
    // signal MIDs, live only -- never saved to Defaults.json. A preview aid
    // so an artist can see a signal lit without starting Play (the traffic
    // simulation is what normally drives this parameter).
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void SetTrafficLightClassPreviewOn(const UObject* WorldContextObject, const FString& ClassName, bool bOn);

    // Selects/deselects every placed ATrafficLightBase actor of ClassName --
    // the traffic-light equivalent of SetClassActorsSelected (which only
    // looks at UCarlaLightSubsystem and would never find these).
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void SetTrafficLightClassActorsSelected(const UObject* WorldContextObject, const FString& ClassName, bool bSelected);

    // Buildings baked by the modular building tool (BP_BuildingConverterToCode)
    // are placed as plain, non-Blueprint AActor instances referencing Content
    // Browser HierarchicalInstancedStaticMeshComponent modules directly --
    // there's no Blueprint class to key a per-class default on like every
    // other group here. Detected purely by material instead: any HISM mesh
    // slot whose static asset lives under /Game/Carla/Static/Building/ and
    // whose name contains "FakeInterior" (the window/interior-illusion
    // family -- the shared M_MaterialMaster base underneath is used by far
    // more than windows, see ResolveStaticProceduralBuildingAsset). Exposed
    // as a single synthetic "ProceduralBuildings" class covering every baked
    // building in the level -- one global emissive control until per-style
    // grouping (by material family) proves necessary.
    //
    // Only HISM slots -- see ScanLegacyBuildingsInLevel for the same
    // material family on plain (non-instanced) StaticMeshComponents, which
    // can't be controlled the same way.
    //
    // A HISM component's material slot is shared by every physical instance
    // it draws (some hold hundreds of windows at once -- confirmed no
    // per-instance custom data exists to vary through C++), so per-window
    // on/off can't be decided here. Instead M_FakeInterior's own graph gates
    // its emissive output with a PerInstanceRandom node compared against a
    // "PercentLitThreshold" parameter -- PercentLit (0..100) is just handed
    // down as that threshold (divided by 100), and the renderer resolves the
    // per-instance randomness deterministically per physical window.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static TArray<FLightAssetSummary> ScanProceduralBuildingsInLevel(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void ApplyProceduralBuildingsEmissiveLive(const UObject* WorldContextObject, float EmissiveIntensity, float PercentLit);

    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void ReapplyProceduralBuildingsToLevel(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void SetProceduralBuildingsSelected(const UObject* WorldContextObject, bool bSelected);

    // Same FakeInterior material family as ScanProceduralBuildingsInLevel,
    // but on plain (non-HISM) StaticMeshComponents: pre-modular buildings
    // (BP_Apt19/BP_Apt12/... Blueprint classes) and standalone placed
    // pieces (StaticMeshActor "SM_Apartment_v0X..."), the kind found across
    // Town01-07, not just the modular-tool-baked Town10 style. Percent Lit
    // here picks what fraction of whole BUILDINGS are lit (not windows,
    // since a plain component has no per-window granularity to gate) -- via
    // a stable per-actor coin flip in C++, see StableActorRandom01 in the
    // .cpp. (PercentLitThreshold + PerInstanceRandom, the mechanism the
    // baked/HISM path above uses, was tried here first and confirmed live
    // in-editor NOT to vary across legacy actors -- every building snapped
    // on together instead of a graduated percentage.)
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static TArray<FLightAssetSummary> ScanLegacyBuildingsInLevel(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void ApplyLegacyBuildingsEmissiveLive(const UObject* WorldContextObject, float EmissiveIntensity, float PercentLit);

    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void ReapplyLegacyBuildingsToLevel(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void SetLegacyBuildingsSelected(const UObject* WorldContextObject, bool bSelected);

    // The full vehicle catalog (Config/VehicleParameters.json -- every model
    // the vehicle factory can spawn, "HasLights" ones only), not a level
    // scan: most rows have InstanceCount 0 on a fresh map. One row per
    // Blueprint class, sampling its shared Emissive material's parameters
    // (see FVehicleLightSummary). Needs a world to spawn into: a vehicle
    // Blueprint assembles its meshes/materials in its Construction Script,
    // which never runs on the bare class default object, so the CDO's own
    // component list comes back empty -- confirmed headless (24/24 vehicles
    // scanned with zero groups present via the CDO alone). Each class is
    // spawned far below the map, inspected, and destroyed immediately, all
    // within this one call -- nothing renders a frame in between.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static TArray<FVehicleLightSummary> ScanVehicleCatalog(const UObject* WorldContextObject);

    // Applies Intensity/GroupIntensity to every actor of ClassName currently
    // in WorldContextObject's world -- covers both any manually placed
    // instances AND a live Preview spawn (see SpawnVehiclePreview), since
    // both are just actors of that class once they exist. WITHOUT touching
    // Config/Lights/Defaults.json.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void ApplyVehicleLightsLive(const UObject* WorldContextObject, const FString& ClassName, float Intensity, const TMap<FString, float>& GroupIntensity);

    // Same apply as ApplyVehicleLightsLive, but on exactly Actor -- no
    // ClassName match, no world sweep. ACarlaWheeledVehicle::BeginPlay uses
    // this (not the class-name version above) so that N already-spawned
    // vehicles of one class don't each get rescanned/rewritten every time
    // another vehicle of that same class spawns.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON")
    static void ApplyVehicleLightsToSingleActor(AActor* Actor, float Intensity, const TMap<FString, float>& GroupIntensity);

    // Gameplay integration, distinct from the two functions above: those
    // apply Intensity uniformly to every exposed emissive group at once
    // (correct for the Light Defaults editor Preview, which is meant to show
    // everything lit for editing). Real gameplay must not do that -- a
    // group's emissive must only glow when that group is actually ON per
    // FVehicleLightState. MaterialGroupValue must already have that gating
    // baked in by the caller (0 for OFF groups, Intensity for ON ones) --
    // this only pushes it to the material. RealLightGroupIntensity is applied
    // unconditionally, same as ApplyVehicleLightsToSingleActor, since the
    // physical light components' own Active/Visibility already gates
    // rendering. Call this again after every FVehicleLightState change (not
    // just once at BeginPlay) since RefreshLightState's own Blueprint graph
    // is opaque to C++ and may repush its own values on every toggle -- this
    // must run after it so our saved values win.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON")
    static void ApplyVehicleLightsRuntimeState(AActor* Actor, const TMap<FString, float>& MaterialGroupValue, const TMap<FString, float>& RealLightGroupIntensity);

    UFUNCTION(BlueprintCallable, Category = "Lights|JSON")
    static bool SaveVehicleLightDefault(const FString& ClassName, float Intensity, const TMap<FString, float>& GroupIntensity);

    UFUNCTION(BlueprintCallable, Category = "Lights|JSON")
    static bool LoadVehicleLightDefault(const FString& ClassName, UPARAM(ref) float& OutIntensity, UPARAM(ref) TMap<FString, float>& OutGroupIntensity);

    // Applies every saved "perVehicleClass" entry to matching actors
    // currently in the world (placed instances and/or a live Preview spawn) --
    // the vehicle-catalog equivalent of ReapplyLightDefaultsToLevel, called
    // from the same "Reapply To Level" button.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void ReapplyVehicleLightsToLevel(const UObject* WorldContextObject);

    // Spawns one instance of ClassName for visual preview: raycasts straight
    // down from the active editor viewport camera's current XY to find the
    // road/ground under it (whatever the artist last navigated to -- no
    // fixed world coordinate needed, and it stays put across Preview clicks
    // as long as the viewport doesn't move), spawns there, then reframes the
    // viewport camera on it. Destroys any previously spawned preview first --
    // at most one at a time. Returns the spawned actor (or null if the
    // raycast found no ground/road within range). Editor-only; a no-op in
    // packaged builds.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static AActor* SpawnVehiclePreview(const UObject* WorldContextObject, const FString& ClassName);

    // Destroys the current preview spawn, if any, without spawning a new
    // one -- used when a Preview toggle is switched off.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void ClearVehiclePreview(const UObject* WorldContextObject);

    // Canonical, fixed set of light-group column names every vehicle's
    // Emissive material is checked against (see FVehicleLightSummary). A
    // BlueprintPure getter so the panel doesn't duplicate this list.
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lights|JSON")
    static TArray<FString> GetVehicleLightGroupNames();

    // Blinkers are gated by Sine('VisualTime') in M_VehicleLightsMaster --
    // 'VisualTime' is a "CarlaParameters" Material Parameter Collection
    // scalar that only advances during a running simulation/Play, so a
    // blinker's Left/Right Blinker parameter (this tool's own column) can
    // sit permanently invisible in a static editor viewport if that clock
    // happens to be parked on a negative half of the sine wave -- confirmed:
    // manually setting VisualTime to 0.1-0.4 or 1.1-1.4 makes it visible.
    // Pins it to 0.25 (middle of the first confirmed-visible window) so
    // Preview can actually show blinker glow without Play running.
    UFUNCTION(BlueprintCallable, Category = "Lights|JSON", meta = (WorldContext = "WorldContextObject"))
    static void PreviewVehicleBlinkers(const UObject* WorldContextObject);

    static FString GetLightDefaultsPath()
    {
        return FPaths::ProjectContentDir() / TEXT("Carla/Config/Lights/Defaults.json");
    }

    static FString GetVehicleParametersPath()
    {
        return FPaths::ProjectContentDir() / TEXT("Carla/Config/VehicleParameters.json");
    }
};
