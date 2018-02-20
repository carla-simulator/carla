// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "SceneCaptureCamera.h"

#include "Components/DrawFrustumComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HighResScreenshot.h"
#include "Materials/Material.h"
#include "Paths.h"



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

static void RemoveShowFlags(FEngineShowFlags &ShowFlags);

ASceneCaptureCamera::ASceneCaptureCamera(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer),
  SizeX(720u),
  SizeY(512u),
  PostProcessEffect(EPostProcessEffect::SceneFinal)
{
  PrimaryActorTick.bCanEverTick = true; /// @todo Does it need to tick?
  PrimaryActorTick.TickGroup = TG_PrePhysics;

  MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CamMesh0"));

  MeshComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

  MeshComp->bHiddenInGame = true;
  MeshComp->CastShadow = false;
  MeshComp->PostPhysicsComponentTick.bCanEverTick = false;
  RootComponent = MeshComp;

  DrawFrustum = CreateDefaultSubobject<UDrawFrustumComponent>(TEXT("DrawFrust0"));
  DrawFrustum->bIsEditorOnly = true;
  DrawFrustum->SetupAttachment(MeshComp);

  CaptureRenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("CaptureRenderTarget0"));
  #if WITH_EDITORONLY_DATA
	CaptureRenderTarget->CompressionNoAlpha = true;
	CaptureRenderTarget->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
  #endif
  CaptureRenderTarget->CompressionSettings = TextureCompressionSettings::TC_Default;
  
  //CaptureRenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::
  CaptureRenderTarget->SRGB=0;
  CaptureRenderTarget->bAutoGenerateMips = false;
  CaptureRenderTarget->AddressX = TextureAddress::TA_Clamp;
  CaptureRenderTarget->AddressY = TextureAddress::TA_Clamp;
  
  //-------------------

  CaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));

  //--addded by jbelon
  //CaptureComponent2D->
  //------------------

  CaptureComponent2D->SetupAttachment(MeshComp);
  
  // Load post-processing materials.
  static ConstructorHelpers::FObjectFinder<UMaterial> DEPTH(DEPTH_MAT_PATH);
  PostProcessDepth = DEPTH.Object;
  static ConstructorHelpers::FObjectFinder<UMaterial> SEMANTIC_SEGMENTATION(SEMANTIC_SEGMENTATION_MAT_PATH);
  PostProcessSemanticSegmentation = SEMANTIC_SEGMENTATION.Object;
}

void ASceneCaptureCamera::PostActorCreated()
{
  Super::PostActorCreated();

  // no need load the editor mesh when there is no editor
#if WITH_EDITOR
  if(MeshComp)
  {
    if (!IsRunningCommandlet())
    {
      if( !MeshComp->GetStaticMesh())
      {
        UStaticMesh* CamMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/MatineeCam_SM.MatineeCam_SM"), NULL, LOAD_None, NULL);
        MeshComp->SetStaticMesh(CamMesh);
      }
    }
  }
#endif // WITH_EDITOR

  // Sync component with CameraActor frustum settings.
  UpdateDrawFrustum();
}

