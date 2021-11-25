// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/LidarCaptureSensor.h"

#include "Carla/Game/CarlaStatics.h"

#include "Components/DrawFrustumComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HighResScreenshot.h"
#include "ContentStreaming.h"

// =============================================================================
// -- Local static methods -----------------------------------------------------
// =============================================================================
static auto LIDAR_CAPTURE_COUNTER = 0u;
// Local namespace to avoid name collisions on unit builds.
namespace LidarCaptureSensor_local_ns {

    static void SetCameraDefaultOverrides(USceneCaptureComponent2D &CaptureComponent2D);

    static void ConfigureShowFlags(FEngineShowFlags &ShowFlags, bool bPostProcessing = true);

    static auto GetQualitySettings(UWorld *World)
    {
        auto Settings = UCarlaStatics::GetCarlaSettings(World);
        check(Settings != nullptr);
        return Settings->GetQualityLevel();
    }

} // namespace LidarCaptureSensor_local_ns

// =============================================================================
// -- ALidarCaptureSensor ------------------------------------------------------
// =============================================================================

ALidarCaptureSensor::ALidarCaptureSensor(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics; // After CameraManager's TG_PrePhysics.
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		CaptureRenderTarget[i] = CreateDefaultSubobject<UTextureRenderTarget2D>(
			FName(*FString::Printf(TEXT("CaptureRenderTarget_d%d"), LIDAR_CAPTURE_COUNTER)));
		CaptureRenderTarget[i]->CompressionSettings = TextureCompressionSettings::TC_Default;
		CaptureRenderTarget[i]->SRGB = false;
		CaptureRenderTarget[i]->bAutoGenerateMips = false;
		CaptureRenderTarget[i]->AddressX = TextureAddress::TA_Clamp;
		CaptureRenderTarget[i]->AddressY = TextureAddress::TA_Clamp;
		CaptureComponent2D[i] = CreateDefaultSubobject<USceneCaptureComponent2D>(
			FName(*FString::Printf(TEXT("SceneCaptureComponent2D_%d"), LIDAR_CAPTURE_COUNTER)));
		CaptureComponent2D[i]->SetupAttachment(RootComponent);
		LidarCaptureSensor_local_ns::SetCameraDefaultOverrides(*CaptureComponent2D[i]);
		LIDAR_CAPTURE_COUNTER++;
	}
	NowCaptureId = 0;
	LidarSpeedMode = true;
}

void ALidarCaptureSensor::Set(const FActorDescription &Description)
{
    Super::Set(Description);
    UActorBlueprintFunctionLibrary::SetLidarCamera(Description, this);
	for(int i = 0; i < MaxCaptureCnt; ++i){
		CaptureComponent2D[i]->SetRelativeRotation(FRotator(0, i * 360.0 / MaxCaptureCnt, 0));
	}
}

void ALidarCaptureSensor::SetImageSize(uint32 InWidth, uint32 InHeight)
{
    ImageWidth = InWidth;
    ImageHeight = InHeight;
}

void ALidarCaptureSensor::SetFOVAngle(const float FOVAngle)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->FOVAngle = FOVAngle;
	}
}

float ALidarCaptureSensor::GetFOVAngle() const
{
	check(CaptureComponent2D[0] != nullptr);
	return CaptureComponent2D[0]->FOVAngle;
}

void ALidarCaptureSensor::SetExposureMethod(EAutoExposureMethod Method)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.AutoExposureMethod = Method;
	}
}

EAutoExposureMethod ALidarCaptureSensor::GetExposureMethod() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.AutoExposureMethod;
}

void ALidarCaptureSensor::SetExposureCompensation(float Compensation)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.AutoExposureBias = Compensation;
	}
}

float ALidarCaptureSensor::GetExposureCompensation() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.AutoExposureBias;
}

void ALidarCaptureSensor::SetShutterSpeed(float Speed)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.CameraShutterSpeed = Speed;
	}
}

float ALidarCaptureSensor::GetShutterSpeed() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.CameraShutterSpeed;
}

