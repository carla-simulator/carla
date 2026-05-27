// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "SceneCaptureSensor_WideAngleLens.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Sensor/PostProcessConfig.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/Engine.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include <util/ue-header-guard-end.h>

#include <cmath>

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

static auto WIDE_ANGLE_LENS_SENSOR_COUNTER = 0u;

// =============================================================================
// -- Local static methods -----------------------------------------------------
// =============================================================================

// Local namespace to avoid name collisions on unit builds.
namespace SceneCaptureSensorWideAngleLens_local_ns {

    static void SetCameraDefaultOverrides(USceneCaptureComponent2D_CARLA& CaptureComponent);

} // namespace SceneCaptureSensorWideAngleLens_local_ns

// =============================================================================
// -- ASceneCaptureSensor_WideAngleLens ----------------------------------------
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
    CubemapRenderMask(0),
    CubemapSampler(CameraModelUtil::GetSampler(SF_AnisotropicLinear)),
    bUseRayTracing(true),
    bEnablePostProcessingEffects(true),
    bEnable16BitFormat(false),
    bRenderPerspective(false),
    bRenderEquirectangular(false),
    bFOVMaskEnable(false)
{
    FaceCaptures.SetNum(6);
    FaceRenderTargets.SetNum(6);

    // Computed in the constructor body, not the initializer list:
    // ComputeCubemapRenderMask() reads bFOVMaskEnable / bRenderEquirectangular,
    // which are declared after CubemapRenderMask and would still be
    // uninitialized during initializer-list evaluation.
    CubemapRenderMask = ComputeCubemapRenderMask();

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    using ProjectionMatrixType = std::conditional_t<
        (bool)ERHIZBuffer::IsInverted,
        FReversedZPerspectiveMatrix,
        FPerspectiveMatrix>;

    constexpr auto Deg2Rad = (PI / 180.0F);
    constexpr auto FOV = 90.0F * Deg2Rad;
    constexpr auto HalfFOV = FOV * 0.5F;

    const auto SensorIndex = WIDE_ANGLE_LENS_SENSOR_COUNTER++;
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
        // Propagate ray-tracing flag to this face capture.
        FaceCapture->bUseRayTracingIfEnabled = bUseRayTracing;
        SceneCaptureSensorWideAngleLens_local_ns::SetCameraDefaultOverrides(*FaceCapture);
    }
}

