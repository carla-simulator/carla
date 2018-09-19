// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureCamera.h"

#include "Carla/Sensor/SensorDataView.h"
#include "Carla/Settings/CarlaSettings.h"

#include "Components/DrawFrustumComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "ConstructorHelpers.h"
#include "CoreGlobals.h"
#include "Engine/CollisionProfile.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Game/CarlaGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/Material.h"

#include <memory>

// =============================================================================
// -- Local static variables ---------------------------------------------------
// =============================================================================

static constexpr auto DEPTH_MAT_PATH =
#if PLATFORM_LINUX
  TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial_GLSL.DepthEffectMaterial_GLSL'");
#elif PLATFORM_WINDOWS
  TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial.DepthEffectMaterial'");
#else
#  error No depth material defined for this platform
#endif

static constexpr auto SEMANTIC_SEGMENTATION_MAT_PATH =
  TEXT("Material'/Carla/PostProcessingMaterials/GTMaterial.GTMaterial'");

// =============================================================================
// -- Local static methods and types -------------------------------------------
// =============================================================================

struct FImageHeaderData
{
  uint64 FrameNumber;
  uint32 Width;
  uint32 Height;
  uint32 Type;
  float FOV;
};

static void SetCameraDefaultOverrides(USceneCaptureComponent2D &CaptureComponent2D);

static void RemoveShowFlags(FEngineShowFlags &ShowFlags);

// =============================================================================
// -- ASceneCaptureCamera ------------------------------------------------------
// =============================================================================

uint32 ASceneCaptureCamera::NumSceneCapture = 0;

ASceneCaptureCamera::ASceneCaptureCamera(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer),
    SizeX(720u),
    SizeY(512u),
    PostProcessEffect(EPostProcessEffect::SceneFinal)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;

  MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CamMesh"));

  MeshComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

  MeshComp->bHiddenInGame = true;
  MeshComp->CastShadow = false;
  MeshComp->PostPhysicsComponentTick.bCanEverTick = false;
  RootComponent = MeshComp;

  DrawFrustum = CreateDefaultSubobject<UDrawFrustumComponent>(TEXT("DrawFrust"));
  DrawFrustum->bIsEditorOnly = true;
  DrawFrustum->SetupAttachment(MeshComp);

  CaptureRenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(
      FName(*FString::Printf(TEXT("CaptureRenderTarget%d"), NumSceneCapture)));
#if WITH_EDITORONLY_DATA
  CaptureRenderTarget->CompressionNoAlpha = true;
  CaptureRenderTarget->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
  CaptureRenderTarget->bUseLegacyGamma = false;
#endif
  CaptureRenderTarget->CompressionSettings = TextureCompressionSettings::TC_Default;
  CaptureRenderTarget->SRGB = false;
  CaptureRenderTarget->bAutoGenerateMips = false;
  CaptureRenderTarget->AddressX = TextureAddress::TA_Clamp;
  CaptureRenderTarget->AddressY = TextureAddress::TA_Clamp;
  CaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(
      TEXT("SceneCaptureComponent2D"));
  CaptureComponent2D->SetupAttachment(MeshComp);
  SetCameraDefaultOverrides(*CaptureComponent2D);

  // Load post-processing materials.
  static ConstructorHelpers::FObjectFinder<UMaterial> DEPTH(
      DEPTH_MAT_PATH);
  PostProcessDepth = DEPTH.Object;
  static ConstructorHelpers::FObjectFinder<UMaterial> SEMANTIC_SEGMENTATION(
      SEMANTIC_SEGMENTATION_MAT_PATH);
  PostProcessSemanticSegmentation = SEMANTIC_SEGMENTATION.Object;
  NumSceneCapture++;
}

