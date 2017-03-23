// Fill out your copyright notice in the Description page of Project Settings.

#include "Carla.h"
#include "SceneCaptureCamera.h"

#include "Components/DrawFrustumComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Game/CarlaGameMode.h"
#include "HighResScreenshot.h"
#include "Paths.h"
#include "StaticMeshResources.h"
#include "TextureResource.h"

ASceneCaptureCamera::ASceneCaptureCamera(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer),
  SizeX(200u),
  SizeY(200u),
  PostProcessEffect(EPostProcessEffect::None)
{
  PrimaryActorTick.bCanEverTick = true;
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

  // Register this camera with the game mode.
  ACarlaGameMode *GameMode = Cast<ACarlaGameMode>(GetWorld()->GetAuthGameMode());
  if (GameMode != nullptr)
    GameMode->RegisterCaptureCamera(*this);
}

void ASceneCaptureCamera::Tick(float Delta)
{
  Super::Tick(Delta);
  // Update the image bitmap.
  FTextureRenderTargetResource* RTResource = CaptureRenderTarget->GameThread_GetRenderTargetResource();
  FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
  ReadPixelFlags.SetLinearToGamma(true);
  RTResource->ReadPixels(ImageBitMap, ReadPixelFlags);
}

FString ASceneCaptureCamera::GetPostProcessEffectAsString() const
{
  const UEnum* ptr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPostProcessEffect"), true);
  if(!ptr)
    return FString("Invalid");
  return ptr->GetEnumName(static_cast<int32>(PostProcessEffect));
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
