// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "BlueprintLibary/LightDefaultsJsonUtils.h"

#include "Carla/Lights/CarlaLightSubsystem.h"
#include "Carla/Traffic/TrafficLightBase.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "JsonObjectConverter.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameterCollection.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/UObjectIterator.h"

#if WITH_EDITOR
#include "Editor.h"
#include "LevelEditorViewport.h"
#endif

namespace
{
    TSharedRef<FJsonObject> LoadLightJsonRoot(const FString& Path)
    {
        FString InputString;
        TSharedPtr<FJsonObject> Root;
        if (FFileHelper::LoadFileToString(InputString, *Path))
        {
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InputString);
            FJsonSerializer::Deserialize(Reader, Root);
        }
        return Root.IsValid() ? Root.ToSharedRef() : MakeShared<FJsonObject>();
    }

    bool SaveLightJsonRoot(const TSharedRef<FJsonObject>& Root, const FString& Path)
    {
        FString OutputString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
        if (!FJsonSerializer::Serialize(Root, Writer))
            return false;
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
        return FFileHelper::SaveStringToFile(OutputString, *Path);
    }

    // "perClass" or "perGroup" sub-object of the shared Defaults.json,
    // creating it if this is the first default saved in that catalog.
    TSharedRef<FJsonObject> GetOrCreateLightCatalog(const TSharedRef<FJsonObject>& Root, const TCHAR* CatalogKey)
    {
        const TSharedPtr<FJsonObject>* Existing = nullptr;
        if (Root->TryGetObjectField(CatalogKey, Existing) && Existing->IsValid())
            return Existing->ToSharedRef();
        TSharedRef<FJsonObject> Catalog = MakeShared<FJsonObject>();
        Root->SetObjectField(CatalogKey, Catalog);
        return Catalog;
    }

    bool SaveDefault(const FLightAssetDefault& Default, const FString& Key, const TCHAR* CatalogKey)
    {
        TSharedRef<FJsonObject> Root = LoadLightJsonRoot(ULightDefaultsJsonUtils::GetLightDefaultsPath());
        TSharedRef<FJsonObject> Catalog = GetOrCreateLightCatalog(Root, CatalogKey);

        TSharedPtr<FJsonObject> Object = FJsonObjectConverter::UStructToJsonObject(Default);
        if (!Object.IsValid())
            return false;
        Catalog->SetObjectField(Key, Object);

        return SaveLightJsonRoot(Root, ULightDefaultsJsonUtils::GetLightDefaultsPath());
    }

    bool LoadDefault(FLightAssetDefault& OutDefault, const FString& Key, const TCHAR* CatalogKey)
    {
        TSharedRef<FJsonObject> Root = LoadLightJsonRoot(ULightDefaultsJsonUtils::GetLightDefaultsPath());
        const TSharedPtr<FJsonObject>* Catalog = nullptr;
        if (!Root->TryGetObjectField(CatalogKey, Catalog) || !Catalog->IsValid())
            return false;

        const TSharedPtr<FJsonObject>* Object = nullptr;
        if (!(*Catalog)->TryGetObjectField(Key, Object) || !Object->IsValid())
            return false;

        return FJsonObjectConverter::JsonObjectToUStruct(Object->ToSharedRef(), &OutDefault);
    }

    // Calls Func on every MaterialInstanceDynamic already assigned to one of
    // TrafficLight's StaticMeshComponent slots that exposes a "lightvalue"
    // scalar parameter -- i.e. a signal lens material, not the housing/body.
    // Never creates a MID (see ApplyTrafficLightDefaultToClassLive): only
    // touches what BP_TLOpenDrive's Construction Script already made and
    // still holds array references to.
    template <typename FuncT>
    void ForEachTrafficLightSignalMID(AActor* TrafficLight, FuncT&& Func)
    {
        TArray<UStaticMeshComponent*> MeshComponents;
        TrafficLight->GetComponents<UStaticMeshComponent>(MeshComponents);
        for (UStaticMeshComponent* MeshComponent : MeshComponents)
        {
            const int32 NumMaterials = MeshComponent->GetNumMaterials();
            for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; ++MaterialIndex)
            {
                if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(MeshComponent->GetMaterial(MaterialIndex)))
                {
                    float Dummy = 0.0f;
                    if (MID->GetScalarParameterValue(TEXT("lightvalue"), Dummy))
                    {
                        Func(MID);
                    }
                }
            }
        }
    }

    const FString ProceduralBuildingsKey(TEXT("ProceduralBuildings"));
    const FString LegacyBuildingsKey(TEXT("LegacyBuildings"));

    // Pulls a "<Letters><Digits>" family token out of a building material's
    // own name where one exists (e.g. "MI_Apt20_withEmissive" -> "Apt20",
    // "MI_Apt21" -> "Apt21") -- the naming convention floor/shape variants
    // of the same family share (BP_Apt20_A_C..F_C all reference some
    // MI_Apt20* material). Empty if the name has no digit-suffixed word.
    FString ExtractNumberedFamilyKey(const FString& MaterialName)
    {
        for (int32 Idx = 0; Idx < MaterialName.Len(); ++Idx)
        {
            if (!FChar::IsAlpha(MaterialName[Idx]))
                continue;
            if (Idx > 0 && FChar::IsAlpha(MaterialName[Idx - 1]))
                continue; // not the start of a word

            int32 WordEnd = Idx;
            while (WordEnd < MaterialName.Len() && FChar::IsAlpha(MaterialName[WordEnd]))
                ++WordEnd;
            int32 DigitsEnd = WordEnd;
            while (DigitsEnd < MaterialName.Len() && FChar::IsDigit(MaterialName[DigitsEnd]))
                ++DigitsEnd;
            if (DigitsEnd > WordEnd)
                return MaterialName.Mid(Idx, DigitsEnd - Idx);
        }
        return FString();
    }

    // True (and fills OutFamilyKey) if Owner has any StaticMeshComponent
    // material living under /Game/Carla/Static/Building/ -- a real
    // light-driven building (BP_Apt*, BP_Factory*, BP_Museum*, ...), not
    // just something an artist happened to tag LightGroup::Building/Other.
    // Unwraps a live-edited slot's MID back to its static parent the same
    // way ResolveStaticProceduralBuildingAsset does, so a repeat scan after
    // an edit still recognizes it.
    //
    // Scans every matching slot (not just the first): confirmed the Apt12
    // family's slot order isn't consistent between variants (Apt12_A's
    // first building slot is MI_Apt12_FakeInterior, Apt12_B's is plain
    // MI_Apt12), so picking only the first slot could group inconsistently.
    // Prefers a digit-suffixed family match wherever found (Apt-style);
    // falls back to the first matching slot's exact material name otherwise
    // -- confirmed BP_Factory01_C/02_C/03_C and BP_Museum01_C/02_C share one
    // identically-named MI_Factory/MI_Museum with no digit suffix at all, so
    // the exact name alone is what groups them.
    bool TryGetLightBuildingFamilyKey(const AActor* Owner, FString& OutFamilyKey)
    {
        if (Owner == nullptr)
            return false;

        TArray<UStaticMeshComponent*> MeshComponents;
        Owner->GetComponents<UStaticMeshComponent>(MeshComponents);

        FString FirstMatchedMaterialName;
        for (UStaticMeshComponent* MeshComponent : MeshComponents)
        {
            const int32 NumMaterials = MeshComponent->GetNumMaterials();
            for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; ++MaterialIndex)
            {
                UMaterialInterface* Material = MeshComponent->GetMaterial(MaterialIndex);
                if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Material))
                    Material = MID->Parent;
                if (Material == nullptr)
                    continue;
                if (!Material->GetPathName().StartsWith(TEXT("/Game/Carla/Static/Building/")))
                    continue;

                if (FirstMatchedMaterialName.IsEmpty())
                    FirstMatchedMaterialName = Material->GetName();

                const FString Family = ExtractNumberedFamilyKey(Material->GetName());
                if (!Family.IsEmpty())
                {
                    OutFamilyKey = Family;
                    return true;
                }
            }
        }

        if (!FirstMatchedMaterialName.IsEmpty())
        {
            OutFamilyKey = FirstMatchedMaterialName;
            return true;
        }
        return false;
    }

    // Every building-piece material instance in the project (walls, doors,
    // corners, windows alike) resolves, several parents up, to the same
    // shared M_MaterialMaster used by nearly everything else in Carla content
    // -- matching on the base material would touch far more than windows.
    // What's actually meant to glow is the "FakeInterior" family specifically
    // (MI_Skysc_FakeInterior_A/B/C, ...), all living under
    // /Game/Carla/Static/Building/ -- so require both the content path and
    // "FakeInterior" in the asset name.
    //
    // A slot that's already been live-edited holds a transient
    // MaterialInstanceDynamic (see CollectProceduralBuildingActors) whose own
    // GetPathName() is no longer under /Game/Carla/... -- walk back to the
    // static instance it wraps (MID->Parent) so repeated scans/selects/edits
    // keep recognizing it instead of only matching once, before the first
    // apply. Returns that static asset (not just a bool) since callers also
    // need its own baked default below.
    UMaterialInterface* ResolveStaticProceduralBuildingAsset(UMaterialInterface* Material)
    {
        if (Material == nullptr)
            return nullptr;

        UMaterialInterface* StaticAsset = Material;
        if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Material))
        {
            StaticAsset = MID->Parent;
        }
        if (StaticAsset == nullptr)
            return nullptr;

        if (!StaticAsset->GetPathName().StartsWith(TEXT("/Game/Carla/Static/Building/")))
            return nullptr;
        if (!StaticAsset->GetName().Contains(TEXT("FakeInterior")))
            return nullptr;

        float Dummy = 0.0f;
        if (!Material->GetScalarParameterValue(TEXT("EmissiveIntensity"), Dummy))
            return nullptr;

        return StaticAsset;
    }

    // One matched FakeInterior mesh slot.
    struct FBuildingSlotRef
    {
        TWeakObjectPtr<UStaticMeshComponent> Component;
        int32 MaterialIndex = 0;
    };

    // Neither baked buildings nor the older pre-modular ones have a
    // Blueprint class worth iterating by (see ScanProceduralBuildingsInLevel
    // / ScanLegacyBuildingsInLevel), so this walks every actor in the level
    // and, for any that own at least one FakeInterior slot on a component
    // whose HISM-ness matches bHismOnly, hands the whole per-actor slot list
    // to Callback. The modular building tool always bakes into HISM
    // components; every pre-modular building/placed piece we've found uses a
    // plain (non-instanced) StaticMeshComponent instead -- so this one flag
    // is what actually separates the two mechanisms (see
    // FLightAssetDefault::bLit).
    template <typename FuncT>
    void CollectBuildingActorsByMechanism(UWorld* World, bool bHismOnly, FuncT&& Callback)
    {
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* Actor = *It;
            if (Actor == nullptr)
                continue;

            TArray<UStaticMeshComponent*> MeshComponents;
            Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
            TArray<FBuildingSlotRef> Slots;
            for (UStaticMeshComponent* MeshComponent : MeshComponents)
            {
                const bool bIsHism = MeshComponent->IsA<UHierarchicalInstancedStaticMeshComponent>();
                if (bIsHism != bHismOnly)
                    continue;

                const int32 NumMaterials = MeshComponent->GetNumMaterials();
                for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; ++MaterialIndex)
                {
                    if (ResolveStaticProceduralBuildingAsset(MeshComponent->GetMaterial(MaterialIndex)) == nullptr)
                        continue;

                    FBuildingSlotRef Slot;
                    Slot.Component = MeshComponent;
                    Slot.MaterialIndex = MaterialIndex;
                    Slots.Add(Slot);
                }
            }
            if (Slots.Num() > 0)
            {
                Callback(Actor, Slots);
            }
        }
    }

    // Applies EmissiveIntensity to one actor's already-collected slot list
    // (see CollectProceduralBuildingActors). Per-window on/off variation is
    // no longer decided here: each matched slot's material is a shared MID
    // covering every instance in that HISM component at once (hundreds of
    // physical windows in some cases, confirmed via NumCustomDataFloats==0 --
    // no per-instance data buffer exists to vary through C++), so a
    // per-slot coin flip here would only ever turn whole component-sized
    // blocks of windows on/off together, not individual ones.
    //
    // M_FakeInterior (see PR discussion) was extended with a
    // "PercentLitThreshold" parameter gated by a PerInstanceRandom node in
    // its own graph: Gate = (PerInstanceRandom < PercentLitThreshold) ? 1 : 0,
    // multiplied into the existing emissive output. PerInstanceRandom is
    // resolved per physical instance by the renderer itself, so this is what
    // actually achieves individual-window variation; PercentLit here just
    // becomes the threshold we hand it (as a 0..1 fraction).
    void ApplyToActorSlots(AActor* /*Actor*/, const TArray<FBuildingSlotRef>& Slots, float EmissiveIntensity, float PercentLit)
    {
        const float Threshold = FMath::Clamp(PercentLit / 100.0f, 0.0f, 1.0f);
        for (const FBuildingSlotRef& Slot : Slots)
        {
            UStaticMeshComponent* Component = Slot.Component.Get();
            if (Component == nullptr)
                continue;

            UMaterialInstanceDynamic* MID = Component->CreateAndSetMaterialInstanceDynamic(Slot.MaterialIndex);
            if (MID == nullptr)
                continue;

            MID->SetScalarParameterValue(TEXT("EmissiveIntensity"), EmissiveIntensity);
            MID->SetScalarParameterValue(TEXT("On/Off"), 1.0f);
            MID->SetScalarParameterValue(TEXT("PercentLitThreshold"), Threshold);
        }
    }

    // A stable pseudo-random value per actor, from a hash of its path (fixed
    // for as long as the actor isn't renamed/moved between levels) -- used
    // to decide whether a whole legacy building is lit, see
    // ApplyToLegacyActorSlots. Tried gating legacy buildings with the same
    // PercentLitThreshold + PerInstanceRandom the baked HISM path uses
    // (single StaticMeshComponent = single "instance", so the per-window
    // math degrades to per-building) -- confirmed live in-editor that it
    // doesn't work: PerInstanceRandom came back effectively identical across
    // different legacy actors, so every building snapped on together at any
    // nonzero threshold instead of a graduated percentage. Deciding it here
    // in C++ instead, per actor, sidesteps whatever that material-side
    // constant turned out to be.
    float StableActorRandom01(const AActor* Actor)
    {
        const uint32 Hash = GetTypeHash(Actor->GetPathName());
        return (Hash % 100000) / 100000.0f;
    }

    // Legacy (non-HISM) equivalent of ApplyToActorSlots. PercentLit still
    // picks what fraction of buildings are lit, just resolved once per actor
    // in C++ (see StableActorRandom01) rather than by the shader -- the
    // whole building is deterministically on or off, no PercentLitThreshold
    // gating needed (pinned to 1.0, i.e. always-pass, since it's irrelevant
    // here).
    void ApplyToLegacyActorSlots(AActor* Actor, const TArray<FBuildingSlotRef>& Slots, float EmissiveIntensity, float PercentLit)
    {
        const float Threshold = FMath::Clamp(PercentLit / 100.0f, 0.0f, 1.0f);
        const bool bLit = StableActorRandom01(Actor) < Threshold;
        for (const FBuildingSlotRef& Slot : Slots)
        {
            UStaticMeshComponent* Component = Slot.Component.Get();
            if (Component == nullptr)
                continue;

            UMaterialInstanceDynamic* MID = Component->CreateAndSetMaterialInstanceDynamic(Slot.MaterialIndex);
            if (MID == nullptr)
                continue;

            MID->SetScalarParameterValue(TEXT("EmissiveIntensity"), bLit ? EmissiveIntensity : 0.0f);
            MID->SetScalarParameterValue(TEXT("On/Off"), bLit ? 1.0f : 0.0f);
            MID->SetScalarParameterValue(TEXT("PercentLitThreshold"), 1.0f);
        }
    }

    // Confirmed identical, same order, across every vehicle's shared
    // "*_Emissive" MaterialInstance checked (Lincoln/MiniCooper/Ambulance) --
    // a project convention, not something to detect per-vehicle. Vehicles
    // with no lights at all (bicycles) simply don't expose these names;
    // GetScalarParameterValue below just returns false for them, no special
    // casing needed. "Special1" is the siren/lightbar group -- a real
    // "special1-N" light component AND a matching material param exist on
    // police/fire/ambulance vehicles, same convention as every other group,
    // just on the sibling M_VehicleLightsMaster_Sirens material (also
    // matched by ResolveVehicleMaterialBase's "VehicleLightsMaster"
    // substring check) instead of the main one. The Sirens material's other
    // params (Orange/Blue/Red/White Frequency, flash rate/color rather than
    // brightness) don't fit this "one float per group" model and are left
    // alone -- artist-tuned in the material itself if needed.
    const TArray<FString> VehicleLightGroupNames = {
        TEXT("Position"), TEXT("Low Beam"), TEXT("High Beam"), TEXT("Fog"),
        TEXT("Brake"), TEXT("Reverse"), TEXT("Left Blinker"), TEXT("Right Blinker"),
        TEXT("Special1"),
    };

    // Real light Intensity values (Ambulance's Low Beam ~500000) are
    // physically-scaled UE5 photometric lux, same story as CarlaLight.cpp's
    // own scale cvars for street lamps -- awkward to type/read directly.
    // The panel divides by this on scan and multiplies back on apply, so an
    // artist works in ~10-100 instead.
    constexpr float VehicleLightIntensityDisplayScale = 10000.0f;

    // Walks a MaterialInstance's Parent chain to the ultimate base UMaterial
    // (not just one level -- an "*_Emissive" MI can itself be instanced
    // again per vehicle color/variant). Also unwraps a live-edited slot's
    // MID first, so a repeat scan/apply after an edit still resolves it.
    UMaterialInterface* ResolveVehicleMaterialBase(UMaterialInterface* Material)
    {
        if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Material))
            Material = MID->Parent;
        UMaterialInterface* Base = Material;
        int32 SafetyCount = 0;
        while (UMaterialInstance* Instance = Cast<UMaterialInstance>(Base))
        {
            if (Instance->Parent == nullptr || ++SafetyCount > 10)
                break;
            Base = Instance->Parent;
        }
        return Base;
    }

    // One matched vehicle-light material slot: NOT detected by the
    // individual MaterialInstance's own name (confirmed too fragile --
    // SM_RBlinker/SM_LBlinker's blinker meshes reuse the SAME
    // MI_Ambulance_Emissive instance as the main light mesh, so that part
    // was fine, but the same "Emissive"-in-the-name heuristic also wrongly
    // pulled in MI_Ambulance_Glass_Emissive_Ext/Int, a completely different
    // system -- interior/exterior glass glow -- with none of the 8 group
    // params). Instead walks to the material's ultimate base and checks
    // that against the shared master both real vehicle-light MIs actually
    // derive from: /Game/.../M_VehicleLightsMaster(_Sirens) (confirmed via
    // the full parent chain on BP_Ambulance). One check, both variants,
    // since "Sirens" is just a name suffix on the same "VehicleLightsMaster"
    // token. Works identically on a live actor or a Blueprint's CDO --
    // CreateDefaultSubobject components are reachable through GetComponents
    // either way, which is what lets ScanVehicleCatalog sample a class's
    // defaults with no level or running episode at all.
    struct FVehicleEmissiveSlotRef
    {
        TWeakObjectPtr<UStaticMeshComponent> Component;
        int32 MaterialIndex = 0;
    };

    TArray<FVehicleEmissiveSlotRef> CollectVehicleEmissiveSlots(AActor* Actor)
    {
        TArray<FVehicleEmissiveSlotRef> Result;
        if (Actor == nullptr)
            return Result;

        TArray<UStaticMeshComponent*> MeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
        for (UStaticMeshComponent* MeshComponent : MeshComponents)
        {
            const int32 NumMaterials = MeshComponent->GetNumMaterials();
            for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; ++MaterialIndex)
            {
                UMaterialInterface* Base = ResolveVehicleMaterialBase(MeshComponent->GetMaterial(MaterialIndex));
                if (Base == nullptr || !Base->GetName().Contains(TEXT("VehicleLightsMaster")))
                    continue;

                FVehicleEmissiveSlotRef Slot;
                Slot.Component = MeshComponent;
                Slot.MaterialIndex = MaterialIndex;
                Result.Add(Slot);
            }
        }
        return Result;
    }

    // True if a real light component's own name (e.g. "front-low_beam-l-1",
    // "back-brake-r-1", confirmed convention across all 24 vehicles) belongs
    // to GroupName. Matches by substring on the same tokens the naming
    // convention itself uses, front+back together under one group (mirrors
    // how the Emissive material already groups its own "Fog"/"Position"/...
    // parameters -- one param lights both the front and back fog lenses),
    // except the two Blinker groups, which the "-l-"/"-r-" token splits.
    // "Position" excludes "plate-position" (the license plate light) --
    // deliberately its own thing, not a car light group.
    bool ComponentNameMatchesGroup(const FString& ComponentName, const FString& GroupName)
    {
        const FString Lower = ComponentName.ToLower();
        if (GroupName == TEXT("Left Blinker"))
            return Lower.Contains(TEXT("blinker")) && Lower.Contains(TEXT("-l-"));
        if (GroupName == TEXT("Right Blinker"))
            return Lower.Contains(TEXT("blinker")) && Lower.Contains(TEXT("-r-"));
        if (GroupName == TEXT("Low Beam"))
            return Lower.Contains(TEXT("low_beam"));
        if (GroupName == TEXT("High Beam"))
            return Lower.Contains(TEXT("high_beam"));
        if (GroupName == TEXT("Position"))
            return Lower.Contains(TEXT("position")) && !Lower.Contains(TEXT("plate"));
        if (GroupName == TEXT("Fog"))
            return Lower.Contains(TEXT("fog"));
        if (GroupName == TEXT("Brake"))
            return Lower.Contains(TEXT("brake"));
        if (GroupName == TEXT("Reverse"))
            return Lower.Contains(TEXT("reverse"));
        if (GroupName == TEXT("Special1"))
            return Lower.Contains(TEXT("special1"));
        return false;
    }

    TArray<ULightComponent*> CollectVehicleLightComponentsForGroup(AActor* Actor, const FString& GroupName)
    {
        TArray<ULightComponent*> Result;
        if (Actor == nullptr)
            return Result;

        TArray<ULightComponent*> AllLights;
        Actor->GetComponents<ULightComponent>(AllLights);
        for (ULightComponent* LightComponent : AllLights)
            if (LightComponent != nullptr && ComponentNameMatchesGroup(LightComponent->GetName(), GroupName))
                Result.Add(LightComponent);
        return Result;
    }

