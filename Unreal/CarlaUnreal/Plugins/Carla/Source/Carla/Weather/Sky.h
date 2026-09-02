#pragma once

#include <util/ue-header-guard-begin.h>
#include "Engine/Scene.h"
#include "GameFramework/Actor.h"
#include <util/ue-header-guard-end.h>

#include "Sky.generated.h"

class UPostProcessComponent;
class UExponentialHeightFogComponent;
class UDirectionalLightComponent;
class USkyLightComponent;
class UVolumetricCloudComponent;
class USkyAtmosphereComponent;

// Curated shortcuts into PostProcessComponent's Settings -- the handful of
// camera/exposure/color-grading fields an artist actually reaches for per
// shot, without hunting through the full "Post Process Volume" category
// (hundreds of fields). This is NOT a second, parallel post process: editing
// these pushes straight into the same Settings that category also edits, and
// they're kept in sync with whatever PostProcess profile is loaded (see
// ASkyBase::SyncCameraParametersFromPostProcess). For anything not listed
// here, go edit "Post Process Volume" directly -- it's still the same volume.
USTRUCT(BlueprintType)
struct FCameraParameters
{
	GENERATED_BODY()

	// Exposure (FPostProcessSettings::AutoExposureMinBrightness/MaxBrightness,
	// in EV100 -- what Weather's own exposure fallback also fills in for any
	// PostProcess profile that doesn't claim them itself).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure")
	float ExposureMinEV = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure")
	float ExposureMaxEV = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure")
	float HighlightContrast = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure")
	float ShadowContrast = 1.0f;

	// Camera (Lens|Camera in the full Post Process Volume category).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ShutterSpeed = 60.0f;

	// AKA metering/exposure method: AEM_Histogram (auto, scene-average --
	// what every profile here uses) vs AEM_Manual (fixed, actually reads
	// ShutterSpeed/ISO/Aperture below for real camera-triangle exposure).
	// ISO/Aperture do nothing visible under Histogram -- flip this to Manual
	// to make them matter.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TEnumAsByte<EAutoExposureMethod> ExposureMode = AEM_Histogram;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ISO = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float Aperture = 4.0f;

	// Depth of Field (Lens|Depth of Field).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Depth Of Field")
	float SensorWidth = 24.576f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Depth Of Field")
	float FocalDistance = 1000.0f;

	// Color Grading. GlobalSaturation/Contrast/Gamma are single-knob
	// shortcuts onto the full RGB+luma FVector4 fields (ColorSaturation/
	// ColorContrast/ColorGamma) -- they set all three color channels equally
	// and leave the luma (w) channel untouched. Use "Post Process Volume" ->
	// Color Grading|Global directly for per-channel control.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Grading")
	float Temperature = 6500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Grading")
	float Tint = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Grading")
	FLinearColor SceneColorTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Grading")
	float GlobalSaturation = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Grading")
	float GlobalContrast = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Grading")
	float GlobalGamma = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Grading")
	float VignetteIntensity = 0.4f;
};

