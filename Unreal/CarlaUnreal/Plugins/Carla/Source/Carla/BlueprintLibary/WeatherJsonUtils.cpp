// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "BlueprintLibary/WeatherJsonUtils.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    TSharedRef<FJsonObject> LoadJsonRoot(const FString& Path)
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

    bool SaveJsonRoot(const TSharedRef<FJsonObject>& Root, const FString& Path)
    {
        FString OutputString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
        if (!FJsonSerializer::Serialize(Root, Writer))
            return false;
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
        return FFileHelper::SaveStringToFile(OutputString, *Path);
    }

    // "time" or "conditions" sub-object of the shared Presets.json, creating
    // it if this is the first preset saved in that catalog.
    TSharedRef<FJsonObject> GetOrCreateCatalog(const TSharedRef<FJsonObject>& Root, const TCHAR* CatalogKey)
    {
        const TSharedPtr<FJsonObject>* Existing = nullptr;
        if (Root->TryGetObjectField(CatalogKey, Existing) && Existing->IsValid())
            return Existing->ToSharedRef();
        TSharedRef<FJsonObject> Catalog = MakeShared<FJsonObject>();
        Root->SetObjectField(CatalogKey, Catalog);
        return Catalog;
    }

    template <typename TPresetStruct>
    bool SavePreset(const TPresetStruct& Preset, const FString& Name, const TCHAR* CatalogKey)
    {
        TSharedRef<FJsonObject> Root = LoadJsonRoot(UWeatherJsonUtils::GetPresetsPath());
        TSharedRef<FJsonObject> Catalog = GetOrCreateCatalog(Root, CatalogKey);

        TSharedPtr<FJsonObject> PresetObject = FJsonObjectConverter::UStructToJsonObject(Preset);
        if (!PresetObject.IsValid())
            return false;
        Catalog->SetObjectField(Name, PresetObject);

        return SaveJsonRoot(Root, UWeatherJsonUtils::GetPresetsPath());
    }

    template <typename TPresetStruct>
    bool LoadPreset(TPresetStruct& OutPreset, const FString& Name, const TCHAR* CatalogKey)
    {
        TSharedRef<FJsonObject> Root = LoadJsonRoot(UWeatherJsonUtils::GetPresetsPath());
        const TSharedPtr<FJsonObject>* Catalog = nullptr;
        if (!Root->TryGetObjectField(CatalogKey, Catalog) || !Catalog->IsValid())
            return false;

        const TSharedPtr<FJsonObject>* PresetObject = nullptr;
        if (!(*Catalog)->TryGetObjectField(Name, PresetObject) || !PresetObject->IsValid())
            return false;

        return FJsonObjectConverter::JsonObjectToUStruct(PresetObject->ToSharedRef(), &OutPreset);
    }

    TArray<FString> GetCatalogNames(const TCHAR* CatalogKey)
    {
        TArray<FString> Names;
        TSharedRef<FJsonObject> Root = LoadJsonRoot(UWeatherJsonUtils::GetPresetsPath());
        const TSharedPtr<FJsonObject>* Catalog = nullptr;
        if (Root->TryGetObjectField(CatalogKey, Catalog) && Catalog->IsValid())
        {
            for (const auto& Pair : (*Catalog)->Values)
                Names.Add(FString(Pair.Key));
        }
        Names.Sort();
        return Names;
    }
}

bool UWeatherJsonUtils::SaveTimePreset(const FWeatherTimePreset& Preset, const FString& Name)
{
    return SavePreset(Preset, Name, TEXT("time"));
}

bool UWeatherJsonUtils::LoadTimePreset(FWeatherTimePreset& OutPreset, const FString& Name)
{
    return LoadPreset(OutPreset, Name, TEXT("time"));
}

bool UWeatherJsonUtils::SaveConditionPreset(const FWeatherConditionPreset& Preset, const FString& Name)
{
    return SavePreset(Preset, Name, TEXT("conditions"));
}

bool UWeatherJsonUtils::LoadConditionPreset(FWeatherConditionPreset& OutPreset, const FString& Name)
{
    return LoadPreset(OutPreset, Name, TEXT("conditions"));
}

TArray<FString> UWeatherJsonUtils::GetAvailableTimePresetNames()
{
    return GetCatalogNames(TEXT("time"));
}

TArray<FString> UWeatherJsonUtils::GetAvailableConditionPresetNames()
{
    return GetCatalogNames(TEXT("conditions"));
}

