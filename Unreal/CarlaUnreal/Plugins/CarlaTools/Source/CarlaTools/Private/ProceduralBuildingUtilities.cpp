// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ProceduralBuildingUtilities.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include <util/ue-header-guard-begin.h>
#include "ProceduralMeshConversion.h"
#include "MeshMerge/MeshMergingSettings.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "IMeshMergeUtilities.h"
#include "Materials/MaterialInstanceConstant.h"
#include "MeshMergeModule.h"
#include "ProceduralMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/SavePackage.h"
#include "PhysicsEngine/BodySetup.h"
#include <util/ue-header-guard-end.h>

void AProceduralBuildingUtilities::GenerateImpostorTexture(const FVector& BuildingSize)
{
  USceneCaptureComponent2D* Camera = NewObject<USceneCaptureComponent2D>(
      this, USceneCaptureComponent2D::StaticClass(), TEXT("ViewProjectionCaptureComponent"));

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

  check(Camera->TextureTarget != nullptr);

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

UProceduralMeshComponent* AProceduralBuildingUtilities::GenerateImpostorGeometry(const FVector& BuildingSize)
{
  // Spawn procedural mesh actor / component
  UProceduralMeshComponent* Mesh = NewObject<UProceduralMeshComponent>(
      this, UProceduralMeshComponent::StaticClass(), TEXT("LOD Impostor Mesh"));

  Mesh->AttachToComponent(
      GetRootComponent(), 
      FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
      FName("LOD Impostor Mesh"));
  AddInstanceComponent(Mesh);
  Mesh->OnComponentCreated();
  Mesh->RegisterComponent();

  check(Mesh != nullptr)

  // FRONT View
  CreateBuildingImpostorGeometryForView(Mesh, BuildingSize, EBuildingCameraView::FRONT);
  // LEFT View
  CreateBuildingImpostorGeometryForView(Mesh, BuildingSize, EBuildingCameraView::LEFT);
  // BACK View
  CreateBuildingImpostorGeometryForView(Mesh, BuildingSize, EBuildingCameraView::BACK);
  // RIGHT View
  CreateBuildingImpostorGeometryForView(Mesh, BuildingSize, EBuildingCameraView::RIGHT);

  return Mesh;

  // Cook new mesh to a static mesh

  // Assign as LOD or store on disk for manual import
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
  check(NewPackage);

  auto& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>(
    "MeshMergeUtilities").GetUtilities();
  
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

  FSavePackageArgs SaveArgs;
  SaveArgs.TopLevelFlags =
    EObjectFlags::RF_Public |
    EObjectFlags::RF_Standalone;
  SaveArgs.Error = GError;
  SaveArgs.bForceByteSwapping = true;
  SaveArgs.bWarnOfLongFilename = true;
  SaveArgs.SaveFlags = SAVE_NoError;

  UPackage::SavePackage(
    NewPackage,
    AssetsToSync[0],
    *FileName,
    SaveArgs);

}

void AProceduralBuildingUtilities::CookProceduralMeshToMesh(
    UProceduralMeshComponent* Mesh,
    const FString& DestinationPath,
    const FString& FileName)
{
  FMeshDescription MeshDescription = BuildMeshDescription(Mesh);

  FString PackageName = DestinationPath + FileName;
  UPackage* NewPackage = CreatePackage(*PackageName);
  check(NewPackage);

  UStaticMesh* StaticMesh = NewObject<UStaticMesh>(NewPackage, *FileName, RF_Public | RF_Standalone);
  StaticMesh->InitResources();
  StaticMesh->SetLightingGuid(FGuid::NewGuid());

  FStaticMeshSourceModel& SrcModel = StaticMesh->AddSourceModel();
  SrcModel.BuildSettings.bRecomputeNormals = false;
  SrcModel.BuildSettings.bRecomputeTangents = false;
  SrcModel.BuildSettings.bRemoveDegenerates = false;
  SrcModel.BuildSettings.bUseHighPrecisionTangentBasis = false;
  SrcModel.BuildSettings.bUseFullPrecisionUVs = false;
  SrcModel.BuildSettings.bGenerateLightmapUVs = true;
  SrcModel.BuildSettings.SrcLightmapIndex = 0;
  SrcModel.BuildSettings.DstLightmapIndex = 1;
  StaticMesh->CreateMeshDescription(0, MoveTemp(MeshDescription));
  StaticMesh->CommitMeshDescription(0);

  if (!Mesh->bUseComplexAsSimpleCollision )
  {
    StaticMesh->CreateBodySetup();
    UBodySetup* BodySetupPtr = StaticMesh->GetBodySetup();
    BodySetupPtr->BodySetupGuid = FGuid::NewGuid();
    BodySetupPtr->AggGeom.ConvexElems = Mesh->ProcMeshBodySetup->AggGeom.ConvexElems;
    BodySetupPtr->bGenerateMirroredCollision = false;
    BodySetupPtr->bDoubleSidedGeometry = true;
    BodySetupPtr->CollisionTraceFlag = CTF_UseDefault;
    BodySetupPtr->CreatePhysicsMeshes();
  }

  TSet<UMaterialInterface*> UniqueMaterials;
  const int32 NumSections = Mesh->GetNumSections();
  for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
  {
    FProcMeshSection *ProcSection =
      Mesh->GetProcMeshSection(SectionIdx);
    UMaterialInterface *Material = Mesh->GetMaterial(SectionIdx);
    UniqueMaterials.Add(Material);
  }

  for (auto* Material : UniqueMaterials)
  {
    StaticMesh->GetStaticMaterials().Add(FStaticMaterial(Material));
  }

  StaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;
  StaticMesh->Build(false);
  StaticMesh->PostEditChange();
  FAssetRegistryModule::AssetCreated(StaticMesh);

  FSavePackageArgs SaveArgs;
  SaveArgs.TopLevelFlags =
      EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
  SaveArgs.Error = GError;
  SaveArgs.bForceByteSwapping = true;
  SaveArgs.bWarnOfLongFilename = true;
  SaveArgs.SaveFlags = SAVE_NoError;

  UPackage::SavePackage(NewPackage,
      StaticMesh,
      *FileName,
      SaveArgs);
}

void AProceduralBuildingUtilities::PlaceBuilding(AActor* Parent, TArray<UHierarchicalInstancedStaticMeshComponent*> Components)
{
  //Security wall.
  if(Parent == nullptr) return;

  //We need to create a component. This line is just for readability purposes.
  UClass* HSMClass = UHierarchicalInstancedStaticMeshComponent::StaticClass();

  //For every hierarchichal component we passed in the function, set 
  //In the parent actor a new hierarchichal component that copies 
  //the instances of the passed one.
  for(int i = 0; i < Components.Num(); i++)
  {
    //Creates the component. The index is needed so every component has a unique name, if not, each iteration
    //will just override the previous one. 
    UHierarchicalInstancedStaticMeshComponent* NewComponent = 
      NewObject<UHierarchicalInstancedStaticMeshComponent>(Parent, HSMClass, FName(Components[i]->GetStaticMesh().GetName() + FString::FromInt(i)));
    
    //Sets static mesh
    NewComponent->SetStaticMesh(Components[i]->GetStaticMesh());

    //Sets the instances transform.
    for(int j = 0; j < Components[i]->GetInstanceCount(); j++)
    {
      FTransform InstanceTransform;
      Components[i]->GetInstanceTransform(j, InstanceTransform, false);
      NewComponent->AddInstance(InstanceTransform, false);
    }

    //Registers the component in the parent actor and makes it visible for the user.
    NewComponent->RegisterComponent();
    NewComponent->AttachToComponent(
      Parent->GetRootComponent(),
      FAttachmentTransformRules::SnapToTargetIncludingScale);
    Parent->AddInstanceComponent(NewComponent);
  }
}

UMaterialInstanceConstant* AProceduralBuildingUtilities::GenerateBuildingMaterialAsset(
    const FString& DuplicateParentPath,
    const FString& DestinationPath,
    const FString& FileName)
{
  const FString BaseMaterialSearchPath = DuplicateParentPath;
  const FString PackageName = DestinationPath + FileName;

  UMaterialInstanceConstant* ParentMaterial = LoadObject<UMaterialInstanceConstant>(nullptr, *BaseMaterialSearchPath);

  check(ParentMaterial != nullptr);

  UPackage* NewPackage = CreatePackage(*PackageName);
  FObjectDuplicationParameters Parameters(ParentMaterial, NewPackage);
  Parameters.DestName = FName(FileName);
  Parameters.DestClass = ParentMaterial->GetClass();
  Parameters.DuplicateMode = EDuplicateMode::Normal;
  Parameters.PortFlags = PPF_Duplicate;

  UMaterialInstanceConstant* DuplicatedMaterial = CastChecked<UMaterialInstanceConstant>(StaticDuplicateObjectEx(Parameters));

  const FString PackageFileName = FPackageName::LongPackageNameToFilename(
      PackageName, 
      ".uasset");
  FSavePackageArgs SaveArgs;
  SaveArgs.TopLevelFlags =
      EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
  SaveArgs.Error = GError;
  SaveArgs.bForceByteSwapping = true;
  SaveArgs.bWarnOfLongFilename = true;
  SaveArgs.SaveFlags = SAVE_NoError;

  UPackage::SavePackage(NewPackage, DuplicatedMaterial, *PackageFileName, SaveArgs);

  return DuplicatedMaterial;
}

void AProceduralBuildingUtilities::RenderImpostorView(
    USceneCaptureComponent2D* Camera,
    const FVector BuildingSize,
    const EBuildingCameraView View)
{
  MoveCameraToViewPosition(Camera, BuildingSize, View);
  Camera->CaptureScene();
  UTextureRenderTarget2D* CameraRT = Camera->TextureTarget;
  UTexture2D* ViewTexture = CameraRT->ConstructTexture2D(GetWorld(), FString("View_").Append(FString::FromInt(View)), EObjectFlags::RF_NoFlags );
  BakeSceneCaptureRTToTextureAtlas(ViewTexture, View);
}

void AProceduralBuildingUtilities::MoveCameraToViewPosition(
    USceneCaptureComponent2D* Camera,
    const FVector BuildingSize,
    const EBuildingCameraView View)
{
  const float BuildingHeight = BuildingSize.Z;
  float ViewAngle = 0.f;
  float BuildingWidth = 0.f;
  float BuildingDepth = 0.f;

  GetWidthDepthFromView(BuildingSize, View, BuildingWidth, BuildingDepth, ViewAngle);

  /* ORTHO */
  float ViewOrthoWidth = FMath::Max(BuildingWidth, BuildingHeight);
  Camera->OrthoWidth = ViewOrthoWidth;

  float CameraDistance = 0.5f * BuildingDepth + 1000.f;

  FVector NewCameraLocation(
      CameraDistance * FMath::Cos(FMath::DegreesToRadians(ViewAngle)),
      CameraDistance * FMath::Sin(FMath::DegreesToRadians(ViewAngle)),
      BuildingSize.Z / 2.0f);

  FRotator NewCameraRotation(0.0f, ViewAngle + 180.f, 0.0f);

  Camera->SetRelativeLocationAndRotation(NewCameraLocation, NewCameraRotation, false, nullptr, ETeleportType::None);
}

void AProceduralBuildingUtilities::CreateBuildingImpostorGeometryForView(
    UProceduralMeshComponent* Mesh,
    const FVector& BuildingSize,
    const EBuildingCameraView View)
{
  // Create vertices based on Building Size
  TArray<FVector> Vertices;
  TArray<int32> Triangles;

  const float BuildingHeight = BuildingSize.Z;
  float BuildingWidth = 0.f;
  float BuildingDepth = 0.f;
  float ViewAngle = 0.f;

  GetWidthDepthFromView(BuildingSize, View, BuildingWidth, BuildingDepth, ViewAngle);

  FVector RotationAxis(0.0f, 0.0f, 1.0f);
  FVector OriginOffset(- 0.5f * BuildingDepth, - 0.5f * BuildingWidth, 0.0f);

  // Vertices are created in local space, then offsetted to compensate origin and finally rotating
  // according to the ViewAngle
  Vertices.Add((FVector(0.0f, 0.0f, 0.0f)                     + OriginOffset).RotateAngleAxis(ViewAngle, RotationAxis));
  Vertices.Add((FVector(0.0f, 0.0f, BuildingHeight)           + OriginOffset).RotateAngleAxis(ViewAngle, RotationAxis));
  Vertices.Add((FVector(0.0f, BuildingWidth, 0.0f)            + OriginOffset).RotateAngleAxis(ViewAngle, RotationAxis));
  Vertices.Add((FVector(0.0f, BuildingWidth, BuildingHeight)  + OriginOffset).RotateAngleAxis(ViewAngle, RotationAxis));

  Triangles.Add(0); Triangles.Add(2); Triangles.Add(1); 
  Triangles.Add(2); Triangles.Add(3); Triangles.Add(1);

  TArray<FVector2D> UVs;
  CalculateViewGeometryUVs(BuildingWidth, BuildingHeight, View, UVs);

  TArray<FVector> Normals;
  Normals.Init(FVector(-1.0f, 0.0f, 0.0f).RotateAngleAxis(ViewAngle, RotationAxis), Vertices.Num());

  TArray<FLinearColor> Colors;
  Colors.Init(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f), Vertices.Num());

  TArray<FProcMeshTangent> Tangents;
  // Tangents.Init(FProcMeshTangent(-1.0f, -1.0f, -1.0f).RotateAngleAxis(ViewAngle, RotationAxis), Vertices.Num());
  Tangents.Init(FProcMeshTangent(FVector(0.0f, 1.0f, 0.0f).RotateAngleAxis(ViewAngle, RotationAxis), false), Vertices.Num());

  Mesh->CreateMeshSection_LinearColor((int32)View, Vertices, Triangles, Normals, UVs, Colors, Tangents, false);
}

