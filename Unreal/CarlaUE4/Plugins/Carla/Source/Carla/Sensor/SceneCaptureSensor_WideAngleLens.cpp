// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "SceneCaptureSensor_WideAngleLens.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "HAL/IConsoleManager.h"

// extern bool CARLA_API GCARLASelectiveRendering;
// extern bool CARLA_API GCARLALightingOnly;

static TAutoConsoleVariable<int32> CVarWideAngleSensorDumpAllFrames(
    TEXT("Carla.WideAngleLens.DumpAllFrames"),
    0,
    TEXT("If enabled, saves all frames of all *_WideAngleLens sensors to disk.\n")
    TEXT("0: Disabled\n")
    TEXT("1: Enabled\n"));

static TAutoConsoleVariable<FString> CVarWideAngleSensorDumpAllFramesPath(
    TEXT("Carla.WideAngleLens.DumpAllFramesPath"),
    TEXT(""),
    TEXT("Sets the destination path when saving frames via \"Carla.WideAngleLens.DumpAllFrames\".\n"));

static TAutoConsoleVariable<int32> CVarWideAngleSensorDumpAllFramesCubemap(
    TEXT("Carla.WideAngleLens.DumpAllFrames.Cubemap"),
    0,
    TEXT("If enabled, saves each individual face of each *_WideAngleLens sensor.\n")
    TEXT("0: Disabled\n")
    TEXT("1: Enabled\n"));

static TAutoConsoleVariable<int32> CVarWideAngleSensorSkipVFTR(
    TEXT("Carla.WideAngleLens.SkipVFTR"),
    0,
    TEXT("If enabled, *_WideAngleLens sensors do not toggle r.VolumetricFog.TemporalReprojection when rendering.\n")
    TEXT("0: Disabled\n")
    TEXT("1: Enabled\n"));

static auto WIDE_ANGLE_LENS_SENSOR_COUNTER_COUNTER = 0u;

// =============================================================================
// -- Local static methods -----------------------------------------------------
// =============================================================================

// Local namespace to avoid name collisions on unit builds.
namespace SceneCaptureSensorWideAngleLens_local_ns {

    static void SetCameraDefaultOverrides(USceneCaptureComponent2D_CARLA& CaptureComponent);

    static void ConfigureShowFlags(FEngineShowFlags& ShowFlags, bool bPostProcessing = true);

    static auto GetQualitySettings(UWorld* World)
    {
        auto Settings = UCarlaStatics::GetCarlaSettings(World);
        check(Settings != nullptr);
        return Settings->GetQualityLevel();
    }

} // namespace SceneCaptureSensorWideAngleLens_local_ns

// =============================================================================
// -- ASceneCaptureSensor_WideAngleLens -----------------------------------------
// =============================================================================

