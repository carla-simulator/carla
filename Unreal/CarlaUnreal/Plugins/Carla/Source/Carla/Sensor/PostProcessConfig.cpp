#include "PostProcessConfig.h"
#include <util/ue-header-guard-begin.h>
#include "Components/SceneCaptureComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include <util/ue-header-guard-end.h>

void FPostProcessConfig::EnablePostProcessingEffects()
{
  // This block of code {
  EngineShowFlags.SetLensFlares(true);
  EngineShowFlags.SetEyeAdaptation(true);
  EngineShowFlags.SetColorGrading(true);
  EngineShowFlags.SetCameraImperfections(true);
  EngineShowFlags.SetDepthOfField(true);
  EngineShowFlags.SetVignette(true);
  EngineShowFlags.SetGrain(true);
  EngineShowFlags.SetSeparateTranslucency(true);
  EngineShowFlags.SetScreenSpaceReflections(true);
  EngineShowFlags.SetTemporalAA(true);
  EngineShowFlags.SetAmbientOcclusion(true);
  EngineShowFlags.SetIndirectLightingCache(true);
  EngineShowFlags.SetLightShafts(true);
  EngineShowFlags.SetPostProcessMaterial(true);
  EngineShowFlags.SetDistanceFieldAO(true);
  // } must be kept in sync with EngineShowFlags.EnableAdvancedFeatures(), AND activate Lumen.
  EngineShowFlags.SetMotionBlur(true);
}