void ASceneCaptureCamera::BeginPlay()
{
  const bool bRemovePostProcessing = (PostProcessEffect != EPostProcessEffect::SceneFinal);

  // Setup render target.
  const bool bInForceLinearGamma = bRemovePostProcessing;
  CaptureRenderTarget->InitCustomFormat(SizeX, SizeY, PF_B8G8R8A8, bInForceLinearGamma);

  CaptureComponent2D->Deactivate();
  CaptureComponent2D->TextureTarget = CaptureRenderTarget;

  // Setup camera post-processing.
  if (PostProcessEffect != EPostProcessEffect::None) {
    CaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
  }
  if (bRemovePostProcessing) {
    RemoveShowFlags(CaptureComponent2D->ShowFlags);
  }
  if (PostProcessEffect == EPostProcessEffect::Depth) {
    CaptureComponent2D->PostProcessSettings.AddBlendable(PostProcessDepth, 1.0f);
  } else if (PostProcessEffect == EPostProcessEffect::SemanticSegmentation) {
    CaptureComponent2D->PostProcessSettings.AddBlendable(PostProcessSemanticSegmentation, 1.0f);
  }

  CaptureComponent2D->UpdateContent();
  CaptureComponent2D->Activate();

  Super::BeginPlay();
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
  if (PostProcessEffect != EPostProcessEffect::SceneFinal) {
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

void ASceneCaptureCamera::Set(const FCameraDescription &CameraDescription)
{
  SetImageSize(CameraDescription.ImageSizeX, CameraDescription.ImageSizeY);
  SetPostProcessEffect(CameraDescription.PostProcessEffect);
  SetFOVAngle(CameraDescription.FOVAngle);
}

void ASceneCaptureCamera::Set(
    const FCameraDescription &CameraDescription,
    const FCameraPostProcessParameters &OverridePostProcessParameters)
{
  auto &PostProcessSettings = CaptureComponent2D->PostProcessSettings;
  PostProcessSettings.bOverride_AutoExposureMethod = true;
  PostProcessSettings.AutoExposureMethod = OverridePostProcessParameters.AutoExposureMethod;
  PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
  PostProcessSettings.AutoExposureMinBrightness = OverridePostProcessParameters.AutoExposureMinBrightness;
  PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
  PostProcessSettings.AutoExposureMaxBrightness = OverridePostProcessParameters.AutoExposureMaxBrightness;
  PostProcessSettings.bOverride_AutoExposureBias = true;
  PostProcessSettings.AutoExposureBias = OverridePostProcessParameters.AutoExposureBias;
  Set(CameraDescription);
}

bool ASceneCaptureCamera::ReadPixels(TArray<FColor> &BitMap) const
{
  if(!CaptureRenderTarget)
  {
	  UE_LOG(LogCarla, Error, TEXT("SceneCaptureCamera: Missing render target"));
	  return false;
  }
  FTextureRenderTargetResource* RTResource = CaptureRenderTarget->GetRenderTargetResource(); //  CaptureRenderTarget->GameThread_GetRenderTargetResource();
  if (RTResource == nullptr) {
    UE_LOG(LogCarla, Error, TEXT("SceneCaptureCamera: Missing render target"));
    return false;
  }
  FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
  ReadPixelFlags.SetLinearToGamma(true);
  return RTResource->ReadPixels(BitMap, ReadPixelFlags);
}

void ASceneCaptureCamera::WritePixels(TArray<FColor>& RawData)
 {
  check(IsInRenderingThread());
  if(!CaptureRenderTarget)
  {
  	UE_LOG(LogCarla, Error, TEXT("SceneCaptureCamera: Missing render target"));
  	return ;
  }
  FRHITexture2D *texture = CaptureRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
  
  uint32 width = texture->GetSizeX();
  uint32 height = texture->GetSizeY();
  uint32 stride;
  uint8 *src = reinterpret_cast<uint8*>(RHILockTexture2D(texture, 0, RLM_ReadOnly, stride, false));
  const uint32 bufferSize = stride * height;
	//Alternative Method 0: deeplearning
  /*void *buffer = FMemory::Malloc(bufferSize, 4);
  if(!buffer)
  {
	UE_LOG(LogCarla, Display, TEXT("Error allocating memory"));
	RHIUnlockTexture2D(texture, 0, false);
	return ;	
  }
  //FMemory::BigBlockMemcpy(buffer, src, bufferSize);*/
  RawData.Empty(width*height);
  for(unsigned y = 0; y < height; ++y)
  {
    for(unsigned x = 0; x < width; ++x)
	{
		//RGBA -> 
		RawData.Add(FColor(src[2],src[1],src[0],src[3]));
		src += 4;
	}
  }
  
  RHIUnlockTexture2D(texture, 0, false);
	//Alternative Method 1
   /*FColor* RHIData = reinterpret_cast<FColor*>(RHILockTexture2D(TextureRHI, 0, RLM_ReadOnly, stride, false, false));
	 if(!RHIData) {
		 UE_LOG(LogCarla,Display,TEXT("ASceneCaptureCamera::WritePixels : RHIData is null"));
	 	 return;
	 }
	 RawData.Append(RHIData,Width*Height);*/
	//Alternative Method 2
  /*uint8* OriginBuffer = reinterpret_cast<uint8*>(RHILockTexture2D(TextureRHI, 0, RLM_ReadOnly, stride, false,false));
	if(OriginBuffer)
	{
	  for (int32 y = 0; y < Height; y++)
      {
         uint8* OriginPtr = &OriginBuffer[(Height - 1 - y) * stride];
         for (int32 x = 0; x < Width; x++)
         {
			 RawData[(Height - 1 - y) * Width] = FColor(*OriginPtr++,*OriginPtr++,*OriginPtr++,*OriginPtr++);
         }
      }
	}
	*/
	//Alternative Method 3
  /*
	FRHITexture2D* Texture2D = TextureRHI->GetTexture2D();
	uint8* TextureBuffer = (uint8*)RHILockTexture2D(Texture2D, 0, RLM_ReadOnly, stride, false, false);
	FMemory::Memcpy( RawData.GetData(), TextureBuffer, stride + (Width*Height*4) );
	*/
	//End of Alternative methods lock
  //RHIUnlockTexture2D(TextureRHI, 0, false, false);
}


void ASceneCaptureCamera::UpdateDrawFrustum()
{
  if(DrawFrustum && CaptureComponent2D)
  {
    DrawFrustum->FrustumStartDist = GNearClippingPlane;

    // 1000 is the default frustum distance, ideally this would be infinite but that might cause rendering issues
    DrawFrustum->FrustumEndDist = (CaptureComponent2D->MaxViewDistanceOverride > DrawFrustum->FrustumStartDist)
      ? CaptureComponent2D->MaxViewDistanceOverride : 1000.0f;

    DrawFrustum->FrustumAngle = CaptureComponent2D->FOVAngle;
    //DrawFrustum->FrustumAspectRatio = CaptureComponent2D->AspectRatio;
  }
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