ASceneCaptureSensor_WideAngleLens::ASceneCaptureSensor_WideAngleLens(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    FaceCaptures(),
    FaceRenderTargets(),
    CaptureRenderTarget(),
    TargetGamma(0.0F),
    ImageWidth(1280U),
    ImageHeight(1280U),
    CameraModel(ECameraModel::Default),
    KannalaBrandtCameraCoefficients
    {
        0.08309221636708493F,
        0.01112126630599195F,
        0.008587261043925865F,
        0.0008542188930970716F
    },
    YFOVAngle(PI * 0.5F),
    XFOVAngle(VerticalToHorizontal(YFOVAngle)),
    YFocalLength(
        CameraModelUtil::ComputeDistance(
            CameraModel,
            YFOVAngle,
            ImageHeight,
            KannalaBrandtCameraCoefficients)),
    LongitudeOffset(),
    FOVFadeSize(),
    CubemapRenderMask(ComputeCubemapRenderMask()),
    CubemapSampler(CameraModelUtil::GetSampler(SF_AnisotropicLinear)),
    bEnablePostProcessingEffects(true),
    bEnable16BitFormat(false),
    bRenderPerspective(false),
    bRenderEquirectangular(false),
    bFOVMaskEnable(false)
{
    FaceCaptures.SetNum(6);
    FaceRenderTargets.SetNum(6);

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    using ProjectionMatrixType = std::conditional_t<
        (bool)ERHIZBuffer::IsInverted,
        FReversedZPerspectiveMatrix,
        FPerspectiveMatrix>;

    constexpr auto Deg2Rad = (PI / 180.0F);
    constexpr auto FOV = 90.0F * Deg2Rad;
    constexpr auto HalfFOV = FOV * 0.5F;

    const auto SensorIndex = WIDE_ANGLE_LENS_SENSOR_COUNTER_COUNTER++;
    const auto FaceIndexBase = SensorIndex * 6;
    const auto ProjectionMatrix = ProjectionMatrixType(HalfFOV, 1.0F, 1.0F, GNearClippingPlane);

    const FVector Forward[] =
    {
        FVector::ForwardVector,
        -FVector::ForwardVector,
        FVector::RightVector,
        -FVector::RightVector,
        FVector::UpVector,
        -FVector::UpVector,
    };

    const FVector Right[] =
    {
        FVector::RightVector,
        -FVector::RightVector,
        -FVector::ForwardVector,
        FVector::ForwardVector,
        FVector::RightVector,
        -FVector::RightVector,
    };

    CaptureRenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(
        FName(*FString::Printf(
            TEXT("CaptureRenderTarget2D-WideLens-Final-d%d"),
            SensorIndex)));

    CaptureRenderTarget->CompressionSettings = TextureCompressionSettings::TC_Default;
    CaptureRenderTarget->SRGB = false;
    CaptureRenderTarget->bAutoGenerateMips = false;
    CaptureRenderTarget->bGPUSharedFlag = true;
    CaptureRenderTarget->AddressX = TextureAddress::TA_Clamp;
    CaptureRenderTarget->AddressY = TextureAddress::TA_Clamp;

    for (uint8 i = 0; i != 6; ++i)
    {
        const auto AbsIndex = FaceIndexBase + i;

        auto& RenderTarget = FaceRenderTargets[i];
        auto& FaceCapture = FaceCaptures[i];

        RenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(
            FName(*FString::Printf(TEXT("CaptureRenderTarget2D-WideLens-Face-d%d"), AbsIndex)));
        check(RenderTarget != nullptr);
        RenderTarget->CompressionSettings = TextureCompressionSettings::TC_Default;
        RenderTarget->SRGB = false;
        RenderTarget->bAutoGenerateMips = false;
        RenderTarget->bGPUSharedFlag = true;
        RenderTarget->AddressX = TextureAddress::TA_Clamp;
        RenderTarget->AddressY = TextureAddress::TA_Clamp;

        FaceCapture = CreateDefaultSubobject<USceneCaptureComponent2D_CARLA>(
            FName(*FString::Printf(TEXT("USceneCaptureComponent2D_CARLA-%d"), AbsIndex)));
        check(FaceCapture != nullptr);
        FaceCapture->SetupAttachment(RootComponent);
        FaceCapture->SetRelativeRotation(FRotationMatrix::MakeFromXY(Forward[i], Right[i]).ToQuat());
        FaceCapture->ViewActor = this;
        FaceCapture->ProjectionType = ECameraProjectionMode::Perspective;
        FaceCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
        FaceCapture->bCaptureOnMovement = false;
        FaceCapture->bCaptureEveryFrame = false;
        FaceCapture->bAlwaysPersistRenderingState = true;
        FaceCapture->bUseCustomProjectionMatrix = true;
        FaceCapture->CustomProjectionMatrix = ProjectionMatrix;
        SceneCaptureSensorWideAngleLens_local_ns::SetCameraDefaultOverrides(*FaceCapture);
    }

}

void ASceneCaptureSensor_WideAngleLens::Set(const FActorDescription& Description)
{
    Super::Set(Description);
    // CaptureRenderTargetCube->SizeX = GetImageWidth();
    UActorBlueprintFunctionLibrary::SetCamera(Description, this);
}

void ASceneCaptureSensor_WideAngleLens::SetImageSize(uint32 InWidth, uint32 InHeight)
{
    bool UpdateRenderMask = InWidth != ImageWidth || InHeight != ImageHeight;

    ImageWidth = InWidth;
    ImageHeight = InHeight;

    if (UpdateRenderMask)
        CubemapRenderMask = ComputeCubemapRenderMask();
}

void ASceneCaptureSensor_WideAngleLens::SetImageSize(int32 Width, int32 Height)
{
    SetImageSize(
        static_cast<uint32>(Width),
        static_cast<uint32>(Height));
}

ECameraModel ASceneCaptureSensor_WideAngleLens::GetCameraModel() const
{
    return CameraModel;
}

void ASceneCaptureSensor_WideAngleLens::SetCameraModel(ECameraModel NewCameraModel)
{
    bool UpdateRenderMask = NewCameraModel != CameraModel;

    CameraModel = NewCameraModel;

    if (UpdateRenderMask)
        CubemapRenderMask = ComputeCubemapRenderMask();
}

float ASceneCaptureSensor_WideAngleLens::GetFOVAngle() const
{
    return YFOVAngle;
}

float ASceneCaptureSensor_WideAngleLens::GetFOVAngleY() const
{
    return YFOVAngle;
}

float ASceneCaptureSensor_WideAngleLens::GetFOVAngleX() const
{
    return XFOVAngle;
}

constexpr auto DegToRad = PI / 180.0F;
constexpr auto RadToDeg = 180.0F / PI;

void ASceneCaptureSensor_WideAngleLens::SetFOVAngle(float NewFOV)
{
    NewFOV *= DegToRad;

    bool UpdateRenderMask = NewFOV != YFOVAngle;

    YFOVAngle = NewFOV;
    XFOVAngle = VerticalToHorizontal(NewFOV);

    YFocalLength = CameraModelUtil::ComputeDistance(
        CameraModel,
        NewFOV,
        ImageHeight,
        KannalaBrandtCameraCoefficients);

    if (UpdateRenderMask)
        CubemapRenderMask = ComputeCubemapRenderMask();
}

void ASceneCaptureSensor_WideAngleLens::SetTargetGamma(float Gamma)
{
    TargetGamma = Gamma;
}

float ASceneCaptureSensor_WideAngleLens::GetFocalLength() const
{
    return YFocalLength;
}

void ASceneCaptureSensor_WideAngleLens::SetFocalLength(float NewFocalLength)
{
    bool UpdateRenderMask = NewFocalLength != YFocalLength;
    
    YFocalLength = NewFocalLength;

    if (UpdateRenderMask)
        CubemapRenderMask = ComputeCubemapRenderMask();
}

void ASceneCaptureSensor_WideAngleLens::SetCameraCoefficients(TArrayView<const float> Coefficients)
{
    bool UpdateRenderMask = KannalaBrandtCameraCoefficients.Num() != Coefficients.Num();
    
    for (uint32 i = 0; i != Coefficients.Num() && !UpdateRenderMask; ++i)
        UpdateRenderMask = UpdateRenderMask || KannalaBrandtCameraCoefficients[i] != Coefficients[i];

    KannalaBrandtCameraCoefficients = TArray<float>(Coefficients);

    if (UpdateRenderMask)
        CubemapRenderMask = ComputeCubemapRenderMask();
}

void ASceneCaptureSensor_WideAngleLens::SetCameraCoefficients(const TArray<float>& Coefficients)
{
    SetCameraCoefficients(TArrayView<const float>(Coefficients));
}

const TArray<float>& ASceneCaptureSensor_WideAngleLens::GetCameraCoefficients() const
{
    return KannalaBrandtCameraCoefficients;
}

UTextureRenderTarget2D* ASceneCaptureSensor_WideAngleLens::GetCaptureRenderTarget()
{
    return CaptureRenderTarget;
}

float ASceneCaptureSensor_WideAngleLens::GetTargetGamma() const
{
    return TargetGamma;
}

bool ASceneCaptureSensor_WideAngleLens::GetRenderPerspective() const
{
    return bRenderPerspective;
}

void ASceneCaptureSensor_WideAngleLens::SetRenderPerspective(bool bEnable)
{
    bool UpdateRenderMask = bRenderPerspective != bEnable;

    bRenderPerspective = bEnable;

    if (UpdateRenderMask)
        CubemapRenderMask = ComputeCubemapRenderMask();
}

bool ASceneCaptureSensor_WideAngleLens::GetRenderEquirectangular() const
{
    return bRenderEquirectangular;
}

void ASceneCaptureSensor_WideAngleLens::SetRenderEquirectangular(bool bEnable)
{
    bool UpdateRenderMask = bRenderEquirectangular != bEnable;

    bRenderEquirectangular = bEnable;

    if (UpdateRenderMask)
        CubemapRenderMask = ComputeCubemapRenderMask();
}

bool ASceneCaptureSensor_WideAngleLens::GetFOVMaskEnable() const
{
    return bFOVMaskEnable;
}

void ASceneCaptureSensor_WideAngleLens::SetFOVMaskEnable(bool bEnable)
{
    bool UpdateRenderMask = bFOVMaskEnable != bEnable;

    bFOVMaskEnable = bEnable;

    if (UpdateRenderMask)
        CubemapRenderMask = ComputeCubemapRenderMask();
}

float ASceneCaptureSensor_WideAngleLens::GetFOVFadeSize() const
{
    return FOVFadeSize;
}

void ASceneCaptureSensor_WideAngleLens::SetFOVFadeSize(float NewFOVFadeSize)
{
    FOVFadeSize = NewFOVFadeSize;
}

float ASceneCaptureSensor_WideAngleLens::GetRenderEquirectangularLongitudeOffset() const
{
    return LongitudeOffset * RadToDeg;
}

void ASceneCaptureSensor_WideAngleLens::SetRenderEquirectangularLongitudeOffset(
    float NewLatitudeOffset)
{
    LongitudeOffset = NewLatitudeOffset * DegToRad;
}

void ASceneCaptureSensor_WideAngleLens::EnqueueRenderSceneImmediate()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureSensor_WideAngleLens::EnqueueRenderSceneImmediate);
    CaptureSceneExtended();
}

