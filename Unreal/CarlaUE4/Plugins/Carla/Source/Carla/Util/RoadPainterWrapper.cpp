// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "RoadPainterWrapper.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ARoadPainterWrapper::ARoadPainterWrapper(){

	MapSize = 0.0f;
  bIsRenderedToTexture = false;

#if WITH_EDITORONLY_DATA

  static ConstructorHelpers::FObjectFinder<UMaterial> RoadMasterMaterial(TEXT(
      "Material'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/M_RoadMaster.M_RoadMaster'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> RoadPresetMaterial(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/M_Road_03.M_Road_03'"));

  RoadNodeMasterMaterial = RoadMasterMaterial.Object;
  RoadNodePresetMaterial = RoadPresetMaterial.Object;

#endif
}

void ARoadPainterWrapper::BeginPlay()
{
  Super::BeginPlay();
  
  if(bIsRenderedToTexture == false)
  {
    PaintAllRoadsEvent();
    bIsRenderedToTexture = true;
  }
}

void ARoadPainterWrapper::GenerateDynamicAssets()
{
  DynamicRoadMaterial = UMaterialInstanceDynamic::Create(RoadNodeMasterMaterial, this, FName(TEXT("Road Painter Material Dynamic")));
  DynamicRoadMaterial->CopyParameterOverrides((UMaterialInstance*)RoadNodePresetMaterial);

  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), FoundActors);
  AStaticMeshActor *RoadMeshActor = nullptr;

  for (int32 i = 0; i < FoundActors.Num() && RoadMeshActor == nullptr; ++i)
  {
  	RoadMeshActor = Cast<AStaticMeshActor>(FoundActors[i]);
  	if (RoadMeshActor)
  	{
  		if (RoadMeshActor->GetName().Equals("Roads_RoadNode") == true)
  		{
  			UE_LOG(LogTemp, Log, TEXT("Found the road actor."), *FoundActors[i]->GetName());
  			RoadMeshActor->GetStaticMeshComponent()->SetMaterial(0, DynamicRoadMaterial);
  		}else
  		{
  		  RoadMeshActor = nullptr;
  		}
  	}
  }
}

void ARoadPainterWrapper::GenerateTexture()
{
  //Create a string containing the texture's path
  FString PackageName = TEXT("/Game/Carla/Static/RoadPainterTextures");
  FString BaseTextureName = FString("RoadMapTexture");
  PackageName += BaseTextureName;

  //Create the package that will store our texture
  UPackage *Package = CreatePackage(*PackageName);

  //Create a unique name for our asset. For example, if a texture named RoadMapTexture already exists the editor
  //will name the new texture as "RoadMapTexture_1"
  FName TextureName = MakeUniqueObjectName(Package, UTextureRenderTarget2D::StaticClass(), FName(*BaseTextureName));
  Package->FullyLoad();

  RoadTexture = NewObject<UTextureRenderTarget2D>(Package, TextureName, RF_Public | RF_Standalone | RF_MarkAsRootSet);
  UTexture2D *InternalTexture2D = NewObject<UTexture2D>(UTexture2D::StaticClass());
  if (RoadTexture)
  {
    //Prevent the object and all its descedants from being deleted during garbage collecion
    RoadTexture->AddToRoot();
    RoadTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
    RoadTexture->SizeX = 2048;
    RoadTexture->SizeY = 2048;
    
    //Initialize the platform data to store necessary information regarding our texture asset
    InternalTexture2D->AddToRoot();
    InternalTexture2D->PlatformData = new FTexturePlatformData();
    InternalTexture2D->PlatformData->SizeX = 2048;
    InternalTexture2D->PlatformData->SizeY = 2048;
    InternalTexture2D->AddressX = TextureAddress::TA_Wrap;
    InternalTexture2D->AddressY = TextureAddress::TA_Wrap;
    
    //Set the texture render target internal texture
    RoadTexture->UpdateTexture2D(InternalTexture2D, ETextureSourceFormat::TSF_RGBA8);
    Package->MarkPackageDirty();
    
    //Notify the editor we created a new asset
    FAssetRegistryModule::AssetCreated(RoadTexture);
    
    //Auto-save the new asset
    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
    UPackage::SavePackage(Package, RoadTexture, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
  }
}

void ARoadPainterWrapper::ModifyRoadMaterialParameters()
{
  GenerateTexture();
  DynamicRoadMaterial->SetScalarParameterValue(FName("Map units (CM)"), MapSize);
  DynamicRoadMaterial->SetTextureParameterValue(FName("Texture Mask"), RoadTexture);
}
