// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Actor/Factory/VehicleActorFactory.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Game/CarlaEpisode.h"

#include <util/ue-header-guard-begin.h>
#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include <util/ue-header-guard-end.h>

TArray<FActorDefinition> AVehicleActorFactory::GetDefinitions()
{
  LoadVehicleParametersArrayFromFile("VehicleParameters.json", VehiclesParams);
  FString UniqueVehicleParameters = GetWorld()->GetMapName().Mid(GetWorld()->StreamingLevelsPrefix.Len()) + "/Vehicles.json";
  LoadVehicleParametersArrayFromFile(UniqueVehicleParameters, MineVehiclesParams);
  VehiclesParams.Append(MineVehiclesParams);
  UActorBlueprintFunctionLibrary::MakeVehicleDefinitions(VehiclesParams, Definitions);
  return Definitions;
}

FActorSpawnResult AVehicleActorFactory::SpawnActor(
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

  if( PostProcessVehicle(SpawnedActor, ActorDescription) ){
    SpawnResult.Status = EActorSpawnResultStatus::Success;
    return SpawnResult;
  }
  SpawnResult.Status = EActorSpawnResultStatus::UnknownError;
  return SpawnResult;
}

TSharedPtr<FJsonObject> AVehicleActorFactory::FVehicleParametersToJsonObject(const FVehicleParameters& VehicleParams)
{
  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

  JsonObject->SetStringField(TEXT("Make"), VehicleParams.Make);
  JsonObject->SetStringField(TEXT("Model"), VehicleParams.Model);
  JsonObject->SetStringField(TEXT("Class"), VehicleParams.Class ? VehicleParams.Class->GetPathName() : "");
  JsonObject->SetNumberField(TEXT("NumberOfWheels"), VehicleParams.NumberOfWheels);
  JsonObject->SetNumberField(TEXT("Generation"), VehicleParams.Generation);
  JsonObject->SetStringField(TEXT("ObjectType"), VehicleParams.ObjectType);
  JsonObject->SetStringField(TEXT("BaseType"), VehicleParams.BaseType);
  JsonObject->SetStringField(TEXT("SpecialType"), VehicleParams.SpecialType);
  JsonObject->SetBoolField(TEXT("HasDynamicDoors"), VehicleParams.HasDynamicDoors);
  JsonObject->SetBoolField(TEXT("HasLights"), VehicleParams.HasLights);

  // Serialize RecommendedColors
  TArray<TSharedPtr<FJsonValue>> ColorsArray;
  for (const FColor& Color : VehicleParams.RecommendedColors)
  {
      TSharedPtr<FJsonObject> ColorObject = MakeShareable(new FJsonObject);
      ColorObject->SetNumberField(TEXT("R"), Color.R);
      ColorObject->SetNumberField(TEXT("G"), Color.G);
      ColorObject->SetNumberField(TEXT("B"), Color.B);
      ColorObject->SetNumberField(TEXT("A"), Color.A);
      ColorsArray.Add(MakeShareable(new FJsonValueObject(ColorObject)));
  }
  JsonObject->SetArrayField(TEXT("RecommendedColors"), ColorsArray);

  // Serialize SupportedDrivers
  TArray<TSharedPtr<FJsonValue>> DriversArray;
  for (int32 DriverID : VehicleParams.SupportedDrivers)
  {
      DriversArray.Add(MakeShareable(new FJsonValueNumber(DriverID)));
  }
  JsonObject->SetArrayField(TEXT("SupportedDrivers"), DriversArray);

  return JsonObject;
}

FString AVehicleActorFactory::FVehicleParametersArrayToJson(const TArray<FVehicleParameters>& VehicleParamsArray)
{
  TArray<TSharedPtr<FJsonValue>> JsonArray;

  for (const FVehicleParameters& VehicleParams : VehicleParamsArray)
  {
      // Convert each FVehicleParameters to a JSON object
      TSharedPtr<FJsonObject> JsonObject = FVehicleParametersToJsonObject(VehicleParams);
      JsonArray.Add(MakeShareable(new FJsonValueObject(JsonObject)));
  }

  // Convert the array of JSON objects into a single JSON object
  TSharedRef<FJsonObject> RootObject = MakeShareable(new FJsonObject);
  RootObject->SetArrayField(TEXT("Vehicles"), JsonArray);

  // Serialize the JSON object into an FString
  FString OutputString;
  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
  FJsonSerializer::Serialize(RootObject, Writer);

  return OutputString;
}

