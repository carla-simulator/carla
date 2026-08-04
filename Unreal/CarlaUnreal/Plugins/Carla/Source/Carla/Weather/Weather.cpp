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
        if (UCurveFloat* SkyIntensityCurve = FindCurve(TEXT("SkyIntensity_Curve")))
        {
            if (USkyLightComponent* SkyLightComponent = FindSkyLightComponent(TEXT("SkyLightComponent")))
                SkyLightComponent->SetIntensity(SkyIntensityCurve->GetFloatValue(Weather.SunAltitudeAngle));
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