#if WITH_EDITOR
    TWeakObjectPtr<AActor> CurrentVehiclePreviewActor;
#endif
}

FString ULightDefaultsJsonUtils::GetLightGroupName(ELightType Group)
{
    // GetNameStringByValue returns it qualified ("ELightType::Street" on some
    // UE flavors), so strip anything before the last "::".
    const UEnum* Enum = StaticEnum<ELightType>();
    FString Name = Enum ? Enum->GetNameStringByValue(static_cast<int64>(Group)) : FString();
    int32 Idx;
    if (Name.FindLastChar(TEXT(':'), Idx))
        Name = Name.RightChop(Idx + 1);
    return Name.IsEmpty() ? TEXT("Other") : Name;
}

bool ULightDefaultsJsonUtils::SaveClassDefault(const FString& ClassName, const FLightAssetDefault& Default)
{
    return SaveDefault(Default, ClassName, TEXT("perClass"));
}

bool ULightDefaultsJsonUtils::LoadClassDefault(const FString& ClassName, FLightAssetDefault& OutDefault)
{
    return LoadDefault(OutDefault, ClassName, TEXT("perClass"));
}

bool ULightDefaultsJsonUtils::SaveGroupDefault(ELightType Group, const FLightAssetDefault& Default)
{
    return SaveDefault(Default, ULightDefaultsJsonUtils::GetLightGroupName(Group), TEXT("perGroup"));
}