UCLASS(Abstract, PrioritizeCategories = "CustomParameters PostProcessProfile WeatherPreset")
class CARLA_API ASkyBase :
	public AActor
{
	GENERATED_BODY()

public:

	ASkyBase(const FObjectInitializer& ObjectInitializer);

	// The actor's FWeatherParameters variable (a Blueprint-added variable on
	// BP_Carla_Sky, currently named "WeatherParameters" -- but matched by
	// TYPE, not name, so it survives being renamed again in the Blueprint
	// editor). The single source of truth every lookup here and in
	// AWeather::ApplyWeatherToSkyActor must go through -- don't re-implement
	// a name-based search elsewhere, that's exactly what broke last time.
	static FWeatherParameters* FindWeatherParameters(AActor* SkyActor);

	// Profile to load/save on PostProcessComponent, i.e. the JSON file name
	// (without extension) under Content/Carla/Config/PostProcess/. Pick an
	// existing one from the dropdown or type a new name to create it on the
	// next SaveProfile.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcessProfile",
		meta = (GetOptions = "GetAvailablePostProcessProfileNamesForPicker"))
	FString ProfileName = TEXT("Default");

	// Writes PostProcessComponent's current Settings -- edited live, in the
	// normal "Settings" category of the component, absolute values, no
	// delta/remap -- to Config/PostProcess/<ProfileName>.json. This is
	// exactly what a spawned camera loads at runtime for that profile.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "PostProcessProfile")
	void SaveProfile();

	// Loads Config/PostProcess/<ProfileName>.json into PostProcessComponent's
	// Settings, replacing whatever is currently set. Also refreshes
	// CameraParameters (below) from the newly loaded Settings so those
	// shortcuts never show a stale value, and re-applies Weather's exposure
	// fallback immediately (see RefreshWeather) instead of waiting for some
	// unrelated later property edit to do it.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "PostProcessProfile")
	void LoadProfile();

	// Name for a brand new profile -- free text, not restricted to the
	// dropdown above. CreateProfile sets ProfileName to this and saves.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcessProfile")
	FString NewProfileName;

	// Sets ProfileName = NewProfileName and saves the current Settings under
	// it, creating Config/PostProcess/<NewProfileName>.json. Use this to
	// start a new profile from the current PostProcessComponent state; the
	// dropdown above picks it up on the next click.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "PostProcessProfile")
	void CreateProfile();

	// Curated shortcuts onto PostProcessComponent's Settings -- see
	// FCameraParameters. Kept in sync with whatever profile LoadProfile last
	// loaded; edit here for the common cases, or go to "Post Process Volume"
	// directly for anything not exposed here.
	//
	// Category is "CustomParameters", same as the Blueprint's own "Sky
	// Parameters"/"Weather Parameters" variable -- sharing one category name
	// (set on that BP variable too, from the Blueprint editor) groups both
	// under a single header instead of two.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomParameters")
	FCameraParameters CameraParameters;

	// Time-of-day preset (sun position only) and weather-condition preset
	// (everything else -- cloudiness/rain/wind/fog/wetness/...), both named
	// entries in the shared catalog Config/Weather/Presets.json. Independent
	// axes: pick any time with any condition. Selecting either one applies it
	// immediately (see PostEditChangeProperty) -- no separate Load button.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeatherPreset",
		meta = (GetOptions = "GetAvailableTimePresetNamesForPicker", DisplayName = "Time Preset"))
	FString TimePresetName = TEXT("Noon");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeatherPreset",
		meta = (GetOptions = "GetAvailableConditionPresetNamesForPicker", DisplayName = "Condition Preset"))
	FString ConditionPresetName = TEXT("Clear");

	// CallInEditor buttons in one category are always shown together, above
	// the category's properties, sorted alphabetically by function name --
	// not declaration order, and Unreal doesn't offer a way to interleave
	// them with the dropdowns/textboxes below. These are named B_/C_/D_...
	// (no A_: that was LoadPreset, removed -- see PostEditChangeProperty) so
	// that alphabetical order matches the intended reading order (the two
	// Saves, then the two MapDefault buttons, then the two Creates, then
	// LoadMapDefault); DisplayName keeps each button's own label clean.

	// Saves the map's current sun position (AWeather::GetCurrentWeather) to
	// the "time" catalog under TimePresetName.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WeatherPreset", meta = (DisplayName = "SaveTimePreset"))
	void B_SaveTimePreset();

	// Saves everything else in the map's current weather to the "conditions"
	// catalog under ConditionPresetName.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WeatherPreset", meta = (DisplayName = "SaveConditionPreset"))
	void C_SaveConditionPreset();

	// Saves this actor's current "Sky Parameters" as this map's default
	// directly -- a full inline snapshot in MapDefaults.json, not a reference
	// to two named Presets.json entries. Use this for a one-off hand-tuned
	// setup that doesn't need to exist as a reusable Time/Condition preset
	// other maps could also pick.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WeatherPreset", meta = (DisplayName = "SaveCurrentWeatherAsMapDefault"))
	void D_SaveCurrentWeatherAsMapDefault();

	// Saves TimePresetName/ConditionPresetName as this map's default pair in
	// MapDefaults.json -- what ACarlaGameModeBase::BeginPlay loads for this
	// map from now on instead of the flat carla::rpc::WeatherParameters::Default.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WeatherPreset", meta = (DisplayName = "SetAsMapDefault"))
	void E_SetAsMapDefault();

	// Free-text name for a brand new time preset. CreateTimePreset sets
	// TimePresetName to this and saves -- e.g. a bespoke "Town10_dusk" you
	// can later pick as another map's default too, same shared catalog.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeatherPreset")
	FString NewTimePresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeatherPreset")
	FString NewConditionPresetName;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WeatherPreset", meta = (DisplayName = "CreateTimePreset"))
	void F_CreateTimePreset();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WeatherPreset", meta = (DisplayName = "CreateConditionPreset"))
	void G_CreateConditionPreset();

	// Loads and applies this map's current default weather from
	// MapDefaults.json -- whichever shape it's in, a named Time/Condition
	// preset pair or a hand-tuned inline snapshot (see
	// UWeatherJsonUtils::GetMapDefaultWeather). There was no way to get back
	// to a map's custom default once you'd hand-edited Sky Parameters away
	// from it, short of re-entering the values by hand; this just re-applies
	// whatever SetAsMapDefault/SaveCurrentWeatherAsMapDefault last saved.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WeatherPreset", meta = (DisplayName = "LoadMapDefault"))
	void H_LoadMapDefault();

