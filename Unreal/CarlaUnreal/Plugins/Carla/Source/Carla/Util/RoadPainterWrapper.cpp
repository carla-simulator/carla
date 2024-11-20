// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "RoadPainterWrapper.h"

#include <util/ue-header-guard-begin.h>
#if WITH_EDITOR
    #include "Misc/FileHelper.h"
#endif
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include <util/ue-header-guard-end.h>

ARoadPainterWrapper::ARoadPainterWrapper(){

#if WITH_EDITORONLY_DATA

  // Initialization of map for translating from the JSON "decal_names" to MaterialInstance

  TArray<AActor*> MeshActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), MeshActors);
  for (int32 i = 0; i < MeshActors.Num(); ++i) {

    AStaticMeshActor *StaticMeshActor = Cast<AStaticMeshActor>(MeshActors[i]);
    if (StaticMeshActor) {

      if (StaticMeshActor->GetName().Contains("Curb", ESearchCase::Type::IgnoreCase) || StaticMeshActor->GetName().Contains("Gutter", ESearchCase::Type::IgnoreCase)) {

        StaticMeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
      }
    }
  }

#endif
}

void ARoadPainterWrapper::BeginPlay()
{
  Super::BeginPlay();

}

void ARoadPainterWrapper::ReadConfigFile(const FString &CurrentMapName, const TMap<FString, FString> &DecalNamesMap)
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
          for (const TPair<FString, FString>& Pair : DecalNamesMap) {
            if (DecalJsonObject->HasField(Pair.Key) == true) {
              DecalPropertiesConfig.DecalMaterials.Add(LoadObject<UMaterialInstanceConstant>(nullptr, *Pair.Value));
              DecalPropertiesConfig.DecalNumToSpawn.Add(DecalJsonObject->GetIntegerField(Pair.Key));
            }
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
