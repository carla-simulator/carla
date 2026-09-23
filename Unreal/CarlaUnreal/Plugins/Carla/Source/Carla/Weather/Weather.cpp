// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Weather/Weather.h"
#include "Carla.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Lights/CarlaLightSubsystem.h"
#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Recorder/CarlaRecorderWeather.h"
#include "Carla/Sensor/SceneCaptureCamera.h"
#include "Carla/Weather/Sky.h"

#include <util/ue-header-guard-begin.h>
#include "Camera/PlayerCameraManager.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "EngineUtils.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/MeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/LightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Curves/CurveFloat.h"
#include "Engine/TextureCube.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
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
    500.0f,
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
    0.0f,
    TEXT("Value forced into the night sky sphere's \"Stars Brightness\" variable ")
    TEXT("(BP_Sky_Sphere, engine content) whenever SunAltitudeAngle < 0, floored the same ")
    TEXT("way as the moon/skylight intensities above. Set 0 (the default) to leave the rig's ")
    TEXT("authored/artist-edited value untouched -- this used to default to 5000, which stomped ")
    TEXT("any hand-tuned \"Stars brightness\" value on the sky sphere BP back up to 5000 on ")
    TEXT("every push, making it look uneditable."),
    ECVF_Default);

// Reverse-engineered from BP_GeneralSceneSettings.UpdateClouds (before it was
// ported here natively): below this Cloudiness value the rig's
// VolumetricCloudComponent gets a fresh MID off the plain MI_Clouds master
// (no density override -- the sphere's 2D texture is doing the actual cloud
// look at low cloudiness); at or above it, a fresh MID off the "Billowy"
// overcast master (M_VolumetricCloud_03_Profiles_Billowy_Inst) with its
// "Cloud Density" scalar driven by C_BillowyDensity.GetFloatValue(Cloudiness)
// -- confirmed by probing the live material param through the same threshold
// BP_GeneralSceneSettings used (90): 90->10, 95->38, 100->247.
static TAutoConsoleVariable<float> CVarCarlaWeatherOvercastThreshold(
    TEXT("carla.Weather.OvercastThreshold"),
    90.0f,
    TEXT("Cloudiness value at/above which the sky rig's VolumetricCloudComponent switches ")
    TEXT("from the plain cloud master material to the Billowy overcast one."),
    ECVF_Default);

// Under evaluation: the hard swap to the Billowy material right at the
// threshold reads as a visible break (two completely different rendering
// techniques). Set to 0 to always use the plain MI_Clouds master and extend
// the BaseNoiseExp formula up through Cloudiness=100, to compare against the
// swapped look.
static TAutoConsoleVariable<bool> CVarCarlaWeatherEnableOvercastClouds(
    TEXT("carla.Weather.EnableOvercastClouds"),
    true,
    TEXT("Whether Cloudiness at/above OvercastThreshold switches the cloud material to the ")
    TEXT("Billowy overcast one. Set false to always use the plain master instead."),
    ECVF_Default);

