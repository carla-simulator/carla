// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#include "Carla/Actor/Factory/BlueprintActorFactory.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Game/CarlaEpisode.h"

#include <util/ue-header-guard-begin.h>
#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include <util/ue-header-guard-end.h>

TArray<FActorDefinition> ABlueprintActorFactory::GetDefinitions()
{
  LoadBlueprintParametersArrayFromFile("BlueprintParameters.json", BlueprintsParams);
  
  UActorBlueprintFunctionLibrary::MakeBlueprintDefinitions(BlueprintsParams, Definitions);
  return Definitions;
}

FActorSpawnResult ABlueprintActorFactory::SpawnActor(
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

  if(PostProcessBlueprint(SpawnedActor, ActorDescription))
  {
    SpawnResult.Status = EActorSpawnResultStatus::Success;
    return SpawnResult;
  }

  SpawnResult.Status = EActorSpawnResultStatus::UnknownError;
  return SpawnResult;
}

TSharedPtr<FJsonObject> ABlueprintActorFactory::FBlueprintParametersToJsonObject(const FBlueprintParameters& BlueprintParams)
{
  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
  JsonObject->SetStringField(TEXT("Name"), BlueprintParams.Name);
  JsonObject->SetStringField(TEXT("Path"), BlueprintParams.Path);

  return JsonObject;
}

FString ABlueprintActorFactory::FBlueprintParametersArrayToJson(const TArray<FBlueprintParameters>& BlueprintParamsArray)
{
  TArray<TSharedPtr<FJsonValue>> JsonArray;

  for (const FBlueprintParameters& BlueprintParams : BlueprintParamsArray)
  {
      // Convert each FBlueprintParameters to a JSON object
      TSharedPtr<FJsonObject> JsonObject = FBlueprintParametersToJsonObject(BlueprintParams);
      JsonArray.Add(MakeShareable(new FJsonValueObject(JsonObject)));
  }

  // Convert the array of JSON objects into a single JSON object
  TSharedRef<FJsonObject> RootObject = MakeShareable(new FJsonObject);
  RootObject->SetArrayField(TEXT("Blueprints"), JsonArray);

  // Serialize the JSON object into an FString
  FString OutputString;
  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
  FJsonSerializer::Serialize(RootObject, Writer);

  return OutputString;
}

void ABlueprintActorFactory::SaveBlueprintParametersArrayToFile(const TArray<FBlueprintParameters>& BlueprintParametersArray, const FString& FileName)
{
    FString FilePath = FPaths::ProjectContentDir() + TEXT("Carla/Config/") + FileName;
    // Convert the array to an FString in JSON format
  FString JsonContent = FBlueprintParametersArrayToJson(BlueprintParametersArray);

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

bool ABlueprintActorFactory::JsonToFBlueprintParameters(const TSharedPtr<FJsonObject> JsonObject, FBlueprintParameters& OutBlueprintParams)
{
    if (!JsonObject.IsValid()) return false;

    JsonObject->TryGetStringField(TEXT("Name"), OutBlueprintParams.Name);
    JsonObject->TryGetStringField(TEXT("Path"), OutBlueprintParams.Path);

    return true;
}

bool ABlueprintActorFactory::JsonToFBlueprintParametersArray(const FString& JsonString, TArray<FBlueprintParameters>& OutBlueprintParamsArray)
{
    TSharedPtr<FJsonObject> RootObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        UE_LOG(LogCarla, Error, TEXT("Failed to parse JSON."));
        return false;
    }

    // Get the "Blueprints" array from the JSON root object
    const TArray<TSharedPtr<FJsonValue>>* BlueprintsArray;
    if (RootObject->TryGetArrayField(TEXT("Blueprints"), BlueprintsArray))
    {
        OutBlueprintParamsArray.Empty();
        for (const TSharedPtr<FJsonValue>& BlueprintValue : *BlueprintsArray)
        {
            TSharedPtr<FJsonObject> BlueprintObject = BlueprintValue->AsObject();
            if (BlueprintObject.IsValid())
            {
                FBlueprintParameters BlueprintParams;
                if (JsonToFBlueprintParameters(BlueprintObject, BlueprintParams))
                {
                    OutBlueprintParamsArray.Add(BlueprintParams);
                }
            }
        }
    }

    return true;
}

void ABlueprintActorFactory::LoadBlueprintParametersArrayFromFile(const FString& FileName, TArray<FBlueprintParameters>& OutBlueprintParamsArray)
{
  FString JsonString;
  FString FilePath = FPaths::ProjectContentDir() + TEXT("Carla/Config/") + FileName;
  FString JsonContent;

  // Load the JSON file content into an FString
  if (FFileHelper::LoadFileToString(JsonContent, *FilePath))
  {
      // Parse the JSON and populate the TArray<FBlueprintParameters>
      if (JsonToFBlueprintParametersArray(JsonContent, OutBlueprintParamsArray))
      {
          UE_LOG(LogCarla, Log, TEXT("Blueprint parameters loaded successfully from %s"), *FilePath);
      }
      else
      {
          UE_LOG(LogCarla, Error, TEXT("Failed to parse Blueprint parameters from %s"), *FilePath);
      }
  }
  else
  {
      UE_LOG(LogCarla, Error, TEXT("Failed to load file: %s"), *FilePath);
  }
}