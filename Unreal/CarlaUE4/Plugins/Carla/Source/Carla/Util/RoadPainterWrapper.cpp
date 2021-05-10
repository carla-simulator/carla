// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
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
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ARoadPainterWrapper::ARoadPainterWrapper(){

  MapSize = 0.0f;
  RoadTextureSizeX = 1024;
  RoadTextureSizeY = 1024;
  bIsRenderedToTexture = false;

#if WITH_EDITORONLY_DATA

  // Initialization of map for translating from the JSON "decal_names" to MaterialInstance

  // Dirt
  DecalNamesMap.Add("dirt1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_01.DI_RoadDirt_01'");
  DecalNamesMap.Add("dirt2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_02.DI_RoadDirt_02'");
  DecalNamesMap.Add("dirt3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_03.DI_RoadDirt_03'");
  DecalNamesMap.Add("dirt4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_04.DI_RoadDirt_04'");
  DecalNamesMap.Add("dirt5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_05.DI_RoadDirt_05'");

  // Drip
  DecalNamesMap.Add("drip1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDrip_01.DI_RoadDrip_01'");
  DecalNamesMap.Add("drip2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDrip_02.DI_RoadDrip_02'");
  DecalNamesMap.Add("drip3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDrip_03.DI_RoadDrip_03'");

  // Road lines
  DecalNamesMap.Add("roadline1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_01.DI_RoadLine_01'");
  DecalNamesMap.Add("roadline2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_02.DI_RoadLine_02'");
  DecalNamesMap.Add("roadline3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_03.DI_RoadLine_03'");
  DecalNamesMap.Add("roadline4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_04.DI_RoadLine_04'");
  DecalNamesMap.Add("roadline5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_05.DI_RoadLine_05'");

  // Tiremarks
  DecalNamesMap.Add("tiremark1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_TireMark_01.DI_TireMark_01'");
  DecalNamesMap.Add("tiremark2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_TireMark_02.DI_TireMark_02'");
  DecalNamesMap.Add("tiremark3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_TireMark_03.DI_TireMark_03'");

  // Tarsnakes
  DecalNamesMap.Add("tarsnake1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake1.DI_tarsnake1'");
  DecalNamesMap.Add("tarsnake2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake2.DI_tarsnake2'");
  DecalNamesMap.Add("tarsnake3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake3.DI_tarsnake3'");
  DecalNamesMap.Add("tarsnake4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake4.DI_tarsnake4'");
  DecalNamesMap.Add("tarsnake5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake5.DI_tarsnake5'");
  DecalNamesMap.Add("tarsnake6", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake6.DI_tarsnake6'");
  DecalNamesMap.Add("tarsnake7", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake7.DI_tarsnake7'");
  DecalNamesMap.Add("tarsnake8", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake8.DI_tarsnake8'");
  DecalNamesMap.Add("tarsnake9", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake9.DI_tarsnake9'");
  DecalNamesMap.Add("tarsnake10", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake10.DI_tarsnake10'");
  DecalNamesMap.Add("tarsnake11", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake11.DI_tarsnake11'");

  // Cracks big
  DecalNamesMap.Add("cracksbig1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig1.DI_cracksBig1'");
  DecalNamesMap.Add("cracksbig2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig2.DI_cracksBig2'");
  DecalNamesMap.Add("cracksbig3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig3.DI_cracksBig3'");
  DecalNamesMap.Add("cracksbig4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig4.DI_cracksBig4'");
  DecalNamesMap.Add("cracksbig5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig5.DI_cracksBig5'");
  DecalNamesMap.Add("cracksbig6", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig6.DI_cracksBig6'");
  DecalNamesMap.Add("cracksbig7", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig7.DI_cracksBig7'");
  DecalNamesMap.Add("cracksbig8", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig8.DI_cracksBig8'");

  // Cracks
  DecalNamesMap.Add("crack1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack01.DI_RoadCrack01'");
  DecalNamesMap.Add("crack2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack02.DI_RoadCrack02'");
  DecalNamesMap.Add("crack3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack05.DI_RoadCrack05'");
  DecalNamesMap.Add("crack4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack09.DI_RoadCrack09'");
  DecalNamesMap.Add("crack5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack11.DI_RoadCrack11'");
  DecalNamesMap.Add("crack6", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack13.DI_RoadCrack13'");
  DecalNamesMap.Add("crack7", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack15.DI_RoadCrack15'");
  DecalNamesMap.Add("crack8", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack16.DI_RoadCrack16'");

  // Manholes
  DecalNamesMap.Add("manhole1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Manhole/DI_Manhole01.DI_Manhole01'");
  DecalNamesMap.Add("manhole2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Manhole/DI_Manhole02.DI_Manhole02'");
  DecalNamesMap.Add("manhole3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Manhole/DI_Manhole03.DI_Manhole03'");

  // Mud
  DecalNamesMap.Add("mud1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/MudDecal/DI_Mud_1.DI_Mud_1'");
  DecalNamesMap.Add("mud2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/MudDecal/DI_Mud_2.DI_Mud_2'");
  DecalNamesMap.Add("mud3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/MudDecal/DI_Mud_3.DI_Mud_3'");
  DecalNamesMap.Add("mud4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/MudDecal/DI_Mud_4.DI_Mud_4'");
  DecalNamesMap.Add("mud5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/MudDecal/DI_Mud_5.DI_Mud_5'");

  // Oil splats
  DecalNamesMap.Add("oilsplat1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/OilSplats/DI_OilSplat1.DI_OilSplat1'");
  DecalNamesMap.Add("oilsplat2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/OilSplats/DI_OilSplat2.DI_OilSplat2'");
  DecalNamesMap.Add("oilsplat3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/OilSplats/DI_OilSplat3.DI_OilSplat3'");
  DecalNamesMap.Add("oilsplat4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/OilSplats/DI_OilSplat4.DI_OilSplat4'");
  DecalNamesMap.Add("oilsplat5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/OilSplats/DI_OilSplat5.DI_OilSplat5'");

  // Get road painter info file
  FString JsonInfoFile;
  // Misc
  DecalNamesMap.Add("gum", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/OilSplats/DI_Gum.DI_Gum'");
  DecalNamesMap.Add("grate", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Manhole/DI_Grate_01_v2.DI_Grate_01_v2'");

  RoadMasterMaterialName = "Material'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LargeMaps/M_RoadMasterTiled.M_RoadMasterTiled'";
  RoadInstanceMaterialName = "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LargeMaps/M_Road_03_Tiled.M_Road_03_Tiled'";

  RoadTextureBrushes.Add("Texture2D'/Game/Carla/Static/GenericMaterials/Tiles/HDTiles/M_SquareTile_v4_d.M_SquareTile_v4_d'");
  //RoadTextureBrushes.Add("Texture2D'/Game/Carla/Static/Decals/Road/OilSplats/Textures/GroundSplatter/T_GroundSplatter_d.T_GroundSplatter_d'");
  //RoadTextureBrushes.Add("Texture2D'/Game/Carla/Static/GenericMaterials/RoadStencil/Alphas/splat1.splat1'");
  //RoadTextureBrushes.Add("Texture2D'/Game/Carla/Static/GenericMaterials/RoadStencil/Alphas/splat2.splat2'");
  //RoadTextureBrushes.Add("Texture2D'/Game/Carla/Static/GenericMaterials/RoadStencil/Alphas/splat3.splat3'");

#endif
}

void ARoadPainterWrapper::BeginPlay()
{
  Super::BeginPlay();

}

void ARoadPainterWrapper::GenerateDynamicAssets()
{
  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), FoundActors);
  AStaticMeshActor *RoadMeshActor = nullptr;

  for (int32 i = 0; i < FoundActors.Num(); ++i)
  {
    RoadMeshActor = Cast<AStaticMeshActor>(FoundActors[i]);
    if (RoadMeshActor)
    {
      ZSizeEvent();

      FString TextureString;
      //Get the string we need
      bool IsRoadPainterReady = JsonParsed->GetBoolField(TEXT("prepared_roadpainter"));
      if (IsRoadPainterReady == false) {

        //Generate the texture we need and save the name.
        //This will be written into a .json file so we can look it up later
        TextureString = GenerateTexture();
      }

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
            if (RoadMeshActor->GetStaticMeshComponent()->GetMaterial(0)->GetName().Contains("MaterialInstance", ESearchCase::Type::CaseSensitive) == false) {
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

      //We write our variables
      TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());
      //Are the textures ready?
      RootObject->SetBoolField("prepared_roadpainter", true);
      //Are the roads rendered to texture already?
      RootObject->SetBoolField("painted_roads", false);
      if (IsRoadPainterReady == false) {

        //We set the name of the newly generated texture into the json file
        RootObject->SetStringField("texture_name", TextureString);
      }
      else {

        TextureString = JsonParsed->GetStringField("texture_name");
        RootObject->SetStringField("texture_name", TextureString);
      }

      FString JsonString;
      TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&JsonString);
      FJsonSerializer::Serialize(RootObject.ToSharedRef(), JsonWriter);
      //Save JSON file
      FFileHelper::SaveStringToFile(JsonString, *AbsolutePathToFile);
    }
  }
}

const FString ARoadPainterWrapper::GenerateTexture()
{
  // Get the path, where the map is, to generate the texture
  FString TexturePackageName = GetWorld()->GetPathName().LeftChop((GetWorld()->GetPathName().GetCharArray().Num() - 1) - (GetWorld()->GetPathName().Find("/", ESearchCase::IgnoreCase, ESearchDir::Type::FromEnd) + 1));
  FString BaseTextureName = FString("RoadMapTexture");
  TexturePackageName += BaseTextureName;

  // Create the package that will store our texture
  TexturePackage = CreatePackage(*TexturePackageName);

  // Create a unique name for our asset. For example, if a texture named RoadMapTexture already exists the editor
  // will name the new texture as "RoadMapTexture_1"
  const FName TextureName = MakeUniqueObjectName(TexturePackage, UTexture2D::StaticClass(), FName(*BaseTextureName));
  FString TextureString = TextureName.ToString();
  TexturePackage->FullyLoad();

  RoadTexture = NewObject<UTexture2D>(TexturePackage, TextureName, RF_Standalone | RF_Public | RF_MarkAsRootSet);
  //UE_LOG(LogTemp, Warning, TEXT("The value of the material is : %f %f %f %f"), MaterialMaskEmissiveValue.R, MaterialMaskEmissiveValue.G, MaterialMaskEmissiveValue.B, MaterialMaskEmissiveValue.A);

  if (RoadTexture)
  {
    // Prevent the object and all its descendants from being deleted during garbage collection
    RoadTexture->AddToRoot();
    RoadTexture->PlatformData = new FTexturePlatformData();
    RoadTexture->PlatformData->SizeX = RoadTextureSizeX;
    RoadTexture->PlatformData->SizeY = RoadTextureSizeY;
    RoadTexture->PlatformData->SetNumSlices(1);
    RoadTexture->PlatformData->PixelFormat = EPixelFormat::PF_B8G8R8A8;

    RoadTexture->UpdateResource();

    // Notify the editor we created a new asset
    FAssetRegistryModule::AssetCreated(RoadTexture);

    return TextureString;
  }

  return TextureString;
}

void ARoadPainterWrapper::StoreRoadWaypointForRender(FVector2D WaypointLocation, float StencilSize) {

  FVector2D TexLocation;
  float TempX, TempY = 0.0f;
  TempX = RoadTextureSizeX / MapSize;
  TempX = WaypointLocation.X * TempX;
  TempX = TempX + (RoadTextureSizeX / 2.0f);

  TempY = RoadTextureSizeY / MapSize;
  TempY = WaypointLocation.Y * TempY;
  TempY = TempY + (RoadTextureSizeY / 2.0f);

  TexLocation.X = TempX;
  TexLocation.Y = TempY;

  RoadWaypointsForTex.Add(TexLocation);
}

void ARoadPainterWrapper::RenderWaypointsToTexture() {

  TArray<uint8> Pixels;
  Pixels.Init(0, RoadTextureSizeX * RoadTextureSizeY * 4);

  for (int32 i = 0; i < RoadWaypointsForTex.Num(); i += 4) {

    Pixels[i] = 0;
    Pixels[i + 1] = 0;
    Pixels[i + 2] = 0;
    Pixels[i + 3] = 255;
  }

  int Row, Column = 0;
  for (int32 i = 0; i < RoadWaypointsForTex.Num(); ++i) {

    Row = FMath::RoundToInt(RoadWaypointsForTex[i].Y) * RoadTextureSizeX;
    Column = FMath::RoundToInt(RoadWaypointsForTex[i].X);
    int PixelLocation = FMath::Abs((Row + Column) * 4);
    Pixels[PixelLocation] = 127;
    Pixels[PixelLocation + 1] = 127;
    Pixels[PixelLocation + 2] = 127;
    Pixels[PixelLocation + 3] = 255;
  }
  
  FTexture2DMipMap *Mip = new FTexture2DMipMap();
  RoadTexture->PlatformData->Mips.Add(Mip);
  Mip->SizeX = RoadTextureSizeX;
  Mip->SizeY = RoadTextureSizeY;
  
  Mip->BulkData.Lock(LOCK_READ_WRITE);
  uint8 *TextureData = (uint8*)Mip->BulkData.Realloc(RoadTextureSizeX * RoadTextureSizeY * 4);
  FMemory::Memcpy(TextureData, Pixels.GetData(), RoadTextureSizeX * RoadTextureSizeY * 4);
  Mip->BulkData.Unlock();

#if WITH_EDITORONLY_DATA
  RoadTexture->Source.Init(RoadTextureSizeX, RoadTextureSizeY, 1, 1, ETextureSourceFormat::TSF_BGRA8, Pixels.GetData());
#endif

  RoadTexture->UpdateResource();
  TexturePackage->MarkPackageDirty();

  // Notify the editor we created a new asset
  FAssetRegistryModule::AssetCreated(RoadTexture);
  
  // Save the new asset
#if WITH_EDITOR
  UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
#endif

  RoadWaypointsForTex.Empty();
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
      // Get decals object array
      auto DecalJsonArray = JsonParsed->GetArrayField(TEXT("decals"));
      for(auto &DecalJsonValue : DecalJsonArray)
      {
        const auto DecalJsonObject = DecalJsonValue->AsObject();

        // Inside the decals object array, get the map name where the decals should be applied
        // If it coincides with the map this object is in, then it's the correct configuration
        FString JsonMapName = DecalJsonObject->GetStringField(TEXT("map_name"));
        if(JsonMapName.Equals(CurrentMapName) == true)
        {
          // With the decal name array we created earlier, we traverse it 
          // and look up it's name in the .json file
          for (int32 i = 0; i < DecalNamesArray.Num(); ++i) {
            DecalPropertiesConfig.DecalMaterials.Add(*DecalNamesMap.Find(DecalNamesArray[i]));
            DecalPropertiesConfig.DecalNumToSpawn.Add(DecalJsonObject->GetIntegerField(DecalNamesArray[i]));
          }

          // Prepare the decal properties struct variable inside the class
          // so the blueprint can read from it, later on
          DecalPropertiesConfig.DecalScale = ReadVectorFromJsonObject(DecalJsonObject->GetObjectField(TEXT("decal_scale")));
          DecalPropertiesConfig.FixedDecalOffset = ReadVectorFromJsonObject(DecalJsonObject->GetObjectField(TEXT("fixed_decal_offset")));
          DecalPropertiesConfig.DecalMinScale = (float)DecalJsonObject->GetNumberField(TEXT("decal_min_scale"));
          DecalPropertiesConfig.DecalMaxScale = (float)DecalJsonObject->GetNumberField(TEXT("decal_max_scale"));
          DecalPropertiesConfig.DecalRandomYaw = (float)DecalJsonObject->GetNumberField(TEXT("decal_random_yaw"));
          DecalPropertiesConfig.RandomOffset = (float)DecalJsonObject->GetNumberField(TEXT("random_offset"));
        }
      }
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
