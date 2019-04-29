// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
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

  // Populate default file definitions array if corresponding field exists in
  // file
  TMap<FString, int> PropIndexes;
  TArray<TSharedPtr<FJsonValue>> ResultPropJsonArray;
  if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
  {
    ResultPropJsonArray = JsonObject->GetArrayField("definitions");
    for (int i = 0; i < ResultPropJsonArray.Num(); ++i)
    {
      TSharedPtr<FJsonObject> PropJsonObject = ResultPropJsonArray[i]->AsObject();

      FString Name = PropJsonObject->GetStringField(PROP_NAME);

      PropIndexes.Add(Name, i);
    }
  }

  // Add Input Prop Parameters
  for (auto &PropParameter : PropParametersArray)
  {
    TSharedPtr<FJsonObject> PropJsonObject;

    int *PropIndex = PropIndexes.Find(PropParameter.Name);
    if (PropIndex)
    {
      PropJsonObject = ResultPropJsonArray[*PropIndex]->AsObject();
    }
    else
    {
      PropJsonObject = MakeShareable(new FJsonObject);
    }

    // Name
    PropJsonObject->SetStringField(PROP_NAME, PropParameter.Name);

    // Path
    PropJsonObject->SetStringField(PROP_PATH, PropParameter.Mesh->GetPathName());

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
    PropJsonObject->SetStringField(PROP_SIZE, GetSize(PropParameter.Size));

    TSharedRef<FJsonValue> PropJsonValue = MakeShareable(new FJsonValueObject(PropJsonObject));
    if (PropIndex)
    {
      ResultPropJsonArray[*PropIndex] = PropJsonValue;
    }
    else
    {
      ResultPropJsonArray.Add(PropJsonValue);

      PropIndexes.Add(PropParameter.Name, ResultPropJsonArray.Num() - 1);
    }
  }

  JsonObject->SetArrayField("definitions", ResultPropJsonArray);

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
  const FString Extension = ".PropRegistry.json";
  const FString WildCard = (FString("*").Append(Extension));

  TArray<FString> PropFileNames;
  IFileManager::Get().FindFilesRecursive(PropFileNames,
      *PropsFolderPath,
      *WildCard,
      true,
      false,
      false);

  // Sort and place Default File First
  PropFileNames.Sort();
  FString DefaultFileName;
  bool bDefaultFileFound = false;
  for (int i = 0; i < PropFileNames.Num() && !bDefaultFileFound; ++i)
  {
    if (PropFileNames[i].Contains("Default"))
    {
      DefaultFileName = PropFileNames[i];
      PropFileNames.RemoveAt(i);
      bDefaultFileFound = true;
    }
  }
  if (bDefaultFileFound)
  {
    PropFileNames.Insert(DefaultFileName, 0);
  }

  // Read all registry files and overwrite default registry values with user
  // registry files
  TMap<FString, TMap<FString, FString>> PropParametersMap;
  for (uint32_t i = 0; i < PropFileNames.Num(); ++i)
  {
    FString FileJsonContent;
    if (FFileHelper::LoadFileToString(FileJsonContent, *PropFileNames[i]))
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

          FString PropName = PropJsonObject->GetStringField(PROP_NAME);
          FString PropPath = PropJsonObject->GetStringField(PROP_PATH);
          FString PropSize = PropJsonObject->GetStringField(PROP_SIZE);

          TMap<FString, FString> Params;
          Params.Add(PROP_PATH, PropPath);
          Params.Add(PROP_SIZE, PropSize);

          PropParametersMap.Add(PropName, Params);
        }
      }
    }
  }

  // Make Definitions from resulting registry parameters
  for (auto &PropParameters : PropParametersMap)
  {
    FActorDefinition Definition;
    FillIdAndTags(Definition, TEXT("static"),  TEXT("prop"), PropParameters.Key);
    AddRecommendedValuesForActorRoleName(Definition, {TEXT("prop")});

    Definition.Attributes.Emplace(FActorAttribute{
      PROP_PATH,
      EActorAttributeType::String,
      *PropParameters.Value[PROP_PATH]});

    Definition.Attributes.Emplace(FActorAttribute{
      PROP_SIZE,
      EActorAttributeType::String,
      *PropParameters.Value[PROP_SIZE]});

    bool Success = UActorBlueprintFunctionLibrary::CheckActorDefinition(Definition);
    if (Success)
    {
      Definitions.Emplace(std::move(Definition));
    }
  }
}
