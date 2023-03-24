// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ProceduralBuildingUtilities.h"

#include "AssetRegistryModule.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "IMeshMergeUtilities.h"
#include "MeshMergeModule.h"
#include "UObject/Class.h"

void AProceduralBuildingUtilities::GenerateImpostor(const FVector& BuildingSize)
{
  USceneCaptureComponent2D* Camera = NewObject<USceneCaptureComponent2D>(this, USceneCaptureComponent2D::StaticClass(), TEXT("ViewProjectionCaptureComponent"));
  
  Camera->AttachToComponent(
      GetRootComponent(), 
      FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
      FName("ViewProjectionCaptureComponentName"));
  AddInstanceComponent(Camera);
  Camera->OnComponentCreated();
  Camera->RegisterComponent();

  check(Camera!=nullptr);

  Camera->ProjectionType = ECameraProjectionMode::Orthographic;
  Camera->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
  Camera->CaptureSource = ESceneCaptureSource::SCS_BaseColor;

  if(Camera->ShowOnlyActors.Num() == 0)
  {
    Camera->ShowOnlyActors.Add(this);
  }

  SetTargetTextureToSceneCaptureComponent(Camera);

  // FRONT View
  RenderImpostorView(Camera, BuildingSize, EBuildingCameraView::FRONT);
  // LEFT View
  RenderImpostorView(Camera, BuildingSize, EBuildingCameraView::LEFT);
  // BACK View
  RenderImpostorView(Camera, BuildingSize, EBuildingCameraView::BACK);
  // RIGHT View
  RenderImpostorView(Camera, BuildingSize, EBuildingCameraView::RIGHT);

  Camera->DestroyComponent();
}

void AProceduralBuildingUtilities::CookProceduralBuildingToMesh(const FString& DestinationPath, const FString& FileName)
{
  TArray<UPrimitiveComponent*> Components;
  TArray<UStaticMeshComponent*> StaticMeshComponents;
  GetComponents<UStaticMeshComponent>(StaticMeshComponents, false); // Components of class

  for(UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
  {
    if(StaticMeshComponent->GetStaticMesh())
    {
      Components.Add(StaticMeshComponent);
    }
  }

  UWorld* World = GetWorld();

  FMeshMergingSettings MeshMergeSettings;
  TArray<UObject*> AssetsToSync;

  FVector NewLocation;
  const float ScreenAreaSize = TNumericLimits<float>::Max();

  FString PackageName = DestinationPath + FileName;
  UPackage* NewPackage = CreatePackage(*PackageName);

  const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();
  MeshUtilities.MergeComponentsToStaticMesh(
      Components,
      World,
      MeshMergeSettings,
      nullptr,
      NewPackage,
      FileName,
      AssetsToSync,
      NewLocation,
      ScreenAreaSize,
      true);

  UPackage::SavePackage(NewPackage,
      AssetsToSync[0],
      EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *FileName,
      GError,
      nullptr,
      true,
      true,
      SAVE_NoError);
}

void AProceduralBuildingUtilities::RenderImpostorView(USceneCaptureComponent2D* Camera, const FVector BuildingSize, const EBuildingCameraView View)
{
  MoveCameraToViewPosition(Camera, BuildingSize, View);
  Camera->CaptureScene();
  UTextureRenderTarget2D* CameraRT = Camera->TextureTarget;
  UTexture2D* ViewTexture = CameraRT->ConstructTexture2D(GetWorld(), FString("View_"+View), EObjectFlags::RF_NoFlags );
  BakeSceneCaptureRTToTextureAtlas(ViewTexture, View);
}

void AProceduralBuildingUtilities::MoveCameraToViewPosition(USceneCaptureComponent2D* Camera, const FVector BuildingSize, const EBuildingCameraView View)
{
  // const float CameraFOVAngle = Camera->FOVAngle; // TODO: Get from camera
  const float BuildingHeight = BuildingSize.Z;
  float ViewAngle = 0.f;
  float BuildingWidth = 0.f;
  float BuildingDepth = 0.f;

  if(View == EBuildingCameraView::FRONT)
  {
    ViewAngle = 0.0f;
    BuildingWidth = FMath::Abs(BuildingSize.Y);
    BuildingDepth = FMath::Abs(BuildingSize.X);
  }
  else if(View == EBuildingCameraView::LEFT)
  {
    ViewAngle = 90.0f;
    BuildingWidth = FMath::Abs(BuildingSize.X);
    BuildingDepth = FMath::Abs(BuildingSize.Y);
  }
  else if(View == EBuildingCameraView::BACK)
  {
    ViewAngle = 180.0f;
    BuildingWidth = FMath::Abs(BuildingSize.Y);
    BuildingDepth = FMath::Abs(BuildingSize.X);
  } 
  else if(View == EBuildingCameraView::RIGHT)
  {
    ViewAngle = 270.0f;
    BuildingWidth = FMath::Abs(BuildingSize.X);
    BuildingDepth = FMath::Abs(BuildingSize.Y);
  }


  //float FrontAspectRatio = BuildingWidth / BuildingHeight;

  // Distance ajusted to height or width depending on which is bigger
  //float DistanceTangentFactor = FrontAspectRatio < 1.0f ? BuildingHeight : BuildingWidth; 


  // float DistanceTangentFactor = FMath::Max(BuildingWidth, BuildingHeight);

  // float CameraDistance = DistanceTangentFactor / (2.0f * FMath::Tan(FMath::DegreesToRadians(CameraFOVAngle / 2.0f)));
  // CameraDistance += BuildingWidth / 2.0f; 

  // FVector NewCameraLocation(
  //     CameraDistance * FMath::Cos(FMath::DegreesToRadians(ViewAngle)) + 0.5f * BuildingSize.X,
  //     CameraDistance * FMath::Sin(FMath::DegreesToRadians(ViewAngle)) + 0.5f * BuildingSize.Y,
  //     BuildingSize.Z / 2.0f);

  /* ORTHO - Works with materials problems */
  float ViewOrthoWidth = FMath::Max(BuildingWidth, BuildingHeight);
  Camera->OrthoWidth = ViewOrthoWidth;

  // float CameraDistance = 50000.0f;
  float CameraDistance = 0.5f * BuildingDepth + 1000.f;

  // FVector NewCameraLocation(
  //     CameraDistance * FMath::Cos(FMath::DegreesToRadians(ViewAngle)) + 0.5f * BuildingSize.X,
  //     CameraDistance * FMath::Sin(FMath::DegreesToRadians(ViewAngle)) + 0.5f * BuildingSize.Y,
  //     BuildingSize.Z / 2.0f);
  FVector NewCameraLocation(
      CameraDistance * FMath::Cos(FMath::DegreesToRadians(ViewAngle)),
      CameraDistance * FMath::Sin(FMath::DegreesToRadians(ViewAngle)),
      BuildingSize.Z / 2.0f);
  /*****************/

  FRotator NewCameraRotation(0.0f, ViewAngle + 180.f, 0.0f);

  Camera->SetRelativeLocationAndRotation(NewCameraLocation, NewCameraRotation, false, nullptr, ETeleportType::None);
}
