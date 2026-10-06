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
#include "Components/SkyAtmosphereComponent.h"
#include "Components/LightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Curves/CurveFloat.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureCube.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "Rendering/SkyAtmosphereCommonData.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"
#include <util/ue-header-guard-end.h>

// Volumetric clouds are lit only by atmosphere sun lights. Index 1 is the
// engine's second slot, so the moon does not take the sun's.
static TAutoConsoleVariable<float> CVarCarlaWeatherSunAltitudeFalloff(
    TEXT("carla.Weather.SunAltitudeFalloff"),
    1.0f,
    TEXT("How much of the physical sin(elevation) falloff to apply to the sun intensity ")
    TEXT("curve. 1 is fully physical, 0 leaves the rig's curve alone."),
    ECVF_Default);

// The renderer's N.L already applies sin(elevation) on the ground, so the
// plain sine counted it twice: 0.3% of midday at +3 degrees.
static TAutoConsoleVariable<float> CVarCarlaWeatherSunFalloffExponent(
    TEXT("carla.Weather.SunFalloffExponent"),
    0.01f,
    TEXT("Power the sin(elevation) sun falloff is raised to. 1 is the plain sine, lower ")
    TEXT("keeps a low sun brighter, 0 disables the falloff."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherSunHorizonFloor(
    TEXT("carla.Weather.SunHorizonFloor"),
    0.05f,
    TEXT("Lower bound on the sin(elevation) falloff factor, as a fraction of the zenith ")
    TEXT("value. 0.004 is a real horizon sun's ~400 lux against ~100000 overhead."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherTransmittanceReleaseDeg(
    TEXT("carla.Weather.TransmittanceReleaseDeg"),
    -6.0f,
    TEXT("Sun altitude, in degrees, by which the transmittance clamp has fully released ")
    TEXT("back to the engine's -90. Between 0 and this it eases off."),
    ECVF_Default);

// The project ships r.DefaultFeature.Bloom=False and only the GoPro profile
// sets bloom, so the viewport sun had no glow. Filled in only when the profile
// does not set it; RGB sensors write their own bloom_intensity.
static TAutoConsoleVariable<float> CVarCarlaWeatherBloomIntensity(
    TEXT("carla.Weather.BloomIntensity"),
    1.0f,
    TEXT("Bloom intensity filled into the sky rig's post-process when the loaded profile ")
    TEXT("does not set one, so the sun glows in the viewport. Negative leaves it alone."),
    ECVF_Default);

// Off by default: the camera profile (carla.PostProcess.Profile) owns the
// exposure, so the viewport and the sensors read the same values. Set it to
// force a bias on the viewport only.
static TAutoConsoleVariable<float> CVarCarlaWeatherExposureBias(
    TEXT("carla.Weather.ExposureBias"),
    -1.0f,
    TEXT("Auto exposure compensation, in EV, forced on the sky rig's post process. ")
    TEXT("Negative leaves the rig's authored value."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherExposureMaxEV(
    TEXT("carla.Weather.ExposureMaxEV"),
    -1.0f,
    TEXT("Upper EV100 bound of the auto exposure range. Negative leaves the rig's value. ")
    TEXT("Above 13 nothing measurably changes."),
    ECVF_Default);

// The rigs saved non-physical GI multipliers (BP_Carla_Sky: sun 0.3 / sky
// light 5; Town10's instance overrides the sun to 3), which flatten shadows
// and differ per map. Physical light is 1. Negative leaves the rig's value.
static TAutoConsoleVariable<float> CVarCarlaWeatherSunIndirectIntensity(
    TEXT("carla.Weather.SunIndirectIntensity"),
    1.0f,
    TEXT("IndirectLightingIntensity forced on the sky rig's sun. Negative leaves the rig's value."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherSkyLightIndirectIntensity(
    TEXT("carla.Weather.SkyLightIndirectIntensity"),
    1.0f,
    TEXT("IndirectLightingIntensity forced on the sky rig's sky light. Negative leaves the rig's value."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherTransmittanceMinElevationDeg(
    TEXT("carla.Weather.TransmittanceMinElevationDeg"),
    0.0f,
    TEXT("Minimum sun elevation, in degrees, used to evaluate the atmospheric tint applied ")
    TEXT("to lit surfaces. The engine default of -90 makes a sunset tint the whole city red. ")
    TEXT("Values above ~5 light the city as if it were day under a night sky."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherDayNightBlendDeg(
    TEXT("carla.Weather.DayNightBlendDeg"),
    8.0f,
    TEXT("Half-width, in degrees of sun altitude, of the band the sun/moon handover is ")
    TEXT("spread over. 0 gives a hard switch at the horizon."),
    ECVF_Default);

// The anti-solar moon would shine from below the horizon across dusk. Held at
// least this high, it acts as a fill light from the dark side of the sky.
static TAutoConsoleVariable<float> CVarCarlaWeatherMoonMinAltitudeDeg(
    TEXT("carla.Weather.MoonMinAltitudeDeg"),
    20.0f,
    TEXT("Lowest altitude, in degrees, the moon is aimed from. Keeps it lighting the city ")
    TEXT("across dusk instead of from below the horizon. 0 is the plain anti-solar point."),
    ECVF_Default);

// The moon's own fade-in, in degrees of sun altitude. Separate from
// DayNightBlendDeg, which also drives NightFactor and the atmosphere flag.
static TAutoConsoleVariable<float> CVarCarlaWeatherMoonFadeStartDeg(
    TEXT("carla.Weather.MoonFadeStartDeg"),
    10.0f,
    TEXT("Sun altitude, in degrees, at which the moon starts to fade in."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherMoonFullAltitudeDeg(
    TEXT("carla.Weather.MoonFullAltitudeDeg"),
    0.0f,
    TEXT("Sun altitude, in degrees, at which the moon reaches full MoonIntensity. ")
    TEXT("Must be below MoonFadeStartDeg; otherwise the moon switches on hard there."),
    ECVF_Default);

// As an atmosphere light the moon gets SkyAtmosphere's flat white disc, which
// read as a second sun at dusk. The textured disc below replaces it.
static TAutoConsoleVariable<bool> CVarCarlaWeatherMoonAtmosphereDisk(
    TEXT("carla.Weather.MoonAtmosphereDisk"),
    false,
    TEXT("Let SkyAtmosphere paint its flat disc for the moon. False hides it; the ")
    TEXT("textured moon disc (carla.Weather.MoonDisc) replaces it."),
    ECVF_Default);

// A plane along the moon light's direction, facing the sky sphere's centre and
// kept inside it. Its materials must NOT be IsSky: the engine would take it for
// the sky dome and paint its "does not cover the screen" debug pattern.
static TAutoConsoleVariable<bool> CVarCarlaWeatherMoonDisc(
    TEXT("carla.Weather.MoonDisc"),
    true,
    TEXT("Draw the textured moon disc along the moon light's direction."),
    ECVF_Default);

// Opaque hides the day sky behind the disc; additive blends with it, but the
// clouds only cover it through the material's approximate cloud fogging.
static TAutoConsoleVariable<bool> CVarCarlaWeatherMoonDiscAdditive(
    TEXT("carla.Weather.MoonDiscAdditive"),
    true,
    TEXT("Draw the moon disc additively over the sky (M_Moon_Additive) instead of as an ")
    TEXT("opaque disc (M_Moon). Additive blends with the day sky; opaque is covered by ")
    TEXT("clouds more exactly."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherMoonDiscContrast(
    TEXT("carla.Weather.MoonDiscContrast"),
    0.7f,
    TEXT("Contrast of the moon texture: 1 as authored, 0 flat grey."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherMoonDiscAngleDeg(
    TEXT("carla.Weather.MoonDiscAngleDeg"),
    2.0f,
    TEXT("Apparent diameter of the moon disc in degrees. The real moon is 0.52; ")
    TEXT("larger reads better on screen."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherMoonDiscDistanceKm(
    TEXT("carla.Weather.MoonDiscDistanceKm"),
    100.0f,
    TEXT("Distance of the moon disc from the sky sphere's centre, in km. Must be beyond ")
    TEXT("the clouds so they cover it. Clamped to MoonDiscSphereFraction of the sphere."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherMoonDiscSphereFraction(
    TEXT("carla.Weather.MoonDiscSphereFraction"),
    0.9f,
    TEXT("Largest distance of the moon disc as a fraction of the sky sphere's radius, ")
    TEXT("so the sphere never draws in front of it."),
    ECVF_Default);

// Narrow and near the horizon: that is where the rig's sun curve collapses and
// exposure opens up, and a disc still at its dusk value blows out.
static TAutoConsoleVariable<float> CVarCarlaWeatherMoonDiscBrightnessStartDeg(
    TEXT("carla.Weather.MoonDiscBrightnessStartDeg"),
    2.0f,
    TEXT("Sun altitude at which the moon disc starts dimming from its dusk brightness."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherMoonDiscBrightnessEndDeg(
    TEXT("carla.Weather.MoonDiscBrightnessEndDeg"),
    -4.0f,
    TEXT("Sun altitude at which the moon disc reaches its night brightness."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherMoonDiscNightBrightness(
    TEXT("carla.Weather.MoonDiscNightBrightness"),
    40.0f,
    TEXT("Emissive brightness of the moon disc at full night. Lower also means less bloom ")
    TEXT("around it, the only per-object bloom control there is."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherMoonDiscDuskBrightness(
    TEXT("carla.Weather.MoonDiscDuskBrightness"),
    3000.0f,
    TEXT("Emissive brightness of the moon disc while the sun is still up, blended to the ")
    TEXT("night value between MoonDiscBrightnessStartDeg and MoonDiscBrightnessEndDeg."),
    ECVF_Default);

// The moon's glow is SkyAtmosphere Mie scattering, not bloom. Mie is shared
// with the sun, so it is only changed from MoonHaloStartDeg down, which also
// dims the sun's own glow just above the horizon. Anisotropy near 1 turns the
// halo blocky: the sky-view LUT cannot resolve that sharp a peak.
static TAutoConsoleVariable<float> CVarCarlaWeatherMoonHaloMieScale(
    TEXT("carla.Weather.MoonHaloMieScale"),
    0.1f,
    TEXT("Factor on the atmosphere's Mie scattering at night, dimming the moon's halo. ")
    TEXT("1 leaves it as the rig sets it."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherMoonHaloMieAnisotropy(
    TEXT("carla.Weather.MoonHaloMieAnisotropy"),
    -1.0f,
    TEXT("Mie anisotropy at night: closer to 1 makes the moon's halo smaller. Negative ")
    TEXT("leaves it as the rig sets it."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherMoonHaloStartDeg(
    TEXT("carla.Weather.MoonHaloStartDeg"),
    3.0f,
    TEXT("Sun altitude at which the night Mie settings (MoonHaloMieScale, ")
    TEXT("MoonHaloMieAnisotropy) start blending in."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherMoonHaloEndDeg(
    TEXT("carla.Weather.MoonHaloEndDeg"),
    0.0f,
    TEXT("Sun altitude at which the night Mie settings are fully applied."),
    ECVF_Default);

// A full moon gives 0.1-0.3 lux. 200 lit a moonlit street ten times brighter
// than one under street lamps. The night clouds come from CityGlow instead.
static TAutoConsoleVariable<float> CVarCarlaWeatherMoonIntensity(
    TEXT("carla.Weather.MoonIntensity"),
    0.5f,
    TEXT("Minimum DirectionalLightComponentMoon intensity (lux) enforced on the sky rig, ")
    TEXT("faded in between MoonFadeStartDeg and MoonFullAltitudeDeg of sun altitude. Set 0 to ")
    TEXT("leave the rig's authored/curve-driven moon intensity untouched."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherNightSkylightIntensity(
    TEXT("carla.Weather.NightSkylightIntensity"),
    10.0f,
    TEXT("Minimum SkyLightComponent intensity enforced on the sky rig whenever ")
    TEXT("SunAltitudeAngle < 0 (same units as the rig's SkyIntensity_Curve). Set 0 to ")
    TEXT("leave the rig's curve-driven skylight intensity untouched."),
    ECVF_Default);

// The rig's sun curve drops from 100000 lux at +1 degree to 5 at -0.5, and the
// sky light curve from 1 to 0 with it, so dusk was a cut to night that auto
// exposure then had to catch up with. Below SunTwilightStartDeg the sun now
// decays log-linearly to SunTwilightEndFraction of its value there, reached at
// SunTwilightEndDeg, and the sky light blends to its night floor over the same
// band. The sky keeps its twilight glow; surfaces stop being lit by the sun
// once TransmittanceReleaseDeg has released. 0 or above restores the cut.
// -10, not the astronomical -18: on camera a city night is dark by the end of
// nautical twilight, and -18 kept most of a night cycle looking like dusk.
static TAutoConsoleVariable<float> CVarCarlaWeatherSunTwilightEndDeg(
    TEXT("carla.Weather.SunTwilightEndDeg"),
    -10.0f,
    TEXT("Sun altitude at which the sun light has faded out below the horizon. ")
    TEXT("0 or above leaves the rig's hard cut at the horizon."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherSunTwilightStartDeg(
    TEXT("carla.Weather.SunTwilightStartDeg"),
    1.0f,
    TEXT("Sun altitude at which the twilight fade starts, from the sun's value there."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherSunTwilightEndFraction(
    TEXT("carla.Weather.SunTwilightEndFraction"),
    0.0001f,
    TEXT("Fraction of the sun's intensity at SunTwilightStartDeg left at SunTwilightEndDeg; ")
    TEXT("the fade is log-linear in between."),
    ECVF_Default);

// The night sky sphere occludes the atmosphere, so showing it at the horizon
// also cut off the twilight glow.
static TAutoConsoleVariable<float> CVarCarlaWeatherSkySphereShowDeg(
    TEXT("carla.Weather.SkySphereShowDeg"),
    -12.0f,
    TEXT("Sun altitude below which the night sky sphere (stars) is shown in front of the ")
    TEXT("atmosphere."),
    ECVF_Default);

// At 16 km the sphere cut off the far volumetric clouds the moment it
// appeared, and its own painted clouds replaced them: the sky visibly jumped.
// Scaled up it sits behind the clouds, and with its painted clouds off the
// night keeps the same volumetric clouds, lit by the moon.
static TAutoConsoleVariable<float> CVarCarlaWeatherSkySphereScale(
    TEXT("carla.Weather.SkySphereScale"),
    10.0f,
    TEXT("Factor on the night sky sphere's size, so it draws behind the volumetric clouds. ")
    TEXT("1 leaves it as authored."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherSkySphereCloudOpacity(
    TEXT("carla.Weather.SkySphereCloudOpacity"),
    0.0f,
    TEXT("\"Cloud opacity\" of the night sky sphere's painted clouds. Negative leaves the ")
    TEXT("asset's value."),
    ECVF_Default);

// With "Colors Determined By Sun Position" on, the sphere's own colour curves
// gave it a dusk sky all night (orange horizon, brown painted clouds), and it
// is switched on at SkySphereShowDeg: the sky jumped from black to dusk there.
// Forced near-black, it matches the atmosphere it hides and only the stars show.
static TAutoConsoleVariable<FString> CVarCarlaWeatherSkySphereNightColor(
    TEXT("carla.Weather.SkySphereNightColor"),
    TEXT("0.002 0.002 0.0025"),
    TEXT("Horizon, zenith and cloud colour of the night sky sphere, \"R G B\". Anything ")
    TEXT("else (e.g. \"off\") leaves the sphere's sun-driven colours."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherStreetLightsOnDeg(
    TEXT("carla.Weather.StreetLightsOnDeg"),
    0.8f,
    TEXT("Sun altitude at or below which street lights are switched on."),
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
// City light pollution scattered by the clouds: emission of the volumetric
// cloud material (WeatherMaterialParameters.CityGlow, M_BasicClouds Emissive)
// times NightFactor. Kept very low and near-grey: on camera, real overcast city
// nights show the clouds as barely visible dark grey (sky L* ~2), not lit
// orange. Per unit of cloud density, weighted to the cloud base in the material
// ((1 - NormAltitudeInLayer)^4): the city lights it from below.
static TAutoConsoleVariable<float> CVarCarlaWeatherCityGlow(
    TEXT("carla.Weather.CityGlow"),
    0.000002f,
    TEXT("Night emission of the clouds from city lights (0 off)."),
    ECVF_Default);

static TAutoConsoleVariable<FString> CVarCarlaWeatherCityGlowColor(
    TEXT("carla.Weather.CityGlowColor"),
    TEXT("1.0 0.85 0.7"),
    TEXT("Colour of the city glow on the clouds, \"R G B\"."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherStarsBrightness(
    TEXT("carla.Weather.StarsBrightness"),
    4.0f,
    TEXT("Value forced into the night sky sphere's \"Stars Brightness\" variable ")
    TEXT("(BP_Sky_Sphere, engine content) whenever SunAltitudeAngle < 0. Set 0 to leave the ")
    TEXT("sphere's authored value alone; any other value replaces it on every push, which is ")
    TEXT("the only way to make the stars FAINTER than the 100 the asset ships."),
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

// Off: the hard swap to the Billowy material at the threshold is a visible
// break (two different rendering techniques), and its thick decks render as
// broken dark cumulus. The plain MI_Clouds master closes into the overcast
// deck continuously instead (see CVarCarlaWeatherDeckStartCloudiness).
static TAutoConsoleVariable<bool> CVarCarlaWeatherEnableOvercastClouds(
    TEXT("carla.Weather.EnableOvercastClouds"),
    false,
    TEXT("Whether Cloudiness at/above OvercastThreshold switches the cloud material to the ")
    TEXT("Billowy overcast one. Set false to always use the plain master instead."),
    ECVF_Default);

// False stops the weather push re-asserting the cloud material.
static TAutoConsoleVariable<float> CVarCarlaWeatherOvercastSunIntensity(
    TEXT("carla.Weather.OvercastSunIntensity"),
    1200.0f,
    TEXT("Sun intensity at Cloudiness 100, blended in from SunIntensity_Curve by Cloudiness ")
    TEXT("and clamped so it can only darken. Negative disables."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherOvercastSkyLightIntensity(
    TEXT("carla.Weather.OvercastSkyLightIntensity"),
    10.0f,
    TEXT("Sky light intensity at Cloudiness 100, blended in from SkyIntensity_Curve by ")
    TEXT("Cloudiness and clamped so it can only brighten. Negative disables."),
    ECVF_Default);

// OvercastSkyLightIntensity multiplies a red sunset capture up to 10x, which
// tints the whole city red, but under a thick deck the ambient is grey. The
// tint of the sun's transmittance relative to the zenith is cancelled, scaled
// by Cloudiness so a clear sunset keeps its red.
static TAutoConsoleVariable<float> CVarCarlaWeatherOvercastAmbientNeutralize(
    TEXT("carla.Weather.OvercastAmbientNeutralize"),
    0.4f,
    TEXT("How much of the sun's atmospheric tint to cancel from the sky light at ")
    TEXT("Cloudiness 100 (scaled by Cloudiness). 0 leaves the sky light white."),
    ECVF_Default);

// The same on the sun itself, so an overcast deck does not glow red.
static TAutoConsoleVariable<float> CVarCarlaWeatherOvercastSunNeutralize(
    TEXT("carla.Weather.OvercastSunNeutralize"),
    0.6f,
    TEXT("How much of the sun's atmospheric tint to cancel from the sun light itself at ")
    TEXT("Cloudiness 100 (scaled by Cloudiness), taking the red out of an overcast deck ")
    TEXT("at sunset. 0 leaves the sun white."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherOvercastAmbientMaxGain(
    TEXT("carla.Weather.OvercastAmbientMaxGain"),
    3.0f,
    TEXT("Largest per-channel gain the sky light neutralisation may apply, so a sun on ")
    TEXT("the horizon cannot turn the ambient blue."),
    ECVF_Default);

// Overcast deck. Real heavy overcast is a continuous grey-white layer, the
// brightest thing in the frame (sky L* ~90 in rain footage, facades and wet
// asphalt well below it). The volumetric clouds alone render it as broken,
// dark, blue-tinted cumulus: the single-scattering + approximate multiple
// scattering model loses most of the light a thick deck diffuses downwards.
// From DeckStartCloudiness to 100 (smoothstep) the plain cloud material closes
// into a continuous layer (BaseNoiseExp -> DeckNoiseExp), gets a softer base
// (ExtinctionScale, domain warp; the authored warp draws one big swirl across
// a closed deck), emits the missing diffuse light (DeckGlow, scaled by the
// sun's intensity) and the sky light boost drops (DeckSkyLightIntensity),
// since the deck itself now feeds the real-time sky light capture.
static TAutoConsoleVariable<float> CVarCarlaWeatherDeckStartCloudiness(
    TEXT("carla.Weather.DeckStartCloudiness"),
    80.0f,
    TEXT("Cloudiness at which the clouds start closing into the overcast deck (full at 100)."),
    ECVF_Default);

// The deck's light (DeckGlow, DeckSkyLightIntensity) ramps in faster than its
// coverage: a deck that has closed but does not glow yet reads as a dark grey
// stage that real overcast does not go through.
static TAutoConsoleVariable<float> CVarCarlaWeatherDeckLightFullCloudiness(
    TEXT("carla.Weather.DeckLightFullCloudiness"),
    92.0f,
    TEXT("Cloudiness at which the deck's emission and sky light reach their full value."),
    ECVF_Default);

// M_BasicClouds' domain warp loop blends its last iteration by the count's
// fraction, so the warp count blends continuously with the deck factor (-1).
// The stock loop truncated the count to an integer (1.99 rendered as 1): a
// value in [0, 1] flips it once at that deck factor instead, for that material.
static TAutoConsoleVariable<float> CVarCarlaWeatherDeckWarpSwitch(
    TEXT("carla.Weather.DeckWarpSwitch"),
    -1.0f,
    TEXT("-1: blend the warp count with the deck factor. 0-1: flip it to DeckWarpCount at that deck factor."),
    ECVF_Default);

// BaseNoiseExp closes the coverage fast at first (a log blend, and the deck's
// glow makes thin cloud read as cloud): the coverage ramp is the deck factor
// to this power, so the gaps close gradually over the upper half of the range.
static TAutoConsoleVariable<float> CVarCarlaWeatherDeckCoverageExponent(
    TEXT("carla.Weather.DeckCoverageExponent"),
    2.0f,
    TEXT("Power applied to the deck factor for the cloud coverage (BaseNoiseExp) blend."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherDeckNoiseExp(
    TEXT("carla.Weather.DeckNoiseExp"),
    1.0f,
    TEXT("Cloud material BaseNoiseExp at Cloudiness 100 (lower = more coverage)."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherDeckExtinctionScale(
    TEXT("carla.Weather.DeckExtinctionScale"),
    0.005f,
    TEXT("Cloud material ExtinctionScale at Cloudiness 100 (softer deck base). Negative leaves the material's."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherDeckWarpCount(
    TEXT("carla.Weather.DeckWarpCount"),
    1.0f,
    TEXT("Cloud material domain warp count at Cloudiness 100. Negative leaves the material's."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherDeckGlow(
    TEXT("carla.Weather.DeckGlow"),
    1.5f,
    TEXT("Cloud emission at Cloudiness 100 with the sun at its peak (0 off)."),
    ECVF_Default);

static TAutoConsoleVariable<FString> CVarCarlaWeatherDeckGlowColor(
    TEXT("carla.Weather.DeckGlowColor"),
    TEXT("1.0 1.0 1.0"),
    TEXT("Colour of the overcast deck emission, \"R G B\"."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherDeckSkyLightIntensity(
    TEXT("carla.Weather.DeckSkyLightIntensity"),
    1.0f,
    TEXT("Replaces OvercastSkyLightIntensity at Cloudiness 100 (blended by the deck factor)."),
    ECVF_Default);

// 0 below DeckStartCloudiness, 1 at Cloudiness 100, smoothstep in between.
static float ComputeCloudDeckFactor(float Cloudiness)
{
    const float Start = FMath::Clamp(CVarCarlaWeatherDeckStartCloudiness.GetValueOnGameThread(), 0.0f, 99.0f);
    const float T = FMath::Clamp((Cloudiness - Start) / (100.0f - Start), 0.0f, 1.0f);
    return T * T * (3.0f - 2.0f * T);
}

// The same from DeckStartCloudiness to DeckLightFullCloudiness.
static float ComputeCloudDeckLightFactor(float Cloudiness)
{
    const float Start = FMath::Clamp(CVarCarlaWeatherDeckStartCloudiness.GetValueOnGameThread(), 0.0f, 99.0f);
    const float Full = FMath::Clamp(CVarCarlaWeatherDeckLightFullCloudiness.GetValueOnGameThread(), Start + 1.0f, 100.0f);
    const float T = FMath::Clamp((Cloudiness - Start) / (Full - Start), 0.0f, 1.0f);
    return T * T * (3.0f - 2.0f * T);
}

// OvercastSkyLightIntensity, giving way to DeckSkyLightIntensity as the deck closes.
static float GetOvercastSkyLightTarget(float Cloudiness)
{
    const float Overcast = CVarCarlaWeatherOvercastSkyLightIntensity.GetValueOnGameThread();
    const float Deck = CVarCarlaWeatherDeckSkyLightIntensity.GetValueOnGameThread();
    if (Overcast < 0.0f || Deck < 0.0f)
        return Overcast;
    return FMath::Lerp(Overcast, Deck, ComputeCloudDeckLightFactor(Cloudiness));
}

static FLinearColor ParseColorCVar(const TAutoConsoleVariable<FString>& CVar, const FLinearColor& Fallback)
{
    TArray<FString> Channels;
    CVar.GetValueOnGameThread().ParseIntoArrayWS(Channels);
    if (Channels.Num() != 3)
        return Fallback;
    return FLinearColor(FCString::Atof(*Channels[0]), FCString::Atof(*Channels[1]), FCString::Atof(*Channels[2]));
}

// bOnlyDarken picks the clamp direction: a ceiling for the sun, a floor for
// the sky light.
static float ComputeNightBlend(float SunAltitudeAngle)
{
    const float Band = CVarCarlaWeatherDayNightBlendDeg.GetValueOnGameThread();
    if (Band <= 0.0f)
        return SunAltitudeAngle < 0.0f ? 1.0f : 0.0f;
    const float T = FMath::Clamp(0.5f - SunAltitudeAngle / (2.0f * Band), 0.0f, 1.0f);
    return T * T * (3.0f - 2.0f * T);   // smoothstep
}

// 0 no moon, 1 full MoonIntensity.
static float ComputeMoonBlend(float SunAltitudeAngle)
{
    const float Start = CVarCarlaWeatherMoonFadeStartDeg.GetValueOnGameThread();
    const float Full = CVarCarlaWeatherMoonFullAltitudeDeg.GetValueOnGameThread();
    if (Start <= Full)
        return SunAltitudeAngle < Full ? 1.0f : 0.0f;
    const float T = FMath::Clamp((Start - SunAltitudeAngle) / (Start - Full), 0.0f, 1.0f);
    return T * T * (3.0f - 2.0f * T);   // smoothstep
}

// 0 day Mie, 1 night Mie.
static float ComputeMoonHaloBlend(float SunAltitudeAngle)
{
    const float Start = CVarCarlaWeatherMoonHaloStartDeg.GetValueOnGameThread();
    const float End = CVarCarlaWeatherMoonHaloEndDeg.GetValueOnGameThread();
    if (Start <= End)
        return SunAltitudeAngle < End ? 1.0f : 0.0f;
    const float T = FMath::Clamp((Start - SunAltitudeAngle) / (Start - End), 0.0f, 1.0f);
    return T * T * (3.0f - 2.0f * T);   // smoothstep
}

// The directional light's VSM cache is keyed on the exact light direction. A
// sun that turns once per tick, while each tick renders several views (the
// viewport and every camera sensor), keeps swapping the cache entry between
// cached and uncached. With a low sun the stale pages show: direct sunlight
// switching on and off in single frames (Town15, sun at +1..+5 degrees, 12 of
// 120 frames; 0 with this). The engine's advice for a light that invalidates
// often but not on every render (VirtualShadowMapCacheManager.cpp) is
// r.Shadow.Virtual.Cache.ForceInvalidateDirectional. It is held on while the
// sun moves and released once the sun has been still for
// VSMSunStillSeconds. It is set by code, so a value set in the console wins.
static TAutoConsoleVariable<bool> CVarCarlaWeatherVSMInvalidateWhileSunMoves(
    TEXT("carla.Weather.VSMInvalidateWhileSunMoves"),
    true,
    TEXT("Hold r.Shadow.Virtual.Cache.ForceInvalidateDirectional on while the sun direction ")
    TEXT("changes. 0 leaves it alone."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherVSMSunStillSeconds(
    TEXT("carla.Weather.VSMSunStillSeconds"),
    1.0f,
    TEXT("World seconds without a sun direction change before the directional VSM cache is ")
    TEXT("used again. World time, not engine frames: in synchronous mode the editor keeps ")
    TEXT("rendering between client ticks, and a frame count released the cvar between two ")
    TEXT("pushes of a moving sun, which brought the flicker back."),
    ECVF_Default);

static FRotator GLastSunRotation(ForceInitToZero);
static double GLastSunMoveWorldSeconds = 0.0;
static bool GForcingDirectionalVSMInvalidate = false;

static void SetForceInvalidateDirectionalVSM(bool bOn)
{
    if (GForcingDirectionalVSMInvalidate == bOn)
        return;
    if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(
            TEXT("r.Shadow.Virtual.Cache.ForceInvalidateDirectional")))
    {
        CVar->Set(bOn ? 1 : 0, ECVF_SetByCode);
        GForcingDirectionalVSMInvalidate = bOn;
        UE_LOG(LogCarla, Log, TEXT("AWeather: directional VSM force-invalidate %s"), bOn ? TEXT("on") : TEXT("off"));
    }
}

static void NoteSunRotation(const UWorld* World, const FRotator& Rotation)
{
    if (!CVarCarlaWeatherVSMInvalidateWhileSunMoves.GetValueOnGameThread())
    {
        SetForceInvalidateDirectionalVSM(false);
        return;
    }
    if (!Rotation.Equals(GLastSunRotation, 1e-4f))
    {
        GLastSunRotation = Rotation;
        GLastSunMoveWorldSeconds = World->GetTimeSeconds();
        SetForceInvalidateDirectionalVSM(true);
    }
}

static void ReleaseDirectionalVSMInvalidateIfSunStill(const UWorld* World)
{
    if (GForcingDirectionalVSMInvalidate && World != nullptr
        && World->GetTimeSeconds() - GLastSunMoveWorldSeconds
            > FMath::Max(CVarCarlaWeatherVSMSunStillSeconds.GetValueOnGameThread(), 0.0f))
        SetForceInvalidateDirectionalVSM(false);
}

// Lumen caches its lighting (radiance cache probes, surface cache) and updates
// it over many frames, nearest first; the sky light's real-time capture is
// time-sliced too. After a jump from day to night the city stayed lit like day
// for 20+ s (Town10, fixed exposure: 24 s, nearest street first, far facades
// last). Held for LightingJumpSettleSeconds of world time after a weather push
// that moves the sun or the clouds by more than the thresholds, the three
// cvars below bring that to ~6 s. Gradual changes (a timeline, a client easing
// the sun) never trigger it. Set by code, so a value set in the console wins.
static TAutoConsoleVariable<float> CVarCarlaWeatherLightingJumpSunDeg(
    TEXT("carla.Weather.LightingJumpSunDeg"),
    5.0f,
    TEXT("Sun altitude change, in degrees, between two weather pushes that counts as a lighting ")
    TEXT("jump and forces Lumen and the sky light capture to update in full. 0 disables it."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherLightingJumpCloudiness(
    TEXT("carla.Weather.LightingJumpCloudiness"),
    30.0f,
    TEXT("Cloudiness change between two weather pushes that counts as a lighting jump. 0 disables it."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherLightingJumpSettleSeconds(
    TEXT("carla.Weather.LightingJumpSettleSeconds"),
    8.0f,
    TEXT("World seconds the full Lumen / sky light updates are held after a lighting jump."),
    ECVF_Default);

static const TCHAR* GLightingJumpCVars[] = {
    TEXT("r.Lumen.RadianceCache.ForceFullUpdate"),
    TEXT("r.LumenScene.Lighting.ForceLightingUpdate"),
    TEXT("r.SkyLight.RealTimeReflectionCapture.TimeSlice")};
static const int32 GLightingJumpValues[] = {1, 1, 0};
static int32 GLightingJumpRestore[UE_ARRAY_COUNT(GLightingJumpCVars)] = {};
static bool GForcingLightingUpdate = false;
static double GLightingJumpWorldSeconds = 0.0;
static float GLastPushedSunAltitude = TNumericLimits<float>::Max();
static float GLastPushedCloudiness = TNumericLimits<float>::Max();

static void SetForceLightingUpdate(bool bOn)
{
    if (GForcingLightingUpdate == bOn)
        return;
    for (int32 i = 0; i < UE_ARRAY_COUNT(GLightingJumpCVars); ++i)
    {
        IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(GLightingJumpCVars[i]);
        if (CVar == nullptr)
            continue;
        if (bOn)
            GLightingJumpRestore[i] = CVar->GetInt();
        CVar->Set(bOn ? GLightingJumpValues[i] : GLightingJumpRestore[i], ECVF_SetByCode);
    }
    GForcingLightingUpdate = bOn;
    UE_LOG(LogCarla, Log, TEXT("AWeather: full Lumen / sky light update %s"), bOn ? TEXT("on") : TEXT("off"));
}

static void NoteLightingChange(const UWorld* World, float SunAltitude, float Cloudiness)
{
    const float SunDeg = CVarCarlaWeatherLightingJumpSunDeg.GetValueOnGameThread();
    const float CloudJump = CVarCarlaWeatherLightingJumpCloudiness.GetValueOnGameThread();
    const bool bFirst = GLastPushedSunAltitude == TNumericLimits<float>::Max();
    const bool bJump = !bFirst && (
        (SunDeg > 0.0f && FMath::Abs(SunAltitude - GLastPushedSunAltitude) > SunDeg) ||
        (CloudJump > 0.0f && FMath::Abs(Cloudiness - GLastPushedCloudiness) > CloudJump));
    GLastPushedSunAltitude = SunAltitude;
    GLastPushedCloudiness = Cloudiness;
    if (bJump)
    {
        GLightingJumpWorldSeconds = World->GetTimeSeconds();
        SetForceLightingUpdate(true);
    }
}

static void ReleaseLightingUpdateIfSettled(const UWorld* World)
{
    if (GForcingLightingUpdate && World != nullptr
        && World->GetTimeSeconds() - GLightingJumpWorldSeconds
            > FMath::Max(CVarCarlaWeatherLightingJumpSettleSeconds.GetValueOnGameThread(), 0.0f))
        SetForceLightingUpdate(false);
}

// 1 at SunTwilightStartDeg and above, decaying to 0 at SunTwilightEndDeg.
// Negative when the twilight fade is disabled.
static float ComputeSunTwilightFactor(float SunAltitudeAngle)
{
    const float End = CVarCarlaWeatherSunTwilightEndDeg.GetValueOnGameThread();
    const float Start = CVarCarlaWeatherSunTwilightStartDeg.GetValueOnGameThread();
    if (End >= 0.0f || Start <= End)
        return -1.0f;
    if (SunAltitudeAngle >= Start)
        return 1.0f;
    if (SunAltitudeAngle <= End)
        return 0.0f;
    const float T = (Start - SunAltitudeAngle) / (Start - End);
    const float EndFraction = FMath::Clamp(
        CVarCarlaWeatherSunTwilightEndFraction.GetValueOnGameThread(), 1e-8f, 1.0f);
    // Eased to zero over the last 3 degrees, so the end fraction does not pop off.
    const float Tail = FMath::SmoothStep(0.0f, 1.0f, FMath::Clamp((SunAltitudeAngle - End) / 3.0f, 0.0f, 1.0f));
    return FMath::Pow(EndFraction, T) * Tail;
}

// Applied on top of what the rig wrote. Tracks its own last write, so a value
// the rig did not rewrite since is not scaled twice.
static void UpdateMoonHalo(AActor* SkyActor, float HaloBlend)
{
    FObjectProperty* AtmosphereProperty = CastField<FObjectProperty>(
        SkyActor->GetClass()->FindPropertyByName(TEXT("SkyAtmosphereComponent")));
    USkyAtmosphereComponent* Atmosphere = AtmosphereProperty != nullptr
        ? Cast<USkyAtmosphereComponent>(AtmosphereProperty->GetObjectPropertyValue_InContainer(SkyActor))
        : nullptr;
    if (Atmosphere == nullptr)
        return;

    static TWeakObjectPtr<USkyAtmosphereComponent> Tracked;
    static float BaseScale = 0.0f, WrittenScale = -1.0f;
    static float BaseAnisotropy = 0.0f, WrittenAnisotropy = -2.0f;
    if (Tracked.Get() != Atmosphere)
    {
        Tracked = Atmosphere;
        WrittenScale = -1.0f;
        WrittenAnisotropy = -2.0f;
    }
    if (Atmosphere->MieScatteringScale != WrittenScale)
        BaseScale = Atmosphere->MieScatteringScale;
    if (Atmosphere->MieAnisotropy != WrittenAnisotropy)
        BaseAnisotropy = Atmosphere->MieAnisotropy;

    const float ScaleFactor = FMath::Lerp(1.0f,
        FMath::Max(CVarCarlaWeatherMoonHaloMieScale.GetValueOnGameThread(), 0.0f), HaloBlend);
    const float NightAnisotropy = CVarCarlaWeatherMoonHaloMieAnisotropy.GetValueOnGameThread();
    const float Anisotropy = NightAnisotropy >= 0.0f
        ? FMath::Lerp(BaseAnisotropy, FMath::Min(NightAnisotropy, 0.999f), HaloBlend)
        : BaseAnisotropy;

    WrittenScale = BaseScale * ScaleFactor;
    WrittenAnisotropy = Anisotropy;
    if (Atmosphere->MieScatteringScale != WrittenScale)
        Atmosphere->SetMieScatteringScale(WrittenScale);
    if (Atmosphere->MieAnisotropy != WrittenAnisotropy)
        Atmosphere->SetMieAnisotropy(WrittenAnisotropy);
}

// A transient component on the sky rig, looked up by name every push and
// recreated if a re-instancing of the rig dropped it.
static void UpdateMoonDisc(AActor* SkyActor, const USceneComponent* MoonLight,
                           float MoonBlend, float SunAltitudeAngle)
{
    static const FName DiscName(TEXT("CarlaMoonDisc"));
    UStaticMeshComponent* Disc = FindObjectFast<UStaticMeshComponent>(SkyActor, DiscName);
    if (!CVarCarlaWeatherMoonDisc.GetValueOnGameThread() || MoonLight == nullptr || MoonBlend <= 0.0f)
    {
        if (Disc != nullptr && Disc->IsVisible())
            Disc->SetVisibility(false);
        return;
    }

    const TCHAR* MaterialPath = CVarCarlaWeatherMoonDiscAdditive.GetValueOnGameThread()
        ? TEXT("/Game/Carla/Blueprints/Weather/Materials/M_Moon_Additive.M_Moon_Additive")
        : TEXT("/Game/Carla/Blueprints/Weather/Materials/M_Moon.M_Moon");
    UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, MaterialPath);
    if (Material == nullptr)
    {
        static bool bWarned = false;
        if (!bWarned)
            UE_LOG(LogCarla, Warning, TEXT("AWeather: moon disc disabled, missing %s"), MaterialPath);
        bWarned = true;
        if (Disc != nullptr && Disc->IsVisible())
            Disc->SetVisibility(false);
        return;
    }

    if (Disc == nullptr)
    {
        UStaticMesh* Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
        if (Plane == nullptr)
        {
            static bool bWarned = false;
            if (!bWarned)
                UE_LOG(LogCarla, Warning, TEXT("AWeather: moon disc disabled, missing /Engine/BasicShapes/Plane"));
            bWarned = true;
            return;
        }
        Disc = NewObject<UStaticMeshComponent>(SkyActor, DiscName, RF_Transient);
        Disc->SetStaticMesh(Plane);
        Disc->SetMobility(EComponentMobility::Movable);
        Disc->SetUsingAbsoluteLocation(true);
        Disc->SetUsingAbsoluteRotation(true);
        Disc->SetUsingAbsoluteScale(true);
        Disc->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Disc->SetCastShadow(false);
        Disc->bAffectDistanceFieldLighting = false;
        Disc->bVisibleInReflectionCaptures = false;
        Disc->bVisibleInRealTimeSkyCaptures = false;
        Disc->bVisibleInRayTracing = false;
        Disc->SetupAttachment(SkyActor->GetRootComponent());
        Disc->RegisterComponent();
    }
    else if (!Disc->IsRegistered())
    {
        Disc->RegisterComponent();
    }

    // The light shines along its forward vector, so the moon sits opposite.
    const FVector ToMoon = -MoonLight->GetForwardVector();

    // The sky sphere is the biggest mesh among the rig's child actors.
    FVector Centre = FVector::ZeroVector;
    float SphereRadius = 0.0f;
    TInlineComponentArray<UChildActorComponent*> RigChildActors;
    SkyActor->GetComponents(RigChildActors);
    for (UChildActorComponent* RigChild : RigChildActors)
    {
        if (RigChild == nullptr || RigChild->GetChildActor() == nullptr)
            continue;
        TInlineComponentArray<UMeshComponent*> ChildMeshes;
        RigChild->GetChildActor()->GetComponents(ChildMeshes);
        for (UMeshComponent* ChildMesh : ChildMeshes)
        {
            if (ChildMesh != nullptr && ChildMesh->Bounds.SphereRadius > SphereRadius)
            {
                SphereRadius = ChildMesh->Bounds.SphereRadius;
                Centre = ChildMesh->Bounds.Origin;
            }
        }
    }
    float Distance = FMath::Max(CVarCarlaWeatherMoonDiscDistanceKm.GetValueOnGameThread(), 0.01f) * 100000.0f;
    if (SphereRadius > 0.0f)
        Distance = FMath::Min(Distance,
            SphereRadius * FMath::Clamp(CVarCarlaWeatherMoonDiscSphereFraction.GetValueOnGameThread(), 0.01f, 1.0f));
    static float LastLoggedDistance = -1.0f;
    if (Distance != LastLoggedDistance)
    {
        UE_LOG(LogCarla, Log, TEXT("AWeather moon disc: sky sphere radius %.0f cm, disc distance %.0f cm"),
            SphereRadius, Distance);
        LastLoggedDistance = Distance;
    }
    const float Angle = FMath::Clamp(CVarCarlaWeatherMoonDiscAngleDeg.GetValueOnGameThread(), 0.01f, 45.0f);
    // BasicShapes/Plane is 100 units across and faces +Z.
    const float Scale = 2.0f * Distance * FMath::Tan(FMath::DegreesToRadians(Angle) * 0.5f) / 100.0f;
    Disc->SetWorldLocationAndRotation(
        Centre + ToMoon * Distance, FRotationMatrix::MakeFromZX(-ToMoon, FVector::UpVector).Rotator());
    Disc->SetWorldScale3D(FVector(Scale));

    // Swapped in place when MoonDiscAdditive changes.
    UMaterialInstanceDynamic* MoonMaterial = Cast<UMaterialInstanceDynamic>(Disc->GetMaterial(0));
    if (MoonMaterial == nullptr || MoonMaterial->Parent != Material)
    {
        MoonMaterial = UMaterialInstanceDynamic::Create(Material, Disc);
        Disc->SetMaterial(0, MoonMaterial);
    }
    if (MoonMaterial != nullptr)
    {
        MoonMaterial->SetScalarParameterValue(TEXT("Contrast"),
            FMath::Clamp(CVarCarlaWeatherMoonDiscContrast.GetValueOnGameThread(), 0.0f, 2.0f));
        const float Start = CVarCarlaWeatherMoonDiscBrightnessStartDeg.GetValueOnGameThread();
        const float End = CVarCarlaWeatherMoonDiscBrightnessEndDeg.GetValueOnGameThread();
        const float T = Start > End
            ? FMath::SmoothStep(0.0f, 1.0f, FMath::Clamp((Start - SunAltitudeAngle) / (Start - End), 0.0f, 1.0f))
            : (SunAltitudeAngle < End ? 1.0f : 0.0f);
        MoonMaterial->SetScalarParameterValue(TEXT("Brightness"), FMath::Lerp(
            CVarCarlaWeatherMoonDiscDuskBrightness.GetValueOnGameThread(),
            CVarCarlaWeatherMoonDiscNightBrightness.GetValueOnGameThread(),
            T));
    }
    if (!Disc->IsVisible())
        Disc->SetVisibility(true);
}

// Applied before the overcast blend, so its peak stays the curve's own.
static float ApplySunAltitudeFalloff(float CurveValue, float CurvePeak, float SunAltitudeAngle)
{
    const float Strength = FMath::Clamp(
        CVarCarlaWeatherSunAltitudeFalloff.GetValueOnGameThread(), 0.0f, 1.0f);
    if (Strength <= 0.0f || CurvePeak <= UE_KINDA_SMALL_NUMBER)
        return CurveValue;

    // Both fractions are of the curve's peak, not of its value at this altitude.
    const float SinElevation =
        FMath::Sin(FMath::DegreesToRadians(FMath::Max(SunAltitudeAngle, 0.0f)));
    const float Exponent = FMath::Max(CVarCarlaWeatherSunFalloffExponent.GetValueOnGameThread(), 0.0f);
    const float Physical = CurvePeak * FMath::Max(
        FMath::Pow(SinElevation, Exponent), CVarCarlaWeatherSunHorizonFloor.GetValueOnGameThread());

    // Min so it only darkens; below the horizon the curve governs.
    return FMath::Min(CurveValue, FMath::Lerp(CurveValue, Physical, Strength));
}

// The curve's peak above the horizon; the overcast targets are quoted at it.
static float SampleCurvePeakAboveHorizon(UCurveFloat* Curve)
{
    float Peak = 0.0f;
    if (Curve != nullptr)
        for (float Altitude = 0.0f; Altitude <= 90.0f; Altitude += 5.0f)
            Peak = FMath::Max(Peak, Curve->GetFloatValue(Altitude));
    return Peak;
}

// CurvePeak > 0 treats OvercastValue as a factor on the curve, 0 as an absolute.
static float ApplyOvercastBlend(float CurveValue, float OvercastValue,
                                float Cloudiness, bool bOnlyDarken, float CurvePeak)
{
    if (OvercastValue < 0.0f)
        return CurveValue;
    const float Overcast = FMath::Clamp(Cloudiness / 100.0f, 0.0f, 1.0f);
    const float Blended = CurvePeak > UE_KINDA_SMALL_NUMBER
        ? CurveValue * FMath::Lerp(1.0f, OvercastValue / CurvePeak, Overcast)
        : FMath::Lerp(CurveValue, OvercastValue, Overcast);
    return bOnlyDarken ? FMath::Min(CurveValue, Blended) : FMath::Max(CurveValue, Blended);
}

// White when there is nothing to cancel, so writing it every push also undoes
// an earlier tint. The sky light stores an 8-bit colour, so the tint comes
// back scaled to a peak of 1 and OutIntensityScale carries the difference.
static FLinearColor ComputeOvercastNeutralTint(const USkyAtmosphereComponent* Atmosphere,
                                               float SunAltitudeAngle, float Cloudiness,
                                               float NeutralizeAmount, float& OutIntensityScale)
{
    OutIntensityScale = 1.0f;
    const float Release = CVarCarlaWeatherTransmittanceReleaseDeg.GetValueOnGameThread();
    const float DayFade = Release < 0.0f
        ? 1.0f - FMath::SmoothStep(0.0f, 1.0f, FMath::Clamp(SunAltitudeAngle / Release, 0.0f, 1.0f))
        : (SunAltitudeAngle >= 0.0f ? 1.0f : 0.0f);
    const float Strength = FMath::Clamp(NeutralizeAmount, 0.0f, 1.0f)
        * FMath::Clamp(Cloudiness / 100.0f, 0.0f, 1.0f) * DayFade;
    if (Atmosphere == nullptr || Strength <= 0.0f)
        return FLinearColor::White;

    // Held at the horizon or above: a ray through the ground comes back black.
    FAtmosphereSetup Setup(*Atmosphere);
    Setup.TransmittanceMinLightElevationAngle = 0.0f;
    const float Elevation = FMath::DegreesToRadians(FMath::Max(SunAltitudeAngle, 0.0f));
    const FLinearColor TSun = Setup.GetTransmittanceAtGroundLevel(
        FVector(FMath::Cos(Elevation), 0.0f, FMath::Sin(Elevation)));
    // Relative to the zenith, or every overcast midday would turn blue.
    const FLinearColor TZenith = Setup.GetTransmittanceAtGroundLevel(FVector::UpVector);
    const FLinearColor T(
        TSun.R / FMath::Max(TZenith.R, UE_KINDA_SMALL_NUMBER),
        TSun.G / FMath::Max(TZenith.G, UE_KINDA_SMALL_NUMBER),
        TSun.B / FMath::Max(TZenith.B, UE_KINDA_SMALL_NUMBER));
    const float Luminance = T.GetLuminance();
    if (Luminance <= UE_KINDA_SMALL_NUMBER)
        return FLinearColor::White;

    const float MaxGain = FMath::Max(CVarCarlaWeatherOvercastAmbientMaxGain.GetValueOnGameThread(), 1.0f);
    FLinearColor Gain(
        FMath::Min(Luminance / FMath::Max(T.R, UE_KINDA_SMALL_NUMBER), MaxGain),
        FMath::Min(Luminance / FMath::Max(T.G, UE_KINDA_SMALL_NUMBER), MaxGain),
        FMath::Min(Luminance / FMath::Max(T.B, UE_KINDA_SMALL_NUMBER), MaxGain),
        1.0f);
    const float GainLuminance = Gain.GetLuminance();
    if (GainLuminance > UE_KINDA_SMALL_NUMBER)
        Gain = Gain * (1.0f / GainLuminance);
    Gain.A = 1.0f;
    FLinearColor Tint = FMath::Lerp(FLinearColor::White, Gain, Strength);
    const float Peak = FMath::Max3(Tint.R, Tint.G, Tint.B);
    if (Peak > 1.0f)
    {
        Tint = Tint * (1.0f / Peak);
        OutIntensityScale = Peak;
    }
    Tint.A = 1.0f;
    return Tint;
}

static TAutoConsoleVariable<float> CVarCarlaWeatherFogDensityScale(
    TEXT("carla.Weather.FogDensityScale"),
    0.001f,
    TEXT("Multiplier taking CARLA's 0-100 FogDensity to the height fog component's density. ")
    TEXT("The shipped 0.001 caps the engine at 0.1, too thin for a heavy-rain veil."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaWeatherCloudShadowExtentKm(
    TEXT("carla.Weather.CloudShadowExtentKm"),
    10.0f,
    TEXT("Radius in km covered by the volumetric cloud shadow map, set on the sky rig's sun ")
    TEXT("light. Smaller means finer shadow map texels and less stepping as the sun rotates, ")
    TEXT("at the cost of clouds no longer shadowing anything further away. 0 or less leaves ")
    TEXT("whatever the rig authored."),
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

    auto UpdateEmitter = [this, Rain](UParticleSystemComponent* Emitter, const FVector& Location, float Yaw)
    {
        if (!Emitter) return;
        // PS_Rain's scaled spawn box is X[-1000,4000], Y[-2000,2000] cm. Turned with
        // the view so it reaches 40 m ahead of it: left axis-aligned it rained on
        // one side of the camera only, and the drops looked parked in one place.
        // World-space, vertically falling drops.
        const FRotator Heading(0.0f, Yaw, 0.0f);
        const FVector Origin = Location + FVector(0.0f, 0.0f, 400.0f);
        Emitter->SetWorldRotation(Heading);
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
        UpdateEmitter(ViewportRain, Camera->GetCameraLocation(), Camera->GetCameraRotation().Yaw);
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
        UpdateEmitter(Emitter, Sensor->GetCaptureComponent2D()->GetComponentLocation(),
            Sensor->GetCaptureComponent2D()->GetComponentRotation().Yaw);
    }
}

void AWeather::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateRain();
    ReleaseDirectionalVSMInvalidateIfSunStill(GetWorld());
    ReleaseLightingUpdateIfSettled(GetWorld());
}

void AWeather::BeginPlay()
{
    Super::BeginPlay();
    // BP_CarlaWeather is saved with "Start with Tick Enabled" off, which left
    // UpdateRain dead: no rain volume followed the spectator or the sensors.
    SetActorTickEnabled(true);
}

void AWeather::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    SetForceInvalidateDirectionalVSM(false);
    SetForceLightingUpdate(false);
    GLastPushedSunAltitude = TNumericLimits<float>::Max();
    GLastPushedCloudiness = TNumericLimits<float>::Max();
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

            // The sphere's painted sun follows its "Directional Light Actor", which is
            // unset here, so the parameters are written directly. The sphere is reached
            // through the rig's child actors.
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
                    // Written on the blueprint variable as well as the material, since
                    // RefreshMaterial pushes the variable into the material.
                    if (FNumericProperty* SunHeightProperty = CastField<FNumericProperty>(
                            FindPropertyByAuthoredName(Candidate->GetClass(), TEXT("Sun Height"))))
                    {
                        SunHeightProperty->SetFloatingPointPropertyValue(
                            SunHeightProperty->ContainerPtrToValuePtr<void>(Candidate),
                            SunDirection.Z);
                    }

                    TInlineComponentArray<UMeshComponent*> SphereMeshes;
                    Candidate->GetComponents(SphereMeshes);
                    for (UMeshComponent* SphereMesh : SphereMeshes)
                    {
                        if (SphereMesh == nullptr)
                            continue;

                        // The sphere occludes the atmosphere, so it is only shown at night.
                        const float ShowBelow = FMath::Min(
                            CVarCarlaWeatherSkySphereShowDeg.GetValueOnGameThread(), 0.0f);
                        const bool bShouldBeVisible = Weather.SunAltitudeAngle < ShowBelow;
                        if (SphereMesh->IsVisible() != bShouldBeVisible)
                            SphereMesh->SetVisibility(bShouldBeVisible);

                        // Tagged once scaled: a push that did not respawn the sphere
                        // must not compound the scale.
                        static const FName ScaledTag(TEXT("CarlaSkySphereScaled"));
                        const float SphereScale = CVarCarlaWeatherSkySphereScale.GetValueOnGameThread();
                        if (SphereScale > 0.0f && SphereScale != 1.0f && !SphereMesh->ComponentHasTag(ScaledTag))
                        {
                            SphereMesh->SetRelativeScale3D(SphereMesh->GetRelativeScale3D() * SphereScale);
                            SphereMesh->ComponentTags.Add(ScaledTag);
                        }

                        // Reuses the existing dynamic instance; unknown parameters are a no-op.
                        if (UMaterialInstanceDynamic* SphereMID =
                                SphereMesh->CreateAndSetMaterialInstanceDynamic(0))
                        {
                            SphereMID->SetVectorParameterValue(
                                TEXT("Light direction"), FLinearColor(SunDirection));
                            SphereMID->SetScalarParameterValue(TEXT("Sun height"), SunDirection.Z);
                            const float CloudOpacity = CVarCarlaWeatherSkySphereCloudOpacity.GetValueOnGameThread();
                            if (CloudOpacity >= 0.0f)
                                SphereMID->SetScalarParameterValue(TEXT("Cloud opacity"), CloudOpacity);
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

            // Their owning components are re-added on every push and recreate the actors,
            // so the components go too.
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

        {
            const float SunIndirect = CVarCarlaWeatherSunIndirectIntensity.GetValueOnGameThread();
            ULightComponent* Sun = FindComponent(TEXT("DirectionalLightComponentSun"));
            if (Sun != nullptr && SunIndirect >= 0.0f && Sun->IndirectLightingIntensity != SunIndirect)
                Sun->SetIndirectLightingIntensity(SunIndirect);
            const float SkyIndirect = CVarCarlaWeatherSkyLightIndirectIntensity.GetValueOnGameThread();
            USkyLightComponent* SkyLight = FindSkyLightComponent(TEXT("SkyLightComponent"));
            if (SkyLight != nullptr && SkyIndirect >= 0.0f && SkyLight->IndirectLightingIntensity != SkyIndirect)
                SkyLight->SetIndirectLightingIntensity(SkyIndirect);
        }

        if (UCurveFloat* SunIntensityCurve = FindCurve(TEXT("SunIntensity_Curve")))
        {
            if (ULightComponent* SunLightComponent = FindComponent(TEXT("DirectionalLightComponentSun")))
            {
                SunLightComponent->SetIntensity(ApplyOvercastBlend(
                    ApplySunAltitudeFalloff(
                        SunIntensityCurve->GetFloatValue(Weather.SunAltitudeAngle),
                        SampleCurvePeakAboveHorizon(SunIntensityCurve),
                        Weather.SunAltitudeAngle),
                    CVarCarlaWeatherOvercastSunIntensity.GetValueOnGameThread(),
                    Weather.Cloudiness, /*bOnlyDarken=*/true,
                    SampleCurvePeakAboveHorizon(SunIntensityCurve)));
                // See the comment on CVarCarlaWeatherSunTwilightEndDeg.
                const float Twilight = ComputeSunTwilightFactor(Weather.SunAltitudeAngle);
                if (Twilight >= 0.0f && Twilight < 1.0f)
                {
                    const float StartAltitude = CVarCarlaWeatherSunTwilightStartDeg.GetValueOnGameThread();
                    const float AtStart = ApplyOvercastBlend(
                        ApplySunAltitudeFalloff(
                            SunIntensityCurve->GetFloatValue(StartAltitude),
                            SampleCurvePeakAboveHorizon(SunIntensityCurve),
                            StartAltitude),
                        CVarCarlaWeatherOvercastSunIntensity.GetValueOnGameThread(),
                        Weather.Cloudiness, /*bOnlyDarken=*/true,
                        SampleCurvePeakAboveHorizon(SunIntensityCurve));
                    SunLightComponent->SetIntensity(FMath::Max(SunLightComponent->Intensity, AtStart * Twilight));
                    // The rig's UpdateSun hides the sun below the horizon.
                    if (Twilight > 0.0f && !SunLightComponent->IsVisible())
                        SunLightComponent->SetVisibility(true);
                }
                // Rigs saved with a black light color render no sunlight at any
                // intensity; the physical tint comes from the color temperature.
                // White unless the overcast neutralisation has something to cancel.
                FObjectProperty* SunAtmosphereProperty = CastField<FObjectProperty>(
                    SkyActor->GetClass()->FindPropertyByName(TEXT("SkyAtmosphereComponent")));
                const USkyAtmosphereComponent* SunAtmosphere = SunAtmosphereProperty != nullptr
                    ? Cast<USkyAtmosphereComponent>(SunAtmosphereProperty->GetObjectPropertyValue_InContainer(SkyActor))
                    : nullptr;
                float SunIntensityScale = 1.0f;
                SunLightComponent->SetLightColor(ComputeOvercastNeutralTint(
                    SunAtmosphere, Weather.SunAltitudeAngle, Weather.Cloudiness,
                    CVarCarlaWeatherOvercastSunNeutralize.GetValueOnGameThread(), SunIntensityScale));
                if (SunIntensityScale != 1.0f)
                    SunLightComponent->SetIntensity(SunLightComponent->Intensity * SunIntensityScale);
            }
        }
        // UpdateSun never applies a rotation. Pitch is negated because a
        // directional light shines along its forward vector.
        if (ULightComponent* SunLightComponent = FindComponent(TEXT("DirectionalLightComponentSun")))
        {
            const FRotator SunRotation(-Weather.SunAltitudeAngle, Weather.SunAzimuthAngle, 0.0f);
            SunLightComponent->SetWorldRotation(SunRotation);
            if (SkyActor->GetWorld() != nullptr && SkyActor->GetWorld()->IsGameWorld())
            {
                NoteSunRotation(SkyActor->GetWorld(), SunRotation);
                NoteLightingChange(SkyActor->GetWorld(), Weather.SunAltitudeAngle, Weather.Cloudiness);
            }
        }

        // The moon likewise, anti-solar; pitch negated as for the sun. Held at
        // MoonMinAltitudeDeg or above.
        if (ULightComponent* MoonLightComponent = FindComponent(TEXT("DirectionalLightComponentMoon")))
        {
            const float MoonMinAltitude = FMath::Max(
                CVarCarlaWeatherMoonMinAltitudeDeg.GetValueOnGameThread(), 0.0f);
            const float MoonAltitude = FMath::Max(-Weather.SunAltitudeAngle, MoonMinAltitude);
            MoonLightComponent->SetWorldRotation(
                FRotator(-MoonAltitude, Weather.SunAzimuthAngle + 180.0f, 0.0f));
        }

        // Set on every push: a push re-instances the components from their archetype.
        if (UDirectionalLightComponent* MoonAtmosphereLight =
                Cast<UDirectionalLightComponent>(FindComponent(TEXT("DirectionalLightComponentMoon"))))
        {
            const bool bWanted = ComputeNightBlend(Weather.SunAltitudeAngle) > 0.0f;
            // The setters dirty the render state themselves.
            if (MoonAtmosphereLight->AtmosphereSunLightIndex != 1)
                MoonAtmosphereLight->SetAtmosphereSunLightIndex(1);
            if (MoonAtmosphereLight->bAtmosphereSunLight != (bWanted ? 1u : 0u))
                MoonAtmosphereLight->SetAtmosphereSunLight(bWanted);
            const FLinearColor DiskScale = CVarCarlaWeatherMoonAtmosphereDisk.GetValueOnGameThread()
                ? FLinearColor::White : FLinearColor::Black;
            if (MoonAtmosphereLight->AtmosphereSunDiskColorScale != DiskScale)
                MoonAtmosphereLight->SetAtmosphereSunDiskColorScale(DiskScale);
        }

        // Bounds the cloud shadow map's 512 texels to a radius that keeps them small
        // enough that a turning sun does not walk cloud density across them.
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
        // -- into the capture. The displayed sky is the legacy sphere, which the
        // capture cannot see, and the rig's SkyAtmosphere is inactive, so the
        // capture rendered a black cubemap and no ambient at any intensity.
        // Activating the atmosphere gives the capture something to see; the
        // main view is unchanged, still showing the opaque sphere in front.
        //
        // Activation is irrelevant for the cloud component: the proxy is added
        // from ShouldComponentAddToScene() && ShouldRender() && IsRegistered(),
        // never from IsActive(). Visibility is the lever.
        {
            FObjectProperty* AtmosphereProperty = CastField<FObjectProperty>(
                SkyActor->GetClass()->FindPropertyByName(TEXT("SkyAtmosphereComponent")));
            UActorComponent* AtmosphereComponent = AtmosphereProperty != nullptr
                ? Cast<UActorComponent>(AtmosphereProperty->GetObjectPropertyValue_InContainer(SkyActor))
                : nullptr;
            if (AtmosphereComponent != nullptr && !AtmosphereComponent->IsActive())
                AtmosphereComponent->SetActive(true);

            // Set every push: a push re-instances the component from its archetype.
            if (USkyAtmosphereComponent* Atmosphere = Cast<USkyAtmosphereComponent>(AtmosphereComponent))
            {
                // Released below the horizon, where the atmosphere's light is the moon.
                const float Release = CVarCarlaWeatherTransmittanceReleaseDeg.GetValueOnGameThread();
                const float Clamped = CVarCarlaWeatherTransmittanceMinElevationDeg.GetValueOnGameThread();
                float MinElevation = Clamped;
                if (Release < 0.0f)
                {
                    const float T = FMath::Clamp(Weather.SunAltitudeAngle / Release, 0.0f, 1.0f);
                    const float Eased = T * T * (3.0f - 2.0f * T);   // smoothstep
                    MinElevation = FMath::Lerp(Clamped, -90.0f, Eased);
                }
                if (!FMath::IsNearlyEqual(Atmosphere->TransmittanceMinLightElevationAngle, MinElevation))
                    Atmosphere->SetTransmittanceMinLightElevationAngle(MinElevation);
            }
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
                SkyLightComponent->SetIntensity(ApplyOvercastBlend(
                    SkyIntensityCurve->GetFloatValue(Weather.SunAltitudeAngle),
                    GetOvercastSkyLightTarget(Weather.Cloudiness),
                    Weather.Cloudiness, /*bOnlyDarken=*/false, /*CurvePeak=*/0.0f));
            FObjectProperty* AtmosphereProperty = CastField<FObjectProperty>(
                SkyActor->GetClass()->FindPropertyByName(TEXT("SkyAtmosphereComponent")));
            const USkyAtmosphereComponent* Atmosphere = AtmosphereProperty != nullptr
                ? Cast<USkyAtmosphereComponent>(AtmosphereProperty->GetObjectPropertyValue_InContainer(SkyActor))
                : nullptr;
            float AmbientIntensityScale = 1.0f;
            SkyLightComponent->SetLightColor(ComputeOvercastNeutralTint(
                Atmosphere, Weather.SunAltitudeAngle, Weather.Cloudiness,
                CVarCarlaWeatherOvercastAmbientNeutralize.GetValueOnGameThread(), AmbientIntensityScale));
            if (AmbientIntensityScale != 1.0f)
                SkyLightComponent->SetIntensity(SkyLightComponent->Intensity * AmbientIntensityScale);
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
        // The moon fades in on its own curve, starting above the horizon.
        const float MoonBlend = ComputeMoonBlend(Weather.SunAltitudeAngle);
        if (MoonBlend > 0.0f)
        {
            const float MoonFloor = CVarCarlaWeatherMoonIntensity.GetValueOnGameThread() * MoonBlend;
            if (ULightComponent* MoonLightComponent = FindComponent(TEXT("DirectionalLightComponentMoon")))
            {
                if (!MoonLightComponent->IsActive())
                    MoonLightComponent->SetActive(true);
                // The rig's UpdateMoon hides the moon while the sun is up.
                if (!MoonLightComponent->IsVisible())
                    MoonLightComponent->SetVisibility(true);
                // Set outright by day: nothing else lowers it as the sun climbs back.
                if (Weather.SunAltitudeAngle >= 0.0f)
                    MoonLightComponent->SetIntensity(MoonFloor);
                else if (MoonFloor > 0.0f && MoonLightComponent->Intensity < MoonFloor)
                    MoonLightComponent->SetIntensity(MoonFloor);
            }
        }
        UpdateMoonDisc(SkyActor, FindComponent(TEXT("DirectionalLightComponentMoon")),
            MoonBlend, Weather.SunAltitudeAngle);
        UpdateMoonHalo(SkyActor, ComputeMoonHaloBlend(Weather.SunAltitudeAngle));

        if (Weather.SunAltitudeAngle < 0.0f)
        {

            const float SkylightFloor = CVarCarlaWeatherNightSkylightIntensity.GetValueOnGameThread();
            // Not while an environment map is set: its intensity is the
            // caller's measurement, the night floor would override it.
            if (USkyLightComponent* SkyLightComponent = GSkyLightMapOverride.Cubemap == nullptr
                ? FindSkyLightComponent(TEXT("SkyLightComponent")) : nullptr)
            {
                if (!SkyLightComponent->IsActive())
                    SkyLightComponent->SetActive(true);
                // Across twilight the capture still sees a lit sky, so the floor
                // is blended in from the day value instead of applied at once.
                float Floor = SkylightFloor;
                const float TwilightEnd = CVarCarlaWeatherSunTwilightEndDeg.GetValueOnGameThread();
                if (TwilightEnd < 0.0f && Weather.SunAltitudeAngle > TwilightEnd && SkylightFloor > 0.0f)
                {
                    // From what the curve and the overcast blend give at the horizon.
                    const float DayValue = FMath::Max(ApplyOvercastBlend(1.0f,
                        GetOvercastSkyLightTarget(Weather.Cloudiness),
                        Weather.Cloudiness, /*bOnlyDarken=*/false, /*CurvePeak=*/0.0f), 1e-3f);
                    const float U = Weather.SunAltitudeAngle / TwilightEnd;
                    Floor = FMath::Pow(DayValue, 1.0f - U) * FMath::Pow(SkylightFloor, U);
                }
                if (Floor > 0.0f && SkyLightComponent->Intensity < Floor)
                    SkyLightComponent->SetIntensity(Floor);
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

            // The property is unset on this rig, so the sphere is reached through the
            // child actors instead.
            if (SphereActor == nullptr)
            {
                TInlineComponentArray<UChildActorComponent*> RigChildActors;
                SkyActor->GetComponents(RigChildActors);
                for (UChildActorComponent* RigChild : RigChildActors)
                {
                    AActor* Candidate = RigChild != nullptr ? RigChild->GetChildActor() : nullptr;
                    if (Candidate != nullptr &&
                        FindPropertyByAuthoredName(Candidate->GetClass(), TEXT("Stars Brightness")) != nullptr)
                    {
                        SphereActor = Candidate;
                        break;
                    }
                }
            }
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
                    // Sets rather than floors, so the value can go below the sphere's authored one.
                    if (StarsFloor > 0.0f)
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

                // After RefreshMaterial, which rewrites the colours and the cloud
                // opacity from the sphere's variables.
                const FLinearColor NightColor = ParseColorCVar(CVarCarlaWeatherSkySphereNightColor, FLinearColor(-1.0f, -1.0f, -1.0f));
                const float CloudOpacity = CVarCarlaWeatherSkySphereCloudOpacity.GetValueOnGameThread();
                TInlineComponentArray<UMeshComponent*> SphereMeshes;
                SphereActor->GetComponents(SphereMeshes);
                for (UMeshComponent* SphereMesh : SphereMeshes)
                {
                    UMaterialInstanceDynamic* SphereMID = SphereMesh != nullptr
                        ? Cast<UMaterialInstanceDynamic>(SphereMesh->GetMaterial(0)) : nullptr;
                    if (SphereMID == nullptr)
                        continue;
                    if (NightColor.R >= 0.0f)
                    {
                        SphereMID->SetVectorParameterValue(TEXT("Horizon color"), NightColor);
                        SphereMID->SetVectorParameterValue(TEXT("Zenith Color"), NightColor);
                        SphereMID->SetVectorParameterValue(TEXT("Cloud color"), NightColor);
                    }
                    if (CloudOpacity >= 0.0f)
                        SphereMID->SetScalarParameterValue(TEXT("Cloud opacity"), CloudOpacity);
                }
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
            // Only outside the moon's fade-in, where the floor above owns it.
            ULightComponent* MoonLightComponent = MoonBlend <= 0.0f
                ? FindComponent(TEXT("DirectionalLightComponentMoon")) : nullptr;
            if (MoonLightComponent != nullptr)
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
                    float BaseNoiseExp = Weather.Cloudiness <= 0.0f
                        ? 6000.0f
                        : FMath::Clamp(100.0f - 0.8f * Weather.Cloudiness, 0.0f, 6000.0f);
                    // See CVarCarlaWeatherDeckStartCloudiness. Every deck
                    // parameter is written on every push, blended from the
                    // base material's own value, so the reused MID never
                    // keeps a deck value after the clouds open up again.
                    const float Deck = ComputeCloudDeckFactor(Weather.Cloudiness);
                    const float DeckNoiseExp = CVarCarlaWeatherDeckNoiseExp.GetValueOnGameThread();
                    if (DeckNoiseExp > 0.0f && BaseNoiseExp > 0.0f)
                        BaseNoiseExp = FMath::Exp(FMath::Lerp(FMath::Loge(BaseNoiseExp), FMath::Loge(DeckNoiseExp),
                            FMath::Pow(Deck, FMath::Max(CVarCarlaWeatherDeckCoverageExponent.GetValueOnGameThread(), 0.01f))));
                    CloudMID->SetScalarParameterValue(TEXT("BaseNoiseExp"), BaseNoiseExp);
                    // Negative deck value: the authored one, still written so a
                    // runtime change of the cvar does not leave a stale value.
                    auto BlendToDeck = [CloudMID, BaseMaterial](const TCHAR* Name, float DeckValue, float Alpha)
                    {
                        float Authored = 0.0f;
                        if (BaseMaterial->GetScalarParameterValue(Name, Authored))
                            CloudMID->SetScalarParameterValue(Name, DeckValue >= 0.0f ? FMath::Lerp(Authored, DeckValue, Alpha) : Authored);
                    };
                    BlendToDeck(TEXT("ExtinctionScale"), CVarCarlaWeatherDeckExtinctionScale.GetValueOnGameThread(), Deck);
                    // M_BasicClouds blends the last warp iteration by the
                    // count's fraction, so the warp blends like the rest. A
                    // DeckWarpSwitch in [0, 1] restores the single flip.
                    const float WarpSwitch = CVarCarlaWeatherDeckWarpSwitch.GetValueOnGameThread();
                    BlendToDeck(TEXT("Perlin FBM Domain Warp Count"), CVarCarlaWeatherDeckWarpCount.GetValueOnGameThread(),
                        WarpSwitch < 0.0f ? Deck : (Deck >= WarpSwitch && Deck > 0.0f ? 1.0f : 0.0f));
                }
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
            CarlaLightSubsystem->NotifyDayTimeChange(
                Weather.SunAltitudeAngle > CVarCarlaWeatherStreetLightsOnDeg.GetValueOnGameThread());
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
            // 0 by day, 1 by night. Materials gate night-only emissives on it.
            const float NightFactor = ComputeNightBlend(Weather.SunAltitudeAngle);
            UKismetMaterialLibrary::SetScalarParameterValue(
                SkyActor->GetWorld(), WeatherMPC, TEXT("NightFactor"), NightFactor);
            // Both cloud emissions share the material input: the city glow at
            // night (see CVarCarlaWeatherCityGlow) and the overcast deck's
            // diffuse light by day (see CVarCarlaWeatherDeckStartCloudiness),
            // which follows the sun's own intensity so it fades with it.
            const FLinearColor CityGlow = ParseColorCVar(CVarCarlaWeatherCityGlowColor, FLinearColor(1.0f, 0.85f, 0.7f))
                * (CVarCarlaWeatherCityGlow.GetValueOnGameThread() * NightFactor);
            float SunFraction = 0.0f;
            if (UCurveFloat* SunCurve = LoadObject<UCurveFloat>(nullptr,
                    TEXT("/Game/Carla/Blueprints/Weather/Weather2_Curves/SunIntensity_2.SunIntensity_2")))
            {
                const float Peak = SampleCurvePeakAboveHorizon(SunCurve);
                if (Peak > UE_KINDA_SMALL_NUMBER)
                    SunFraction = FMath::Clamp(ApplySunAltitudeFalloff(
                        SunCurve->GetFloatValue(Weather.SunAltitudeAngle), Peak, Weather.SunAltitudeAngle) / Peak, 0.0f, 1.0f);
            }
            const FLinearColor DeckGlow = ParseColorCVar(CVarCarlaWeatherDeckGlowColor, FLinearColor::White)
                * (FMath::Max(CVarCarlaWeatherDeckGlow.GetValueOnGameThread(), 0.0f)
                    * ComputeCloudDeckLightFactor(Weather.Cloudiness) * SunFraction);
            UKismetMaterialLibrary::SetVectorParameterValue(
                SkyActor->GetWorld(), WeatherMPC, TEXT("CityGlow"), CityGlow + DeckGlow);
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
            FillSkyPostProcessFallback(PostProcessComponent);
        }
    }
}

void AWeather::FillSkyPostProcessFallback(UPostProcessComponent* PostProcessComponent)
{
    if (PostProcessComponent == nullptr)
        return;
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
    // Only when the cvar is set (off by default): forced over the profile.
    const float ExposureBias = CVarCarlaWeatherExposureBias.GetValueOnGameThread();
    if (ExposureBias >= 0.0f)
    {
        Settings.bOverride_AutoExposureBias = true;
        Settings.AutoExposureBias = ExposureBias;
    }
    // Only when the profile does not set bloom. The sky rig remembers that the
    // bloom is ours (until a profile is loaded), so the cvar stays live.
    ASkyBase* Sky = Cast<ASkyBase>(PostProcessComponent->GetOwner());
    const bool bBloomIsOurs = Sky != nullptr && Sky->bBloomFromWeatherFallback;
    const float BloomIntensity = CVarCarlaWeatherBloomIntensity.GetValueOnGameThread();
    if (BloomIntensity >= 0.0f && (!Settings.bOverride_BloomIntensity || bBloomIsOurs))
    {
        Settings.bOverride_BloomIntensity = true;
        Settings.BloomIntensity = BloomIntensity;
        if (Sky != nullptr)
            Sky->bBloomFromWeatherFallback = true;
    }
    else if (BloomIntensity < 0.0f && bBloomIsOurs)
    {
        // Cvar switched off: hand bloom back to the engine default.
        Settings.bOverride_BloomIntensity = false;
        Sky->bBloomFromWeatherFallback = false;
    }
    const float ExposureMaxEV = CVarCarlaWeatherExposureMaxEV.GetValueOnGameThread();
    if (ExposureMaxEV >= 0.0f)
    {
        Settings.bOverride_AutoExposureMaxBrightness = true;
        Settings.AutoExposureMaxBrightness = ExposureMaxEV;
    }
    PostProcessComponent->bUnbound = true;
}

void AWeather::UpdateStreetLightsForDayNight()
{
    if (!DayNightCycle)
        return;

    UCarlaLightSubsystem* CarlaLightSubsystem = GetWorld()->GetSubsystem<UCarlaLightSubsystem>();
    if (CarlaLightSubsystem == nullptr)
        return;

    // Sun above StreetLightsOnDeg = day, at or below = night. Broadcasting on every
    // weather update is harmless: registered lights just re-receive the same
    // state when nothing changed.
    const bool bIsDay = Weather.SunAltitudeAngle > CVarCarlaWeatherStreetLightsOnDeg.GetValueOnGameThread();
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