void ALidarCaptureSensor::SetISO(float ISO)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.CameraISO = ISO;
	}
}

float ALidarCaptureSensor::GetISO() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.CameraISO;
}

void ALidarCaptureSensor::SetAperture(float Aperture)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.DepthOfFieldFstop = Aperture;
	}
}

float ALidarCaptureSensor::GetAperture() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.DepthOfFieldFstop;
}

void ALidarCaptureSensor::SetFocalDistance(float Distance)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.DepthOfFieldFocalDistance = Distance;
	}
}

float ALidarCaptureSensor::GetFocalDistance() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.DepthOfFieldFocalDistance;
}

void ALidarCaptureSensor::SetDepthBlurAmount(float Amount)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.DepthOfFieldDepthBlurAmount = Amount;
	}
}

float ALidarCaptureSensor::GetDepthBlurAmount() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.DepthOfFieldDepthBlurAmount;
}

void ALidarCaptureSensor::SetDepthBlurRadius(float Radius)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.DepthOfFieldDepthBlurRadius = Radius;
	}
}

float ALidarCaptureSensor::GetDepthBlurRadius() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.DepthOfFieldDepthBlurRadius;
}

void ALidarCaptureSensor::SetDepthOfFieldMinFstop(float MinFstop)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.DepthOfFieldMinFstop = MinFstop;
	}
}

float ALidarCaptureSensor::GetDepthOfFieldMinFstop() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.DepthOfFieldMinFstop;
}

void ALidarCaptureSensor::SetBladeCount(int Count)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.DepthOfFieldBladeCount = Count;
	}
}

int ALidarCaptureSensor::GetBladeCount() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.DepthOfFieldBladeCount;
}

void ALidarCaptureSensor::SetFilmSlope(float Slope)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.FilmSlope = Slope;
	}
}

float ALidarCaptureSensor::GetFilmSlope() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.FilmSlope;
}

void ALidarCaptureSensor::SetFilmToe(float Toe)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.FilmToe = Toe; // FilmToeAmount?
	}
}

float ALidarCaptureSensor::GetFilmToe() const
{
	check(CaptureComponent2D[0] != nullptr);
	return CaptureComponent2D[0]->PostProcessSettings.FilmToe;
}

void ALidarCaptureSensor::SetFilmShoulder(float Shoulder)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.FilmShoulder = Shoulder;
	}
}

float ALidarCaptureSensor::GetFilmShoulder() const
{
	check(CaptureComponent2D[0] != nullptr);
	return CaptureComponent2D[0]->PostProcessSettings.FilmShoulder;
}

void ALidarCaptureSensor::SetFilmBlackClip(float BlackClip)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.FilmBlackClip = BlackClip;
	}
}

float ALidarCaptureSensor::GetFilmBlackClip() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.FilmBlackClip;
}

void ALidarCaptureSensor::SetFilmWhiteClip(float WhiteClip)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.FilmWhiteClip = WhiteClip;
	}
}

float ALidarCaptureSensor::GetFilmWhiteClip() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.FilmWhiteClip;
}

void ALidarCaptureSensor::SetExposureMinBrightness(float Brightness)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.AutoExposureMinBrightness = Brightness;
	}
}

float ALidarCaptureSensor::GetExposureMinBrightness() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.AutoExposureMinBrightness;
}

void ALidarCaptureSensor::SetExposureMaxBrightness(float Brightness)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.AutoExposureMaxBrightness = Brightness;
	}
}

float ALidarCaptureSensor::GetExposureMaxBrightness() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.AutoExposureMaxBrightness;
}

void ALidarCaptureSensor::SetExposureSpeedDown(float Speed)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.AutoExposureSpeedDown = Speed;
	}
}

float ALidarCaptureSensor::GetExposureSpeedDown() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.AutoExposureSpeedDown;
}

void ALidarCaptureSensor::SetExposureSpeedUp(float Speed)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.AutoExposureSpeedUp = Speed;
	}
}

