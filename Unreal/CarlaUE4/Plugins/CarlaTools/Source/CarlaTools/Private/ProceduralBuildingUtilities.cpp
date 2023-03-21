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

void AProceduralBuildingUtilities::GenerateImpostor()
{
  // TODO: 
  USceneCaptureComponent2D* Camera = Cast<USceneCaptureComponent2D>(GetComponentByClass(USceneCaptureComponent2D::StaticClass()));

  check(Camera!=nullptr);

  const FVector BuildingSize(400.0f, -400.0f, 2030.0f); // TODO: Change Hardcoded default debugging values to real values

  // FRONT
  RenderImpostorView(Camera, BuildingSize, EBuildingCameraView::FRONT);
  // LEFT
  RenderImpostorView(Camera, BuildingSize, EBuildingCameraView::LEFT);
  // BACK
  RenderImpostorView(Camera, BuildingSize, EBuildingCameraView::BACK);
  // RIGHT
  RenderImpostorView(Camera, BuildingSize, EBuildingCameraView::RIGHT);
  
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
  const float CameraFOVAngle = 60.0f; // TODO: Get from camera
  float BuildingHeight = BuildingSize.Z;
  float ViewAngle = 0.f;
  float BuildingWidth = 0.f;

  if(View == EBuildingCameraView::FRONT)
  {
    ViewAngle = 0.0f;
    BuildingWidth = FMath::Abs(BuildingSize.X);
  }
  else if(View == EBuildingCameraView::LEFT)
  {
    ViewAngle = 90.0f;
    BuildingWidth = FMath::Abs(BuildingSize.Y);
  }
  else if(View == EBuildingCameraView::BACK)
  {
    ViewAngle = 180.0f;
    BuildingWidth = FMath::Abs(BuildingSize.X);
  } 
  else if(View == EBuildingCameraView::RIGHT)
  {
    ViewAngle = 270.0f;
    BuildingWidth = FMath::Abs(BuildingSize.Y);
  }


  float FrontAspectRation = BuildingWidth / BuildingHeight;

  float CameraDistance = BuildingHeight / (2.0f * FMath::Tan(FMath::DegreesToRadians(CameraFOVAngle / 2.0f)));
  CameraDistance += BuildingWidth / 2.0f;

  FVector NewCameraLocation(
      CameraDistance * FMath::Cos(FMath::DegreesToRadians(ViewAngle)) + 0.5f * BuildingSize.X,
      CameraDistance * FMath::Sin(FMath::DegreesToRadians(ViewAngle)) + 0.5f * BuildingSize.Y,
      BuildingSize.Z / 2.0f);

  FRotator NewCameraRotation(0.0f, ViewAngle + 180.f, 0.0f);

  Camera->SetRelativeLocationAndRotation(NewCameraLocation, NewCameraRotation, false, nullptr, ETeleportType::None);
}