bool ULightDefaultsJsonUtils::LoadGroupDefault(ELightType Group, FLightAssetDefault& OutDefault)
{
    return LoadDefault(OutDefault, ULightDefaultsJsonUtils::GetLightGroupName(Group), TEXT("perGroup"));
}

bool ULightDefaultsJsonUtils::GetLightDefault(const FString& ClassName, ELightType Group, FLightAssetDefault& OutDefault)
{
    if (LoadClassDefault(ClassName, OutDefault))
        return true;
    return LoadGroupDefault(Group, OutDefault);
}

FString ULightDefaultsJsonUtils::GetLightClassKey(AActor* Owner, bool& bOutIsBuildingClass)
{
    if (Owner == nullptr)
    {
        bOutIsBuildingClass = false;
        return FString();
    }

    FString FamilyKey;
    if (TryGetLightBuildingFamilyKey(Owner, FamilyKey))
    {
        bOutIsBuildingClass = true;
        return FamilyKey;
    }

    bOutIsBuildingClass = false;
    // Keyed by the owning actor's class (BP_StreetLight01_C, ...): every
    // light-bearing actor places the same BP_Lights component, so keying by
    // the component's own class would collapse every light in the level
    // into one bucket.
    return Owner->GetClass()->GetName();
}

TArray<FLightAssetSummary> ULightDefaultsJsonUtils::ScanLightAssetsInLevel(const UObject* WorldContextObject)
{
    TArray<FLightAssetSummary> Result;

    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    UCarlaLightSubsystem* Subsystem = World ? World->GetSubsystem<UCarlaLightSubsystem>() : nullptr;
    if (Subsystem == nullptr)
        return Result;

    TMap<FString, int32> ClassNameToIndex;
    for (const auto& Pair : Subsystem->GetLights())
    {
        UCarlaLight* Light = Pair.Value;
        if (Light == nullptr || Light->GetOwner() == nullptr)
            continue;

        bool bIsBuildingClass = false;
        const FString ClassName = GetLightClassKey(Light->GetOwner(), bIsBuildingClass);
        if (int32* ExistingIndex = ClassNameToIndex.Find(ClassName))
        {
            Result[*ExistingIndex].InstanceCount++;
            continue;
        }

        FLightAssetSummary Summary;
        Summary.ClassName = ClassName;
        Summary.LightGroup = Light->GetLightType();
        Summary.InstanceCount = 1;
        Summary.bIsBuildingClass = bIsBuildingClass;
        Summary.Current.Color = Light->GetLightColor();
        Summary.Current.Intensity = Light->GetLightIntensity();
        Summary.Current.EmissiveIntensity = Light->GetEmissiveIntensity();

        FLightAssetDefault SavedDefault;
        if (GetLightDefault(ClassName, Summary.LightGroup, SavedDefault))
        {
            Summary.Current = SavedDefault;
            Summary.bHasSavedDefault = true;
        }

        ClassNameToIndex.Add(ClassName, Result.Num());
        Result.Add(Summary);
    }

    Result.Sort([](const FLightAssetSummary& A, const FLightAssetSummary& B)
    {
        return A.ClassName < B.ClassName;
    });
    return Result;
}