FWeatherParameters UWeatherJsonUtils::MergeWeatherPresets(
    const FWeatherParameters& Base,
    const FWeatherTimePreset& TimePreset,
    const FWeatherConditionPreset& ConditionPreset)
{
    FWeatherParameters Result = Base;

    Result.SunAzimuthAngle = TimePreset.SunAzimuthAngle;
    Result.SunAltitudeAngle = TimePreset.SunAltitudeAngle;

    Result.Cloudiness = ConditionPreset.Cloudiness;
    Result.Precipitation = ConditionPreset.Precipitation;
    Result.PrecipitationDeposits = ConditionPreset.PrecipitationDeposits;
    Result.WindIntensity = ConditionPreset.WindIntensity;
    Result.FogDensity = ConditionPreset.FogDensity;
    Result.FogDistance = ConditionPreset.FogDistance;
    Result.FogFalloff = ConditionPreset.FogFalloff;
    Result.Wetness = ConditionPreset.Wetness;
    Result.ScatteringIntensity = ConditionPreset.ScatteringIntensity;
    Result.MieScatteringScale = ConditionPreset.MieScatteringScale;
    Result.RayleighScatteringScale = ConditionPreset.RayleighScatteringScale;

    return Result;
}

bool UWeatherJsonUtils::GetMapDefaultPresetNames(const FString& MapName, FString& OutTimeName, FString& OutConditionName)
{
    TSharedRef<FJsonObject> Root = LoadJsonRoot(GetMapDefaultsPath());
    const TSharedPtr<FJsonObject>* MapEntry = nullptr;
    if (!Root->TryGetObjectField(MapName, MapEntry) || !MapEntry->IsValid())
        return false;

    return (*MapEntry)->TryGetStringField(TEXT("time"), OutTimeName)
        && (*MapEntry)->TryGetStringField(TEXT("condition"), OutConditionName);
}

bool UWeatherJsonUtils::SetMapDefaultPresetNames(const FString& MapName, const FString& TimeName, const FString& ConditionName)
{
    TSharedRef<FJsonObject> Root = LoadJsonRoot(GetMapDefaultsPath());

    TSharedRef<FJsonObject> MapEntry = MakeShared<FJsonObject>();
    MapEntry->SetStringField(TEXT("time"), TimeName);
    MapEntry->SetStringField(TEXT("condition"), ConditionName);
    Root->SetObjectField(MapName, MapEntry);

    return SaveJsonRoot(Root, GetMapDefaultsPath());
}

bool UWeatherJsonUtils::GetMapDefaultWeather(const FString& MapName, const FWeatherParameters& Base, FWeatherParameters& OutWeather)
{
    TSharedRef<FJsonObject> Root = LoadJsonRoot(GetMapDefaultsPath());
    const TSharedPtr<FJsonObject>* MapEntry = nullptr;
    if (!Root->TryGetObjectField(MapName, MapEntry) || !MapEntry->IsValid())
        return false;

    FString TimeName, ConditionName;
    const bool bIsPresetPair = (*MapEntry)->TryGetStringField(TEXT("time"), TimeName)
        && (*MapEntry)->TryGetStringField(TEXT("condition"), ConditionName);
    if (bIsPresetPair)
    {
        FWeatherTimePreset TimePreset;
        FWeatherConditionPreset ConditionPreset;
        if (!LoadTimePreset(TimePreset, TimeName) || !LoadConditionPreset(ConditionPreset, ConditionName))
            return false;
        OutWeather = MergeWeatherPresets(Base, TimePreset, ConditionPreset);
        return true;
    }

    // Not a {time, condition} pair -- the entry itself is a full inline
    // FWeatherParameters snapshot (see SetMapDefaultWeather). Start from Base
    // so a future field this snapshot predates keeps Base's value instead of
    // silently zeroing.
    OutWeather = Base;
    return FJsonObjectConverter::JsonObjectToUStruct((*MapEntry).ToSharedRef(), &OutWeather);
}

bool UWeatherJsonUtils::SetMapDefaultWeather(const FString& MapName, const FWeatherParameters& Weather)
{
    TSharedRef<FJsonObject> Root = LoadJsonRoot(GetMapDefaultsPath());

    TSharedPtr<FJsonObject> WeatherObject = FJsonObjectConverter::UStructToJsonObject(Weather);
    if (!WeatherObject.IsValid())
        return false;
    Root->SetObjectField(MapName, WeatherObject);

    return SaveJsonRoot(Root, GetMapDefaultsPath());
}