uint8 ASceneCaptureSensor_WideAngleLens::FindFaceIndex(FVector2D UV) const
{
    const float R = hypotf(UV.X, UV.Y);
    const float Theta = CameraModelUtil::ComputeAngle(CameraModel, R, KannalaBrandtCameraCoefficients);
    const float HalfPi = PI / 2.0f;
    const float Phi = HalfPi - Theta;
    const float Rho = atan2f(UV.Y, UV.X);

    float PhiSin = 0.0F;
    float PhiCos = 0.0F;
    float RhoSin = 0.0F;
    float RhoCos = 0.0F;

    FMath::SinCos(&PhiSin, &PhiCos, Phi);
    FMath::SinCos(&RhoSin, &RhoCos, Rho);

    auto Direction = FVector(PhiSin, PhiCos * RhoCos, -PhiCos * RhoSin);
    Direction.Normalize();

    auto DirectionAbs = Direction.GetAbs();

    if (DirectionAbs.X >= DirectionAbs.Y && DirectionAbs.X >= DirectionAbs.Z)
        return Direction.X < 0 ? 1U : 0U;
    else if (DirectionAbs.Y >= DirectionAbs.X && DirectionAbs.Y >= DirectionAbs.Z)
        return Direction.Y < 0 ? 3U : 2U;
    else
        return Direction.Z < 0 ? 5U : 4U;
}