void ULightDefaultsJsonUtils::ReapplyLightDefaultsToLevel(const UObject* WorldContextObject)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    UCarlaLightSubsystem* Subsystem = World ? World->GetSubsystem<UCarlaLightSubsystem>() : nullptr;
    if (Subsystem == nullptr)
        return;

    for (const auto& Pair : Subsystem->GetLights())
    {
        UCarlaLight* Light = Pair.Value;
        if (Light == nullptr || Light->GetOwner() == nullptr)
            continue;

        bool bIsBuildingClass = false;
        FLightAssetDefault Default;
        if (GetLightDefault(GetLightClassKey(Light->GetOwner(), bIsBuildingClass), Light->GetLightType(), Default))
        {
            Light->SetLightColor(Default.Color);
            Light->SetLightIntensity(Default.Intensity);
            Light->SetEmissiveIntensity(Default.EmissiveIntensity);
        }
    }
}

void ULightDefaultsJsonUtils::ApplyLightDefaultToClassLive(const UObject* WorldContextObject, const FString& ClassName, const FLightAssetDefault& Value)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    UCarlaLightSubsystem* Subsystem = World ? World->GetSubsystem<UCarlaLightSubsystem>() : nullptr;
    if (Subsystem == nullptr)
        return;

    for (const auto& Pair : Subsystem->GetLights())
    {
        UCarlaLight* Light = Pair.Value;
        if (Light == nullptr || Light->GetOwner() == nullptr)
            continue;
        bool bIsBuildingClass = false;
        if (GetLightClassKey(Light->GetOwner(), bIsBuildingClass) != ClassName)
            continue;

        Light->SetLightColor(Value.Color);
        Light->SetLightIntensity(Value.Intensity);
        Light->SetEmissiveIntensity(Value.EmissiveIntensity);
    }
}