// Every weather push re-asserts the cloud material (MI_Clouds or the Billowy
// master) and overwrites BaseNoiseExp / Cloud Density on the rig's
// VolumetricCloudComponent. That makes a component authored or swapped in by
// hand in BP_Carla_Sky unmeasurable on its own terms -- it never survives the
// first push. Set false to leave the component exactly as the blueprint
// authored it, to test whether the sky flicker in #9884 follows the rig's
// specific cloud asset or any volumetric cloud at all.
// Radius in kilometres that the volumetric cloud shadow map covers. The map's
// texel size is this divided by its resolution, and coarse texels are what
// makes a rotating sun step the sky -- see the block that applies it in
// ApplyWeatherToSkyActor. Exposed so it can be swept live instead of needing a
// rebuild per value; 20 is what was measured and shipped.
// Heavy rain reads as a bright, flat, colourless scene where the far end of the
// street dissolves into grey -- not as a dark one. Measured off three reference
// photographs of a Tokyo downpour: saturation 3-5%, mid-distance contrast
// 16-22, channels within ~2 levels of each other. CARLA at Cloudiness 100 and
// FogDensity 100 sits at saturation 24%, contrast 60 and blue running 22 levels
// over red, because FogDensity is scaled by 0.001 below and tops out at an
// engine density of 0.1, which is far too thin to produce that veil.
//
// These expose the two knobs so the look can be swept from the console without
// a rebuild. Both default to current behaviour.
// Both light intensities come from curves keyed on SunAltitudeAngle alone --
// SunIntensity_Curve and SkyIntensity_Curve on the rig -- so Cloudiness does
// not affect them at all. At Cloudiness 100 the sun still delivers the 100000
// units of a clear midday, which is what blows out the fog on the sun side and
// leaves one half of the frame far brighter than the other. Measured on
// Town10 with heavy rain: dropping the sun to 20000 takes the blown-out
// fraction from 2.8% to 0.0%.
//
// Exposed as direct overrides rather than a Cloudiness curve on purpose: the
// obvious formula, sun down and sky light up, was measured and does NOT hold
// the exposure. CARLA's sky light captures the sky in real time, and that sky
// is lit by the same sun, so dimming the sun dims the capture too -- at sun 0
// with the sky light at double intensity the whole scene sits at brightness
// 6.8 out of 255. Choosing the right relationship needs an artist, so this
// gives them a dial instead of baking in a guess.
static TAutoConsoleVariable<float> CVarCarlaWeatherSunIntensityOverride(
    TEXT("carla.Weather.SunIntensityOverride"),
    -1.0f,
    TEXT("If >= 0, overrides the sky rig's sun intensity after SunIntensity_Curve has been ")
    TEXT("applied. Negative keeps the curve's value."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherSkyLightIntensityOverride(
    TEXT("carla.Weather.SkyLightIntensityOverride"),
    -1.0f,
    TEXT("If >= 0, overrides the sky rig's sky light intensity after SkyIntensity_Curve has ")
    TEXT("been applied. Negative keeps the curve's value."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherFogDensityScale(
    TEXT("carla.Weather.FogDensityScale"),
    0.001f,
    TEXT("Multiplier taking CARLA's 0-100 FogDensity to the height fog component's density. ")
    TEXT("The shipped 0.001 caps the engine at 0.1, too thin for a heavy-rain veil."),
    ECVF_Default);

// Grey level forced on the fog's inscattering colour. The fog otherwise takes
// its colour from the sky atmosphere, which is blue, and that blue is most of
// the cast that separates CARLA's rain from the references.
static TAutoConsoleVariable<float> CVarCarlaWeatherFogGrey(
    TEXT("carla.Weather.FogGrey"),
    -1.0f,
    TEXT("If >= 0, forces the height fog inscattering colour to this neutral grey. ")
    TEXT("Negative leaves whatever the rig and the sky atmosphere produce."),
    ECVF_Default);

// Cloud ExtinctionScale, i.e. how much light the cloud layer actually blocks.
// The rig authors 0.05, which lets the sun through and keeps the clouds white
// and puffy whatever the Cloudiness says.
static TAutoConsoleVariable<float> CVarCarlaWeatherCloudExtinction(
    TEXT("carla.Weather.CloudExtinction"),
    -1.0f,
    TEXT("If >= 0, overrides the cloud material's ExtinctionScale on every weather push. ")
    TEXT("Negative leaves the material's own value (0.05)."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherCloudShadowExtentKm(
    TEXT("carla.Weather.CloudShadowExtentKm"),
    10.0f,
    TEXT("Radius in km covered by the volumetric cloud shadow map, set on the sky rig's sun ")
    TEXT("light. Smaller means finer shadow map texels and less stepping as the sun rotates, ")
    TEXT("at the cost of clouds no longer shadowing anything further away. 0 or less leaves ")
    TEXT("whatever the rig authored."),
    ECVF_Default);

// Scales how dark the cloud shadow is, and so scales the amplitude of whatever
// stepping remains. 1.0 is the rig's authored value.
static TAutoConsoleVariable<float> CVarCarlaWeatherCloudShadowStrength(
    TEXT("carla.Weather.CloudShadowStrength"),
    -1.0f,
    TEXT("Cloud shadow strength on the sky rig's sun light, for both the sky and the surface. ")
    TEXT("Negative leaves whatever the rig authored (1.0)."),
    ECVF_Default);

static TAutoConsoleVariable<bool> CVarCarlaWeatherDriveCloudMaterial(
    TEXT("carla.Weather.DriveCloudMaterial"),
    true,
    TEXT("Whether weather pushes assign the cloud material and its density parameters to ")
    TEXT("the sky rig's VolumetricCloudComponent. Set false to leave the blueprint's own setup alone."),
    ECVF_Default);

AWeather::AWeather(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrecipitationPostProcessMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(
        TEXT("Material'/Game/Carla/Static/GenericMaterials/FX/ScreenDust/M_LensRain.M_LensRain'")).Object;

    DustStormPostProcessMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(
        TEXT("Material'/Game/Carla/Static/GenericMaterials/FX/ScreenDust/M_screenDust_wind.M_screenDust_wind'")).Object;

    RainParameters = ConstructorHelpers::FObjectFinder<UMaterialParameterCollection>(
        TEXT("/Game/Carla/Blueprints/Weather/Materials/WeatherMaterialParameters.WeatherMaterialParameters")).Object;
    RainTemplate = ConstructorHelpers::FObjectFinder<UParticleSystem>(
        TEXT("/Game/Carla/Static/FX/Particles/Rain/PS_Rain.PS_Rain")).Object;
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics;
    RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void UCarlaRainCameraModifier::ModifyPostProcess(float DeltaSeconds,
    float& BlendWeight, FPostProcessSettings& Settings)
{
    if (RainMaterial && RainWeight > 0.0f)
    {
        Settings.AddBlendable(RainMaterial, RainWeight);
        BlendWeight = 1.0f;
    }
}

UParticleSystemComponent* AWeather::CreateRainEmitter(AActor* Owner, bool bSensor)
{
    if (!RainTemplate)
        return nullptr;
    UParticleSystemComponent* Emitter = nullptr;
    if (bSensor)
    {
        // RGB blueprints already attach PS_Rain. Adopt it, including its BP
        // references, rather than creating duplicate rain or invalidating them.
        TInlineComponentArray<UParticleSystemComponent*> Components(Owner);
        for (auto* Component : Components)
            if (Component->Template == RainTemplate)
            {
                Emitter = Component;
                break;
            }
    }
    const bool bNewComponent = Emitter == nullptr;
    if (bNewComponent)
        Emitter = NewObject<UParticleSystemComponent>(Owner);
    Emitter->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    Emitter->bAutoActivate = false;
    Emitter->bAutoDestroy = false;
    Emitter->SetTemplate(RainTemplate);
    // Each capture sees only its own volume. The spectator volume is excluded
    // from captures, so adding cameras never increases another view's density.
    Emitter->SetOnlyOwnerSee(bSensor);
    Emitter->SetVisibleInSceneCaptureOnly(bSensor);
    Emitter->SetHiddenInSceneCapture(!bSensor);
    Emitter->SetCastShadow(false);
    if (bNewComponent)
    {
        Owner->AddInstanceComponent(Emitter);
        Emitter->RegisterComponent();
    }
    Emitter->SetWorldScale3D(FVector(2.0f));
    if (auto* Material = Emitter->CreateDynamicMaterialInstance(0))
    {
        Material->SetScalarParameterValue(TEXT("StreakBrightness"), 1.2f);
        Material->SetScalarParameterValue(TEXT("StreakOpacity"), 0.65f);
    }
    return Emitter;
}

void AWeather::UpdateRain()
{
    if (!GetWorld() || !GetWorld()->IsGameWorld())
        return;

    const float Rain = FMath::Clamp(Weather.Precipitation / 100.0f, 0.0f, 1.0f);
    // Explicitly drive the lens collection without depending on a town's sky BP.
    if (RainParameters)
        UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), RainParameters, TEXT("Precipitation"), Rain);

    auto UpdateEmitter = [this, Rain](UParticleSystemComponent* Emitter, const FVector& Location)
    {
        if (!Emitter) return;
        // PS_Rain's scaled spawn box is X[-1000,4000], Y[-2000,2000] cm.
        // Center it on the view and retain world-space, vertically falling drops.
        const FVector Origin = Location + FVector(-1500.0f, 0.0f, 400.0f);
        if (FVector::DistSquared(Emitter->GetComponentLocation(), Origin) > FMath::Square(2000.0f))
            Emitter->DeactivateImmediate(); // Do not leave a trail after teleporting.
        Emitter->SetWorldLocation(Origin);
        Emitter->SetFloatParameter(TEXT("RainDensity"), Rain * (4000.0f / 0.85f));
        // Legacy distribution maps Z [0,1] to [0,-1000] cm/s. Negative input clamps to zero.
        Emitter->SetVectorParameter(TEXT("RainVelocity"),
            FVector(FMath::Clamp(Weather.WindIntensity, 0.0f, 100.0f) * 3.0f, 0.0f, 1.0f));
        if (Rain <= 0.0f)
            Emitter->DeactivateImmediate();
        else if (!Emitter->IsActive())
            Emitter->ActivateSystem(true);
    };

    if (auto* Camera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
    {
        if (!IsValid(RainCameraModifier) || RainCameraModifier->GetOuter() != Camera)
        {
            if (IsValid(RainCameraModifier))
                if (auto* PreviousCamera = Cast<APlayerCameraManager>(RainCameraModifier->GetOuter()))
                    PreviousCamera->RemoveCameraModifier(RainCameraModifier);
            RainCameraModifier = Cast<UCarlaRainCameraModifier>(
                Camera->AddNewCameraModifier(UCarlaRainCameraModifier::StaticClass()));
        }
        if (RainCameraModifier)
        {
            RainCameraModifier->RainMaterial = PrecipitationPostProcessMaterial;
            RainCameraModifier->RainWeight = Rain;
        }
        if (!ViewportRain && Rain > 0.0f)
            ViewportRain = CreateRainEmitter(this, false);
        UpdateEmitter(ViewportRain, Camera->GetCameraLocation());
    }
    else if (ViewportRain)
        ViewportRain->DeactivateImmediate();

    for (auto It = SensorRain.CreateIterator(); It; ++It)
        if (!It.Key().IsValid() || !It.Value().IsValid())
        {
            if (It.Value().IsValid()) It.Value()->DestroyComponent();
            It.RemoveCurrent();
        }
    for (TActorIterator<ASceneCaptureCamera> It(GetWorld()); It; ++It)
    {
        auto* Sensor = *It;
        auto* Emitter = SensorRain.FindRef(Sensor).Get();
        if (!Emitter && Rain > 0.0f)
        {
            Emitter = CreateRainEmitter(Sensor, true);
            SensorRain.Add(Sensor, Emitter);
        }
        UpdateEmitter(Emitter, Sensor->GetCaptureComponent2D()->GetComponentLocation());
    }
}

void AWeather::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateRain();
}

void AWeather::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    for (auto& Entry : SensorRain)
        if (Entry.Value.IsValid()) Entry.Value->DestroyComponent();
    SensorRain.Empty();
    if (IsValid(RainCameraModifier))
        if (auto* Camera = Cast<APlayerCameraManager>(RainCameraModifier->GetOuter()))
            Camera->RemoveCameraModifier(RainCameraModifier);
    RainCameraModifier = nullptr;
    Super::EndPlay(EndPlayReason);
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
        // Removing a material from ActiveBlendables does not remove it from
        // an existing camera. Clear stale weather passes when rain/dust stops;
        // leave unrelated camera post-process materials intact.
        if (!ActiveBlendables.Contains(PrecipitationPostProcessMaterial))
            Sensor->GetCaptureComponent2D()->PostProcessSettings.RemoveBlendable(PrecipitationPostProcessMaterial);
        if (!ActiveBlendables.Contains(DustStormPostProcessMaterial))
            Sensor->GetCaptureComponent2D()->PostProcessSettings.RemoveBlendable(DustStormPostProcessMaterial);
        for (auto& ActiveBlendable : ActiveBlendables)
            Sensor->GetCaptureComponent2D()->PostProcessSettings.AddBlendable(ActiveBlendable.Key, ActiveBlendable.Value);
    }
}

// Environment-map override for the sky light (set_sky_light_map RPC). The
// cubemap lives in the transient package and is rooted while set, but the
// override is scoped to the world it was set on: it is re-applied to that
// world's sky rig on every weather push, and dropped when that world is
// cleaned up (load_world / reload_world), so a freshly loaded level always
// starts on its own real-time atmosphere capture -- a client that loaded a
// new map never asked for the previous map's lighting.
namespace
{
    struct FSkyLightMapOverride
    {
        UTextureCube* Cubemap = nullptr;
        float Intensity = 1.0f;
        TWeakObjectPtr<UWorld> World;
    };
    FSkyLightMapOverride GSkyLightMapOverride;
    FDelegateHandle GSkyLightMapWorldCleanupHandle;

    bool SkyLightMapAppliesTo(const UWorld* World)
    {
        return GSkyLightMapOverride.Cubemap != nullptr && World != nullptr &&
            GSkyLightMapOverride.World.Get() == World;
    }

    // What the override rewrites on a sky light, saved the first time it is
    // applied to that component and put back by ClearSkyLightMap. The
    // real-time capture honours bLowerHemisphereIsBlack too, so leaving it
    // cleared would change the rig's ambient after the map is removed.
    struct FSkyLightSavedState
    {
        TEnumAsByte<ESkyLightSourceType> SourceType;
        bool bLowerHemisphereIsBlack;
    };
    TMap<TWeakObjectPtr<USkyLightComponent>, FSkyLightSavedState> GSkyLightSavedStates;

    void ApplySkyLightMapToComponent(USkyLightComponent* SkyLightComponent)
    {
        if (!GSkyLightSavedStates.Contains(SkyLightComponent))
        {
            GSkyLightSavedStates.Add(SkyLightComponent,
                {SkyLightComponent->SourceType, SkyLightComponent->bLowerHemisphereIsBlack});
        }
        if (SkyLightComponent->bRealTimeCapture)
            SkyLightComponent->SetRealTimeCaptureEnabled(false);
        if (SkyLightComponent->SourceType != SLS_SpecifiedCubemap)
        {
            SkyLightComponent->SourceType = SLS_SpecifiedCubemap;
            SkyLightComponent->MarkRenderStateDirty();
            SkyLightComponent->SetCaptureIsDirty();
        }
        if (SkyLightComponent->bLowerHemisphereIsBlack)
        {
            SkyLightComponent->bLowerHemisphereIsBlack = false;
            SkyLightComponent->MarkRenderStateDirty();
        }
        SkyLightComponent->SetSourceCubemapAngle(0.0f);
        SkyLightComponent->SetCubemap(GSkyLightMapOverride.Cubemap);
        if (SkyLightComponent->Cubemap != GSkyLightMapOverride.Cubemap)
        {
            // SetCubemap is a no-op on a Static-mobility light; the rig's
            // skylight is meant to be Stationary, but do not depend on it.
            SkyLightComponent->Cubemap = GSkyLightMapOverride.Cubemap;
        }
        // Always re-process: the cubemap contents may be new even when the
        // pointer is not (see SkyLightMap.cpp on object naming).
        SkyLightComponent->MarkRenderStateDirty();
        SkyLightComponent->SetCaptureIsDirty();
        SkyLightComponent->SetLightColor(FLinearColor::White);
        SkyLightComponent->SetIntensity(GSkyLightMapOverride.Intensity);
        if (!SkyLightComponent->IsActive())
            SkyLightComponent->SetActive(true);
    }

    // Forget the override: unroot the cubemap (the transient object is then
    // collected with the next GC pass) and drop the saved component states,
    // whose weak keys are dead once the owning world has gone anyway.
    void DropSkyLightMapOverride()
    {
        if (GSkyLightMapOverride.Cubemap != nullptr)
            GSkyLightMapOverride.Cubemap->RemoveFromRoot();
        GSkyLightMapOverride = FSkyLightMapOverride{};
        GSkyLightSavedStates.Empty();
        if (GSkyLightMapWorldCleanupHandle.IsValid())
        {
            FWorldDelegates::OnWorldCleanup.Remove(GSkyLightMapWorldCleanupHandle);
            GSkyLightMapWorldCleanupHandle.Reset();
        }
    }

    // load_world / reload_world tear the current world down through
    // UWorld::CleanupWorld before the next level is brought up; that is the
    // end of the override's scope. Other worlds (the editor world, a level
    // instance) are not ours and are left alone.
    void OnSkyLightMapWorldCleanup(UWorld* World, bool /*bSessionEnded*/, bool /*bCleanupResources*/)
    {
        if (World == nullptr || GSkyLightMapOverride.World.Get() != World)
            return;
        DropSkyLightMapOverride();
        UE_LOG(LogCarla, Log, TEXT("AWeather: sky light map dropped with world %s"), *World->GetName());
    }
}

bool AWeather::SetSkyLightMap(UWorld* World, UTextureCube* Cubemap, float Intensity)
{
    if (Cubemap == nullptr || World == nullptr)
        return false;
    if (GSkyLightMapOverride.Cubemap != nullptr && GSkyLightMapOverride.Cubemap != Cubemap)
        GSkyLightMapOverride.Cubemap->RemoveFromRoot();
    Cubemap->AddToRoot();
    if (GSkyLightMapOverride.World.Get() != World)
    {
        // Setting on a different world than the previous override: whatever
        // that world's rigs remembered is gone with it.
        GSkyLightSavedStates.Empty();
    }
    GSkyLightMapOverride.Cubemap = Cubemap;
    GSkyLightMapOverride.Intensity = Intensity;
    GSkyLightMapOverride.World = World;
    if (!GSkyLightMapWorldCleanupHandle.IsValid())
        GSkyLightMapWorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddStatic(&OnSkyLightMapWorldCleanup);

    int32 Applied = 0;
    TArray<AActor*> SkyActors;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyBase::StaticClass(), SkyActors);
    for (AActor* SkyActor : SkyActors)
    {
        ASkyBase* Sky = Cast<ASkyBase>(SkyActor);
        if (USkyLightComponent* SkyLightComponent = Sky != nullptr ? Sky->GetSkyLightComponent() : nullptr)
        {
            ApplySkyLightMapToComponent(SkyLightComponent);
            ++Applied;
        }
    }
    UE_LOG(LogCarla, Log, TEXT("AWeather: sky light map set (intensity %.3f) on %d sky rig(s)"), Intensity, Applied);
    return Applied > 0;
}

void AWeather::ClearSkyLightMap(UWorld* World)
{
    // Restore the rigs first (the saved states go away with the override).
    TArray<AActor*> SkyActors;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyBase::StaticClass(), SkyActors);
    for (AActor* SkyActor : SkyActors)
    {
        ASkyBase* Sky = Cast<ASkyBase>(SkyActor);
        if (USkyLightComponent* SkyLightComponent = Sky != nullptr ? Sky->GetSkyLightComponent() : nullptr)
        {
            SkyLightComponent->SetCubemap(nullptr);
            if (const FSkyLightSavedState* Saved = GSkyLightSavedStates.Find(SkyLightComponent))
            {
                SkyLightComponent->SourceType = Saved->SourceType;
                SkyLightComponent->bLowerHemisphereIsBlack = Saved->bLowerHemisphereIsBlack;
                SkyLightComponent->MarkRenderStateDirty();
                SkyLightComponent->SetCaptureIsDirty();
            }
            SkyLightComponent->SetRealTimeCaptureEnabled(true);
        }
    }
    DropSkyLightMapOverride();
    // Restores the curve intensity and the real-time capture through the
    // normal weather push.
    if (UCarlaEpisode* Episode = UCarlaStatics::GetCurrentEpisode(World))
        if (AWeather* Weather = Episode->GetWeather())
            Weather->PushWeatherToSky();
    UE_LOG(LogCarla, Log, TEXT("AWeather: sky light map cleared"));
}

bool AWeather::HasSkyLightMap()
{
    return GSkyLightMapOverride.Cubemap != nullptr;
}

void AWeather::PushWeatherToSky()
{
    TArray<AActor*> SkyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyBase::StaticClass(), SkyActors);
    for (AActor* SkyActor : SkyActors)
        ApplyWeatherToSkyActor(SkyActor, Weather);
}

