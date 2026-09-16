// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Carla/Weather/WeatherParameters.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/Paths.h"
#include "WeatherJsonUtils.generated.h"

// Sun position only -- everything a "time of day" preset (Sunrise, Noon,
// Sunset, Night...) needs to say.
USTRUCT(BlueprintType)
struct FWeatherTimePreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "360.0"))
    float SunAzimuthAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-90.0", ClampMax = "90.0"))
    float SunAltitudeAngle = 75.0f;
};

// Everything else in FWeatherParameters -- what a "weather condition" preset
// (Clear, Cloudy, Rain...) means to set, independent of time of day.
USTRUCT(BlueprintType)
struct FWeatherConditionPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Cloudiness = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Precipitation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float PrecipitationDeposits = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float WindIntensity = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float FogDensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float FogDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float FogFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Wetness = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float ScatteringIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float MieScatteringScale = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float RayleighScatteringScale = 0.0331f;
};

UCLASS()
class CARLA_API UWeatherJsonUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    // Both catalogs -- every named time preset and every named condition
    // preset -- live together in one file, Config/Weather/Presets.json:
    //   { "time": { "Noon": {...}, "Town10_dusk": {...} },
    //     "conditions": { "Clear": {...}, "Rain": {...} } }
    // A custom preset invented while tuning one map (e.g. "Town10_dusk")
    // lands in the same shared catalog, so it can be picked for any other
    // map's default too -- nothing is namespaced per-map.
    UFUNCTION(BlueprintCallable, Category = "Weather|JSON")
    static bool SaveTimePreset(const FWeatherTimePreset& Preset, const FString& Name);

    UFUNCTION(BlueprintCallable, Category = "Weather|JSON")
    static bool LoadTimePreset(UPARAM(ref) FWeatherTimePreset& OutPreset, const FString& Name);

    UFUNCTION(BlueprintCallable, Category = "Weather|JSON")
    static bool SaveConditionPreset(const FWeatherConditionPreset& Preset, const FString& Name);

    UFUNCTION(BlueprintCallable, Category = "Weather|JSON")
    static bool LoadConditionPreset(UPARAM(ref) FWeatherConditionPreset& OutPreset, const FString& Name);

    // Names of every saved preset in each catalog. Back the "Get Options"
    // dropdowns on ASkyBase::TimePresetName / ConditionPresetName.
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weather|JSON")
    static TArray<FString> GetAvailableTimePresetNames();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weather|JSON")
    static TArray<FString> GetAvailableConditionPresetNames();

    // Applies TimePreset's sun position and ConditionPreset's everything-else
    // on top of Base (so fields neither preset touches -- there aren't any
    // today, but future ones -- keep whatever Base already had).
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weather|JSON")
    static FWeatherParameters MergeWeatherPresets(
        const FWeatherParameters& Base,
        const FWeatherTimePreset& TimePreset,
        const FWeatherConditionPreset& ConditionPreset);

    // Per-map default preset names, read from MapDefaults.json:
    //   { "Town10HD_Opt": { "time": "Noon", "condition": "Clear" } }
    // Returns false if the map has no entry.
    UFUNCTION(BlueprintCallable, Category = "Weather|JSON")
    static bool GetMapDefaultPresetNames(const FString& MapName, FString& OutTimeName, FString& OutConditionName);

    // Writes/overwrites MapName's entry in MapDefaults.json as a reference to
    // two named catalog presets.
    UFUNCTION(BlueprintCallable, Category = "Weather|JSON")
    static bool SetMapDefaultPresetNames(const FString& MapName, const FString& TimeName, const FString& ConditionName);

    // MapName's entry, resolved regardless of which of the two shapes
    // MapDefaults.json holds it in:
    //   { "Town10HD_Opt": { "time": "Noon", "condition": "Clear" } }         -- named presets, merged onto Base
    //   { "Town10HD_Opt": { "sunAzimuthAngle": 180, "cloudiness": 5, ... } } -- a full inline snapshot
    // A map's hand-tuned one-off setup doesn't need a named preset invented
    // just to be reachable as its default -- and doesn't pollute the shared
    // Presets.json catalog other maps pick from. Returns false if the map has
    // no entry, or a named-preset entry names a preset that no longer exists.
    UFUNCTION(BlueprintCallable, Category = "Weather|JSON")
    static bool GetMapDefaultWeather(const FString& MapName, const FWeatherParameters& Base, UPARAM(ref) FWeatherParameters& OutWeather);

    // Writes/overwrites MapName's entry in MapDefaults.json as a full inline
    // snapshot of Weather -- no named preset involved on either side.
    UFUNCTION(BlueprintCallable, Category = "Weather|JSON")
    static bool SetMapDefaultWeather(const FString& MapName, const FWeatherParameters& Weather);

    static FString GetPresetsPath()
    {
        return FPaths::ProjectContentDir() / TEXT("Carla/Config/Weather/Presets.json");
    }

    static FString GetMapDefaultsPath()
    {
        return FPaths::ProjectContentDir() / TEXT("Carla/Config/Weather/MapDefaults.json");
    }
};
