// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/CarlaBlueprintRegistry.h"
#include "Carla/Game/CarlaStatics.h"

void UCarlaBlueprintRegistry::AddToCarlaBlueprintRegistry(const TArray<FPropParameters> &PropParametersArray)
{
  // Load default props file
  const FString PropsFolderPath = FPaths::ProjectContentDir() + "/Carla/Config";
  const FString DefaultPropFilePath = PropsFolderPath + "/" + "Default.PropRegistry.json";

  FString JsonString;
  FFileHelper::LoadFileToString(JsonString, *DefaultPropFilePath);

  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
  TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

  TArray<TSharedPtr<FJsonValue>> PropJsonArray;

  // Populate definitions array if correspoding field exists in file
  if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
  {
    PropJsonArray = JsonObject->GetArrayField("definitions");
  }

  // Add Input Prop Parameters
  for (auto &PropParameter : PropParametersArray)
  {
    TSharedPtr<FJsonObject> PropJsonObject = MakeShareable(new FJsonObject);

    // Name
    PropJsonObject->SetStringField("name", PropParameter.Name);

    // Path
    PropJsonObject->SetStringField("path", PropParameter.Mesh->GetPathName());

    // Size
    auto GetSize = [](EPropSize Value) {
      switch (Value)
      {
        case EPropSize::Tiny:    return TEXT("tiny");
        case EPropSize::Small:   return TEXT("small");
        case EPropSize::Medium:  return TEXT("medium");
        case EPropSize::Big:     return TEXT("big");
        case EPropSize::Huge:    return TEXT("huge");
        default:                 return TEXT("unknown");
      }
    };
    PropJsonObject->SetStringField("size", GetSize(PropParameter.Size));

    // Add Json Prop to array
    TSharedRef<FJsonValue> PropJsonValue = MakeShareable(new FJsonValueObject(PropJsonObject));
    PropJsonArray.Add(PropJsonValue);
  }

  JsonObject->SetArrayField("definitions", PropJsonArray);

  // Serialize file
  FString OutputString;
  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
  FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

  // Save file
  FFileHelper::SaveStringToFile(OutputString, *DefaultPropFilePath);

}

void UCarlaBlueprintRegistry::LoadPropDefinitions(TArray<FActorDefinition> &Definitions)
{
  // Loads prop registry json files
  const FString PropsFolderPath = FPaths::ProjectContentDir() + "/Carla/Config/";
  TArray<FString> PropNameList;
  IFileManager::Get().FindFilesRecursive(PropNameList,
      *PropsFolderPath,
      TEXT("*.PropRegistry.json"),
      true,
      false,
      false);

  // Make prop definition for each prop registry file
  for (auto &PropFilePath : PropNameList)
  {
    FString FileJsonContent;
    if (FFileHelper::LoadFileToString(FileJsonContent, *PropFilePath))
    {
      TSharedPtr<FJsonObject> JsonParsed;
      TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(FileJsonContent);
      if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
      {
        auto PropJsonArray = JsonParsed->GetArrayField("definitions");

        for (auto &PropJsonValue : PropJsonArray)
        {
          // Read Prop Json
          TSharedPtr<FJsonObject> PropJsonObject = PropJsonValue->AsObject();

          FString PropName = PropJsonObject->GetStringField("name");
          FString PropPath = PropJsonObject->GetStringField("path");
          FString PropSize = PropJsonObject->GetStringField("size");

          // Create Definition if not repeated
          if (!PropNames.Contains(PropName))
          {
            FActorDefinition Definition;
            FillIdAndTags(Definition, TEXT("static"),  TEXT("prop"), PropName);
            AddRecommendedValuesForActorRoleName(Definition, {TEXT("prop")});

            Definition.Attributes.Emplace(FActorAttribute{
              TEXT("path"),
              EActorAttributeType::String,
              *PropPath});

            Definition.Attributes.Emplace(FActorAttribute{
              TEXT("size"),
              EActorAttributeType::String,
              *PropSize});

            bool Success = UActorBlueprintFunctionLibrary::CheckActorDefinition(Definition);
            if (Success)
            {
              Definitions.Emplace(std::move(Definition));
              PropNames.Add(PropName);
            }
          }

        }
      }
    }
  }
}