void AProceduralBuildingUtilities::GetWidthDepthFromView(
    const FVector& BuildingSize,
    const EBuildingCameraView View,
    float& OutWidth,
    float& OutDepth,
    float& OutViewAngle)
{
  switch(View)
  {
    case EBuildingCameraView::FRONT:
      OutViewAngle = 0.0f;
      OutWidth = FMath::Abs(BuildingSize.Y);
      OutDepth = FMath::Abs(BuildingSize.X);
      break;

    case EBuildingCameraView::LEFT:
      OutViewAngle = 90.0f;
      OutWidth = FMath::Abs(BuildingSize.X);
      OutDepth = FMath::Abs(BuildingSize.Y);
      break;

    case EBuildingCameraView::BACK:
      OutViewAngle = 180.0f;
      OutWidth = FMath::Abs(BuildingSize.Y);
      OutDepth = FMath::Abs(BuildingSize.X);
      break;

    case EBuildingCameraView::RIGHT:
      OutViewAngle = 270.0f;
      OutWidth = FMath::Abs(BuildingSize.X);
      OutDepth = FMath::Abs(BuildingSize.Y);
      break;

    default:
      OutViewAngle = 0.0f;
      OutWidth = 0.0f;
      OutDepth = 0.0f;
  }
}

void AProceduralBuildingUtilities::CalculateViewGeometryUVs(
    const float BuildingWidth,
    const float BuildingHeight,
    const EBuildingCameraView View,
    TArray<FVector2D>& OutUVs)
{
  // Calculate UVs from 0 to 1
  //  ------------
  // |  uv1  uv3  |
  // |  uv0  uv2  |
  //  ------------

  FVector2D OriginOffset;
  switch(View)
  {
    case EBuildingCameraView::FRONT:
      OriginOffset = FVector2D(0.3f, 0.3f);
      break;

    case EBuildingCameraView::LEFT:
      OriginOffset = FVector2D(0.3f, 0.4f);
      break;

    case EBuildingCameraView::BACK:
      OriginOffset = FVector2D(0.4f, 0.3f);
      break;

    case EBuildingCameraView::RIGHT:
      OriginOffset = FVector2D(0.4f, 0.4f);
      break;

    default:
      OriginOffset = FVector2D(0.0f, 0.0f);
  }

  float WidthRatio = BuildingWidth / BuildingHeight;
  float HeightRatio = BuildingHeight / BuildingWidth;

  FVector2D X0, X1, X2, X3;

  if(WidthRatio < 1.0f)
  {
    // Fit Vertically
    X0 = FVector2D(0.05f - 0.05f * WidthRatio, 0.1f) + OriginOffset;
    X1 = FVector2D(0.05f - 0.05f * WidthRatio, 0.0f) + OriginOffset;
    X2 = FVector2D(0.05f + 0.05f * WidthRatio, 0.1f) + OriginOffset;
    X3 = FVector2D(0.05f + 0.05f * WidthRatio, 0.0f) + OriginOffset;
  }
  else
  {
    // Fit Horizontally
    X0 = FVector2D(0.0f, 0.05f + 0.05f * HeightRatio) + OriginOffset;
    X1 = FVector2D(0.0f, 0.05f - 0.05f * HeightRatio) + OriginOffset;
    X2 = FVector2D(0.1f, 0.05f + 0.05f * HeightRatio) + OriginOffset;
    X3 = FVector2D(0.1f, 0.05f - 0.05f * HeightRatio) + OriginOffset;
  }

  // Fix offset to fit in view quadrant
  OutUVs.Add(X0);
  OutUVs.Add(X1);
  OutUVs.Add(X2);
  OutUVs.Add(X3);
}