protected:

	// Editing ANY property on a placed Blueprint actor in the editor Details
	// panel reruns its Construction Script, and BP_Carla_Sky's own
	// construction resets components like VolumetricCloudComponent's material
	// back to their authored/instance default -- undoing whatever
	// RefreshWeather/LoadPreset just pushed the moment you touch an unrelated
	// field. OnConstruction runs right after every such rerun (and once at
	// spawn/Play), so re-push "Sky Parameters" here to make that self-healing:
	// any edit still ends with the rig showing the current weather.
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	// Applies TimePresetName/ConditionPresetName as soon as either dropdown
	// is changed -- OnConstruction can't do this itself (it fires on every
	// property edit with no way to tell which one changed; re-applying the
	// selected presets on every WeatherParameters/CameraParameters edit too
	// would stomp hand-tuning right back to the preset every keystroke).
	// Replaces the old separate LoadPreset button.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Loads TimePresetName + ConditionPresetName and applies the merged
	// result through the map's AWeather actor (same pipeline the API/client
	// uses), so the sky rig actually refreshes -- not just this actor's own
	// WeatherParameters variable, which the rig only reads from when told to.
	void ApplyTimeAndConditionPresets();

	// Writes CameraParameters into PostProcessComponent->Settings (claiming
	// each field's bOverride_*), and the reverse -- reads Settings back into
	// CameraParameters. See FCameraParameters and LoadProfile.
	void PushCameraParameters();
	void SyncCameraParametersFromPostProcess();

	// Re-applies this actor's own "Sky Parameters" as they currently stand,
	// through AWeather. Internal helper -- LoadProfile calls this to
	// re-clamp exposure right after loading; OnConstruction does the
	// equivalent push directly. Not exposed as a button: it was redundant
	// there (OnConstruction already runs it on every edit), replaced by the
	// more useful LoadMapDefault.
	void RefreshWeather();

	// GetOptions callbacks need a UFUNCTION taking no world-context args;
	// thin wrappers around the shared JSON utils lookups.
	UFUNCTION()
	static TArray<FString> GetAvailablePostProcessProfileNamesForPicker();

	UFUNCTION()
	static TArray<FString> GetAvailableTimePresetNamesForPicker();

	UFUNCTION()
	static TArray<FString> GetAvailableConditionPresetNamesForPicker();


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UPostProcessComponent* PostProcessComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UExponentialHeightFogComponent* ExponentialHeightFogComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UDirectionalLightComponent* DirectionalLightComponentSun;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UDirectionalLightComponent* DirectionalLightComponentMoon;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkyLightComponent* SkyLightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UVolumetricCloudComponent* VolumetricCloudComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkyAtmosphereComponent* SkyAtmosphereComponent;


};
