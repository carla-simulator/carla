// Fill out your copyright notice in the Description page of Project Settings.

#include "Carla.h"
#include "SceneCaptureCamera.h"

#include "Components/DrawFrustumComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HighResScreenshot.h"
#include "Paths.h"
#include "StaticMeshResources.h"
#include "TextureResource.h"

// -- Static methods -----------------------------------------------------------

static void SaveRenderTargetToDisk(UTextureRenderTarget2D* InRenderTarget, FString Filename)
{
  FTextureRenderTargetResource* RTResource = InRenderTarget->GameThread_GetRenderTargetResource();

  FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
  ReadPixelFlags.SetLinearToGamma(true);

  TArray<FColor> OutBMP;
  RTResource->ReadPixels(OutBMP, ReadPixelFlags);
  for (FColor& color : OutBMP)
  {
    color.A = 255;
  }
  FIntPoint DestSize(InRenderTarget->GetSurfaceWidth(), InRenderTarget->GetSurfaceHeight());
  FString ResultPath;
  FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();
  HighResScreenshotConfig.SaveImage(Filename, OutBMP, DestSize, &ResultPath);
}

// -- ASceneCaptureCamera ------------------------------------------------------

ASceneCaptureCamera::ASceneCaptureCamera(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer),
  SaveToFolder(FPlatformProcess::UserDir()),
  FileName("capture_%05d.png"),
  CapturesPerSecond(15.0f),
  SizeX(200u),
  SizeY(200u)
{
  PrimaryActorTick.bCanEverTick = true;

  MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CamMesh0"));

  MeshComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

  MeshComp->bHiddenInGame = true;
  MeshComp->CastShadow = false;
  MeshComp->PostPhysicsComponentTick.bCanEverTick = false;
  RootComponent = MeshComp;

  DrawFrustum = CreateDefaultSubobject<UDrawFrustumComponent>(TEXT("DrawFrust0"));
  DrawFrustum->bIsEditorOnly = true;
  DrawFrustum->SetupAttachment(GetMeshComp());

  CaptureRenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("CaptureRenderTarget0"));

  CaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));
  CaptureComponent2D->SetupAttachment(GetMeshComp());
}

ASceneCaptureCamera::~ASceneCaptureCamera() {}

void ASceneCaptureCamera::OnInterpToggle(bool bEnable)
{
  CaptureComponent2D->SetVisibility(bEnable);
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

void ASceneCaptureCamera::PostActorCreated()
{
  Super::PostActorCreated();

  // no need load the editor mesh when there is no editor
#if WITH_EDITOR
  if(GetMeshComp())
  {
    if (!IsRunningCommandlet())
    {
      if( !GetMeshComp()->GetStaticMesh())
      {
        UStaticMesh* CamMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/MatineeCam_SM.MatineeCam_SM"), NULL, LOAD_None, NULL);
        GetMeshComp()->SetStaticMesh(CamMesh);
      }
    }
  }
#endif

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
  CaptureComponent2D->UpdateContent();
  CaptureComponent2D->Activate();
}

void ASceneCaptureCamera::Tick(float Delta)
{
  Super::Tick(Delta);

  const float CaptureUpdateTime = 1.0f / CapturesPerSecond;
  ElapsedTimeSinceLastCapture += Delta;

  if (bCaptureScene && (ElapsedTimeSinceLastCapture >= CaptureUpdateTime))
  {
    if (CaptureComponent2D == nullptr)
    {
      UE_LOG(LogCarla, Error, TEXT("Missing capture component"));
    }

    if (CaptureRenderTarget)
    {
      // Capture scene.
      FString FilePath = FPaths::Combine(SaveToFolder, FString::Printf(*FileName, CaptureFileNameCount));
      //UE_LOG(LogCarla, Log, TEXT("Delta %fs: Capture %s"), ElapsedTimeSinceLastCapture, *FilePath);
      SaveRenderTargetToDisk(CaptureRenderTarget, FilePath);
      ++CaptureFileNameCount;
    }
    else
    {
      UE_LOG(LogCarla, Error, TEXT("Missing render target"));
    }
    ElapsedTimeSinceLastCapture = 0.0f;
  }
}

UStaticMeshComponent* ASceneCaptureCamera::GetMeshComp() const
{
  return MeshComp;
}

USceneCaptureComponent2D* ASceneCaptureCamera::GetCaptureComponent2D() const
{
  return CaptureComponent2D;
}

UDrawFrustumComponent* ASceneCaptureCamera::GetDrawFrustum() const
{
  return DrawFrustum;
}
