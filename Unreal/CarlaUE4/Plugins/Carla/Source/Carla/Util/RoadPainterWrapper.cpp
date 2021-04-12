// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "RoadPainterWrapper.h"

#if WITH_EDITOR
    #include "FileHelper.h"
#endif
#include "JsonObject.h"
#include "JsonSerializer.h"
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

  //Initialization of map for translating from the JSON decal_names as a MaterialInstance
  DecalNamesMap.Add("dirt1", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_01.DI_RoadDirt_01'")).Object);
  DecalNamesMap.Add("dirt2", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_02.DI_RoadDirt_02'")).Object);
  DecalNamesMap.Add("dirt3", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_03.DI_RoadDirt_03'")).Object);
  DecalNamesMap.Add("dirt4", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_04.DI_RoadDirt_04'")).Object);
  DecalNamesMap.Add("dirt5", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_05.DI_RoadDirt_05'")).Object);

  DecalNamesMap.Add("drip1", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDrip_01.DI_RoadDrip_01'")).Object);
  DecalNamesMap.Add("drip2", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDrip_02.DI_RoadDrip_02'")).Object);
  DecalNamesMap.Add("drip3", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDrip_03.DI_RoadDrip_03'")).Object);

  DecalNamesMap.Add("roadline1", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_01.DI_RoadLine_01'")).Object);
  DecalNamesMap.Add("roadline2", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_02.DI_RoadLine_02'")).Object);
  DecalNamesMap.Add("roadline3", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_03.DI_RoadLine_03'")).Object);
  DecalNamesMap.Add("roadline4", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_04.DI_RoadLine_04'")).Object);
  DecalNamesMap.Add("roadline5", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_05.DI_RoadLine_05'")).Object);

  DecalNamesMap.Add("tiremark1", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_TireMark_01.DI_TireMark_01'")).Object);
  DecalNamesMap.Add("tiremark2", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_TireMark_02.DI_TireMark_02'")).Object);
  DecalNamesMap.Add("tiremark3", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_TireMark_03.DI_TireMark_03'")).Object);

  DecalNamesMap.Add("tarsnake1", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake1.DI_tarsnake1'")).Object);
  DecalNamesMap.Add("tarsnake2", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake2.DI_tarsnake2'")).Object);
  DecalNamesMap.Add("tarsnake3", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake3.DI_tarsnake3'")).Object);
  DecalNamesMap.Add("tarsnake4", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake4.DI_tarsnake4'")).Object);
  DecalNamesMap.Add("tarsnake5", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake5.DI_tarsnake5'")).Object);
  DecalNamesMap.Add("tarsnake6", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake6.DI_tarsnake6'")).Object);
  DecalNamesMap.Add("tarsnake7", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake7.DI_tarsnake7'")).Object);
  DecalNamesMap.Add("tarsnake8", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake8.DI_tarsnake8'")).Object);
  DecalNamesMap.Add("tarsnake9", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake9.DI_tarsnake9'")).Object);
  DecalNamesMap.Add("tarsnake10", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake10.DI_tarsnake10'")).Object);
  DecalNamesMap.Add("tarsnake11", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake11.DI_tarsnake11'")).Object);

  DecalNamesMap.Add("cracksbig1", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig1.DI_cracksBig1'")).Object);
  DecalNamesMap.Add("cracksbig2", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig2.DI_cracksBig2'")).Object);
  DecalNamesMap.Add("cracksbig3", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig3.DI_cracksBig3'")).Object);
  DecalNamesMap.Add("cracksbig4", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig4.DI_cracksBig4'")).Object);
  DecalNamesMap.Add("cracksbig5", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig5.DI_cracksBig5'")).Object);
  DecalNamesMap.Add("cracksbig6", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig6.DI_cracksBig6'")).Object);
  DecalNamesMap.Add("cracksbig7", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig7.DI_cracksBig7'")).Object);
  DecalNamesMap.Add("cracksbig8", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig8.DI_cracksBig8'")).Object);

  DecalNamesMap.Add("crack1", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack01.DI_RoadCrack01'")).Object);
  DecalNamesMap.Add("crack2", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack02.DI_RoadCrack02'")).Object);
  DecalNamesMap.Add("crack3", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack05.DI_RoadCrack05'")).Object);
  DecalNamesMap.Add("crack4", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack09.DI_RoadCrack09'")).Object);
  DecalNamesMap.Add("crack5", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack11.DI_RoadCrack11'")).Object);
  DecalNamesMap.Add("crack6", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack13.DI_RoadCrack13'")).Object);
  DecalNamesMap.Add("crack7", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack15.DI_RoadCrack15'")).Object);
  DecalNamesMap.Add("crack8", ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack16.DI_RoadCrack16'")).Object);


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
  ZSizeEvent();

  if(RoadTexture == nullptr) GenerateTexture();

  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), FoundActors);
  AStaticMeshActor *RoadMeshActor = nullptr;

  for (int32 i = 0; i < FoundActors.Num(); ++i)
  {
  	RoadMeshActor = Cast<AStaticMeshActor>(FoundActors[i]);
  	if (RoadMeshActor)
  	{
  		if (RoadMeshActor->GetName().Contains("Roads_Road", ESearchCase::Type::CaseSensitive) == true)
  		{
        //Create the dynamic material instance for the road (which will hold the map size and road texture)
        UMaterialInstanceDynamic* MI = UMaterialInstanceDynamic::Create(RoadNodeMasterMaterial, NULL);
        MI->CopyParameterOverrides((UMaterialInstance*)RoadNodePresetMaterial);
        MI->SetScalarParameterValue(FName("Map units (CM)"), MapSize);
        MI->SetTextureParameterValue(FName("Texture Mask"), RoadTexture);
        RoadMeshActor->GetStaticMeshComponent()->SetMaterial(0, MI);
  		}
  	}
  }
}

void ARoadPainterWrapper::GenerateTexture()
{
  //Create a string containing the texture's path
  FString PackageName = TEXT("/Game/map_package/RoadPainterTextures/");
  FString BaseTextureName = FString("RoadMapTexture");
  PackageName += BaseTextureName;

  //Create the package that will store our texture
  UPackage *Package = CreatePackage(*PackageName);

  //Create a unique name for our asset. For example, if a texture named RoadMapTexture already exists the editor
  //will name the new texture as "RoadMapTexture_1"
  const FName TextureName = MakeUniqueObjectName(Package, UTextureRenderTarget2D::StaticClass(), FName(*BaseTextureName));
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
    FString PackageFilename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
    UPackage::SavePackage(Package, RoadTexture, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFilename, GError, nullptr, true, true, SAVE_NoError);
  }
}

void ARoadPainterWrapper::ReadConfigFile(const FString &CurrentMapName)
{
  // Get road painter configuration file
  FString JsonConfigFile;

  TArray<FString> FileList;
  IFileManager::Get().FindFilesRecursive(FileList, *(FPaths::ProjectContentDir()),
 *(FString("roadpainter_decals.json")), true, false, false);

  if(FFileHelper::LoadFileToString(JsonConfigFile, *(IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FileList[0]))))
  {
    TSharedPtr<FJsonObject> JsonParsed;
    TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonConfigFile);
    if(FJsonSerializer::Deserialize(JsonReader, JsonParsed))
    {
      //Get decals object array
      auto DecalJsonArray = JsonParsed->GetArrayField(TEXT("decals"));
      for(auto &DecalJsonValue : DecalJsonArray)
      {
        const auto DecalJsonObject = DecalJsonValue->AsObject();

        //Inside the decals object array, get the map name where the decals should be applied
        //If it coincides with the map this object is in, then it's the correct configuration
        FString JsonMapName = DecalJsonObject->GetStringField(TEXT("map_name"));
        if(JsonMapName.Equals(CurrentMapName) == true)
        {
          //Get the decal types array
          auto DecalTypesJsonArray = DecalJsonObject->GetArrayField(TEXT("decal_types"));

          UMaterialInstance **MaterialExists = nullptr;
          FString DecalName = "";
          int32 NumDecals = 0;
          for (auto &DecalTypeValue : DecalTypesJsonArray) {

            const auto DecalTypeObject = DecalTypeValue->AsObject();
            DecalName = DecalTypeObject->GetStringField(TEXT("decal_name"));
            NumDecals = DecalTypeObject->GetIntegerField(TEXT("num_decals"));

            MaterialExists = DecalNamesMap.Find(DecalName);

            if (MaterialExists != nullptr)
            {
              DecalPropertiesConfig.DecalMaterials.Add(*MaterialExists);
              DecalPropertiesConfig.DecalNumToSpawn.Add(NumDecals);
            }
            else
            {
              UE_LOG(LogTemp, Warning, TEXT("Could not find the designated decal (via it's decal name %s)"), *DecalName);
            }
          }

          //Prepare the decal properties struct variable inside the class
          //so the blueprint can read from it, later on
          DecalPropertiesConfig.DecalScale = ReadVectorFromJsonObject(DecalJsonObject->GetObjectField(TEXT("decal_scale")));
          DecalPropertiesConfig.FixedDecalOffset = ReadVectorFromJsonObject(DecalJsonObject->GetObjectField(TEXT("fixed_decal_offset")));
          DecalPropertiesConfig.DecalMinScale = (float)DecalJsonObject->GetNumberField(TEXT("decal_min_scale"));
          DecalPropertiesConfig.DecalMaxScale = (float)DecalJsonObject->GetNumberField(TEXT("decal_max_scale"));
          DecalPropertiesConfig.DecalRandomYaw = (float)DecalJsonObject->GetNumberField(TEXT("decal_random_yaw"));
          DecalPropertiesConfig.RandomOffset = (float)DecalJsonObject->GetNumberField(TEXT("random_offset"));
        }
      }
      //UE_LOG(LogTemp, Warning, TEXT("Displaying JSON variables, %f %f"), DecalPropertiesConfig.DecalRandomYaw, DecalPropertiesConfig.DecalScale.X);
    }
  }
}

FVector ARoadPainterWrapper::ReadVectorFromJsonObject(TSharedPtr<FJsonObject> JsonObject)
{
  FVector ObjectVector;
  ObjectVector.X = (float)JsonObject->GetNumberField(TEXT("x_axis"));
  ObjectVector.Y = (float)JsonObject->GetNumberField(TEXT("y_axis"));
  ObjectVector.Z = (float)JsonObject->GetNumberField(TEXT("z_axis"));
  return ObjectVector;
}