void ULightDefaultsJsonUtils::SetClassActorsSelected(const UObject* WorldContextObject, const FString& ClassName, bool bSelected)
{
#if WITH_EDITOR
    if (GEditor == nullptr)
        return;

    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    UCarlaLightSubsystem* Subsystem = World ? World->GetSubsystem<UCarlaLightSubsystem>() : nullptr;
    if (Subsystem == nullptr)
        return;

    for (const auto& Pair : Subsystem->GetLights())
    {
        UCarlaLight* Light = Pair.Value;
        AActor* Owner = Light ? Light->GetOwner() : nullptr;
        if (Owner == nullptr)
            continue;
        bool bIsBuildingClass = false;
        if (GetLightClassKey(Owner, bIsBuildingClass) != ClassName)
            continue;
        GEditor->SelectActor(Owner, bSelected, /*bNotify=*/false);
    }
    GEditor->NoteSelectionChange();
#endif
}

TArray<FLightAssetSummary> ULightDefaultsJsonUtils::ScanTrafficLightAssetsInLevel(const UObject* WorldContextObject)
{
    TArray<FLightAssetSummary> Result;

    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return Result;

    TMap<FString, int32> ClassNameToIndex;
    for (TActorIterator<ATrafficLightBase> It(World); It; ++It)
    {
        ATrafficLightBase* TrafficLight = *It;
        if (TrafficLight == nullptr)
            continue;

        const FString ClassName = TrafficLight->GetClass()->GetName();
        if (int32* ExistingIndex = ClassNameToIndex.Find(ClassName))
        {
            Result[*ExistingIndex].InstanceCount++;
            continue;
        }

        FLightAssetSummary Summary;
        Summary.ClassName = ClassName;
        Summary.InstanceCount = 1;
        Summary.Current.Color = FLinearColor::White;
        Summary.Current.Intensity = 0.0f;

        // Sample the first signal MID found as the starting value if there's
        // no saved default yet.
        ForEachTrafficLightSignalMID(TrafficLight, [&Summary](UMaterialInstanceDynamic* MID)
        {
            if (Summary.Current.EmissiveIntensity == 0.0f)
            {
                float Value = 0.0f;
                if (MID->GetScalarParameterValue(TEXT("lightvalue"), Value))
                {
                    Summary.Current.EmissiveIntensity = Value;
                }
            }
        });

        FLightAssetDefault SavedDefault;
        if (LoadClassDefault(ClassName, SavedDefault))
        {
            Summary.Current.EmissiveIntensity = SavedDefault.EmissiveIntensity;
            Summary.bHasSavedDefault = true;
        }

        ClassNameToIndex.Add(ClassName, Result.Num());
        Result.Add(Summary);
    }

    Result.Sort([](const FLightAssetSummary& A, const FLightAssetSummary& B)
    {
        return A.ClassName < B.ClassName;
    });
    return Result;
}

void ULightDefaultsJsonUtils::ApplyTrafficLightDefaultToClassLive(const UObject* WorldContextObject, const FString& ClassName, float EmissiveIntensity)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return;

    for (TActorIterator<ATrafficLightBase> It(World); It; ++It)
    {
        ATrafficLightBase* TrafficLight = *It;
        if (TrafficLight == nullptr || TrafficLight->GetClass()->GetName() != ClassName)
            continue;

        ForEachTrafficLightSignalMID(TrafficLight, [EmissiveIntensity](UMaterialInstanceDynamic* MID)
        {
            MID->SetScalarParameterValue(TEXT("lightvalue"), EmissiveIntensity);
        });
    }
}

void ULightDefaultsJsonUtils::ReapplyTrafficLightDefaultsToLevel(const UObject* WorldContextObject)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return;

    for (TActorIterator<ATrafficLightBase> It(World); It; ++It)
    {
        ATrafficLightBase* TrafficLight = *It;
        if (TrafficLight == nullptr)
            continue;

        FLightAssetDefault Default;
        if (!LoadClassDefault(TrafficLight->GetClass()->GetName(), Default))
            continue;

        ForEachTrafficLightSignalMID(TrafficLight, [&Default](UMaterialInstanceDynamic* MID)
        {
            MID->SetScalarParameterValue(TEXT("lightvalue"), Default.EmissiveIntensity);
        });
    }
}

void ULightDefaultsJsonUtils::SetTrafficLightClassPreviewOn(const UObject* WorldContextObject, const FString& ClassName, bool bOn)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return;

    const float OnOffValue = bOn ? 1.0f : 0.0f;
    for (TActorIterator<ATrafficLightBase> It(World); It; ++It)
    {
        ATrafficLightBase* TrafficLight = *It;
        if (TrafficLight == nullptr || TrafficLight->GetClass()->GetName() != ClassName)
            continue;

        ForEachTrafficLightSignalMID(TrafficLight, [OnOffValue](UMaterialInstanceDynamic* MID)
        {
            MID->SetScalarParameterValue(TEXT("On/Off"), OnOffValue);
        });
    }
}

void ULightDefaultsJsonUtils::SetTrafficLightClassActorsSelected(const UObject* WorldContextObject, const FString& ClassName, bool bSelected)
{
#if WITH_EDITOR
    if (GEditor == nullptr)
        return;

    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return;

    for (TActorIterator<ATrafficLightBase> It(World); It; ++It)
    {
        ATrafficLightBase* TrafficLight = *It;
        if (TrafficLight == nullptr || TrafficLight->GetClass()->GetName() != ClassName)
            continue;
        GEditor->SelectActor(TrafficLight, bSelected, /*bNotify=*/false);
    }
    GEditor->NoteSelectionChange();
#endif
}