void AWeather::ApplyWeatherToSkyActor(AActor* SkyActor, const FWeatherParameters& Weather)
{
    if (SkyActor == nullptr)
        return;

    {
        // Blueprint member variables get decorated FNames in their generated
        // class (exact FindPropertyByName misses them), so match on the
        // authored name instead -- same lesson as the vehicle "Beam Lights"
        // variable in CarlaWheeledVehicle.cpp. Shared by every reflection
        // lookup in this function.
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

        // The rig stores the parameters it renders from in a blueprint variable
        // and refreshes every component from its blueprint Update function.
        // Matched by type via ASkyBase::FindWeatherParameters, not by name --
        // a name-based lookup here silently broke (no warning triggered,
        // because "SkyParameters"/"Sky Parameters" both still resolved to
        // *something* is wrong; it just returned null and this whole write
        // no-opped) the moment that Blueprint variable got renamed to
        // "WeatherParameters". Precipitation/Wetness visuals -- driven by the
        // legacy UpdateAtmosphereAndPrecipitation Blueprint call below, which
        // reads this struct rather than taking Weather as a parameter -- are
        // what silently went stale; native pushes further down (sun, clouds,
        // fog, exposure) read straight from the Weather parameter and were
        // never affected.
        if (FWeatherParameters* SkyParameters = ASkyBase::FindWeatherParameters(SkyActor))
            *SkyParameters = Weather;
        else
            UE_LOG(LogCarla, Warning, TEXT("AWeather: no WeatherParameters property on %s"),
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

        // SetSkySphere (just called above) spawns a fresh stock-engine
        // BP_Sky_Sphere and tries to attach it to this rig's root
        // (PostProcessComponent), but that root's Mobility is Movable while
        // the stock sphere's root is Static -- UE refuses to attach a Static
        // child to a non-Static parent ("AttachTo: ... is not static ...
        // Aborting", logged every push). The spawn itself still succeeds and
        // "SkySphere" still gets assigned, so this is silent rather than
        // fatal: the sphere just renders unattached at wherever it spawned
        // instead of following the rig. Finish the attach here natively,
        // forcing the spawned instance's root to Movable first (a fresh
        // stock actor with no other purpose -- safe to change).
        {
            FObjectProperty* SphereProperty = CastField<FObjectProperty>(
                FindPropertyByAuthoredName(SkyActor->GetClass(), TEXT("SkySphere")));
            if (SphereProperty == nullptr)
                SphereProperty = CastField<FObjectProperty>(
                    FindPropertyByAuthoredName(SkyActor->GetClass(), TEXT("Sky Sphere")));
            AActor* SphereActor = SphereProperty != nullptr
                ? Cast<AActor>(SphereProperty->GetObjectPropertyValue_InContainer(SkyActor))
                : nullptr;
            USceneComponent* SphereRoot = SphereActor != nullptr ? SphereActor->GetRootComponent() : nullptr;
            if (SphereRoot != nullptr && SphereRoot->GetAttachParent() == nullptr)
            {
                if (SphereRoot->Mobility == EComponentMobility::Static)
                    SphereRoot->SetMobility(EComponentMobility::Movable);
                SphereActor->AttachToActor(SkyActor, FAttachmentTransformRules::KeepWorldTransform);
            }

            // The sphere paints its own sun, and the two material parameters
            // that place it -- "Light direction" and "Sun height" -- are only
            // ever written by the blueprint when its "Directional Light Actor"
            // reference is set. On this rig that reference is empty, so they
            // keep the material's defaults: direction (1, 0, 0) and height 0.
            //
            // Measured live at SunAltitudeAngle 25 / SunAzimuthAngle 170: the
            // directional light pointed the scene's light in from
            // (0.89, -0.16, 0.42) while the painted sun sat at +X on the
            // horizon. So the sun you see and the light that lights the scene
            // disagree, and worse, the painted one does not move at all as the
            // azimuth turns -- any client driving a day cycle through the
            // weather API gets a sun frozen in place.
            //
            // Compute the direction from the weather rather than reading it
            // back off the light: the light's own rotation is applied further
            // down in this same function, so reading it here would use the
            // previous push's value, and deriving both from the same source
            // is what keeps them from drifting apart again.
            //
            // Do NOT go through the "SkySphere" property to find it: on
            // Town10's rig that property reads None, so anything hanging off
            // it silently does nothing. The sphere that actually renders is a
            // child actor of this rig, so walk the child actor components.
            {
                const FVector SunDirection =
                    -FRotator(-Weather.SunAltitudeAngle, Weather.SunAzimuthAngle, 0.0f).Vector();

                TArray<AActor*> SkyCandidates;
                if (SphereActor != nullptr)
                    SkyCandidates.Add(SphereActor);
                TInlineComponentArray<UChildActorComponent*> RigChildActors;
                SkyActor->GetComponents(RigChildActors);
                for (UChildActorComponent* RigChild : RigChildActors)
                    if (RigChild != nullptr && RigChild->GetChildActor() != nullptr)
                        SkyCandidates.AddUnique(RigChild->GetChildActor());

                for (AActor* Candidate : SkyCandidates)
                {
                    TInlineComponentArray<UMeshComponent*> SphereMeshes;
                    Candidate->GetComponents(SphereMeshes);
                    for (UMeshComponent* SphereMesh : SphereMeshes)
                    {
                        if (SphereMesh == nullptr)
                            continue;
                        // Returns the existing dynamic instance when there
                        // already is one, so this does not pile up a new
                        // material per push. Setting a parameter the material
                        // does not have is a no-op, which is what keeps this
                        // safe to apply to every mesh hanging off the rig.
                        if (UMaterialInstanceDynamic* SphereMID =
                                SphereMesh->CreateAndSetMaterialInstanceDynamic(0))
                        {
                            SphereMID->SetVectorParameterValue(
                                TEXT("Light direction"), FLinearColor(SunDirection));
                            SphereMID->SetScalarParameterValue(TEXT("Sun height"), SunDirection.Z);
                        }
                    }
                }
            }

            // PIE start reliably left duplicate attached actors -- not just
            // the sphere (SetSkySphere's respawn: destroy-old, spawn-new,
            // doesn't reliably find/destroy the previous one across
            // PIE's construction timing), but independently a duplicate
            // DirectionalLight too (SetSunActorReference, called earlier in
            // the UpdateFunctionNames loop above, manages its own actor
            // reference the same lossy way) -- tripping the "multiple
            // directional lights competing" render warning. Rather than
            // chase the exact PIE timing for each function separately, make
            // this self-healing and general: whatever ends up attached to
            // this rig, keep at most one instance per class.
            {
                TMap<UClass*, TArray<AActor*>> AttachedByClass;
                TArray<AActor*> AttachedToSky;
                SkyActor->GetAttachedActors(AttachedToSky);
                for (AActor* AttachedActor : AttachedToSky)
                    if (AttachedActor != nullptr)
                        AttachedByClass.FindOrAdd(AttachedActor->GetClass()).Add(AttachedActor);

                for (const TPair<UClass*, TArray<AActor*>>& Pair : AttachedByClass)
                {
                    const TArray<AActor*>& Instances = Pair.Value;
                    // Keep the one "SkySphere" actually points to when this
                    // is its class; otherwise keep whichever is last (order
                    // is not meaningful here, just needs to be consistent).
                    AActor* ToKeep = (SphereActor != nullptr && Instances.Contains(SphereActor))
                        ? SphereActor : Instances.Last();
                    for (AActor* Instance : Instances)
                    {
                        if (Instance != ToKeep)
                        {
                            TArray<AActor*> OrphanChildren;
                            Instance->GetAttachedActors(OrphanChildren);
                            for (AActor* OrphanChild : OrphanChildren)
                                if (OrphanChild != nullptr)
                                    OrphanChild->Destroy();
                            Instance->Destroy();
                        }
                    }

                    // SetSunActorReference (called earlier in the
                    // UpdateFunctionNames loop) links a stock-engine
                    // ADirectionalLight onto this rig, on top of our own
                    // Sun/Moon components -- a THIRD directional light,
                    // confirmed in the outliner ("DirectionalLight0") and over
                    // the render warning ("Multiple directional lights are
                    // competing..."). Neutralize the survivor the same way
                    // Sky.cpp's constructor already does for the Moon: below
                    // the Sun's ForwardShadingPriority, and out of the running
                    // for SkyAtmosphere's single sun-light slot (every
                    // DirectionalLightComponent defaults bAtmosphereSunLight
                    // true -- left alone, this stray light could win that slot
                    // over our real Sun, which is what actually broke
                    // SkyAtmosphere/rendered a black sky once the sun rose,
                    // independent of the render warning). Cheap and
                    // idempotent, run every push like the dedup above.
                    if (UDirectionalLightComponent* StrayLight =
                            ToKeep != nullptr ? ToKeep->FindComponentByClass<UDirectionalLightComponent>() : nullptr)
                    {
                        if (StrayLight->ForwardShadingPriority != -1)
                            StrayLight->SetForwardShadingPriority(-1);
                        if (StrayLight->IsUsedAsAtmosphereSunLight())
                            StrayLight->SetAtmosphereSunLight(false);
                    }
                }
            }

            // Destroying the surplus child ACTORS above is not enough. Each was
            // owned by a UChildActorComponent that SetSkySphere and
            // SetSunActorReference add to this rig on EVERY weather push, and a
            // surviving component immediately recreates its child actor -- so
            // the actor dedup above was treating the symptom while the
            // components grew without bound.
            //
            // Measured: 2 components leaked per push (8 -> 208 over 100 pushes
            // with identical weather values and the sun held still; 208 -> 6
            // with this fix), so a client driving weather at 10 Hz accumulates
            // hundreds of coincident opaque sky spheres. Unbounded memory and
            // performance growth for anyone using the Weather API.
            //
            // Not the cause of the sky flicker in #9884, despite an earlier
            // version of this comment claiming so: that flicker is the
            // volumetric cloud shadow map, and it reproduces with this fix
            // applied and only the two legitimate components present. Piling
            // up spheres does destabilise the sky on its own -- they keep the
            // sun state they had when they stopped being current and then
            // fight over the depth test -- but that is this defect, not the
            // reported one.
            //
            // Keep one component per child-actor class, preferring the one
            // whose child actor the dedup above chose to keep.
            // DestroyComponent takes the child actor with it.
            {
                TInlineComponentArray<UChildActorComponent*> ChildActorComponents;
                SkyActor->GetComponents(ChildActorComponents);
                TMap<UClass*, TArray<UChildActorComponent*>> ComponentsByChildClass;
                for (UChildActorComponent* ChildComponent : ChildActorComponents)
                    if (ChildComponent != nullptr && ChildComponent->GetChildActorClass() != nullptr)
                        ComponentsByChildClass.FindOrAdd(ChildComponent->GetChildActorClass()).Add(ChildComponent);

                for (const TPair<UClass*, TArray<UChildActorComponent*>>& Pair : ComponentsByChildClass)
                {
                    const TArray<UChildActorComponent*>& Components = Pair.Value;
                    if (Components.Num() < 2)
                        continue;
                    UChildActorComponent* ComponentToKeep = Components.Last();
                    for (UChildActorComponent* ChildComponent : Components)
                    {
                        if (ChildComponent->GetChildActor() != nullptr
                            && ChildComponent->GetChildActor() == SphereActor)
                        {
                            ComponentToKeep = ChildComponent;
                            break;
                        }
                    }
                    for (UChildActorComponent* ChildComponent : Components)
                        if (ChildComponent != ComponentToKeep)
                            ChildComponent->DestroyComponent();
                }
            }
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
            FogComponent->SetFogDensity(
                Weather.FogDensity * CVarCarlaWeatherFogDensityScale.GetValueOnGameThread());
            const float FogGrey = CVarCarlaWeatherFogGrey.GetValueOnGameThread();
            if (FogGrey >= 0.0f)
                FogComponent->SetFogInscatteringColor(FLinearColor(FogGrey, FogGrey, FogGrey, 1.0f));
            FogComponent->SetStartDistance(Weather.FogDistance * 100.0f);
            FogComponent->SetFogCutoffDistance(0.0f);
        }

        // The blueprint's ControlSunIntensity has a severed exec chain (its
        // entry sets SunTrayectory and dead-ends), so the curve-driven light
        // intensities never apply. Evaluate the curves directly and drive the
        // rig components; the curve assets remain the content-side tuning
        // source. Loaded by fixed path (BP_CarlaWeather's own default values
        // for these two variables) rather than read off an AWeather instance,
        // so this function works with no AWeather actor placed in the level
        // at all -- see ASkyBase::RefreshWeather/LoadPreset.
        // Loaded per call, never cached in function-local statics: a raw
        // static UObject pointer is invisible to the garbage collector, and
        // these assets are otherwise unreferenced once the world that first
        // loaded them is purged. With the old static cache, the second
        // episode of a session (e.g. any generate_opendrive_world load)
        // evaluated a freed UCurveFloat and crashed inside FRichCurve::Eval
        // (SIGSEGV in ApplyWeatherToSkyActor <- GameMode BeginPlay). While
        // the asset is alive LoadObject is a FindObject hit, so per-call
        // loading costs nothing measurable at weather-push frequency.
        auto FindCurve = [](const TCHAR* PropertyName) -> UCurveFloat*
        {
            if (FCString::Strcmp(PropertyName, TEXT("SunIntensity_Curve")) == 0)
                return LoadObject<UCurveFloat>(nullptr,
                    TEXT("/Game/Carla/Blueprints/Weather/Weather2_Curves/SunIntensity_2.SunIntensity_2"));
            if (FCString::Strcmp(PropertyName, TEXT("SkyIntensity_Curve")) == 0)
                return LoadObject<UCurveFloat>(nullptr,
                    TEXT("/Game/Carla/Blueprints/Weather/Weather2_Curves/SkylightIntensity_2.SkylightIntensity_2"));
            return nullptr;
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
                const float SunIntensityOverride = CVarCarlaWeatherSunIntensityOverride.GetValueOnGameThread();
                if (SunIntensityOverride >= 0.0f)
                    SunLightComponent->SetIntensity(SunIntensityOverride);
                // Rigs saved with a black light color render no sunlight at any
                // intensity; the physical tint comes from the color temperature.
                SunLightComponent->SetLightColor(FLinearColor::White);
            }
        }
        // UpdateSun never applies a rotation. Pitch is negated because a
        // directional light shines along its forward vector.
        if (ULightComponent* SunLightComponent = FindComponent(TEXT("DirectionalLightComponentSun")))
        {
            SunLightComponent->SetWorldRotation(
                FRotator(-Weather.SunAltitudeAngle, Weather.SunAzimuthAngle, 0.0f));
        }

        // The cloud shadow map is a single 512-texel map (512 * the light's
        // CloudShadowMapResolutionScale, capped by
        // r.VolumetricCloud.ShadowMap.MaxResolution) spread over
        // CloudShadowExtent kilometres of radius. The rig ships 150 km, which
        // is ~586 m per texel: rotating the sun walks cloud density across
        // texels that coarse and the clouds' self-shadowing changes in visible
        // steps, which the atmosphere then carries into cloud-free sky as well
        // -- issue #9884. 10 km still covers far more than any CARLA town, and
        // at ~39 m per texel the stepping stops being visible.
        //
        // Measured on the viewport (Town10, sun altitude 2, cloudiness 30,
        // 0.5 deg of azimuth per captured frame; residual from a 9-frame
        // moving average, normalised by the ROI's own mean): horizon sky
        // 0.0718 with 80 consecutive-frame steps over 3 levels at 150 km,
        // 0.0109 with 13 at 20 km, and 0.0045 with 4 once the filtering in
        // DefaultScalability.ini is added on top. The floor, with the shadow
        // map switched off entirely, is 0.0004 with 0 steps. The road ROI,
        // which must not move, stayed at its own floor throughout. Confirmed
        // by eye on a rotating sun, which a residual score alone does not
        // establish.
        //
        // The trade: the map no longer covers the distant world, so clouds
        // stop shadowing sky far from the camera and the horizon sits brighter
        // than it did at 150 km (level 90.6 -> 167.8 of 255).
        //
        // 10 rather than 20 km comes from a later sweep, scored by splitting
        // the viewport into 288 tiles and counting the tiles that step at all,
        // which is what caught a residue the hand-placed ROIs had missed
        // entirely. From an elevated camera over the cloud field, sun at
        // altitude 2: 90 tiles at 20 km, 69 at 10 km, 61 at 5 km, against 38
        // with the shadow map off. The lever is spent by 10 km -- 5 km buys
        // almost nothing and its median residue is slightly worse.
        //
        // Weakening CloudShadowStrength was measured too and is deliberately
        // NOT done: at 0.25 it lands near the floor, but only because it
        // attenuates the cloud shadow itself by the same factor. That is
        // fading the feature out, not fixing the artifact.
        if (UDirectionalLightComponent* SunDirectionalLight =
                Cast<UDirectionalLightComponent>(FindComponent(TEXT("DirectionalLightComponentSun"))))
        {
            bool bSunLightRenderStateDirty = false;
            const float CloudShadowExtentKm = CVarCarlaWeatherCloudShadowExtentKm.GetValueOnGameThread();
            if (CloudShadowExtentKm > 0.0f && SunDirectionalLight->CloudShadowExtent != CloudShadowExtentKm)
            {
                SunDirectionalLight->CloudShadowExtent = CloudShadowExtentKm;
                bSunLightRenderStateDirty = true;
            }
            const float CloudShadowStrength = CVarCarlaWeatherCloudShadowStrength.GetValueOnGameThread();
            if (CloudShadowStrength >= 0.0f
                && (SunDirectionalLight->CloudShadowStrength != CloudShadowStrength
                    || SunDirectionalLight->CloudShadowOnSurfaceStrength != CloudShadowStrength))
            {
                SunDirectionalLight->CloudShadowStrength = CloudShadowStrength;
                SunDirectionalLight->CloudShadowOnSurfaceStrength = CloudShadowStrength;
                bSunLightRenderStateDirty = true;
            }
            if (bSunLightRenderStateDirty)
            {
                SunDirectionalLight->MarkRenderStateDirty();
            }
        }
        // The rig's Moon light ships with AffectsWorld off -- a disabled
        // light contributes nothing to the scene no matter what its
        // Intensity is set to below, which is why night always rendered
        // pitch black regardless of the moon/skylight floor cvars. Always
        // on, not just at night: harmless during the day (Intensity there is
        // whatever the curve/floor logic below leaves it at), and this way
        // there's nothing left to toggle when the sun crosses the horizon.
        if (ULightComponent* MoonLightComponentAffects = FindComponent(TEXT("DirectionalLightComponentMoon")))
        {
            if (!MoonLightComponentAffects->bAffectsWorld)
            {
                MoonLightComponentAffects->bAffectsWorld = true;
                MoonLightComponentAffects->MarkRenderStateDirty();
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
        // free.
        //
        // The VolumetricCloudComponent is NOT activated here either, but do
        // not read that as it being switched off: activation is irrelevant for
        // that component type. CreateRenderState_Concurrent adds the proxy
        // when ShouldComponentAddToScene() && ShouldRender() && IsRegistered()
        // and never consults IsActive(), so the rig's clouds render regardless.
        // Verified live on Town10's placed BP_Carla_Sky_C_15: visible = True,
        // is_active = False, with a MID off MI_Clouds assigned -- and they are
        // plainly in frame. Visibility, not activation, is the lever. An
        // earlier comment here asserted the opposite and sent the #9884
        // investigation away from the real cause for a whole session.
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
            if (SkyLightMapAppliesTo(SkyActor->GetWorld()))
            {
                // Environment map set through set_sky_light_map on this
                // world: the measured panorama replaces the atmosphere
                // capture as the ambient and reflection source, and its
                // intensity is the caller's, not the curve's. Reapplied on
                // every weather push so that a set_weather call cannot
                // silently revert to the real-time capture. A rig in any
                // other world (a newly loaded level) takes the else branch.
                ApplySkyLightMapToComponent(SkyLightComponent);
            }
            else
            {
            if (!SkyLightComponent->bRealTimeCapture)
                SkyLightComponent->SetRealTimeCaptureEnabled(true);
            if (!SkyLightComponent->IsActive())
                SkyLightComponent->SetActive(true);
            if (UCurveFloat* SkyIntensityCurve = FindCurve(TEXT("SkyIntensity_Curve")))
                SkyLightComponent->SetIntensity(SkyIntensityCurve->GetFloatValue(Weather.SunAltitudeAngle));
            const float SkyLightIntensityOverride = CVarCarlaWeatherSkyLightIntensityOverride.GetValueOnGameThread();
            if (SkyLightIntensityOverride >= 0.0f)
                SkyLightComponent->SetIntensity(SkyLightIntensityOverride);
            }
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
            // Not while an environment map is set: its intensity is the
            // caller's measurement, the night floor would override it.
            if (USkyLightComponent* SkyLightComponent = GSkyLightMapOverride.Cubemap == nullptr
                ? FindSkyLightComponent(TEXT("SkyLightComponent")) : nullptr)
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
            // this stays gated to night. FindPropertyByAuthoredName is
            // shared, defined at the top of this function's SkyActor loop.
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
        else
        {
            // Day: bring the moon back down from whatever night floor last
            // set it to. Unlike the sun/skylight above (pushed unconditionally
            // from their curves every single call, so they self-correct both
            // ways), the moon has no such day-side reset anywhere -- only the
            // night-only floor clamp above, which only ever raises it. Without
            // this, one night floor-clamp leaves the moon lit at that
            // intensity forever, becoming a second active directional light
            // competing with the sun by day ("Multiple directional lights are
            // competing to be the single one used for forward shading..." --
            // confirmed via headless test: moon intensity clamped to a night
            // floor stayed there across a follow-up day update) and polluting
            // the SkyAtmosphere/SkyLight capture. bAffectsWorld/Active are
            // deliberately left alone (see the comment above where they're
            // forced on) -- zero intensity alone makes it contribute nothing.
            if (ULightComponent* MoonLightComponent = FindComponent(TEXT("DirectionalLightComponentMoon")))
            {
                if (MoonLightComponent->Intensity != 0.0f)
                    MoonLightComponent->SetIntensity(0.0f);
            }
        }
    }

    // Cloud density. Ported from BP_GeneralSceneSettings.UpdateClouds (that
    // actor is gone -- this used to depend on it being placed in the level,
    // which routinely wasn't the case in the editor, silently leaving clouds
    // disconnected from Weather.Cloudiness). See CVarCarlaWeatherOvercastThreshold
    // above for how this was reverse-engineered.
    if (CVarCarlaWeatherDriveCloudMaterial.GetValueOnGameThread())
    {
        FObjectProperty* CloudComponentProperty = CastField<FObjectProperty>(
            SkyActor->GetClass()->FindPropertyByName(TEXT("VolumetricCloudComponent")));
        UVolumetricCloudComponent* CloudComponent = CloudComponentProperty != nullptr
            ? Cast<UVolumetricCloudComponent>(CloudComponentProperty->GetObjectPropertyValue_InContainer(SkyActor))
            : nullptr;
        if (CloudComponent != nullptr)
        {
            // Per-call loads, not GC-invisible static caches -- see FindCurve.
            UCurveFloat* const DensityCurve = LoadObject<UCurveFloat>(nullptr,
                TEXT("/Game/Carla/Blueprints/Weather/CloudsBillowy/C_BillowyDensity.C_BillowyDensity"));
            UMaterialInterface* const NormalCloudMaterial = LoadObject<UMaterialInterface>(nullptr,
                TEXT("/Game/Carla/Static/FX/VolumetricClouds/MI_Clouds.MI_Clouds"));
            UMaterialInterface* const BillowyCloudMaterial = LoadObject<UMaterialInterface>(nullptr,
                TEXT("/Game/Carla/Static/GenericMaterials/VolumetricClouds/Masters/M_VolumetricCloud_03_Profiles_Billowy_Inst.M_VolumetricCloud_03_Profiles_Billowy_Inst"));

            const float OvercastThreshold = CVarCarlaWeatherOvercastThreshold.GetValueOnGameThread();
            const bool bOvercast = CVarCarlaWeatherEnableOvercastClouds.GetValueOnGameThread()
                && Weather.Cloudiness >= OvercastThreshold;
            UMaterialInterface* BaseMaterial = bOvercast ? BillowyCloudMaterial : NormalCloudMaterial;
            if (BaseMaterial != nullptr)
            {
                // A brand new MID every push (this runs on every weather
                // update, not just when Cloudiness crosses the threshold)
                // resets VolumetricCloudComponent's temporal accumulation
                // each time -- a visible pop/flash even when the parameters
                // end up identical. Reuse the existing MID when the base
                // material (Normal vs Billowy) hasn't actually changed; only
                // create a fresh one on the first push or an actual
                // threshold cross. Cached by component here rather than read
                // back via CloudComponent->GetMaterial(): that getter didn't
                // reliably report the MID just assigned by SetMaterial below
                // (still returned the previous push's, so the read-back
                // approach always saw a "different" parent and recreated
                // every time -- this side-steps whatever that mismatch was).
                static TMap<TWeakObjectPtr<UVolumetricCloudComponent>, TWeakObjectPtr<UMaterialInstanceDynamic>> CloudMIDCache;
                TWeakObjectPtr<UMaterialInstanceDynamic>* CachedMID = CloudMIDCache.Find(CloudComponent);
                UMaterialInstanceDynamic* CloudMID = (CachedMID != nullptr && CachedMID->IsValid()
                        && CachedMID->Get()->Parent == BaseMaterial)
                    ? CachedMID->Get()
                    : nullptr;
                if (CloudMID == nullptr)
                {
                    CloudMID = UMaterialInstanceDynamic::Create(BaseMaterial, CloudComponent);
                    CloudMIDCache.Add(CloudComponent, CloudMID);
                }
                if (bOvercast)
                {
                    // Below MI_Clouds's own "BaseNoiseExp" scalar is what
                    // actually thins/thickens the cloud layer under the
                    // overcast threshold (probed live against
                    // BP_GeneralSceneSettings.UpdateClouds before removing
                    // it: exactly linear -- 100 - 0.8*Cloudiness -- for
                    // Cloudiness in [1, 89], collapsing to a huge exponent
                    // (~invisible clouds) only right at 0. Leaving this
                    // param at the material's own unrelated default is what
                    // produced the single ugly cloud mass instead of a
                    // normal layer.
                    if (DensityCurve != nullptr)
                        CloudMID->SetScalarParameterValue(TEXT("Cloud Density"), DensityCurve->GetFloatValue(Weather.Cloudiness));
                }
                else
                {
                    const float BaseNoiseExp = Weather.Cloudiness <= 0.0f
                        ? 6000.0f
                        : FMath::Clamp(100.0f - 0.8f * Weather.Cloudiness, 0.0f, 6000.0f);
                    CloudMID->SetScalarParameterValue(TEXT("BaseNoiseExp"), BaseNoiseExp);
                }
                const float CloudExtinction = CVarCarlaWeatherCloudExtinction.GetValueOnGameThread();
                if (CloudExtinction >= 0.0f)
                    CloudMID->SetScalarParameterValue(TEXT("ExtinctionScale"), CloudExtinction);
                CloudComponent->SetMaterial(CloudMID);
            }
            else
            {
                UE_LOG(LogCarla, Warning, TEXT("AWeather: cloud master material missing for %s"), *SkyActor->GetName());
            }
        }
    }

    // Day/night light broadcast. AWeather::ApplyWeather already does this
    // (respecting the artist-facing DayNightCycle toggle) when an AWeather
    // actor exists, but street lamps otherwise only ever turned on when
    // actually playing: ASkyBase::RefreshWeather/LoadPreset fall back to
    // calling this function directly with no AWeather placed, and that path
    // never reached UpdateStreetLightsForDayNight. Broadcasting again here is
    // harmless when an AWeather is present too -- registered lights just
    // re-receive the same state.
    if (UWorld* World = SkyActor->GetWorld())
    {
        if (UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>())
            CarlaLightSubsystem->NotifyDayTimeChange(Weather.SunAltitudeAngle > 0.0f);
    }

    // Wind + Wetness, both on the same global collection. WindIntensity: no
    // one was pushing it at all (UpdateAtmosphereAndPrecipitation's exec
    // chain for it is severed, same pattern as everything else natively
    // re-driven in this function) -- M_VegetationMaster divides it by 10
    // before feeding its wind shader function, so this pushes the raw 0-100
    // CARLA value, not normalized.
    //
    // Wetness: UpdateAtmosphereAndPrecipitation DOES push it (and Puddles/
    // Ripples/Precipitation alongside it) -- but normalized to 0-1 first
    // (confirmed live: Weather.Wetness=90 -> collection value 0.9). Puddles
    // reads fine at that scale. Wetness does not: MF_WetSurfaceFx's own
    // "Wetness" section divides the collection value by 100 *again* before
    // using it (confirmed in the material graph, and by hardcoding 100
    // straight into that Divide node -- wets correctly; the collection read
    // is the only broken link). A pre-normalized 0-1 input run through
    // another /100 lands at ~0.009 -- functionally zero, which is exactly
    // "wetness does nothing". Puddles/Ripples/Precipitation are left alone
    // (already correct at 0-1); only Wetness gets overridden here, raw, right
    // after the BP call above wrote the wrong value.
    {
        // Per-call load, not a GC-invisible static cache -- see FindCurve.
        UMaterialParameterCollection* const WeatherMPC = LoadObject<UMaterialParameterCollection>(nullptr,
            TEXT("/Game/Carla/Blueprints/Weather/Materials/WeatherMaterialParameters.WeatherMaterialParameters"));
        if (WeatherMPC != nullptr && SkyActor->GetWorld() != nullptr)
        {
            UKismetMaterialLibrary::SetScalarParameterValue(
                SkyActor->GetWorld(), WeatherMPC, TEXT("WindIntensity"), Weather.WindIntensity);
            UKismetMaterialLibrary::SetScalarParameterValue(
                SkyActor->GetWorld(), WeatherMPC, TEXT("Wetness"), Weather.Wetness);
        }
    }

    // Viewport exposure fallback. The project ships with auto exposure
    // disabled by default (r.DefaultFeature.AutoExposure=False) and the rigs
    // were saved for the old frozen ~15 lux suns, so a physical 100k lux sun
    // white-outs the main view with no exposure settings at all. This fills
    // in the same histogram exposure the RGB sensor uses by default (bias 0,
    // EV100 range [10,12], speeds 3/1) -- but ONLY for fields the currently
    // loaded PostProcess profile (Content/Carla/Config/PostProcess/*.json)
    // doesn't already claim via bOverride_*. Profiles set their own Method
    // and Bias (e.g. GoPro.json: AEM_Manual/0) and rely on this to supply
    // Min/Max/SpeedUp/SpeedDown, which they intentionally leave unset. This
    // runs on every editor property edit (via OnConstruction), so forcing
    // these unconditionally used to stomp the profile's Method/Bias back to
    // Histogram/0 the instant you touched anything on the Sky actor,
    // including just switching ProfileName -- e.g. GoPro's Manual exposure
    // never stuck, and switching profiles never visibly changed exposure.
    {
        FObjectProperty* PostProcessProperty = CastField<FObjectProperty>(
            SkyActor->GetClass()->FindPropertyByName(TEXT("PostProcessComponent")));
        UPostProcessComponent* PostProcessComponent = PostProcessProperty != nullptr
            ? Cast<UPostProcessComponent>(PostProcessProperty->GetObjectPropertyValue_InContainer(SkyActor))
            : nullptr;
        if (PostProcessComponent != nullptr)
        {
            FPostProcessSettings& Settings = PostProcessComponent->Settings;
            if (!Settings.bOverride_AutoExposureMethod)
            {
                Settings.bOverride_AutoExposureMethod = true;
                Settings.AutoExposureMethod = AEM_Histogram;
            }
            if (!Settings.bOverride_AutoExposureBias)
            {
                Settings.bOverride_AutoExposureBias = true;
                Settings.AutoExposureBias = 0.0f;
            }
            if (!Settings.bOverride_AutoExposureMinBrightness)
            {
                Settings.bOverride_AutoExposureMinBrightness = true;
                Settings.AutoExposureMinBrightness = 10.0f;
            }
            if (!Settings.bOverride_AutoExposureMaxBrightness)
            {
                Settings.bOverride_AutoExposureMaxBrightness = true;
                Settings.AutoExposureMaxBrightness = 12.0f;
            }
            if (!Settings.bOverride_AutoExposureSpeedUp)
            {
                Settings.bOverride_AutoExposureSpeedUp = true;
                Settings.AutoExposureSpeedUp = 3.0f;
            }
            if (!Settings.bOverride_AutoExposureSpeedDown)
            {
                Settings.bOverride_AutoExposureSpeedDown = true;
                Settings.AutoExposureSpeedDown = 1.0f;
            }
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
    UpdateRain();

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
    // Only re-sync this new sensor's own postprocess blendables (rain/dust)
    // with the already-active weather. A full PushWeatherToSky() here
    // respawns BP_Sky_Sphere and reapplies the exposure clamp on every
    // camera sensor spawn, which is redundant (global weather state hasn't
    // changed) and visibly jitters clouds/postprocess each time a client
    // spawns a camera.
    CheckWeatherPostProcessEffects();
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
