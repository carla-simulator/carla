// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#include "Carla/Actor/Factory/WalkerActorFactory.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Walker/WalkerBase.h"

#include <util/ue-header-guard-begin.h>
#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include <util/ue-header-guard-end.h>

TArray<FActorDefinition> AWalkerActorFactory::GetDefinitions()
{
  LoadWalkerParametersArrayFromFile("WalkerParameters.json", WalkersParams);
  
  UActorBlueprintFunctionLibrary::MakePedestrianDefinitions(WalkersParams, Definitions);
  return Definitions;
}

FActorSpawnResult AWalkerActorFactory::SpawnActor(
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

  if(PostProcessWalker(SpawnedActor, ActorDescription))
  {
    SpawnResult.Status = EActorSpawnResultStatus::Success;
    return SpawnResult;
  }

  SpawnResult.Status = EActorSpawnResultStatus::UnknownError;
  return SpawnResult;
}

TSharedPtr<FJsonObject> AWalkerActorFactory::FWalkerParametersToJsonObject(const FPedestrianParameters& WalkerParams)
{
  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
  JsonObject->SetStringField(TEXT("Id"), WalkerParams.Id);
  JsonObject->SetStringField(TEXT("Class"), WalkerParams.Class ? WalkerParams.Class->GetPathName() : "");

  FString GenderString;
  switch(WalkerParams.Gender)
  {
    case EPedestrianGender::Other:
        GenderString = FString("Other");
        break;
    case EPedestrianGender::Female:
        GenderString = FString("Female");
        break;
    case EPedestrianGender::Male:
        GenderString = FString("Male");
        break;
    default:
        GenderString = FString("INVALID");
        break;
  }
  JsonObject->SetStringField(TEXT("Gender"), GenderString);

  FString AgeString;
  switch(WalkerParams.Age)
  {
    case EPedestrianAge::Child:
        AgeString = FString("Child");
        break;
    case EPedestrianAge::Teenager:
        AgeString = FString("Teenager");
        break;
    case EPedestrianAge::Adult:
        AgeString = FString("Adult");
        break;
    case EPedestrianAge::Elderly:
        AgeString = FString("Elderly");
        break;
    default:
        AgeString = FString("INVALID");
        break;
  }
  JsonObject->SetStringField(TEXT("Age"), AgeString);

  //JsonObject->SetNumberField(TEXT("Speed"), InSpeed);
  TArray<TSharedPtr<FJsonValue>> SpeedArray;
  for (const float& Speed : WalkerParams.Speed)
  {
      TSharedPtr<FJsonObject> SpeedObject = MakeShareable(new FJsonObject);
      SpeedObject->SetNumberField(TEXT("Speed"), Speed);
      SpeedArray.Add(MakeShareable(new FJsonValueObject(SpeedObject)));
  }
  JsonObject->SetArrayField(TEXT("Speeds"), SpeedArray);

  JsonObject->SetNumberField(TEXT("Generation"), WalkerParams.Generation);

  return JsonObject;
}

FString AWalkerActorFactory::FWalkerParametersArrayToJson(const TArray<FPedestrianParameters>& WalkerParamsArray)
{
  TArray<TSharedPtr<FJsonValue>> JsonArray;

  for (const FPedestrianParameters& WalkerParams : WalkerParamsArray)
  {
      // Convert each FPedestrianParameters to a JSON object
      TSharedPtr<FJsonObject> JsonObject = FWalkerParametersToJsonObject(WalkerParams);
      JsonArray.Add(MakeShareable(new FJsonValueObject(JsonObject)));
  }

  // Convert the array of JSON objects into a single JSON object
  TSharedRef<FJsonObject> RootObject = MakeShareable(new FJsonObject);
  RootObject->SetArrayField(TEXT("Walkers"), JsonArray);

  // Serialize the JSON object into an FString
  FString OutputString;
  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
  FJsonSerializer::Serialize(RootObject, Writer);

  return OutputString;
}

