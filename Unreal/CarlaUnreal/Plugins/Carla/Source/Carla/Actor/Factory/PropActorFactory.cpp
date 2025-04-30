// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#include "Carla/Actor/Factory/PropActorFactory.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Game/CarlaEpisode.h"

#include <util/ue-header-guard-begin.h>
#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include <util/ue-header-guard-end.h>

TArray<FActorDefinition> APropActorFactory::GetDefinitions()
{
  LoadPropParametersArrayFromFile("PropParameters.json", PropsParams);
  
  UActorBlueprintFunctionLibrary::MakePropDefinitions(PropsParams, Definitions);
  return Definitions;
}

FActorSpawnResult APropActorFactory::SpawnActor(
    const FTransform &SpawnAtTransform,
    const FActorDescription &ActorDescription)
{
  FActorSpawnResult SpawnResult;
  if(!IsValid(ActorDescription.Class))
  {
    UE_LOG(LogCarla, Error, TEXT("Actor Description Class is null."));
    SpawnResult.Status = EActorSpawnResultStatus::InvalidDescription;
    return SpawnResult;
  }

  AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorDescription.Class, SpawnAtTransform);
  SpawnResult.Actor = SpawnedActor;

  if(SpawnedActor == nullptr)
  {
    SpawnResult.Status = EActorSpawnResultStatus::Collision;
    return SpawnResult;
  }

  if(PostProcessProp(SpawnedActor, ActorDescription))
  {
    SpawnResult.Status = EActorSpawnResultStatus::Success;
    return SpawnResult;
  }

  SpawnResult.Status = EActorSpawnResultStatus::UnknownError;
  return SpawnResult;
}

TSharedPtr<FJsonObject> APropActorFactory::FPropParametersToJsonObject(const FPropParameters& PropParams)
{
  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

  JsonObject->SetStringField(TEXT("Name"), PropParams.Name);
  JsonObject->SetStringField(TEXT("Mesh"), PropParams.Mesh->GetPathName());

  FString PropSizeString;
  switch(PropParams.Size)
  {
    case EPropSize::Tiny:
        PropSizeString = FString("Tiny");
        break;
    case EPropSize::Small:
        PropSizeString = FString("Small");
        break;
    case EPropSize::Medium:
        PropSizeString = FString("Medium");
        break;
    case EPropSize::Big:
        PropSizeString = FString("Big");
        break;
    case EPropSize::Huge:
        PropSizeString = FString("Huge");
        break;
    default:
        PropSizeString = FString("INVALID");
        break;
  }
  JsonObject->SetStringField(TEXT("Size"), PropSizeString);
  return JsonObject;
}

FString APropActorFactory::FPropParametersArrayToJson(const TArray<FPropParameters>& PropParamsArray)
{
  TArray<TSharedPtr<FJsonValue>> JsonArray;

  for (const FPropParameters& PropParams : PropParamsArray)
  {
      // Convert each FPropParameters to a JSON object
      TSharedPtr<FJsonObject> JsonObject = FPropParametersToJsonObject(PropParams);
      JsonArray.Add(MakeShareable(new FJsonValueObject(JsonObject)));
  }

  // Convert the array of JSON objects into a single JSON object
  TSharedRef<FJsonObject> RootObject = MakeShareable(new FJsonObject);
  RootObject->SetArrayField(TEXT("Props"), JsonArray);

  // Serialize the JSON object into an FString
  FString OutputString;
  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
  FJsonSerializer::Serialize(RootObject, Writer);

  return OutputString;
}

void APropActorFactory::SavePropParametersArrayToFile(const TArray<FPropParameters>& PropParametersArray, const FString& FileName)
{
    FString FilePath = FPaths::ProjectContentDir() + TEXT("Carla/Config/") + FileName;
    // Convert the array to an FString in JSON format
  FString JsonContent = FPropParametersArrayToJson(PropParametersArray);

    // Save the JSON to a file
    if (FFileHelper::SaveStringToFile(JsonContent, *FilePath))
    {
        UE_LOG(LogCarla, Log, TEXT("JSON file successfully saved at: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogCarla, Error, TEXT("Failed to save JSON file at: %s"), *FilePath);
    }
}

bool APropActorFactory::JsonToFPropParameters(const TSharedPtr<FJsonObject> JsonObject, FPropParameters& OutPropParams)
{
    if (!JsonObject.IsValid()) return false;

    JsonObject->TryGetStringField(TEXT("Name"), OutPropParams.Name);

    // Convert "Mesh" string back to a FMesh reference
    FString MeshPath;
    JsonObject->TryGetStringField(TEXT("Mesh"), MeshPath);
    OutPropParams.Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *(MeshPath)));

    FString PropSizeString;
    JsonObject->TryGetStringField(TEXT("Size"), PropSizeString);

    if(PropSizeString == FString("Tiny")){
        OutPropParams.Size = EPropSize::Tiny;
    }
    else if(PropSizeString == FString("Small"))
    {
        OutPropParams.Size = EPropSize::Small;
    }
    else if(PropSizeString == FString("Medium"))
    {
        OutPropParams.Size = EPropSize::Medium;
    }
    else if(PropSizeString == FString("Big"))
    {
        OutPropParams.Size = EPropSize::Big;
    }
    else if(PropSizeString == FString("Huge"))
    {
        OutPropParams.Size = EPropSize::Huge;
    }
    else
    {
        OutPropParams.Size = EPropSize::INVALID;
    }

    return true;
}

bool APropActorFactory::JsonToFPropParametersArray(const FString& JsonString, TArray<FPropParameters>& OutPropParamsArray)
{
    TSharedPtr<FJsonObject> RootObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        UE_LOG(LogCarla, Error, TEXT("Failed to parse JSON."));
        return false;
    }

    // Get the "Props" array from the JSON root object
    const TArray<TSharedPtr<FJsonValue>>* PropsArray;
    if (RootObject->TryGetArrayField(TEXT("Props"), PropsArray))
    {
        OutPropParamsArray.Empty();
        for (const TSharedPtr<FJsonValue>& PropValue : *PropsArray)
        {
            TSharedPtr<FJsonObject> PropObject = PropValue->AsObject();
            if (PropObject.IsValid())
            {
                FPropParameters PropParams;
                if (JsonToFPropParameters(PropObject, PropParams))
                {
                    OutPropParamsArray.Add(PropParams);
                }
            }
        }
    }

    return true;
}

void APropActorFactory::LoadPropParametersArrayFromFile(const FString& FileName, TArray<FPropParameters>& OutPropParamsArray)
{
  FString JsonString;
  FString FilePath = FPaths::ProjectContentDir() + TEXT("Carla/Config/") + FileName;
  FString JsonContent;

  // Load the JSON file content into an FString
  if (FFileHelper::LoadFileToString(JsonContent, *FilePath))
  {
      // Parse the JSON and populate the TArray<FPropParameters>
      if (JsonToFPropParametersArray(JsonContent, OutPropParamsArray))
      {
          UE_LOG(LogCarla, Log, TEXT("Prop parameters loaded successfully from %s"), *FilePath);
      }
      else
      {
          UE_LOG(LogCarla, Error, TEXT("Failed to parse Prop parameters from %s"), *FilePath);
      }
  }
  else
  {
      UE_LOG(LogCarla, Error, TEXT("Failed to load file: %s"), *FilePath);
  }
}