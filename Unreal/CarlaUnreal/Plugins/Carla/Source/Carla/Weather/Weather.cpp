// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Weather/Weather.h"
#include "Carla.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Lights/CarlaLightSubsystem.h"
#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Recorder/CarlaRecorderWeather.h"
#include "Carla/Sensor/SceneCaptureCamera.h"
#include "Carla/Weather/Sky.h"

#include <util/ue-header-guard-begin.h>
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkyLightComponent.h"
#include "Curves/CurveFloat.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"
#include <util/ue-header-guard-end.h>

// This project runs a FIXED day-calibrated exposure
// (r.DefaultFeature.AutoExposure=False); CheckWeatherPostProcessEffects below
// clamps the sky rig's post process to histogram EV100 [10,12] bias 0 for
// sensor determinism. A near-black night scene always meters to the bright
// end of that clamp (EV100=10), where the exposure multiplier is
// 1/(K*2^EV100) with UE's EV100->luminance calibration constant K=1.2, i.e.
// ~1/(1.2*1024) = 0.000814. UDirectionalLightComponent::Intensity is lux, so
// asphalt (~7% albedo, surface radiance = illuminance*albedo/pi) needs about
// this much total ambient illuminance to land at a dim-but-visible sRGB pixel
// value: target pixel 10-25/255 -> linear ~0.0008-0.006 (undoing ~2.2 gamma)
// -> scene radiance ~1-7 cd/m^2 (dividing by the 0.000814 exposure multiplier)
// -> illuminance = radiance*pi/albedo ~= 45-315 lux, midpoint ~130 lux. Split
// ~100 lux on the moon directional (the dominant single-direction term) and
// the rest as ambient fill: a uniform sky of luminance L produces horizontal
// illuminance E = pi*L, so a 30 lux ambient contribution needs L ~= 10, hence
// the skylight floor below.
static TAutoConsoleVariable<float> CVarCarlaWeatherMoonIntensity(
    TEXT("carla.Weather.MoonIntensity"),
    100.0f,
    TEXT("Minimum DirectionalLightComponentMoon intensity (lux) enforced on the sky rig ")
    TEXT("whenever SunAltitudeAngle < 0. Set 0 to leave the rig's authored/curve-driven ")
    TEXT("moon intensity untouched."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherNightSkylightIntensity(
    TEXT("carla.Weather.NightSkylightIntensity"),
    10.0f,
    TEXT("Minimum SkyLightComponent intensity enforced on the sky rig whenever ")
    TEXT("SunAltitudeAngle < 0 (same units as the rig's SkyIntensity_Curve). Set 0 to ")
    TEXT("leave the rig's curve-driven skylight intensity untouched."),
    ECVF_Default);

// The night sky is pure black on every map (verified content-side, not a
// generated-map defect): BP_Carla_Sky's SetSkySphere respawns a child actor
// of the stock engine /Engine/EngineSky/BP_Sky_Sphere on every weather push
// and stores it in its own "SkySphere" object variable (see PushWeatherToSky
// below). That sphere's material, M_Sky_Panning_Clouds2, already ships star
// rendering -- confirmed by dumping strings from the .uasset directly: a
// MaterialExpressionScalarParameter named "Stars brightness" and a
// TextureSample referencing /Engine/EngineSky/T_Sky_Stars. The sphere
// blueprint exposes it through two of its own variables (also confirmed via
// strings, including their tooltips baked into the asset): "Colors
// Determined By Sun Position" (bool -- "If enabled, sky colors will change
// according to the sun's position"; when off, the sphere's "Sun Height"
// value -- whose own tooltip is "If no directional light is assigned, this
// value determines the height of the sun" -- stays frozen at its authored
// fallback instead of tracking the Directional Light Actor BP_Carla_Sky
// already assigns every push, which would permanently suppress the
// sun-height-gated star blend regardless of the actual time of day) and
// "Stars Brightness" (float -- "Multiplier for the brightness of the stars
// when the sun is below the horizon"). CARLA's own BP_Carla_Sky/BP_
// CarlaWeather carry a dead "Stars Intensity Over Sun Altitude" comment/graph
// fragment (same severed-exec-chain pattern documented throughout this file)
// but, unlike SunIntensity_Curve/SkyIntensity_Curve, there is no backing
// UCurveFloat asset anywhere in the content for it -- confirmed by searching
// the plugin Content tree -- so there is nothing to re-wire; drive the two
// stock engine variables directly instead, the same way this file already
// routes around every other severed exec chain in the rig.
// Both are held gated to SunAltitudeAngle < 0, deliberately not applied
// during the day even though a fresh sphere actor makes it safe either way
// (SetSkySphere respawns it every push, so nothing here can leak across
// pushes): "Colors Determined By Sun Position" may also drive the sphere's
// OWN internal Horizon/Zenith/Cloud color curves, which would compete with
// CARLA's curve-driven day colors already pushed by UpdateSkySphereColor
// earlier in the same push -- unverifiable without an editor, so day is left
// completely untouched to avoid any color-grading regression risk.
// StarsBrightness is a material-scalar knob, not a physical unit. The
// white-texel estimate (~100, from the EV100=10 exposure multiplier above)
// rendered stars at max pixel ~7: the star texture's lit texels sample far
// below white. 5000 was calibrated visually on a generated Town03 world at
// sun -30 -- a dense readable star field (sky-crop max ~145) with no bloom
// halos or day-side effect (the whole block is gated to SunAltitudeAngle<0).
static TAutoConsoleVariable<float> CVarCarlaWeatherStarsBrightness(
    TEXT("carla.Weather.StarsBrightness"),
    5000.0f,
    TEXT("Value forced into the night sky sphere's \"Stars Brightness\" variable ")
    TEXT("(BP_Sky_Sphere, engine content) whenever SunAltitudeAngle < 0, floored the same ")
    TEXT("way as the moon/skylight intensities above. Set 0 to leave the rig's authored ")
    TEXT("value untouched."),
    ECVF_Default);

AWeather::AWeather(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrecipitationPostProcessMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(
        TEXT("Material'/Game/Carla/Static/GenericMaterials/FX/ScreenDust/M_screenDrops.M_screenDrops'")).Object;

    DustStormPostProcessMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(
        TEXT("Material'/Game/Carla/Static/GenericMaterials/FX/ScreenDust/M_screenDust_wind.M_screenDust_wind'")).Object;

    PrimaryActorTick.bCanEverTick = false;
    RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void AWeather::CheckWeatherPostProcessEffects()
{
    if (Weather.Precipitation > 0.0f)
        ActiveBlendables.Add(MakeTuple(PrecipitationPostProcessMaterial, Weather.Precipitation / 100.0f));
    else
        ActiveBlendables.Remove(PrecipitationPostProcessMaterial);

    if (Weather.DustStorm > 0.0f)
        ActiveBlendables.Add(MakeTuple(DustStormPostProcessMaterial, Weather.DustStorm / 100.0f));
    else
        ActiveBlendables.Remove(DustStormPostProcessMaterial);

    TArray<AActor*> SensorActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASceneCaptureCamera::StaticClass(), SensorActors);
    for (AActor* SensorActor : SensorActors)
    {
        ASceneCaptureCamera* Sensor = Cast<ASceneCaptureCamera>(SensorActor);
        for (auto& ActiveBlendable : ActiveBlendables)
            Sensor->GetCaptureComponent2D()->PostProcessSettings.AddBlendable(ActiveBlendable.Key, ActiveBlendable.Value);
    }
}

void AWeather::PushWeatherToSky()
{
    TArray<AActor*> SkyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyBase::StaticClass(), SkyActors);
    for (AActor* SkyActor : SkyActors)
    {
        // The rig stores the parameters it renders from in a blueprint variable
        // and refreshes every component from its blueprint Update function.
        FProperty* Property = SkyActor->GetClass()->FindPropertyByName(TEXT("Sky Parameters"));
        if (Property == nullptr)
            Property = SkyActor->GetClass()->FindPropertyByName(TEXT("SkyParameters"));
        FStructProperty* StructProperty = CastField<FStructProperty>(Property);
        if (StructProperty != nullptr && StructProperty->Struct == FWeatherParameters::StaticStruct())
            StructProperty->SetValue_InContainer(SkyActor, &Weather);
        else
            UE_LOG(LogCarla, Warning, TEXT("AWeather: no WeatherParameters 'Sky Parameters' property on %s"),
                *SkyActor->GetName());

        // The rig's blueprint 'Update' only reaches UpdateClouds (the rest of
        // its exec chain was never wired in the ue5-dev content rework), so
        // run the individual refresh functions in dependency order instead.
        // NOTE: the blueprint also has an UpdateNight function, deliberately
        // NOT in this list: it multiplies the sky sphere material's current
        // "Horizon color" by ~0.004 in place, i.e. it was authored to run
        // exactly once at a day->night transition. Running it on every
        // weather push collapses the sphere to pure black (no stars, no
        // horizon glow) after a single call. The sphere is respawned fresh
        // by SetSkySphere each push, and UpdateSkySphereColor plus the
        // sphere's own RefreshMaterial already produce the day/night look
        // from absolute curve values.
        static const TCHAR* UpdateFunctionNames[] = {
            TEXT("SetSunActorReference"),
            TEXT("SetSkySphere"),
            TEXT("UpdateSun"),
            TEXT("UpdateClouds"),
            TEXT("UpdateFog"),
            TEXT("UpdateMoon"),
            TEXT("UpdateAtmosphereAndPrecipitation"),
            TEXT("UpdateSkySphere"),
            TEXT("UpdateSkySphereColor")};
        for (const TCHAR* FunctionName : UpdateFunctionNames)
        {
            UFunction* Function = SkyActor->FindFunction(FunctionName);
            if (Function != nullptr && Function->ParmsSize == 0)
                SkyActor->ProcessEvent(Function, nullptr);
            else
                UE_LOG(LogCarla, Warning, TEXT("AWeather: skipping '%s' on %s (missing or has parameters)"),
                    FunctionName, *SkyActor->GetName());
        }

        // The blueprint's UpdateFog runs, but the values it leaves on the
        // height fog component were saved for the old small towns and break
        // on large maps: a 1 km FogCutoffDistance erases fog on everything
        // beyond it (distant buildings render crisp while the mid ground is
        // fogged), the CARLA FogDistance parameter (meters) is applied raw as
        // centimeters so fog starts at the camera, and FogDensity/100 is 5x
        // Unreal's already-hazy default. Re-map the parameters here with the
        // content left untouched: meters to centimeters for the start
        // distance, no hard cutoff, and a density scale whose extremes keep
        // CARLA semantics (0 = clear, 100 = dense fog).
        FObjectProperty* FogProperty = CastField<FObjectProperty>(
            SkyActor->GetClass()->FindPropertyByName(TEXT("ExponentialHeightFogComponent")));
        UExponentialHeightFogComponent* FogComponent = FogProperty != nullptr
            ? Cast<UExponentialHeightFogComponent>(FogProperty->GetObjectPropertyValue_InContainer(SkyActor))
            : nullptr;
        if (FogComponent != nullptr)
        {
            FogComponent->SetFogDensity(Weather.FogDensity * 0.001f);
            FogComponent->SetStartDistance(Weather.FogDistance * 100.0f);
            FogComponent->SetFogCutoffDistance(0.0f);
        }

        // The blueprint's ControlSunIntensity has a severed exec chain (its
        // entry sets SunTrayectory and dead-ends), so the curve-driven light
        // intensities never apply. Evaluate the curves bound on this weather
        // blueprint and drive the rig components directly; the curve assets
        // remain the content-side tuning source.
        auto FindCurve = [this](const TCHAR* PropertyName) -> UCurveFloat*
        {
            FObjectProperty* CurveProperty =
                CastField<FObjectProperty>(GetClass()->FindPropertyByName(PropertyName));
            return CurveProperty != nullptr
                ? Cast<UCurveFloat>(CurveProperty->GetObjectPropertyValue_InContainer(this))
                : nullptr;
        };
        auto FindComponent = [SkyActor](const TCHAR* PropertyName) -> ULightComponent*
        {
            FObjectProperty* ComponentProperty =
                CastField<FObjectProperty>(SkyActor->GetClass()->FindPropertyByName(PropertyName));
            return ComponentProperty != nullptr
                ? Cast<ULightComponent>(ComponentProperty->GetObjectPropertyValue_InContainer(SkyActor))
                : nullptr;
        };
        // USkyLightComponent is a sibling of ULightComponent (both derive from
        // ULightComponentBase, not from each other), and SetIntensity is
        // declared separately on each subclass -- it needs its own,
        // correctly-typed finder. The old FindComponent-based lookup below
        // always Cast<ULightComponent>'d the sky light to nullptr, so
        // SkyIntensity_Curve silently never reached the component; fixed here.
        auto FindSkyLightComponent = [SkyActor](const TCHAR* PropertyName) -> USkyLightComponent*
        {
            FObjectProperty* ComponentProperty =
                CastField<FObjectProperty>(SkyActor->GetClass()->FindPropertyByName(PropertyName));
            return ComponentProperty != nullptr
                ? Cast<USkyLightComponent>(ComponentProperty->GetObjectPropertyValue_InContainer(SkyActor))
                : nullptr;
        };

        if (UCurveFloat* SunIntensityCurve = FindCurve(TEXT("SunIntensity_Curve")))
        {
            if (ULightComponent* SunLightComponent = FindComponent(TEXT("DirectionalLightComponentSun")))
            {
                SunLightComponent->SetIntensity(SunIntensityCurve->GetFloatValue(Weather.SunAltitudeAngle));
                // Rigs saved with a black light color render no sunlight at any
                // intensity; the physical tint comes from the color temperature.
                SunLightComponent->SetLightColor(FLinearColor::White);
            }
        }
        // The rig ships EVERY component with bAutoActivate false on the
        // instance (the same UE4-era authoring defect documented for the moon
        // below and for street lamps in CarlaLight.cpp), and until now only
        // the night branch below ever re-activated the skylight -- so by day
        // the skylight was simply off and shadows received zero ambient light
        // against the physical 100k lux sun from SunIntensity_Curve: every
        // shaded surface rendered pitch black.
        //
        // Activating the skylight alone is not enough. Its authored source is
        // black (SLS_SpecifiedCubemap with a null cubemap) and the
        // bRealTimeCapture flag that overrides that source renders ONLY sky
        // components -- SkyAtmosphere, VolumetricCloud, IsSky-flagged meshes
        // -- into the capture. The sky CARLA actually displays is the stock
        // engine BP_Sky_Sphere mesh (not IsSky-flagged, invisible to the
        // capture), and the rig's own SkyAtmosphereComponent is inactive like
        // everything else, so the real-time capture rendered an empty scene:
        // a black cubemap, zero ambient at any intensity (verified: black
        // shadows with GI disabled entirely, with the skylight active).
        //
        // So: activate the SkyAtmosphere for the capture to see. In the main
        // view it stays hidden behind the opaque legacy sphere, so the
        // rendered sky look does not change; the capture ignores the sphere
        // and sees the atmosphere, yielding a physically-scaled ambient in
        // the same photometric units as the sun that tracks sun altitude for
        // free. The VolumetricCloudComponent is deliberately left inactive:
        // the sphere already paints clouds, and volumetric ones would
        // composite in front of it as a second cloud layer.
        {
            FObjectProperty* AtmosphereProperty = CastField<FObjectProperty>(
                SkyActor->GetClass()->FindPropertyByName(TEXT("SkyAtmosphereComponent")));
            UActorComponent* AtmosphereComponent = AtmosphereProperty != nullptr
                ? Cast<UActorComponent>(AtmosphereProperty->GetObjectPropertyValue_InContainer(SkyActor))
                : nullptr;
            if (AtmosphereComponent != nullptr && !AtmosphereComponent->IsActive())
                AtmosphereComponent->SetActive(true);
        }
        // Skylight: force active + real-time capture (the latter is already
        // set on the asset; enforced defensively in case a rig resave clears
        // it -- supported on Stationary mobility and on by default via
        // r.SkyLight.RealTimeReflectionCapture). SkyIntensity_Curve (1.0 by
        // day, 0 at night) remains the multiplier on the capture.
        if (USkyLightComponent* SkyLightComponent = FindSkyLightComponent(TEXT("SkyLightComponent")))
        {
            if (!SkyLightComponent->bRealTimeCapture)
                SkyLightComponent->SetRealTimeCaptureEnabled(true);
            if (!SkyLightComponent->IsActive())
                SkyLightComponent->SetActive(true);
            if (UCurveFloat* SkyIntensityCurve = FindCurve(TEXT("SkyIntensity_Curve")))
                SkyLightComponent->SetIntensity(SkyIntensityCurve->GetFloatValue(Weather.SunAltitudeAngle));
            UE_LOG(LogCarla, Verbose, TEXT(
                "AWeather sky light: active=%d intensity=%.3f realtimecapture=%d mobility=%d visible=%d"),
                SkyLightComponent->IsActive() ? 1 : 0,
                SkyLightComponent->Intensity,
                SkyLightComponent->IsRealTimeCaptureEnabled() ? 1 : 0,
                int(SkyLightComponent->Mobility.GetValue()),
                SkyLightComponent->IsVisible() ? 1 : 0);
        }
        else
        {
            UE_LOG(LogCarla, Warning, TEXT("AWeather: no SkyLightComponent found on %s"), *SkyActor->GetName());
        }

        // Night ambient floor. BP_Carla_Sky's DirectionalLightComponentMoon
        // contributes nothing visible once the sun sets: either it carries
        // the same UE4-era authoring defect CarlaLight.cpp documents and
        // fixes for street lamps (bAutoActivate off on the component
        // instance, silently culled by UE5.8 instead of just ignored like
        // UE4), or its authored/curve-driven intensity is a physically-tiny
        // real-moonlight lux value that is invisible under this project's
        // fixed daylight-calibrated exposure -- or both. Address both
        // defensively: force the moon and sky light active, then clamp their
        // intensity up to a floor (never down, so this can never darken
        // whatever the curves already produced) whenever the sun is below
        // the horizon. See the cvar comments above for the exposure math
        // behind the default floor values.
        if (Weather.SunAltitudeAngle < 0.0f)
        {
            const float MoonFloor = CVarCarlaWeatherMoonIntensity.GetValueOnGameThread();
            if (ULightComponent* MoonLightComponent = FindComponent(TEXT("DirectionalLightComponentMoon")))
            {
                if (!MoonLightComponent->IsActive())
                    MoonLightComponent->SetActive(true);
                if (MoonFloor > 0.0f && MoonLightComponent->Intensity < MoonFloor)
                    MoonLightComponent->SetIntensity(MoonFloor);
            }

            const float SkylightFloor = CVarCarlaWeatherNightSkylightIntensity.GetValueOnGameThread();
            if (USkyLightComponent* SkyLightComponent = FindSkyLightComponent(TEXT("SkyLightComponent")))
            {
                if (!SkyLightComponent->IsActive())
                    SkyLightComponent->SetActive(true);
                if (SkylightFloor > 0.0f && SkyLightComponent->Intensity < SkylightFloor)
                    SkyLightComponent->SetIntensity(SkylightFloor);
            }

            // Stars. SetSkySphere (in the UpdateFunctionNames loop above) has
            // already respawned this push's sphere actor by the time we get
            // here, so "SkySphere" always resolves to the fresh instance. See
            // the cvar comment above for the full reflection path and why
            // this stays gated to night.
            // Blueprint member variables get decorated FNames in their
            // generated class (exact FindPropertyByName misses them), so
            // match on the authored name instead -- same lesson as the
            // vehicle "Beam Lights" variable in CarlaWheeledVehicle.cpp.
            const auto FindPropertyByAuthoredName =
                [](const UClass* Class, const TCHAR* AuthoredName) -> FProperty*
            {
                for (TFieldIterator<FProperty> It(Class); It; ++It)
                {
                    if ((*It)->GetAuthoredName() == AuthoredName || (*It)->GetName() == AuthoredName)
                        return *It;
                }
                return nullptr;
            };

            FObjectProperty* SphereProperty = CastField<FObjectProperty>(
                FindPropertyByAuthoredName(SkyActor->GetClass(), TEXT("SkySphere")));
            if (SphereProperty == nullptr)
                SphereProperty = CastField<FObjectProperty>(
                    FindPropertyByAuthoredName(SkyActor->GetClass(), TEXT("Sky Sphere")));
            AActor* SphereActor = SphereProperty != nullptr
                ? Cast<AActor>(SphereProperty->GetObjectPropertyValue_InContainer(SkyActor))
                : nullptr;
            UE_LOG(LogCarla, Verbose, TEXT("AWeather night sky: sphere property %s, actor %s"),
                SphereProperty ? TEXT("found") : TEXT("MISSING"),
                SphereActor ? *SphereActor->GetName() : TEXT("null"));
            if (SphereActor != nullptr)
            {
                if (FBoolProperty* ColorsBySunProperty = CastField<FBoolProperty>(
                        FindPropertyByAuthoredName(SphereActor->GetClass(), TEXT("Colors Determined By Sun Position"))))
                    ColorsBySunProperty->SetPropertyValue_InContainer(SphereActor, true);
                else
                    UE_LOG(LogCarla, Verbose, TEXT("AWeather night sky: 'Colors Determined By Sun Position' MISSING on %s"),
                        *SphereActor->GetClass()->GetName());

                // UE5 blueprints store float variables as doubles, so match
                // any numeric property rather than FFloatProperty.
                const float StarsFloor = CVarCarlaWeatherStarsBrightness.GetValueOnGameThread();
                if (FNumericProperty* StarsBrightnessProperty = CastField<FNumericProperty>(
                        FindPropertyByAuthoredName(SphereActor->GetClass(), TEXT("Stars Brightness"))))
                {
                    void* ValuePtr = StarsBrightnessProperty->ContainerPtrToValuePtr<void>(SphereActor);
                    const double CurrentStarsBrightness =
                        StarsBrightnessProperty->GetFloatingPointPropertyValue(ValuePtr);
                    if (StarsFloor > 0.0f && CurrentStarsBrightness < StarsFloor)
                        StarsBrightnessProperty->SetFloatingPointPropertyValue(ValuePtr, StarsFloor);
                }
                else
                    UE_LOG(LogCarla, Verbose, TEXT("AWeather night sky: 'Stars Brightness' MISSING on %s"),
                        *SphereActor->GetClass()->GetName());

                // Push the variables above into the sphere's dynamic material
                // instance. UpdateSkySphere/UpdateSkySphereColor may already
                // do this as part of their own (working, since day renders
                // correctly) exec chains; calling it again here is a cheap,
                // idempotent resync so our two variable writes are guaranteed
                // to reach the material regardless.
                UFunction* RefreshMaterialFunction = SphereActor->FindFunction(TEXT("RefreshMaterial"));
                if (RefreshMaterialFunction != nullptr && RefreshMaterialFunction->ParmsSize == 0)
                    SphereActor->ProcessEvent(RefreshMaterialFunction, nullptr);
            }
        }
    }

    // BP_GeneralSceneSettings owns the post-process volume that keeps the
    // exposure in step with the sun (its Update was another dead call site in
    // BP_CarlaWeather). Only invoke it when a sky rig exists: its blueprint
    // retries itself endlessly when it cannot find an ASkyBase.
    if (SkyActors.Num() > 0)
    {
        UClass* SceneSettingsClass = LoadClass<AActor>(nullptr,
            TEXT("/Game/Carla/Blueprints/BP_GeneralSceneSettings.BP_GeneralSceneSettings_C"));
        AActor* SceneSettingsActor = SceneSettingsClass != nullptr
            ? UGameplayStatics::GetActorOfClass(GetWorld(), SceneSettingsClass)
            : nullptr;
        UFunction* SettingsUpdateFunction = SceneSettingsActor != nullptr
            ? SceneSettingsActor->FindFunction(TEXT("Update"))
            : nullptr;
        if (SettingsUpdateFunction != nullptr && SettingsUpdateFunction->ParmsSize == 0)
            SceneSettingsActor->ProcessEvent(SettingsUpdateFunction, nullptr);
    }

    // Viewport exposure. The project ships with auto exposure disabled by
    // default (r.DefaultFeature.AutoExposure=False) and the rigs were saved
    // for the old frozen ~15 lux suns, so a physical 100k lux sun white-outs
    // the main view. Apply the same histogram exposure the RGB sensor uses by
    // default (bias 0, EV100 range [10,12], speeds 3/1) to the sky rig's
    // post-process volume. This must run AFTER BP_GeneralSceneSettings.Update,
    // which rewrites the whole post-process struct with its own values.
    for (AActor* SkyActor : SkyActors)
    {
        FObjectProperty* PostProcessProperty = CastField<FObjectProperty>(
            SkyActor->GetClass()->FindPropertyByName(TEXT("PostProcessComponent")));
        UPostProcessComponent* PostProcessComponent = PostProcessProperty != nullptr
            ? Cast<UPostProcessComponent>(PostProcessProperty->GetObjectPropertyValue_InContainer(SkyActor))
            : nullptr;
        if (PostProcessComponent != nullptr)
        {
            FPostProcessSettings& Settings = PostProcessComponent->Settings;
            Settings.bOverride_AutoExposureMethod = true;
            Settings.AutoExposureMethod = AEM_Histogram;
            Settings.bOverride_AutoExposureBias = true;
            Settings.AutoExposureBias = 0.0f;
            Settings.bOverride_AutoExposureMinBrightness = true;
            Settings.AutoExposureMinBrightness = 10.0f;
            Settings.bOverride_AutoExposureMaxBrightness = true;
            Settings.AutoExposureMaxBrightness = 12.0f;
            Settings.bOverride_AutoExposureSpeedUp = true;
            Settings.AutoExposureSpeedUp = 3.0f;
            Settings.bOverride_AutoExposureSpeedDown = true;
            Settings.AutoExposureSpeedDown = 1.0f;
            PostProcessComponent->bUnbound = true;
        }
    }
}

void AWeather::UpdateStreetLightsForDayNight()
{
    if (!DayNightCycle)
        return;

    UCarlaLightSubsystem* CarlaLightSubsystem = GetWorld()->GetSubsystem<UCarlaLightSubsystem>();
    if (CarlaLightSubsystem == nullptr)
        return;

    // Sun above the horizon = day, at or below = night. Broadcasting on every
    // weather update is harmless: registered lights just re-receive the same
    // state when nothing changed.
    const bool bIsDay = Weather.SunAltitudeAngle > 0.0f;
    UE_LOG(LogCarla, Log, TEXT("AWeather: broadcasting day/night change (bIsDay=%d) to %d registered CarlaLights"),
        bIsDay ? 1 : 0, CarlaLightSubsystem->NumLights());
    CarlaLightSubsystem->NotifyDayTimeChange(bIsDay);
}

void AWeather::ApplyWeather(const FWeatherParameters& InWeather)
{
    SetWeather(InWeather);
    CheckWeatherPostProcessEffects();

#ifdef CARLA_WEATHER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("Changing weather:"));
    UE_LOG(LogCarla, Log, TEXT("  - Cloudiness = %.2f"), Weather.Cloudiness);
    UE_LOG(LogCarla, Log, TEXT("  - Precipitation = %.2f"), Weather.Precipitation);
    UE_LOG(LogCarla, Log, TEXT("  - PrecipitationDeposits = %.2f"), Weather.PrecipitationDeposits);
    UE_LOG(LogCarla, Log, TEXT("  - WindIntensity = %.2f"), Weather.WindIntensity);
    UE_LOG(LogCarla, Log, TEXT("  - SunAzimuthAngle = %.2f"), Weather.SunAzimuthAngle);
    UE_LOG(LogCarla, Log, TEXT("  - SunAltitudeAngle = %.2f"), Weather.SunAltitudeAngle);
    UE_LOG(LogCarla, Log, TEXT("  - FogDensity = %.2f"), Weather.FogDensity);
    UE_LOG(LogCarla, Log, TEXT("  - FogDistance = %.2f"), Weather.FogDistance);
    UE_LOG(LogCarla, Log, TEXT("  - FogFalloff = %.2f"), Weather.FogFalloff);
    UE_LOG(LogCarla, Log, TEXT("  - Wetness = %.2f"), Weather.Wetness);
    UE_LOG(LogCarla, Log, TEXT("  - ScatteringIntensity = %.2f"), Weather.ScatteringIntensity);
    UE_LOG(LogCarla, Log, TEXT("  - MieScatteringScale = %.2f"), Weather.MieScatteringScale);
    UE_LOG(LogCarla, Log, TEXT("  - RayleighScatteringScale = %.2f"), Weather.RayleighScatteringScale);
    UE_LOG(LogCarla, Log, TEXT("  - DustStorm = %.2f"), Weather.DustStorm);
#endif // CARLA_WEATHER_EXTRA_LOG

    // Call the blueprint that actually changes the weather.
    RefreshWeather(Weather);
    PushWeatherToSky();
    UpdateStreetLightsForDayNight();

    // record the weather event
    ACarlaRecorder *Recorder = UCarlaStatics::GetRecorder(GetWorld());
    if (Recorder && Recorder->IsEnabled())
    {
        CarlaRecorderWeather RecorderWeather;
        RecorderWeather.Cloudiness              = InWeather.Cloudiness;
        RecorderWeather.Precipitation           = InWeather.Precipitation;
        RecorderWeather.PrecipitationDeposits   = InWeather.PrecipitationDeposits;
        RecorderWeather.WindIntensity           = InWeather.WindIntensity;
        RecorderWeather.SunAzimuthAngle         = InWeather.SunAzimuthAngle;
        RecorderWeather.SunAltitudeAngle        = InWeather.SunAltitudeAngle;
        RecorderWeather.FogDensity              = InWeather.FogDensity;
        RecorderWeather.FogDistance             = InWeather.FogDistance;
        RecorderWeather.FogFalloff              = InWeather.FogFalloff;
        RecorderWeather.Wetness                 = InWeather.Wetness;
        RecorderWeather.ScatteringIntensity     = InWeather.ScatteringIntensity;
        RecorderWeather.MieScatteringScale      = InWeather.MieScatteringScale;
        RecorderWeather.RayleighScatteringScale = InWeather.RayleighScatteringScale;
        RecorderWeather.DustStorm               = InWeather.DustStorm;
        Recorder->AddWeather(RecorderWeather);
    }
}

void AWeather::NotifyWeather(ASensor* Sensor)
{
    CheckWeatherPostProcessEffects();

    // Call the blueprint that actually changes the weather.
    RefreshWeather(Weather);
    PushWeatherToSky();
    UpdateStreetLightsForDayNight();
}

void AWeather::SetWeather(const FWeatherParameters& InWeather)
{
    Weather = InWeather;
}

void AWeather::SetDayNightCycle(const bool& active)
{
    DayNightCycle = active;
}

#if WITH_EDITOR
void AWeather::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    ApplyWeather(Weather);
}
#endif