void ASceneCaptureCamera::PostActorCreated()
{
  Super::PostActorCreated();

  // no need load the editor mesh when there is no editor
#if WITH_EDITOR
  if (MeshComp)
  {
    if (!IsRunningCommandlet())
    {
      if (!MeshComp->GetStaticMesh())
      {
        UStaticMesh *CamMesh = LoadObject<UStaticMesh>(
            NULL,
            TEXT("/Engine/EditorMeshes/MatineeCam_SM.MatineeCam_SM"),
            NULL,
            LOAD_None,
            NULL);
        MeshComp->SetStaticMesh(CamMesh);
      }
    }
  }
#endif   // WITH_EDITOR

  // Sync component with CameraActor frustum settings.
  UpdateDrawFrustum();
}

void ASceneCaptureCamera::BeginPlay()
{
  const bool bRemovePostProcessing = (PostProcessEffect != EPostProcessEffect::SceneFinal);

  // Setup render target.
  const bool bInForceLinearGamma = bRemovePostProcessing;
  CaptureRenderTarget->InitCustomFormat(SizeX, SizeY, PF_B8G8R8A8, bInForceLinearGamma);
  if (!IsValid(CaptureComponent2D) || CaptureComponent2D->IsPendingKill())
  {
    CaptureComponent2D = NewObject<USceneCaptureComponent2D>(this, TEXT("SceneCaptureComponent2D"));
    CaptureComponent2D->SetupAttachment(MeshComp);
  }
  CaptureComponent2D->Deactivate();
  CaptureComponent2D->TextureTarget = CaptureRenderTarget;

  // Setup camera post-processing depending on the quality level:
  const UCarlaGameInstance *GameInstance  = Cast<UCarlaGameInstance>(GetWorld()->GetGameInstance());
  check(GameInstance != nullptr);
  const UCarlaSettings &CarlaSettings = GameInstance->GetCarlaSettings();
  switch (PostProcessEffect)
  {
    case EPostProcessEffect::None:
      break;
    case EPostProcessEffect::SceneFinal:
    {
      // We set LDR for high quality because it will include post-fx and HDR for
      // low quality to avoid high contrast.
      switch (CarlaSettings.GetQualitySettingsLevel())
      {
        case EQualitySettingsLevel::Low:
          CaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;
          break;
        default:
          // LDR is faster than HDR (smaller bitmap array).
          CaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
          break;
      }
      break;
    }
    default:
      CaptureComponent2D->CaptureSource = SCS_FinalColorLDR;
      break;
  }

  if (bRemovePostProcessing)
  {
    RemoveShowFlags(CaptureComponent2D->ShowFlags);
  }

  if (PostProcessEffect == EPostProcessEffect::Depth)
  {
    CaptureComponent2D->PostProcessSettings.AddBlendable(PostProcessDepth, 1.0f);
  }
  else if (PostProcessEffect == EPostProcessEffect::SemanticSegmentation)
  {
    CaptureComponent2D->PostProcessSettings.AddBlendable(PostProcessSemanticSegmentation, 1.0f);
  }

  CaptureComponent2D->UpdateContent();
  CaptureComponent2D->Activate();

  // Make sure that there is enough time in the render queue.
  UKismetSystemLibrary::ExecuteConsoleCommand(
      GetWorld(),
      FString("g.TimeoutForBlockOnRenderFence 300000"));

  Super::BeginPlay();
}

void ASceneCaptureCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  if (NumSceneCapture != 0)
  {
    NumSceneCapture = 0;
  }
}

void ASceneCaptureCamera::Tick(const float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  const auto FrameNumber = GFrameCounter;

  if (IsVulkanPlatform(GMaxRHIShaderPlatform))
  {
    auto fn = [=](FRHICommandListImmediate &RHICmdList) {
        WritePixelsNonBlocking(FrameNumber, RHICmdList);
      };
    ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
        FWritePixelsNonBlocking,
        decltype(fn), write_function_vulkan, fn,
    {
      write_function_vulkan(RHICmdList);
    });
  }
  else
  {
    auto fn = [=]() {
        WritePixels(FrameNumber);
      };
    ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
        FWritePixels,
        decltype(fn), write_function, fn,
    {
      write_function();
    });
  }
}

float ASceneCaptureCamera::GetFOVAngle() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->FOVAngle;
}