void FPostProcessConfig::DisablePostProcessingEffects()
{
  EngineShowFlags.SetAmbientOcclusion(false);
  EngineShowFlags.SetAntiAliasing(false);
  EngineShowFlags.SetVolumetricFog(false);
  // EngineShowFlags.SetAtmosphericFog(false);
  // EngineShowFlags.SetAudioRadius(false);
  // EngineShowFlags.SetBillboardSprites(false);
  EngineShowFlags.SetBloom(false);
  // EngineShowFlags.SetBounds(false);
  // EngineShowFlags.SetBrushes(false);
  // EngineShowFlags.SetBSP(false);
  // EngineShowFlags.SetBSPSplit(false);
  // EngineShowFlags.SetBSPTriangles(false);
  // EngineShowFlags.SetBuilderBrush(false);
  // EngineShowFlags.SetCameraAspectRatioBars(false);
  // EngineShowFlags.SetCameraFrustums(false);
  EngineShowFlags.SetCameraImperfections(false);
  EngineShowFlags.SetCameraInterpolation(false);
  // EngineShowFlags.SetCameraSafeFrames(false);
  // EngineShowFlags.SetCollision(false);
  // EngineShowFlags.SetCollisionPawn(false);
  // EngineShowFlags.SetCollisionVisibility(false);
  EngineShowFlags.SetColorGrading(false);
  // EngineShowFlags.SetCompositeEditorPrimitives(false);
  // EngineShowFlags.SetConstraints(false);
  // EngineShowFlags.SetCover(false);
  // EngineShowFlags.SetDebugAI(false);
  // EngineShowFlags.SetDecals(false);
  // EngineShowFlags.SetDeferredLighting(false);
  EngineShowFlags.SetDepthOfField(false);
  EngineShowFlags.SetDiffuse(false);
  EngineShowFlags.SetDirectionalLights(false);
  EngineShowFlags.SetDirectLighting(false);
  // EngineShowFlags.SetDistanceCulledPrimitives(false);
  // EngineShowFlags.SetDistanceFieldAO(false);
  // EngineShowFlags.SetDistanceFieldGI(false);
  EngineShowFlags.SetDynamicShadows(false);
  // EngineShowFlags.SetEditor(false);
  EngineShowFlags.SetEyeAdaptation(false);
  EngineShowFlags.SetFog(false);
  // EngineShowFlags.SetGame(false);
  // EngineShowFlags.SetGameplayDebug(false);
  // EngineShowFlags.SetGBufferHints(false);
  EngineShowFlags.SetGlobalIllumination(false);
  EngineShowFlags.SetGrain(false);
  // EngineShowFlags.SetGrid(false);
  // EngineShowFlags.SetHighResScreenshotMask(false);
  // EngineShowFlags.SetHitProxies(false);
  EngineShowFlags.SetHLODColoration(false);
  EngineShowFlags.SetHMDDistortion(false);
  // EngineShowFlags.SetIndirectLightingCache(false);
  // EngineShowFlags.SetInstancedFoliage(false);
  // EngineShowFlags.SetInstancedGrass(false);
  // EngineShowFlags.SetInstancedStaticMeshes(false);
  // EngineShowFlags.SetLandscape(false);
  // EngineShowFlags.SetLargeVertices(false);
  EngineShowFlags.SetLensFlares(false);
  EngineShowFlags.SetLightComplexity(false);
  EngineShowFlags.SetLightFunctions(false);
  EngineShowFlags.SetLightInfluences(false);
  EngineShowFlags.SetLighting(false);
  EngineShowFlags.SetLightMapDensity(false);
  EngineShowFlags.SetLightRadius(false);
  EngineShowFlags.SetLightShafts(false);
  // EngineShowFlags.SetLOD(false);
  EngineShowFlags.SetLODColoration(false);
  // EngineShowFlags.SetMaterials(false);
  // EngineShowFlags.SetMaterialTextureScaleAccuracy(false);
  // EngineShowFlags.SetMeshEdges(false);
  // EngineShowFlags.SetMeshUVDensityAccuracy(false);
  // EngineShowFlags.SetModeWidgets(false);
  EngineShowFlags.SetMotionBlur(false);
  // EngineShowFlags.SetNavigation(false);
  EngineShowFlags.SetOnScreenDebug(false);
  // EngineShowFlags.SetOutputMaterialTextureScales(false);
  // EngineShowFlags.SetOverrideDiffuseAndSpecular(false);
  // EngineShowFlags.SetPaper2DSprites(false);
  EngineShowFlags.SetParticles(false);
  // EngineShowFlags.SetPivot(false);
  EngineShowFlags.SetPointLights(false);
  // EngineShowFlags.SetPostProcessing(false);
  // EngineShowFlags.SetPostProcessMaterial(false);
  // EngineShowFlags.SetPrecomputedVisibility(false);
  // EngineShowFlags.SetPrecomputedVisibilityCells(false);
  // EngineShowFlags.SetPreviewShadowsIndicator(false);
  // EngineShowFlags.SetPrimitiveDistanceAccuracy(false);
  // EngineShowFlags.SetQuadOverdraw(false);
  // EngineShowFlags.SetReflectionEnvironment(false);
  // EngineShowFlags.SetReflectionOverride(false);
  EngineShowFlags.SetRefraction(false);
  // EngineShowFlags.SetRendering(false);
  EngineShowFlags.SetSceneColorFringe(false);
  // EngineShowFlags.SetScreenPercentage(false);
  EngineShowFlags.SetScreenSpaceAO(false);
  EngineShowFlags.SetScreenSpaceReflections(false);
  // EngineShowFlags.SetSelection(false);
  // EngineShowFlags.SetSelectionOutline(false);
  // EngineShowFlags.SetSeparateTranslucency(false);
  // EngineShowFlags.SetShaderComplexity(false);
  // EngineShowFlags.SetShaderComplexityWithQuadOverdraw(false);
  // EngineShowFlags.SetShadowFrustums(false);
  // EngineShowFlags.SetSkeletalMeshes(false);
  // EngineShowFlags.SetSkinCache(false);
  EngineShowFlags.SetSkyLighting(false);
  // EngineShowFlags.SetSnap(false);
  // EngineShowFlags.SetSpecular(false);
  // EngineShowFlags.SetSplines(false);
  EngineShowFlags.SetSpotLights(false);
  // EngineShowFlags.SetStaticMeshes(false);
  EngineShowFlags.SetStationaryLightOverlap(false);
  // EngineShowFlags.SetStereoRendering(false);
  // EngineShowFlags.SetStreamingBounds(false);
  EngineShowFlags.SetSubsurfaceScattering(false);
  // EngineShowFlags.SetTemporalAA(false);
  // EngineShowFlags.SetTessellation(false);
  // EngineShowFlags.SetTestImage(false);
  // EngineShowFlags.SetTextRender(false);
  // EngineShowFlags.SetTexturedLightProfiles(false);
  EngineShowFlags.SetTonemapper(false);
  // EngineShowFlags.SetTranslucency(false);
  // EngineShowFlags.SetVectorFields(false);
  // EngineShowFlags.SetVertexColors(false);
  // EngineShowFlags.SetVignette(false);
  // EngineShowFlags.SetVisLog(false);
  // EngineShowFlags.SetVisualizeAdaptiveDOF(false);
  // EngineShowFlags.SetVisualizeBloom(false);
  EngineShowFlags.SetVisualizeBuffer(false);
  EngineShowFlags.SetVisualizeDistanceFieldAO(false);
  EngineShowFlags.SetVisualizeDOF(false);
  EngineShowFlags.SetVisualizeHDR(false);
  EngineShowFlags.SetVisualizeLightCulling(false);
  EngineShowFlags.SetVisualizeMeshDistanceFields(false);
  EngineShowFlags.SetVisualizeMotionBlur(false);
  EngineShowFlags.SetVisualizeOutOfBoundsPixels(false);
  EngineShowFlags.SetVisualizeSenses(false);
  EngineShowFlags.SetVisualizeShadingModels(false);
  EngineShowFlags.SetVisualizeSSR(false);
  EngineShowFlags.SetVisualizeSSS(false);
  // EngineShowFlags.SetVolumeLightingSamples(false);
  // EngineShowFlags.SetVolumes(false);
  // EngineShowFlags.SetWidgetComponents(false);
  // EngineShowFlags.SetWireframe(false);
}

FPostProcessConfig::FPostProcessConfig(
  FPostProcessSettings& InPostProcessSettings,
  FEngineShowFlags& InEngineShowFlags) :
  PostProcessSettings(InPostProcessSettings),
  EngineShowFlags(InEngineShowFlags)
{
}

void FPostProcessConfig::UpdateFromSceneCaptureComponent(const USceneCaptureComponent& Component)
{
  EngineShowFlags = Component.ShowFlags;
}

void FPostProcessConfig::UpdateFromSceneCaptureComponent2D(const USceneCaptureComponent2D& Component)
{
  EngineShowFlags = Component.ShowFlags;
  PostProcessSettings = Component.PostProcessSettings;
}

void FPostProcessConfig::EnablePostProcessingEffects(bool Enable)
{
  if (Enable)
    EnablePostProcessingEffects();
  else
    DisablePostProcessingEffects();
}