uint8 ASceneCaptureSensor_WideAngleLens::ComputeCubemapRenderMask() const
{
    static const float Threshold = 1e-2;
    static const float Pi = PI;
    static const float HalfPi = Pi / 2.0f;
    static const float Sqrt2 = sqrtf(2.0f);

    auto Mask = 1U << CubeFace_PosX; // Render front face by default.

    const auto FOV = FVector2D(GetFOVAngleX(), GetFOVAngleY()) * (GetFOVMaskEnable() ? 1 : Sqrt2);

    if (FOV.Y > HalfPi)
    {
        Mask |= 1U << CubeFace_PosZ;
        Mask |= 1U << CubeFace_NegZ;
    }

    if (FOV.X > HalfPi)
    {
        Mask |= 1U << CubeFace_PosY;
        Mask |= 1U << CubeFace_NegY;
    }

    if (FOV.X > Pi || FOV.Y > Pi)
    {
        Mask |= 1U << CubeFace_NegX;
    }

    return (uint8)Mask;
}

void ASceneCaptureSensor_WideAngleLens::CaptureSceneExtended()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureSensor_WideAngleLens::CaptureSceneExtended);

    bool SkipVFTR = CVarWideAngleSensorSkipVFTR.GetValueOnAnyThread() != 0;

    if (!SkipVFTR)
    {
        FlushRenderingCommands();
        GEngine->Exec(GetWorld(), TEXT("r.VolumetricFog.TemporalReprojection 0"));
    }

    for (uint8 i = 0; i < 6; ++i)
        if (CubemapRenderMask & (1U << i))
            FaceCaptures[i]->CaptureScene();

    ENQUEUE_RENDER_COMMAND(WideAngleLensCommand)([this](FRHICommandListImmediate& RHICmdList)
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(WideAngleLensCommand);

        if (this->IsPendingKill())
            return;

        FRDGBuilder GraphBuilder(RHICmdList);

        UTextureRenderTarget2D* RenderTargets[] =
        {
            FaceRenderTargets[0],
            FaceRenderTargets[1],
            FaceRenderTargets[2],
            FaceRenderTargets[3],
            FaceRenderTargets[4],
            FaceRenderTargets[5]
        };

        using namespace CameraModelUtil;

        FDistortCubemapToImageOptions DistortedOptions = { };
        DistortedOptions.KannalaBrandtCoefficients = KannalaBrandtCameraCoefficients;
        DistortedOptions.YFOVAngle = YFOVAngle;
        DistortedOptions.YFocalLength = YFocalLength;
        DistortedOptions.LongitudeOffset = LongitudeOffset;
        DistortedOptions.FOVFadeSize = FOVFadeSize;
        DistortedOptions.CameraModel = CameraModel;
        DistortedOptions.bRenderEquirectangular = bRenderEquirectangular;
        DistortedOptions.bFOVMaskEnable = bFOVMaskEnable;
        DistortedOptions.bRenderPerspective = bRenderPerspective;

        DistortCubemapToImage(
            GraphBuilder,
            CaptureRenderTarget,
            RenderTargets,
            CubemapSampler,
            DistortedOptions);

        GraphBuilder.Execute();
    });

    if (CVarWideAngleSensorDumpAllFrames.GetValueOnAnyThread() == 1)
    {
        static thread_local auto FrameCounter = 0U;

        const TCHAR* Names[(size_t)ECameraModel::MaxEnum] =
        {
            TEXT("Perspective"),
            TEXT("Stereographic"),
            TEXT("Equidistant"),
            TEXT("Equisolid"),
            TEXT("Orthographic"),
            TEXT("KannalaBrandt"),
        };

        auto CameraTypeName = Names[(uint8)CameraModel];
        auto Path = CVarWideAngleSensorDumpAllFramesPath.GetValueOnAnyThread();

        if (CVarWideAngleSensorDumpAllFramesCubemap.GetValueOnAnyThread())
        {
            for (uint8 FaceIndex = 0; FaceIndex != 6; ++FaceIndex)
            {
                FPixelReader::SavePixelsToDisk(
                    *FaceRenderTargets[FaceIndex],
                    FString::Printf(
                        TEXT("%s/Frame-%s-%u-Face-%u.png"),
                        *Path,
                        CameraTypeName,
                        FrameCounter,
                        FaceIndex)).Wait();
            }
        }

        FPixelReader::SavePixelsToDisk(
            *CaptureRenderTarget,
            FString::Printf(
                TEXT("%s/Frame-%s-%u-Final.png"),
                *Path,
                CameraTypeName,
                FrameCounter)).Wait();

        ++FrameCounter;
    }
    
    if (!SkipVFTR)
    {
        FlushRenderingCommands();
        GEngine->Exec(GetWorld(), TEXT("r.VolumetricFog.TemporalReprojection 1"));
    }
}