float ALidarCaptureSensor::GetExposureSpeedUp() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.AutoExposureSpeedUp;
}

void ALidarCaptureSensor::SetExposureCalibrationConstant(float Constant)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.AutoExposureCalibrationConstant = Constant;
	}
}

float ALidarCaptureSensor::GetExposureCalibrationConstant() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.AutoExposureCalibrationConstant;
}

void ALidarCaptureSensor::SetMotionBlurIntensity(float Intensity)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.MotionBlurAmount = Intensity;
	}
}

float ALidarCaptureSensor::GetMotionBlurIntensity() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.MotionBlurAmount;
}

void ALidarCaptureSensor::SetMotionBlurMaxDistortion(float MaxDistortion)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.MotionBlurMax = MaxDistortion;
	}
}

float ALidarCaptureSensor::GetMotionBlurMaxDistortion() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.MotionBlurMax;
}

void ALidarCaptureSensor::SetMotionBlurMinObjectScreenSize(float ScreenSize)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.MotionBlurPerObjectSize = ScreenSize;
	}
}

float ALidarCaptureSensor::GetMotionBlurMinObjectScreenSize() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.MotionBlurPerObjectSize;
}

void ALidarCaptureSensor::SetWhiteTemp(float Temp)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.WhiteTemp = Temp;
	}
}

float ALidarCaptureSensor::GetWhiteTemp() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.WhiteTemp;
}

void ALidarCaptureSensor::SetWhiteTint(float Tint)
{
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		check(CaptureComponent2D[i] != nullptr);
		CaptureComponent2D[i]->PostProcessSettings.WhiteTint = Tint;
	}
}

float ALidarCaptureSensor::GetWhiteTint() const
{
    check(CaptureComponent2D[0] != nullptr);
    return CaptureComponent2D[0]->PostProcessSettings.WhiteTint;
}

void ALidarCaptureSensor::BeginPlay()
{
    using namespace LidarCaptureSensor_local_ns;

    // Setup render target.

    // Determine the gamma of the player.

    const bool bInForceLinearGamma = !bEnablePostProcessingEffects;
	for (int i = 0; i < MaxCaptureCnt; ++i) {
		CaptureRenderTarget[i]->InitCustomFormat(ImageWidth, ImageHeight, PF_B8G8R8A8, bInForceLinearGamma);
		if (bEnablePostProcessingEffects)
		{
			CaptureRenderTarget[i]->TargetGamma = TargetGamma;
		}

		check(IsValid(CaptureComponent2D[i]) && !CaptureComponent2D[i]->IsPendingKill());

		CaptureComponent2D[i]->Deactivate();
		CaptureComponent2D[i]->TextureTarget = CaptureRenderTarget[i];

		// Call derived classes to set up their things.
		SetUpSceneCaptureComponent(*CaptureComponent2D[i]);

		if (bEnablePostProcessingEffects &&
			(LidarCaptureSensor_local_ns::GetQualitySettings(GetWorld()) == EQualityLevel::Low))
		{
			CaptureComponent2D[i]->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;
		}
		else
		{
			// LDR is faster than HDR (smaller bitmap array).
			CaptureComponent2D[i]->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		}

		CaptureComponent2D[i]->UpdateContent();
		CaptureComponent2D[i]->Activate();

		// Make sure that there is enough time in the render queue.
		

		LidarCaptureSensor_local_ns::ConfigureShowFlags(CaptureComponent2D[i]->ShowFlags,
			bEnablePostProcessingEffects);
	}
	//CaptureComponent2D[0]->Activate();
	UKismetSystemLibrary::ExecuteConsoleCommand(
		GetWorld(),
		FString("g.TimeoutForBlockOnRenderFence 300000"));
    Super::BeginPlay();
}

