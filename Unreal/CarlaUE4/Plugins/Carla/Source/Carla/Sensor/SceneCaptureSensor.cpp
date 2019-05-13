// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureSensor.h"

#include "Carla/Game/CarlaStatics.h"

#include "Components/DrawFrustumComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HighResScreenshot.h"
#include "Slate/SceneViewport.h"

static auto SCENE_CAPTURE_COUNTER = 0u;

// =============================================================================
// -- Local static methods -----------------------------------------------------
// =============================================================================

// Local namespace to avoid name collisions on unit builds.
namespace SceneCaptureSensor_local_ns {

  static void SetCameraDefaultOverrides(USceneCaptureComponent2D &CaptureComponent2D);

  static void ConfigureShowFlags(FEngineShowFlags &ShowFlags, bool bPostProcessing = true);

  static auto GetQualitySettings(UWorld *World)
  {
    auto Settings = UCarlaStatics::GetCarlaSettings(World);
    check(Settings != nullptr);
    return Settings->GetQualityLevel();
  }

} // namespace SceneCaptureSensor_local_ns

// =============================================   SceneCaptureSensor_local_ns::ConfigureShowFlags(CaptureComponent2D->ShowFlags, bEnablePostProcessingEffects);

  CaptureComponent2D->UpdateContent();
  CaptureComponent2D->Activate();

  // Make sure that there is enough time in the render queue.
  UKismetSystemLibrary::ExecuteConsoleCommand(
      GetWorld(),
      FString("g.TimeoutForBlockOnRenderFence 300000"));

  Super::BeginPlay();
}

void ASceneCaptureSensor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  SCENE_CAPTURE_COUNTER = 0u;
}

void ASceneCaptureSensor::UpdateDrawFrustum()
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

namespace SceneCaptureSensor_local_ns {

  static void SetCameraDefaultOverrides(USceneCaptureComponent2D &CaptureComponent2D)
  {
    auto &PostProcessSettings = CaptureComponent2D.PostProcessSettings;
    PostProcessSettings.bOverride_AutoExposureMethod = false;
    PostProcessSettings.AutoExposureMethod = AEM_Histogram;
    PostProcessSettings.bOverride_AutoExposureMinBrightness = false;
    PostProcessSettings.AutoExposureMinBrightness = 0.27f;
    PostProcessSettings.bOverride_AutoExposureMaxBrightness = false;
    PostProcessSettings.AutoExposureMaxBrightness = 5.0f;
    PostProcessSettings.bOverride_AutoExposureBias = false;
    PostProcessSettings.AutoExposureBias = -3.5f;
  }

  // Remove the show flags that might interfere with post-processing effects like
  // depth and semantic segmentation.
  static void ConfigureShowFlags(FEngineShowFlags &ShowFlags, bool bPostProcessing)
  {
    if (bPostProcessing)
    {
      ShowFlags.EnableAdvancedFeatures();
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

} // namespace SceneCaptureSensor_local_ns