void ASceneCaptureSensor_WideAngleLens::BeginPlay()
{
    const bool bInForceLinearGamma = !bEnablePostProcessingEffects;

    const auto Format = bEnable16BitFormat ? PF_FloatRGBA : PF_B8G8R8A8;
    const auto Side = std::max(GetImageWidth(), GetImageHeight());

    CaptureRenderTarget->InitCustomFormat(
        GetImageWidth(),
        GetImageHeight(),
        Format,
        bInForceLinearGamma);

    for (auto FaceRenderTarget : FaceRenderTargets)
    {
        FaceRenderTarget->InitCustomFormat(
            Side, Side,
            Format,
            bInForceLinearGamma);
    }

    if (bEnablePostProcessingEffects)
    {
        for (auto Face : FaceRenderTargets)
            Face->TargetGamma = TargetGamma;
        CaptureRenderTarget->TargetGamma = TargetGamma;
    }


    for (uint8 i = 0; i != 6; ++i)
    {
        FaceCaptures[i]->Deactivate();
        FaceCaptures[i]->TextureTarget = FaceRenderTargets[i];
    }

    SetUpSceneCaptureComponents(FaceCaptures);

    for (auto FaceCapture : FaceCaptures)
    {
        FaceCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
        FaceCapture->UpdateContent();
        FaceCapture->Activate();
    }

    // Make sure that there is enough time in the render queue.
    UKismetSystemLibrary::ExecuteConsoleCommand(
        GetWorld(),
        FString("g.TimeoutForBlockOnRenderFence 300000"));

    for (auto FaceCapture : FaceCaptures)
    {
        SceneCaptureSensorWideAngleLens_local_ns::ConfigureShowFlags(
            FaceCapture->ShowFlags,
            bEnablePostProcessingEffects);
    }

    // This ensures the camera is always spawning the raindrops in case the
    // weather was previously set to have rain.
    GetEpisode().GetWeather()->NotifyWeather(this);

    Super::BeginPlay();
}