void ASceneCaptureCamera::SetImageSize(uint32 otherSizeX, uint32 otherSizeY)
{
  SizeX = otherSizeX;
  SizeY = otherSizeY;
}

void ASceneCaptureCamera::SetPostProcessEffect(EPostProcessEffect otherPostProcessEffect)
{
  PostProcessEffect = otherPostProcessEffect;
  auto &PostProcessSettings = CaptureComponent2D->PostProcessSettings;
  if (PostProcessEffect != EPostProcessEffect::SceneFinal)
  {
    PostProcessSettings.bOverride_AutoExposureMethod = false;
    PostProcessSettings.bOverride_AutoExposureMinBrightness = false;
    PostProcessSettings.bOverride_AutoExposureMaxBrightness = false;
    PostProcessSettings.bOverride_AutoExposureBias = false;
  }
}

void ASceneCaptureCamera::SetFOVAngle(const float FOVAngle)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->FOVAngle = FOVAngle;
}

void ASceneCaptureCamera::SetTargetGamma(const float TargetGamma)
{
  check(CaptureRenderTarget != nullptr);
  CaptureRenderTarget->TargetGamma = TargetGamma;
}

void ASceneCaptureCamera::Set(const UCameraDescription &CameraDescription)
{
  Super::Set(CameraDescription);

  if (CameraDescription.bOverrideCameraPostProcessParameters)
  {
    auto &Override = CameraDescription.CameraPostProcessParameters;
    auto &PostProcessSettings = CaptureComponent2D->PostProcessSettings;
    PostProcessSettings.bOverride_AutoExposureMethod = true;
    PostProcessSettings.AutoExposureMethod = Override.AutoExposureMethod;
    PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
    PostProcessSettings.AutoExposureMinBrightness = Override.AutoExposureMinBrightness;
    PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
    PostProcessSettings.AutoExposureMaxBrightness = Override.AutoExposureMaxBrightness;
    PostProcessSettings.bOverride_AutoExposureBias = true;
    PostProcessSettings.AutoExposureBias = Override.AutoExposureBias;
  }
  SetImageSize(CameraDescription.ImageSizeX, CameraDescription.ImageSizeY);
  SetPostProcessEffect(CameraDescription.PostProcessEffect);
  SetFOVAngle(CameraDescription.FOVAngle);
}

bool ASceneCaptureCamera::ReadPixels(TArray<FColor> &BitMap) const
{
  if (!CaptureRenderTarget)
  {
    UE_LOG(LogCarla, Error, TEXT("SceneCaptureCamera: Missing render target"));
    return false;
  }
  FTextureRenderTargetResource *RTResource =
      CaptureRenderTarget->GameThread_GetRenderTargetResource();
  if (RTResource == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("SceneCaptureCamera: Missing render target"));
    return false;
  }
  FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
  ReadPixelFlags.SetLinearToGamma(true);
  return RTResource->ReadPixels(BitMap, ReadPixelFlags);
}

void ASceneCaptureCamera::WritePixelsNonBlocking(
    const uint64 FrameNumber,
    FRHICommandListImmediate &rhi_cmd_list) const
{
  check(IsInRenderingThread());
  if (!CaptureRenderTarget)
  {
    UE_LOG(LogCarla, Error, TEXT("SceneCaptureCamera: Missing render target"));
    return;
  }
  FTextureRenderTarget2DResource *RenderResource =
      (FTextureRenderTarget2DResource *) CaptureRenderTarget->Resource;
  FTextureRHIParamRef texture = RenderResource->GetRenderTargetTexture();
  if (!texture)
  {
    UE_LOG(LogCarla, Error, TEXT("SceneCaptureCamera: Missing render target texture"));
    return;
  }
  FImageHeaderData ImageHeader = {
    FrameNumber,
    SizeX,
    SizeY,
    PostProcessEffect::ToUInt(PostProcessEffect),
    CaptureComponent2D->FOVAngle
  };

  TArray<FColor> Pixels;
  rhi_cmd_list.ReadSurfaceData(
      texture,
      FIntRect(0, 0, RenderResource->GetSizeXY().X, RenderResource->GetSizeXY().Y),
      Pixels,
      FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));
  FSensorDataView DataView(
      GetId(),
      FReadOnlyBufferView{reinterpret_cast<const void *>(&ImageHeader), sizeof(ImageHeader)},
      FReadOnlyBufferView{Pixels});
  WriteSensorData(DataView);
}