void ALidarCaptureSensor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Add the view information every tick. Its only used for one tick and then
    // removed by the streamer.
	/*
	IStreamingManager::Get().AddViewInformation(
		CaptureComponent2D[NowCaptureId]->GetComponentLocation(),
		ImageWidth,
		ImageWidth / FMath::Tan(CaptureComponent2D[NowCaptureId]->FOVAngle));
	
	*/
	if (LidarSpeedMode) {
		for (int i = 0; i < MaxCaptureCnt; ++i) {
			CaptureComponent2D[i]->Deactivate();
		}
		NowCaptureId = (NowCaptureId + 1) % MaxCaptureCnt;
		CaptureComponent2D[NowCaptureId]->UpdateContent();
	}
	/*for (int i = 0; i < MaxCaptureCnt; ++i) {
		if (i != NowCaptureId) {
			CaptureComponent2D[i]->SetComponentTickEnabled(false);
		}
		else {
			CaptureComponent2D[i]->SetComponentTickEnabled(true);
		}
	}
	*/
}

void ALidarCaptureSensor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    LIDAR_CAPTURE_COUNTER = 0u;
}
/*
void ALidarCaptureSensor::CaptureScene()
{
	CaptureComponent2D[NowCaptureId]->CaptureScene();
}
*/
// =============================================================================
// -- Local static functions implementations -----------------------------------
// =============================================================================

namespace LidarCaptureSensor_local_ns {

    static void SetCameraDefaultOverrides(USceneCaptureComponent2D &CaptureComponent2D)
    {
        auto &PostProcessSettings = CaptureComponent2D.PostProcessSettings;

        // Depth of field
        PostProcessSettings.bOverride_DepthOfFieldMethod = true;
        PostProcessSettings.DepthOfFieldMethod = EDepthOfFieldMethod::DOFM_CircleDOF;
        PostProcessSettings.bOverride_DepthOfFieldFocalDistance = true;
        PostProcessSettings.bOverride_DepthOfFieldDepthBlurAmount = true;
        PostProcessSettings.bOverride_DepthOfFieldDepthBlurRadius = true;

        // Exposure
        PostProcessSettings.bOverride_AutoExposureMethod = true;
        PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
        PostProcessSettings.bOverride_AutoExposureBias = true;
        PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
        PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
        PostProcessSettings.bOverride_AutoExposureSpeedUp = true;
        PostProcessSettings.bOverride_AutoExposureSpeedDown = true;
        PostProcessSettings.bOverride_AutoExposureCalibrationConstant = true;

        // Camera
        PostProcessSettings.bOverride_CameraShutterSpeed = true;
        PostProcessSettings.bOverride_CameraISO = true;
        PostProcessSettings.bOverride_DepthOfFieldFstop = true;
        PostProcessSettings.bOverride_DepthOfFieldMinFstop = true;
        PostProcessSettings.bOverride_DepthOfFieldBladeCount = true;

        // Film (Tonemapper)
        PostProcessSettings.bOverride_FilmSlope = true;
        PostProcessSettings.bOverride_FilmToe = true;
        PostProcessSettings.bOverride_FilmShoulder = true;
        PostProcessSettings.bOverride_FilmWhiteClip = true;
        PostProcessSettings.bOverride_FilmBlackClip = true;

        // Motion blur
        PostProcessSettings.bOverride_MotionBlurAmount = true;
        PostProcessSettings.MotionBlurAmount = 0.45f;
        PostProcessSettings.bOverride_MotionBlurMax = true;
        PostProcessSettings.MotionBlurMax = 0.35f;
        PostProcessSettings.bOverride_MotionBlurPerObjectSize = true;
        PostProcessSettings.MotionBlurPerObjectSize = 0.1f;

        // Color Grading
        PostProcessSettings.bOverride_WhiteTemp = true;
        PostProcessSettings.bOverride_WhiteTint = true;
    }