void ASceneCaptureSensor_WideAngleLens::PrePhysTick(float DeltaSeconds)
{
    Super::PrePhysTick(DeltaSeconds);

    auto CaptureComponents = GetCaptureComponents2D();

    // Add the view information every tick. It's only used for one tick and then
    // removed by the streamer.
    IStreamingManager::Get().AddViewInformation(
        CaptureComponents[0]->GetComponentLocation(),
        ImageWidth,
        ImageWidth / FMath::Tan(CaptureComponents[0]->FOVAngle));
}

void ASceneCaptureSensor_WideAngleLens::PostPhysTick(UWorld* World, ELevelTick TickType, float DeltaTime)
{
    Super::PostPhysTick(World, TickType, DeltaTime);
}

void ASceneCaptureSensor_WideAngleLens::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    WIDE_ANGLE_LENS_SENSOR_COUNTER_COUNTER = 0u;
    FlushRenderingCommands();
}

TArrayView<USceneCaptureComponent2D_CARLA*> ASceneCaptureSensor_WideAngleLens::GetCaptureComponents2D()
{
    return FaceCaptures;
}

float ASceneCaptureSensor_WideAngleLens::VerticalToHorizontal(
    float Value) const
{
    Value *= GetImageWidth();
    Value /= GetImageHeight();
    return Value;
}



// =============================================================================
// -- Local static functions implementations -----------------------------------
// =============================================================================

namespace SceneCaptureSensorWideAngleLens_local_ns {

    static void SetCameraDefaultOverrides(USceneCaptureComponent2D_CARLA& CaptureComponent)
    {
        FPostProcessSettings& PostProcessSettings = CaptureComponent.PostProcessSettings;
        PostProcessSettings.bOverride_VignetteIntensity = true;
        PostProcessSettings.VignetteIntensity = 0;
        PostProcessSettings.bOverride_DepthOfFieldVignetteSize = true;
        PostProcessSettings.DepthOfFieldVignetteSize = 0;
        PostProcessSettings.bOverride_AutoExposureMethod = true;
        PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
    }

    // Remove the show flags that might interfere with post-processing effects
    // like depth and semantic segmentation.
    static void ConfigureShowFlags(FEngineShowFlags& ShowFlags, bool bPostProcessing)
    {
        if (bPostProcessing)
            ShowFlags.EnableAdvancedFeatures();

        ShowFlags.SetTemporalAA(false);
        ShowFlags.SetScreenSpaceReflections(false);
        // ShowFlags.SetVolumetricFogTemporalReprojection(false);
        ShowFlags.SetDistanceFieldAO(false);

        if (bPostProcessing)
        {
            ShowFlags.SetMotionBlur(true);
            return;
        }

        // ShowFlags.SetAtmosphericFog(false);
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
        // ShowFlags.SetVisualizeAdaptiveDOF(false);
        // ShowFlags.SetVisualizeBloom(false);
        ShowFlags.SetVisualizeBuffer(false);
        ShowFlags.SetVisualizeDistanceFieldAO(false);
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

} // namespace SceneCaptureSensorWideAngleLens_local_ns