void ASceneCaptureSensor_WideAngleLens::Set(const FActorDescription& Description)
{
    Super::Set(Description);
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
    // Clamp to a sane minimum: negative values would wrap to enormous
    // dimensions when cast to uint32.
    SetImageSize(
        static_cast<uint32>(FMath::Max(Width, 1)),
        static_cast<uint32>(FMath::Max(Height, 1)));
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

    for (uint32 i = 0; i != (uint32)Coefficients.Num() && !UpdateRenderMask; ++i)
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

void ASceneCaptureSensor_WideAngleLens::SetUseRayTracing(bool Enable)
{
    bUseRayTracing = Enable;
    for (auto FaceCapture : FaceCaptures)
    {
        if (FaceCapture != nullptr)
            FaceCapture->bUseRayTracingIfEnabled = Enable;
    }
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
    // Equirectangular projection samples the full sphere, so every cube face
    // must be rendered regardless of the configured FOV.
    if (bRenderEquirectangular)
        return (1U << CubeFace_PosX) | (1U << CubeFace_NegX) |
               (1U << CubeFace_PosY) | (1U << CubeFace_NegY) |
               (1U << CubeFace_PosZ) | (1U << CubeFace_NegZ);

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

    // Capture the previous value so it can be restored afterwards, instead of
    // unconditionally forcing the cvar back on and clobbering the project /
    // scalability setting for every other view in the scene.
    IConsoleVariable* VolumetricFogTRVar = nullptr;
    int32 PreviousVolumetricFogTR = 1;

    if (!SkipVFTR)
    {
        VolumetricFogTRVar = IConsoleManager::Get().FindConsoleVariable(
            TEXT("r.VolumetricFog.TemporalReprojection"));
        if (VolumetricFogTRVar != nullptr)
        {
            PreviousVolumetricFogTR = VolumetricFogTRVar->GetInt();
            FlushRenderingCommands();
            VolumetricFogTRVar->Set(TEXT("0"), ECVF_SetByCode);
        }
    }

    for (uint8 i = 0; i < 6; ++i)
        if (CubemapRenderMask & (1U << i))
            FaceCaptures[i]->CaptureScene();

    // Snapshot the data the render-thread pass needs so the lambda does not
    // dereference `this` after the actor may have been destroyed. The render
    // targets and sampler are still resolved through a TWeakObjectPtr so we
    // skip the dispatch cleanly if the owning actor is gone.
    TWeakObjectPtr<ASceneCaptureSensor_WideAngleLens> WeakSelf(this);
    UTextureRenderTarget2D* RenderTargetsSnapshot[] =
    {
        FaceRenderTargets[0],
        FaceRenderTargets[1],
        FaceRenderTargets[2],
        FaceRenderTargets[3],
        FaceRenderTargets[4],
        FaceRenderTargets[5]
    };
    UTextureRenderTarget2D* CaptureRenderTargetSnapshot = CaptureRenderTarget;
    FRHISamplerState* CubemapSamplerSnapshot = CubemapSampler;

    CameraModelUtil::FDistortCubemapToImageOptions DistortedOptions = { };
    DistortedOptions.KannalaBrandtCoefficients = KannalaBrandtCameraCoefficients;
    DistortedOptions.YFOVAngle = YFOVAngle;
    DistortedOptions.YFocalLength = YFocalLength;
    DistortedOptions.LongitudeOffset = LongitudeOffset;
    DistortedOptions.FOVFadeSize = FOVFadeSize;
    DistortedOptions.CameraModel = CameraModel;
    DistortedOptions.bRenderEquirectangular = bRenderEquirectangular;
    DistortedOptions.bFOVMaskEnable = bFOVMaskEnable;
    DistortedOptions.bRenderPerspective = bRenderPerspective;

    ENQUEUE_RENDER_COMMAND(WideAngleLensCommand)(
        [WeakSelf,
         RenderTargetsSnapshot,
         CaptureRenderTargetSnapshot,
         CubemapSamplerSnapshot,
         DistortedOptions](FRHICommandListImmediate& RHICmdList)
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(WideAngleLensCommand);

        if (!WeakSelf.IsValid())
            return;

        FRDGBuilder GraphBuilder(RHICmdList);

        UTextureRenderTarget2D* RenderTargets[] =
        {
            RenderTargetsSnapshot[0],
            RenderTargetsSnapshot[1],
            RenderTargetsSnapshot[2],
            RenderTargetsSnapshot[3],
            RenderTargetsSnapshot[4],
            RenderTargetsSnapshot[5]
        };

        CameraModelUtil::DistortCubemapToImage(
            GraphBuilder,
            CaptureRenderTargetSnapshot,
            RenderTargets,
            CubemapSamplerSnapshot,
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

    if (VolumetricFogTRVar != nullptr)
    {
        FlushRenderingCommands();
        VolumetricFogTRVar->Set(*FString::FromInt(PreviousVolumetricFogTR), ECVF_SetByCode);
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
        // Use PostProcessConfig for each face capture to honour the
        // ue5-dev convention (mirrors ASceneCaptureSensor::BeginPlay).
        auto PostProcessConfig = FPostProcessConfig(
            FaceCapture->PostProcessSettings,
            FaceCapture->ShowFlags);
        PostProcessConfig.UpdateFromSceneCaptureComponent2D(*FaceCapture);
        PostProcessConfig.EnablePostProcessingEffects(bEnablePostProcessingEffects);
        FaceCapture->ShowFlags = PostProcessConfig.EngineShowFlags;
        FaceCapture->PostProcessSettings = PostProcessConfig.PostProcessSettings;

        FaceCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
        FaceCapture->UpdateContent();
        FaceCapture->Activate();
    }

    // Raise the render-fence block timeout: a single wide-angle capture
    // enqueues up to 6 face scene-captures plus the RDG composite pass, which
    // can exceed the default timeout on heavy scenes and trip a false-positive
    // render-fence fatal assert. This is a process-wide setting; CARLA already
    // runs the render pipeline outside interactive-latency budgets, so a wider
    // watchdog is harmless for other views.
    UKismetSystemLibrary::ExecuteConsoleCommand(
        GetWorld(),
        FString("g.TimeoutForBlockOnRenderFence 300000"));

    // This ensures the camera is always spawning the raindrops in case the
    // weather was previously set to have rain.
    auto Weather = GetEpisode().GetWeather();
    if (Weather != nullptr)
        Weather->NotifyWeather(this);

    Super::BeginPlay();
}

void ASceneCaptureSensor_WideAngleLens::PrePhysTick(float DeltaSeconds)
{
    Super::PrePhysTick(DeltaSeconds);

    auto CaptureComponents = GetCaptureComponents2D();

    // Add the view information every tick. It's only used for one tick and then
    // removed by the streamer.
    // FOVAngle is stored in degrees; convert to radians and use the
    // half-angle tangent, guarding against a non-positive denominator.
    const float HalfFOVTan = FMath::Tan(
        0.5f * FMath::DegreesToRadians(CaptureComponents[0]->FOVAngle));
    IStreamingManager::Get().AddViewInformation(
        CaptureComponents[0]->GetComponentLocation(),
        ImageWidth,
        HalfFOVTan > 0.0f ? ImageWidth / HalfFOVTan : ImageWidth);
}

void ASceneCaptureSensor_WideAngleLens::PostPhysTick(UWorld* World, ELevelTick TickType, float DeltaTime)
{
    Super::PostPhysTick(World, TickType, DeltaTime);
}

void ASceneCaptureSensor_WideAngleLens::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    // The sensor counter is process-wide and only used to derive unique
    // subobject names at construction. Resetting it here would let a
    // surviving sibling sensor collide with the indices of the next spawned
    // sensor.
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


} // namespace SceneCaptureSensorWideAngleLens_local_ns