void ASceneCaptureCamera::WritePixels(const uint64 FrameNumber) const
{
  FRHITexture2D *texture = CaptureRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
  if (!texture)
  {
    UE_LOG(LogCarla, Error, TEXT("SceneCaptureCamera: Missing render texture"));
    return;
  }
  const uint32 num_bytes_per_pixel = 4;    // PF_R8G8B8A8
  const uint32 width = texture->GetSizeX();
  const uint32 height = texture->GetSizeY();
  const uint32 dest_stride = width * height * num_bytes_per_pixel;
  uint32 src_stride;
  uint8 *src = reinterpret_cast<uint8 *>(
      RHILockTexture2D(texture, 0, RLM_ReadOnly, src_stride, false));
  FImageHeaderData ImageHeader = {
    FrameNumber,
    width,
    height,
    PostProcessEffect::ToUInt(PostProcessEffect),
    CaptureComponent2D->FOVAngle
  };

  std::unique_ptr<uint8[]> dest = nullptr;
  // Direct 3D uses additional rows in the buffer,so we need check the result
  // stride from the lock:
  if (IsD3DPlatform(GMaxRHIShaderPlatform, false) && (dest_stride != src_stride))
  {
    const uint32 copy_row_stride = width * num_bytes_per_pixel;
    dest = std::make_unique<uint8[]>(dest_stride);
    // Copy per row
    uint8 *dest_row = dest.get();
    uint8 *src_row = src;
    for (uint32 Row = 0; Row < height; ++Row)
    {
      FMemory::Memcpy(dest_row, src_row, copy_row_stride);
      dest_row += copy_row_stride;
      src_row += src_stride;
    }
    src = dest.get();
  }

  const FSensorDataView DataView(
      GetId(),
      FReadOnlyBufferView{reinterpret_cast<const void *>(&ImageHeader), sizeof(ImageHeader)},
      FReadOnlyBufferView{src, dest_stride});

  WriteSensorData(DataView);
  RHIUnlockTexture2D(texture, 0, false);
}

void ASceneCaptureCamera::UpdateDrawFrustum()
{
  if (DrawFrustum && CaptureComponent2D)
  {
    DrawFrustum->FrustumStartDist = GNearClippingPlane;

    // 1000 is the default frustum distance, ideally this would be infinite but
    // that might cause rendering issues.
    DrawFrustum->FrustumEndDist =
      (CaptureComponent2D->MaxViewDistanceOverride > DrawFrustum->FrustumStartDist)
      ? CaptureComponent2D->MaxViewDistanceOverride : 1000.0f;

    DrawFrustum->FrustumAngle = CaptureComponent2D->FOVAngle;
  }
}

// =============================================================================
// -- Local static functions implementations -----------------------------------
// =============================================================================

static void SetCameraDefaultOverrides(USceneCaptureComponent2D &CaptureComponent2D)
{
  auto &PostProcessSettings = CaptureComponent2D.PostProcessSettings;
  PostProcessSettings.bOverride_AutoExposureMethod = true;
  PostProcessSettings.AutoExposureMethod = AEM_Histogram;
  PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
  PostProcessSettings.AutoExposureMinBrightness = 0.27f;
  PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
  PostProcessSettings.AutoExposureMaxBrightness = 5.0f;
  PostProcessSettings.bOverride_AutoExposureBias = true;
  PostProcessSettings.AutoExposureBias = -3.5f;
}

// Remove the show flags that might interfere with post-processing effects like
// depth and semantic segmentation.
static void RemoveShowFlags(FEngineShowFlags &ShowFlags)
{
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