    // Remove the show flags that might interfere with post-processing effects
    // like depth and semantic segmentation.
    static void ConfigureShowFlags(FEngineShowFlags &ShowFlags, bool bPostProcessing)
    {
        if (bPostProcessing)
        {
            ShowFlags.EnableAdvancedFeatures();
            ShowFlags.SetMotionBlur(true);
            return;
        }

        ShowFlags.SetAmbientOcclusion(false);
        ShowFlags.SetAntiAliasing(false);
        ShowFlags.SetAtmosphericFog(false);
        // ShowFlags.SetAudioRadius(false);
        // ShowFlags.SetBillboardSprites(false);
        ShowFlags.SetBloom(false);
        // ShowFlags.SetBounds(false);
        // ShowFlags.SetBrushes(false);
        // ShowFlags.SetBSP(false);
        // ShowFlags.SetBSPSplit(false);
        // ShowFlags.SetBSPTriangles(false);
        // ShowFlags.SetBuilderBrush(false);
        // ShowFlags.SetCameraAspectRatioBars(false);
        // ShowFlags.SetCameraFrustums(false);
        ShowFlags.SetCameraImperfections(false);
        ShowFlags.SetCameraInterpolation(false);
        // ShowFlags.SetCameraSafeFrames(false);
        // ShowFlags.SetCollision(false);
        // ShowFlags.SetCollisionPawn(false);
        // ShowFlags.SetCollisionVisibility(false);
        ShowFlags.SetColorGrading(false);
        // ShowFlags.SetCompositeEditorPrimitives(false);
        // ShowFlags.SetConstraints(false);
        // ShowFlags.SetCover(false);
        // ShowFlags.SetDebugAI(false);
        // ShowFlags.SetDecals(false);
        // ShowFlags.SetDeferredLighting(false);
        ShowFlags.SetDepthOfField(false);
        ShowFlags.SetDiffuse(false);
        ShowFlags.SetDirectionalLights(false);
        ShowFlags.SetDirectLighting(false);
        // ShowFlags.SetDistanceCulledPrimitives(false);
        // ShowFlags.SetDistanceFieldAO(false);
        // ShowFlags.SetDistanceFieldGI(false);
        ShowFlags.SetDynamicShadows(false);
        // ShowFlags.SetEditor(false);
        ShowFlags.SetEyeAdaptation(false);
        ShowFlags.SetFog(false);
        // ShowFlags.SetGame(false);
        // ShowFlags.SetGameplayDebug(false);
        // ShowFlags.SetGBufferHints(false);
        ShowFlags.SetGlobalIllumination(false);
        ShowFlags.SetGrain(false);
        // ShowFlags.SetGrid(false);
        // ShowFlags.SetHighResScreenshotMask(false);
        // ShowFlags.SetHitProxies(false);
        ShowFlags.SetHLODColoration(false);
        ShowFlags.SetHMDDistortion(false);
        // ShowFlags.SetIndirectLightingCache(false);
        // ShowFlags.SetInstancedFoliage(false);
        // ShowFlags.SetInstancedGrass(false);
        // ShowFlags.SetInstancedStaticMeshes(false);
        // ShowFlags.SetLandscape(false);
        // ShowFlags.SetLargeVertices(false);
        ShowFlags.SetLensFlares(false);
        ShowFlags.SetLevelColoration(false);
        ShowFlags.SetLightComplexity(false);
        ShowFlags.SetLightFunctions(false);
        ShowFlags.SetLightInfluences(false);
        ShowFlags.SetLighting(false);
        ShowFlags.SetLightMapDensity(false);
        ShowFlags.SetLightRadius(false);
        ShowFlags.SetLightShafts(false);
        // ShowFlags.SetLOD(false);
        ShowFlags.SetLODColoration(false);
        // ShowFlags.SetMaterials(false);
        // ShowFlags.SetMaterialTextureScaleAccuracy(false);
        // ShowFlags.SetMeshEdges(false);
        // ShowFlags.SetMeshUVDensityAccuracy(false);
        // ShowFlags.SetModeWidgets(false);
        ShowFlags.SetMotionBlur(false);
        // ShowFlags.SetNavigation(false);
        ShowFlags.SetOnScreenDebug(false);
        // ShowFlags.SetOutputMaterialTextureScales(false);
        // ShowFlags.SetOverrideDiffuseAndSpecular(false);
        // ShowFlags.SetPaper2DSprites(false);
        ShowFlags.SetParticles(false);
        // ShowFlags.SetPivot(false);
        ShowFlags.SetPointLights(false);
        // ShowFlags.SetPostProcessing(false);
        // ShowFlags.SetPostProcessMaterial(false);
        // ShowFlags.SetPrecomputedVisibility(false);
        // ShowFlags.SetPrecomputedVisibilityCells(false);
        // ShowFlags.SetPreviewShadowsIndicator(false);
        // ShowFlags.SetPrimitiveDistanceAccuracy(false);
        ShowFlags.SetPropertyColoration(false);
        // ShowFlags.SetQuadOverdraw(false);
        // ShowFlags.SetReflectionEnvironment(false);
        // ShowFlags.SetReflectionOverride(false);
        ShowFlags.SetRefraction(false);
        // ShowFlags.SetRendering(false);
        ShowFlags.SetSceneColorFringe(false);
        // ShowFlags.SetScreenPercentage(false);
        ShowFlags.SetScreenSpaceAO(false);
        ShowFlags.SetScreenSpaceReflections(false);
        // ShowFlags.SetSelection(false);
        // ShowFlags.SetSelectionOutline(false);
        // ShowFlags.SetSeparateTranslucency(false);
        // ShowFlags.SetShaderComplexity(false);
        // ShowFlags.SetShaderComplexityWithQuadOverdraw(false);
        // ShowFlags.SetShadowFrustums(false);
        // ShowFlags.SetSkeletalMeshes(false);
        // ShowFlags.SetSkinCache(false);
        ShowFlags.SetSkyLighting(false);
        // ShowFlags.SetSnap(false);
        // ShowFlags.SetSpecular(false);
        // ShowFlags.SetSplines(false);
        ShowFlags.SetSpotLights(false);
        // ShowFlags.SetStaticMeshes(false);
        ShowFlags.SetStationaryLightOverlap(false);
        // ShowFlags.SetStereoRendering(false);
        // ShowFlags.SetStreamingBounds(false);
        ShowFlags.SetSubsurfaceScattering(false);
        // ShowFlags.SetTemporalAA(false);
        // ShowFlags.SetTessellation(false);
        // ShowFlags.SetTestImage(false);
        // ShowFlags.SetTextRender(false);
        // ShowFlags.SetTexturedLightProfiles(false);
        ShowFlags.SetTonemapper(false);
        // ShowFlags.SetTranslucency(false);
        // ShowFlags.SetVectorFields(false);
        // ShowFlags.SetVertexColors(false);
        // ShowFlags.SetVignette(false);
        // ShowFlags.SetVisLog(false);
        ShowFlags.SetVisualizeAdaptiveDOF(false);
        ShowFlags.SetVisualizeBloom(false);
        ShowFlags.SetVisualizeBuffer(false);
        ShowFlags.SetVisualizeDistanceFieldAO(false);
        ShowFlags.SetVisualizeDistanceFieldGI(false);
        ShowFlags.SetVisualizeDOF(false);
        ShowFlags.SetVisualizeHDR(false);
        ShowFlags.SetVisualizeLightCulling(false);
        ShowFlags.SetVisualizeLPV(false);
        ShowFlags.SetVisualizeMeshDistanceFields(false);
        ShowFlags.SetVisualizeMotionBlur(false);
        ShowFlags.SetVisualizeOutOfBoundsPixels(false);
        ShowFlags.SetVisualizeSenses(false);
        ShowFlags.SetVisualizeShadingModels(false);
        ShowFlags.SetVisualizeSSR(false);
        ShowFlags.SetVisualizeSSS(false);
        // ShowFlags.SetVolumeLightingSamples(false);
        // ShowFlags.SetVolumes(false);
        // ShowFlags.SetWidgetComponents(false);
        // ShowFlags.SetWireframe(false);
    }

} // namespace LidarCaptureSensor_local_ns