void AWalkerActorFactory::SaveWalkerParametersArrayToFile(const TArray<FPedestrianParameters>& WalkerParametersArray, const FString& FileName)
{
    FString FilePath = FPaths::ProjectContentDir() + TEXT("Carla/Config/") + FileName;
    // Convert the array to an FString in JSON format
    FString JsonContent = FWalkerParametersArrayToJson(WalkerParametersArray);

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

bool AWalkerActorFactory::JsonToFWalkerParameters(const TSharedPtr<FJsonObject> JsonObject, FPedestrianParameters& OutWalkerParams)
{
    if (!JsonObject.IsValid()) return false;

    JsonObject->TryGetStringField(TEXT("Id"), OutWalkerParams.Id);

    FString ClassPath;
    JsonObject->TryGetStringField(TEXT("Class"), ClassPath); // Custom conversion required if needed
    OutWalkerParams.Class = StaticLoadClass(AWalkerBase::StaticClass(), nullptr, *ClassPath);

    FString GenderString;
    JsonObject->TryGetStringField(TEXT("Gender"), GenderString);
    
    if(GenderString == FString("Other")){
        OutWalkerParams.Gender = EPedestrianGender::Other;
    }
    else if(GenderString == FString("Female"))
    {
        OutWalkerParams.Gender = EPedestrianGender::Female;
    }
    else if(GenderString == FString("Male"))
    {
        OutWalkerParams.Gender = EPedestrianGender::Male;
    }
    else
    {
        OutWalkerParams.Gender = EPedestrianGender::INVALID;
    }

    FString AgeString;
    JsonObject->TryGetStringField(TEXT("Age"), AgeString);
    
    if(AgeString == FString("Child")){
        OutWalkerParams.Age = EPedestrianAge::Child;
    }
    else if(AgeString == FString("Teenager"))
    {
        OutWalkerParams.Age = EPedestrianAge::Teenager;
    }
    else if(AgeString == FString("Adult"))
    {
        OutWalkerParams.Age = EPedestrianAge::Adult;
    }
    else if(AgeString == FString("Elderly"))
    {
        OutWalkerParams.Age = EPedestrianAge::Elderly;
    }
    else
    {
        OutWalkerParams.Age = EPedestrianAge::INVALID;
    }

    // Parse Speeds
    const TArray<TSharedPtr<FJsonValue>>* SpeedsArray;
    if (JsonObject->TryGetArrayField(TEXT("Speeds"), SpeedsArray))
    {
        OutWalkerParams.Speed.Empty();
        for (const TSharedPtr<FJsonValue>& SpeedValue : *SpeedsArray)
        {
            TSharedPtr<FJsonObject> SpeedObject = SpeedValue->AsObject();
            if (SpeedObject.IsValid())
            {
                float Speed;
                Speed = SpeedObject->GetNumberField(TEXT("Speed"));
                OutWalkerParams.Speed.Add(Speed);
            }
        }
    }

    JsonObject->TryGetNumberField(TEXT("Generation"), OutWalkerParams.Generation);

    return true;
}

bool AWalkerActorFactory::JsonToFWalkerParametersArray(const FString& JsonString, TArray<FPedestrianParameters>& OutWalkerParamsArray)
{
    TSharedPtr<FJsonObject> RootObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        UE_LOG(LogCarla, Error, TEXT("Failed to parse JSON."));
        return false;
    }

    // Get the "Walkers" array from the JSON root object
    const TArray<TSharedPtr<FJsonValue>>* WalkersArray;
    if (RootObject->TryGetArrayField(TEXT("Walkers"), WalkersArray))
    {
        OutWalkerParamsArray.Empty();
        for (const TSharedPtr<FJsonValue>& WalkerValue : *WalkersArray)
        {
            TSharedPtr<FJsonObject> WalkerObject = WalkerValue->AsObject();
            if (WalkerObject.IsValid())
            {
                FPedestrianParameters WalkerParams;
                if (JsonToFWalkerParameters(WalkerObject, WalkerParams))
                {
                    OutWalkerParamsArray.Add(WalkerParams);
                }
            }
        }
    }

    return true;
}

void AWalkerActorFactory::LoadWalkerParametersArrayFromFile(const FString& FileName, TArray<FPedestrianParameters>& OutWalkerParamsArray)
{
  FString JsonString;
  FString FilePath = FPaths::ProjectContentDir() + TEXT("Carla/Config/") + FileName;
  FString JsonContent;

  // Load the JSON file content into an FString
  if (FFileHelper::LoadFileToString(JsonContent, *FilePath))
  {
      // Parse the JSON and populate the TArray<FPedestrianParameters>
      if (JsonToFWalkerParametersArray(JsonContent, OutWalkerParamsArray))
      {
          UE_LOG(LogCarla, Log, TEXT("Walker parameters loaded successfully from %s"), *FilePath);
      }
      else
      {
          UE_LOG(LogCarla, Error, TEXT("Failed to parse Walker parameters from %s"), *FilePath);
      }
  }
  else
  {
      UE_LOG(LogCarla, Error, TEXT("Failed to load file: %s"), *FilePath);
  }
}