void AVehicleActorFactory::SaveVehicleParametersArrayToFile(const TArray<FVehicleParameters>& VehicleParamsArray, const FString& FileName)
{
  FString FilePath = FPaths::ProjectContentDir() + TEXT("Carla/Config/") + FileName;
  // Convert the array to an FString in JSON format
  FString JsonContent = FVehicleParametersArrayToJson(VehicleParamsArray);

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

bool AVehicleActorFactory::JsonToFVehicleParameters(const TSharedPtr<FJsonObject> JsonObject, FVehicleParameters& OutVehicleParams)
{
    if (!JsonObject.IsValid()) return false;

    JsonObject->TryGetStringField(TEXT("Make"), OutVehicleParams.Make);
    JsonObject->TryGetStringField(TEXT("Model"), OutVehicleParams.Model);
    // Convert "Class" string back to a class reference if necessary
    FString ClassPath;
    JsonObject->TryGetStringField(TEXT("Class"), ClassPath); // Custom conversion required if needed
    OutVehicleParams.Class = StaticLoadClass(ACarlaWheeledVehicle::StaticClass(), nullptr, *ClassPath);

    JsonObject->TryGetNumberField(TEXT("NumberOfWheels"), OutVehicleParams.NumberOfWheels);
    JsonObject->TryGetNumberField(TEXT("Generation"), OutVehicleParams.Generation);
    JsonObject->TryGetStringField(TEXT("ObjectType"), OutVehicleParams.ObjectType);
    JsonObject->TryGetStringField(TEXT("BaseType"), OutVehicleParams.BaseType);
    JsonObject->TryGetStringField(TEXT("SpecialType"), OutVehicleParams.SpecialType);
    JsonObject->TryGetBoolField(TEXT("HasDynamicDoors"), OutVehicleParams.HasDynamicDoors);
    JsonObject->TryGetBoolField(TEXT("HasLights"), OutVehicleParams.HasLights);

    // Parse RecommendedColors
    const TArray<TSharedPtr<FJsonValue>>* ColorsArray;
    if (JsonObject->TryGetArrayField(TEXT("RecommendedColors"), ColorsArray))
    {
        OutVehicleParams.RecommendedColors.Empty();
        for (const TSharedPtr<FJsonValue>& ColorValue : *ColorsArray)
        {
            TSharedPtr<FJsonObject> ColorObject = ColorValue->AsObject();
            if (ColorObject.IsValid())
            {
                FColor Color;
                Color.R = ColorObject->GetIntegerField(TEXT("R"));
                Color.G = ColorObject->GetIntegerField(TEXT("G"));
                Color.B = ColorObject->GetIntegerField(TEXT("B"));
                Color.A = ColorObject->GetIntegerField(TEXT("A"));
                OutVehicleParams.RecommendedColors.Add(Color);
            }
        }
    }

    // Parse SupportedDrivers
    const TArray<TSharedPtr<FJsonValue>>* DriversArray;
    if (JsonObject->TryGetArrayField(TEXT("SupportedDrivers"), DriversArray))
    {
        OutVehicleParams.SupportedDrivers.Empty();
        for (const TSharedPtr<FJsonValue>& DriverValue : *DriversArray)
        {
            OutVehicleParams.SupportedDrivers.Add(DriverValue->AsNumber());
        }
    }

    return true;
}

bool AVehicleActorFactory::JsonToFVehicleParametersArray(const FString& JsonString, TArray<FVehicleParameters>& OutVehicleParamsArray)
{
    TSharedPtr<FJsonObject> RootObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        UE_LOG(LogCarla, Error, TEXT("Failed to parse JSON."));
        return false;
    }

    // Get the "Vehicles" array from the JSON root object
    const TArray<TSharedPtr<FJsonValue>>* VehiclesArray;
    if (RootObject->TryGetArrayField(TEXT("Vehicles"), VehiclesArray))
    {
        OutVehicleParamsArray.Empty();
        for (const TSharedPtr<FJsonValue>& VehicleValue : *VehiclesArray)
        {
            TSharedPtr<FJsonObject> VehicleObject = VehicleValue->AsObject();
            if (VehicleObject.IsValid())
            {
                FVehicleParameters VehicleParams;
                if (JsonToFVehicleParameters(VehicleObject, VehicleParams))
                {
                    OutVehicleParamsArray.Add(VehicleParams);
                }
            }
        }
    }

    return true;
}

void AVehicleActorFactory::LoadVehicleParametersArrayFromFile(const FString& FileName, TArray<FVehicleParameters>& OutVehicleParamsArray)
{
  FString JsonString;
  FString FilePath = FPaths::ProjectContentDir() + TEXT("Carla/Config/") + FileName;
  FString JsonContent;

  // Load the JSON file content into an FString
  if (FFileHelper::LoadFileToString(JsonContent, *FilePath))
  {
      // Parse the JSON and populate the TArray<FVehicleParameters>
      if (JsonToFVehicleParametersArray(JsonContent, OutVehicleParamsArray))
      {
          UE_LOG(LogCarla, Log, TEXT("Vehicle parameters loaded successfully from %s"), *FilePath);
      }
      else
      {
          UE_LOG(LogCarla, Error, TEXT("Failed to parse vehicle parameters from %s"), *FilePath);
      }
  }
  else if(FileName == "VehicleParameters.json")
  {
      UE_LOG(LogCarla, Error, TEXT("Failed to load file: %s"), *FilePath);
  }
  else 
  {
    UE_LOG(LogCarla, Warning, TEXT("Additional file not found: %s"), *FilePath);
  }
}
