// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

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
#include "StaticMeshResources.h"
#include "TextureResource.h"

static constexpr auto DEPTH_MAT_PATH =
    TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial.DepthEffectMaterial'");
static constexpr auto SEMANTIC_SEGMENTATION_MAT_PATH =
    TEXT("Material'/Carla/PostProcessingMaterials/GTMaterial.GTMaterial'");

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

  CaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));
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
  Super::BeginPlay();

  CaptureRenderTarget->InitCustomFormat(SizeX, SizeY, PF_B8G8R8A8, false);
  CaptureRenderTarget->UpdateResource();

  CaptureComponent2D->Deactivate();
  CaptureComponent2D->TextureTarget = CaptureRenderTarget;
  if (PostProcessEffect == EPostProcessEffect::SceneFinal) {
    CaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
  } else if (PostProcessEffect == EPostProcessEffect::Depth) {
    CaptureComponent2D->PostProcessSettings.AddBlendable(PostProcessDepth, 1.0f);
  } else if (PostProcessEffect == EPostProcessEffect::SemanticSegmentation) {
    CaptureComponent2D->PostProcessSettings.AddBlendable(PostProcessSemanticSegmentation, 1.0f);
  }
  CaptureComponent2D->UpdateContent();
  CaptureComponent2D->Activate();
}

void ASceneCaptureCamera::SetImageSize(uint32 otherSizeX, uint32 otherSizeY)
{
  SizeX = otherSizeX;
  SizeY = otherSizeY;
}

void ASceneCaptureCamera::SetPostProcessEffect(EPostProcessEffect otherPostProcessEffect)
{
  PostProcessEffect = otherPostProcessEffect;
}

void ASceneCaptureCamera::Set(const FCameraDescription &CameraDescription)
{
  SetImageSize(CameraDescription.ImageSizeX, CameraDescription.ImageSizeY);
  SetPostProcessEffect(CameraDescription.PostProcessEffect);
}

bool ASceneCaptureCamera::ReadPixels(TArray<FColor> &BitMap) const
{
  FTextureRenderTargetResource* RTResource = CaptureRenderTarget->GameThread_GetRenderTargetResource();
  if (RTResource == nullptr) {
    UE_LOG(LogCarla, Error, TEXT("SceneCaptureCamera: Missing render target"));
    return false;
  }
  FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
  ReadPixelFlags.SetLinearToGamma(true);
  return RTResource->ReadPixels(BitMap, ReadPixelFlags);
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