TArray<FLightAssetSummary> ULightDefaultsJsonUtils::ScanProceduralBuildingsInLevel(const UObject* WorldContextObject)
{
    TArray<FLightAssetSummary> Result;

    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return Result;

    int32 ActorCount = 0;
    float SampleEmissive = 0.0f;
    bool bFoundSample = false;
    CollectBuildingActorsByMechanism(World, /*bHismOnly=*/true, [&](AActor*, const TArray<FBuildingSlotRef>& Slots)
    {
        ++ActorCount;
        if (bFoundSample)
            return;
        for (const FBuildingSlotRef& Slot : Slots)
        {
            UStaticMeshComponent* Component = Slot.Component.Get();
            UMaterialInterface* Material = Component ? Component->GetMaterial(Slot.MaterialIndex) : nullptr;
            float Value = 0.0f;
            if (Material != nullptr && Material->GetScalarParameterValue(TEXT("EmissiveIntensity"), Value))
            {
                SampleEmissive = Value;
                bFoundSample = true;
                break;
            }
        }
    });

    if (ActorCount == 0)
        return Result;

    FLightAssetSummary Summary;
    Summary.ClassName = ProceduralBuildingsKey;
    Summary.LightGroup = ELightType::Building;
    Summary.InstanceCount = ActorCount;
    Summary.Current.Color = FLinearColor::White;
    Summary.Current.Intensity = 50.0f; // Percent Lit -- see ApplyToActorSlots.
    Summary.Current.EmissiveIntensity = SampleEmissive;

    FLightAssetDefault SavedDefault;
    if (LoadClassDefault(ProceduralBuildingsKey, SavedDefault))
    {
        Summary.Current = SavedDefault;
        Summary.bHasSavedDefault = true;
    }

    Result.Add(Summary);
    return Result;
}

void ULightDefaultsJsonUtils::ApplyProceduralBuildingsEmissiveLive(const UObject* WorldContextObject, float EmissiveIntensity, float PercentLit)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return;

    CollectBuildingActorsByMechanism(World, /*bHismOnly=*/true, [EmissiveIntensity, PercentLit](AActor* Actor, const TArray<FBuildingSlotRef>& Slots)
    {
        ApplyToActorSlots(Actor, Slots, EmissiveIntensity, PercentLit);
    });
}

void ULightDefaultsJsonUtils::ReapplyProceduralBuildingsToLevel(const UObject* WorldContextObject)
{
    FLightAssetDefault Default;
    if (!LoadClassDefault(ProceduralBuildingsKey, Default))
        return;
    ApplyProceduralBuildingsEmissiveLive(WorldContextObject, Default.EmissiveIntensity, Default.Intensity);
}

void ULightDefaultsJsonUtils::SetProceduralBuildingsSelected(const UObject* WorldContextObject, bool bSelected)
{
#if WITH_EDITOR
    if (GEditor == nullptr)
        return;

    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return;

    CollectBuildingActorsByMechanism(World, /*bHismOnly=*/true, [bSelected](AActor* Actor, const TArray<FBuildingSlotRef>&)
    {
        GEditor->SelectActor(Actor, bSelected, /*bNotify=*/false);
    });
    GEditor->NoteSelectionChange();
#endif
}

TArray<FLightAssetSummary> ULightDefaultsJsonUtils::ScanLegacyBuildingsInLevel(const UObject* WorldContextObject)
{
    TArray<FLightAssetSummary> Result;

    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return Result;

    int32 ActorCount = 0;
    float SampleEmissive = 0.0f;
    bool bFoundSample = false;
    CollectBuildingActorsByMechanism(World, /*bHismOnly=*/false, [&](AActor*, const TArray<FBuildingSlotRef>& Slots)
    {
        ++ActorCount;
        if (bFoundSample)
            return;
        for (const FBuildingSlotRef& Slot : Slots)
        {
            UStaticMeshComponent* Component = Slot.Component.Get();
            UMaterialInterface* Material = Component ? Component->GetMaterial(Slot.MaterialIndex) : nullptr;
            float Value = 0.0f;
            if (Material != nullptr && Material->GetScalarParameterValue(TEXT("EmissiveIntensity"), Value))
            {
                SampleEmissive = Value;
                bFoundSample = true;
                break;
            }
        }
    });

    if (ActorCount == 0)
        return Result;

    FLightAssetSummary Summary;
    Summary.ClassName = LegacyBuildingsKey;
    Summary.LightGroup = ELightType::Building;
    Summary.InstanceCount = ActorCount;
    Summary.Current.Color = FLinearColor::White;
    Summary.Current.Intensity = 50.0f; // Percent Lit -- per-building, see ApplyToActorSlots.
    Summary.Current.EmissiveIntensity = SampleEmissive;

    FLightAssetDefault SavedDefault;
    if (LoadClassDefault(LegacyBuildingsKey, SavedDefault))
    {
        Summary.Current = SavedDefault;
        Summary.bHasSavedDefault = true;
    }

    Result.Add(Summary);
    return Result;
}

void ULightDefaultsJsonUtils::ApplyLegacyBuildingsEmissiveLive(const UObject* WorldContextObject, float EmissiveIntensity, float PercentLit)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return;

    CollectBuildingActorsByMechanism(World, /*bHismOnly=*/false, [EmissiveIntensity, PercentLit](AActor* Actor, const TArray<FBuildingSlotRef>& Slots)
    {
        ApplyToLegacyActorSlots(Actor, Slots, EmissiveIntensity, PercentLit);
    });
}

void ULightDefaultsJsonUtils::ReapplyLegacyBuildingsToLevel(const UObject* WorldContextObject)
{
    FLightAssetDefault Default;
    if (!LoadClassDefault(LegacyBuildingsKey, Default))
        return;
    ApplyLegacyBuildingsEmissiveLive(WorldContextObject, Default.EmissiveIntensity, Default.Intensity);
}

void ULightDefaultsJsonUtils::SetLegacyBuildingsSelected(const UObject* WorldContextObject, bool bSelected)
{
#if WITH_EDITOR
    if (GEditor == nullptr)
        return;

    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return;

    CollectBuildingActorsByMechanism(World, /*bHismOnly=*/false, [bSelected](AActor* Actor, const TArray<FBuildingSlotRef>&)
    {
        GEditor->SelectActor(Actor, bSelected, /*bNotify=*/false);
    });
    GEditor->NoteSelectionChange();
#endif
}

TArray<FString> ULightDefaultsJsonUtils::GetVehicleLightGroupNames()
{
    return VehicleLightGroupNames;
}

TArray<FVehicleLightSummary> ULightDefaultsJsonUtils::ScanVehicleCatalog(const UObject* WorldContextObject)
{
    TArray<FVehicleLightSummary> Result;

    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return Result;

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *GetVehicleParametersPath()))
        return Result;

    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
        return Result;

    const TArray<TSharedPtr<FJsonValue>>* VehiclesArray = nullptr;
    if (!Root->TryGetArrayField(TEXT("Vehicles"), VehiclesArray))
        return Result;

    for (const TSharedPtr<FJsonValue>& Entry : *VehiclesArray)
    {
        const TSharedPtr<FJsonObject>* VehicleObj = nullptr;
        if (!Entry->TryGetObject(VehicleObj) || !VehicleObj->IsValid())
            continue;

        bool bHasLights = false;
        (*VehicleObj)->TryGetBoolField(TEXT("HasLights"), bHasLights);
        if (!bHasLights)
            continue;

        FString ClassPath;
        if (!(*VehicleObj)->TryGetStringField(TEXT("Class"), ClassPath))
            continue;

        UClass* VehicleClass = LoadObject<UClass>(nullptr, *ClassPath);
        if (VehicleClass == nullptr)
        {
            UE_LOG(LogCarla, Warning, TEXT("ScanVehicleCatalog: failed to load class %s"), *ClassPath);
            continue;
        }

        // Spawned (not the CDO): a vehicle Blueprint assembles its meshes
        // and materials in its Construction Script, which the bare class
        // default object never runs -- confirmed headless, every one of the
        // 24 vehicles scanned via the CDO alone came back with zero groups
        // present. Far below the map and destroyed before this function
        // returns, so nothing actually renders it.
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.ObjectFlags |= RF_Transient;
        AActor* TempActor = World->SpawnActor<AActor>(VehicleClass, FVector(0.0f, 0.0f, -1000000.0f), FRotator::ZeroRotator, SpawnParams);
        if (TempActor == nullptr)
            continue;

        FString Make, Model;
        (*VehicleObj)->TryGetStringField(TEXT("Make"), Make);
        (*VehicleObj)->TryGetStringField(TEXT("Model"), Model);

        FVehicleLightSummary Summary;
        Summary.ClassName = VehicleClass->GetName();
        Summary.DisplayName = (Make.IsEmpty() && Model.IsEmpty())
            ? Summary.ClassName
            : FString::Printf(TEXT("%s %s"), *Make, *Model);
        Summary.Intensity = 1.0f;
        for (const FString& GroupName : VehicleLightGroupNames)
            Summary.GroupPresent.Add(GroupName, false);

        // Global Intensity is still sampled off the material's own
        // "Intensity" param as a starting value (its role at apply time has
        // changed though -- see ApplyVehicleLightsLive -- it now writes into
        // every exposed group param, not just this one, which confirmed
        // doing nothing visible on its own).
        bool bFoundIntensity = false;
        for (const FVehicleEmissiveSlotRef& Slot : CollectVehicleEmissiveSlots(TempActor))
        {
            UStaticMeshComponent* Component = Slot.Component.Get();
            UMaterialInterface* Material = Component ? Component->GetMaterial(Slot.MaterialIndex) : nullptr;
            if (Material == nullptr || bFoundIntensity)
                continue;

            float Value = 0.0f;
            if (Material->GetScalarParameterValue(TEXT("Intensity"), Value))
            {
                Summary.Intensity = Value;
                bFoundIntensity = true;
            }
        }

        // Per-group columns: the real Spot/Rect/Point light components'
        // physical Intensity, not the material -- see
        // CollectVehicleLightComponentsForGroup. Present/initial value
        // sampled from whichever matching component is found first.
        TArray<ULightComponent*> AllLightComponents;
        TempActor->GetComponents<ULightComponent>(AllLightComponents);
        for (ULightComponent* LightComponent : AllLightComponents)
        {
            if (LightComponent == nullptr)
                continue;
            for (const FString& GroupName : VehicleLightGroupNames)
            {
                if (!ComponentNameMatchesGroup(LightComponent->GetName(), GroupName))
                    continue;
                Summary.GroupPresent[GroupName] = true;
                if (!Summary.GroupIntensity.Contains(GroupName))
                    Summary.GroupIntensity.Add(GroupName, LightComponent->Intensity / VehicleLightIntensityDisplayScale);
            }
        }

        float SavedIntensity = Summary.Intensity;
        TMap<FString, float> SavedGroups;
        if (LoadVehicleLightDefault(Summary.ClassName, SavedIntensity, SavedGroups))
        {
            Summary.Intensity = SavedIntensity;
            for (const TPair<FString, float>& Pair : SavedGroups)
                if (Summary.GroupPresent.Contains(Pair.Key))
                    Summary.GroupIntensity.Add(Pair.Key, Pair.Value);
            Summary.bHasSavedDefault = true;
        }

        TempActor->Destroy();
        Result.Add(Summary);
    }

    Result.Sort([](const FVehicleLightSummary& A, const FVehicleLightSummary& B)
    {
        return A.DisplayName < B.DisplayName;
    });
    return Result;
}

namespace
{
    // The actual per-actor work, factored out so ACarlaWheeledVehicle's own
    // BeginPlay (ActivateVehicleLightComponents) can apply a saved default
    // to just itself -- calling the world-sweeping ApplyVehicleLightsLive
    // from every spawning vehicle's BeginPlay would rescan every
    // already-spawned vehicle of the same class each time (O(n^2) for n
    // vehicles of one class, e.g. a generate_traffic.py run), pointless
    // since each of those already applied this exact same thing to itself
    // when IT spawned.
    void ApplyVehicleLightsToActor(AActor* Actor, float Intensity, const TMap<FString, float>& GroupIntensity)
    {
        if (Actor == nullptr)
            return;

        // Global Intensity -> every exposed emissive group parameter at
        // once (Position, Low Beam, ...): the material's own standalone
        // "Intensity" param confirmed doing nothing visible by itself, so
        // this is what actually makes the single global field useful.
        for (const FVehicleEmissiveSlotRef& Slot : CollectVehicleEmissiveSlots(Actor))
        {
            UStaticMeshComponent* Component = Slot.Component.Get();
            if (Component == nullptr)
                continue;

            UMaterialInstanceDynamic* MID = Component->CreateAndSetMaterialInstanceDynamic(Slot.MaterialIndex);
            if (MID == nullptr)
                continue;

            for (const FString& GroupName : VehicleLightGroupNames)
                MID->SetScalarParameterValue(*GroupName, Intensity);
        }

        // Per-group columns -> the real Spot/Rect/Point light components'
        // physical Intensity (see CollectVehicleLightComponentsForGroup),
        // not the material.
        for (const TPair<FString, float>& Pair : GroupIntensity)
        {
            for (ULightComponent* LightComponent : CollectVehicleLightComponentsForGroup(Actor, Pair.Key))
            {
                LightComponent->SetIntensity(Pair.Value * VehicleLightIntensityDisplayScale);
            }
        }
    }
}

void ULightDefaultsJsonUtils::ApplyVehicleLightsLive(const UObject* WorldContextObject, const FString& ClassName, float Intensity, const TMap<FString, float>& GroupIntensity)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor == nullptr || Actor->GetClass()->GetName() != ClassName)
            continue;
        ApplyVehicleLightsToActor(Actor, Intensity, GroupIntensity);
    }
}

void ULightDefaultsJsonUtils::ApplyVehicleLightsToSingleActor(AActor* Actor, float Intensity, const TMap<FString, float>& GroupIntensity)
{
    ApplyVehicleLightsToActor(Actor, Intensity, GroupIntensity);
}

void ULightDefaultsJsonUtils::ApplyVehicleLightsRuntimeState(AActor* Actor, const TMap<FString, float>& MaterialGroupValue, const TMap<FString, float>& RealLightGroupIntensity)
{
    if (Actor == nullptr)
        return;

    for (const FVehicleEmissiveSlotRef& Slot : CollectVehicleEmissiveSlots(Actor))
    {
        UStaticMeshComponent* Component = Slot.Component.Get();
        if (Component == nullptr)
            continue;

        UMaterialInstanceDynamic* MID = Component->CreateAndSetMaterialInstanceDynamic(Slot.MaterialIndex);
        if (MID == nullptr)
            continue;

        for (const FString& GroupName : VehicleLightGroupNames)
        {
            const float* Value = MaterialGroupValue.Find(GroupName);
            MID->SetScalarParameterValue(*GroupName, Value != nullptr ? *Value : 0.0f);
        }
    }

    for (const TPair<FString, float>& Pair : RealLightGroupIntensity)
    {
        for (ULightComponent* LightComponent : CollectVehicleLightComponentsForGroup(Actor, Pair.Key))
        {
            LightComponent->SetIntensity(Pair.Value * VehicleLightIntensityDisplayScale);
        }
    }
}

bool ULightDefaultsJsonUtils::SaveVehicleLightDefault(const FString& ClassName, float Intensity, const TMap<FString, float>& GroupIntensity)
{
    TSharedRef<FJsonObject> Root = LoadLightJsonRoot(GetLightDefaultsPath());
    TSharedRef<FJsonObject> Catalog = GetOrCreateLightCatalog(Root, TEXT("perVehicleClass"));

    TSharedRef<FJsonObject> Entry = MakeShared<FJsonObject>();
    Entry->SetNumberField(TEXT("Intensity"), Intensity);
    TSharedRef<FJsonObject> GroupsObj = MakeShared<FJsonObject>();
    for (const TPair<FString, float>& Pair : GroupIntensity)
        GroupsObj->SetNumberField(Pair.Key, Pair.Value);
    Entry->SetObjectField(TEXT("GroupIntensity"), GroupsObj);
    Catalog->SetObjectField(ClassName, Entry);

    return SaveLightJsonRoot(Root, GetLightDefaultsPath());
}

bool ULightDefaultsJsonUtils::LoadVehicleLightDefault(const FString& ClassName, float& OutIntensity, TMap<FString, float>& OutGroupIntensity)
{
    TSharedRef<FJsonObject> Root = LoadLightJsonRoot(GetLightDefaultsPath());
    const TSharedPtr<FJsonObject>* Catalog = nullptr;
    if (!Root->TryGetObjectField(TEXT("perVehicleClass"), Catalog) || !Catalog->IsValid())
        return false;

    const TSharedPtr<FJsonObject>* Entry = nullptr;
    if (!(*Catalog)->TryGetObjectField(ClassName, Entry) || !Entry->IsValid())
        return false;

    double IntensityValue = OutIntensity;
    (*Entry)->TryGetNumberField(TEXT("Intensity"), IntensityValue);
    OutIntensity = static_cast<float>(IntensityValue);

    const TSharedPtr<FJsonObject>* GroupsObj = nullptr;
    if ((*Entry)->TryGetObjectField(TEXT("GroupIntensity"), GroupsObj) && GroupsObj->IsValid())
    {
        for (const auto& Pair : (*GroupsObj)->Values)
        {
            double Value = 0.0;
            if (Pair.Value->TryGetNumber(Value))
                OutGroupIntensity.Add(FString(*Pair.Key), static_cast<float>(Value));
        }
    }
    return true;
}

void ULightDefaultsJsonUtils::ReapplyVehicleLightsToLevel(const UObject* WorldContextObject)
{
    TSharedRef<FJsonObject> Root = LoadLightJsonRoot(GetLightDefaultsPath());
    const TSharedPtr<FJsonObject>* Catalog = nullptr;
    if (!Root->TryGetObjectField(TEXT("perVehicleClass"), Catalog) || !Catalog->IsValid())
        return;

    for (const auto& Pair : (*Catalog)->Values)
    {
        const FString ClassName(*Pair.Key);
        float Intensity = 1.0f;
        TMap<FString, float> GroupIntensity;
        if (LoadVehicleLightDefault(ClassName, Intensity, GroupIntensity))
            ApplyVehicleLightsLive(WorldContextObject, ClassName, Intensity, GroupIntensity);
    }
}

AActor* ULightDefaultsJsonUtils::SpawnVehiclePreview(const UObject* WorldContextObject, const FString& ClassName)
{
#if WITH_EDITOR
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr || GEditor == nullptr)
        return nullptr;

    ClearVehiclePreview(WorldContextObject);

    UClass* VehicleClass = nullptr;
    for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
    {
        if (ClassIt->GetName() == ClassName)
        {
            VehicleClass = *ClassIt;
            break;
        }
    }
    if (VehicleClass == nullptr)
        return nullptr;

    FLevelEditorViewportClient* ViewportClient = GCurrentLevelEditingViewportClient;
    if (ViewportClient == nullptr)
        return nullptr;

    // Straight down from wherever the artist last navigated the viewport --
    // no fixed world coordinate needed, and it stays put across repeated
    // Preview clicks as long as the viewport doesn't move in between.
    const FVector CameraLocation = ViewportClient->GetViewLocation();
    const FVector TraceStart(CameraLocation.X, CameraLocation.Y, CameraLocation.Z + 2000.0f);
    const FVector TraceEnd(CameraLocation.X, CameraLocation.Y, CameraLocation.Z - 100000.0f);

    FHitResult Hit;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    if (!World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
        return nullptr;

    const FVector SpawnLocation = Hit.Location + FVector(0.0f, 0.0f, 50.0f);
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Spawned = World->SpawnActor<AActor>(VehicleClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (Spawned == nullptr)
        return nullptr;

    CurrentVehiclePreviewActor = Spawned;

    // Fixed offset behind/above/to the side, framing the whole car -- same
    // idea as the lw_suite.py test script's cam_pose, just via the editor
    // viewport instead of a spawned camera actor.
    const FVector CamOffset(-700.0f, -500.0f, 350.0f);
    const FVector CamLocation = SpawnLocation + CamOffset;
    ViewportClient->SetViewLocation(CamLocation);
    ViewportClient->SetViewRotation((SpawnLocation - CamLocation).Rotation());
    ViewportClient->Invalidate();

    return Spawned;
#else
    return nullptr;
#endif
}

void ULightDefaultsJsonUtils::ClearVehiclePreview(const UObject* WorldContextObject)
{
#if WITH_EDITOR
    if (AActor* Existing = CurrentVehiclePreviewActor.Get())
        Existing->Destroy();
    CurrentVehiclePreviewActor = nullptr;
#endif
}

void ULightDefaultsJsonUtils::PreviewVehicleBlinkers(const UObject* WorldContextObject)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (World == nullptr)
        return;
    UMaterialParameterCollection* Collection = LoadObject<UMaterialParameterCollection>(
        nullptr, TEXT("/Game/Carla/Blueprints/Game/CarlaParameters.CarlaParameters"));
    if (Collection == nullptr)
        return;
    UKismetMaterialLibrary::SetScalarParameterValue(World, Collection, TEXT("VisualTime"), 0.25f);
}

