// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Actor/CarlaBlueprintRegistry.h"
#include "Carla.h"
#include "Carla/Game/CarlaStatics.h"

#include <util/ue-header-guard-begin.h>
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include <util/ue-header-guard-end.h>

namespace CommonAttributes {
  static const FString PATH = FPaths::ProjectContentDir();
  static const FString DEFAULT = TEXT("/Carla/Config/Default");
  static const FString DEFINITIONS = TEXT("props");
}

namespace PropAttributes {
  static const FString REGISTRY_FORMAT = TEXT(".Package.json");
  static const FString NAME = TEXT("name");
  static const FString MESH_PATH = TEXT("path");
  static const FString SIZE = TEXT("size");
}

static FString PropSizeTypeToString(EPropSize PropSizeType)
{
  const UEnum *ptr = FindObject<UEnum>(nullptr, TEXT("/Script/Carla.EPropSize"), true);
  if (!ptr)
  {
    return FString("unknown");
  }
  return ptr->GetNameStringByIndex(static_cast<int32>(PropSizeType));
}

static EPropSize StringToPropSizeType(FString PropSize)
{
  const UEnum *EnumPtr = FindObject<UEnum>(nullptr, TEXT("/Script/Carla.EPropSize"), true);
  if (EnumPtr)
  {
    return (EPropSize) EnumPtr->GetIndexByName(FName(*PropSize));
  }
  return EPropSize::INVALID;
}

void UCarlaBlueprintRegistry::AddToCarlaBlueprintRegistry(const TArray<FPropParameters> &PropParametersArray)
{
  TArray<TSharedPtr<FJsonValue>> ResultPropJsonArray;

  // Load default props file
  FString DefaultPropFilePath = CommonAttributes::PATH + CommonAttributes::DEFAULT +
      PropAttributes::REGISTRY_FORMAT;
  FString JsonString;
  FFileHelper::LoadFileToString(JsonString, *DefaultPropFilePath);

  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
  TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

  // Fill Array of Props and save indexes of each prop in PropIndexes Map
  TMap<FString, int> PropIndexes;
  if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
  {
    ResultPropJsonArray = JsonObject->GetArrayField(CommonAttributes::DEFINITIONS);
    for (int32 i = 0; i < ResultPropJsonArray.Num(); ++i)
    {
      TSharedPtr<FJsonObject> PropJsonObject = ResultPropJsonArray[i]->AsObject();

      FString Name = PropJsonObject->GetStringField(PropAttributes::NAME);

      PropIndexes.Add(Name, i);
    }
  }

  // Add Input Props or Update them if already exists
  for (auto &PropParameter : PropParametersArray)
  {
    TSharedPtr<FJsonObject> PropJsonObject;

    // Create object or update existing one
    int *PropIndex = PropIndexes.Find(PropParameter.Name);
    if (PropIndex)
    {
      PropJsonObject = ResultPropJsonArray[*PropIndex]->AsObject();
    }
    else
    {
      PropJsonObject = MakeShareable(new FJsonObject);
    }

    // Fill Prop Json
    PropJsonObject->SetStringField(PropAttributes::NAME, PropParameter.Name);
    PropJsonObject->SetStringField(PropAttributes::MESH_PATH, PropParameter.Mesh->GetPathName());
    PropJsonObject->SetStringField(PropAttributes::SIZE, PropSizeTypeToString(PropParameter.Size));

    // Add or Update
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

  // Update Json Object
  JsonObject->SetArrayField(CommonAttributes::DEFINITIONS, ResultPropJsonArray);

  // Serialize file
  FString OutputString;
  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
  FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

  // Save file
  FFileHelper::SaveStringToFile(OutputString, *DefaultPropFilePath);

}

void UCarlaBlueprintRegistry::LoadPropDefinitions(TArray<FPropParameters> &PropParametersArray)
{

  // Find all Package.json files inside Unreal Content folder
  const FString WildCard = FString("*").Append(PropAttributes::REGISTRY_FORMAT);

  TArray<FString> PropFileNames;
  IFileManager::Get().FindFilesRecursive(PropFileNames,
      *CommonAttributes::PATH,
      *WildCard,
      true,
      false,
      false);

  // Sort and place Default File First if it exists
  PropFileNames.Sort();
  FString DefaultFileName;
  for (int32 i = 0; i < PropFileNames.Num() && DefaultFileName.IsEmpty(); ++i)
  {
    if (PropFileNames[i].Contains(CommonAttributes::DEFAULT))
    {
      DefaultFileName = PropFileNames[i];
      PropFileNames.RemoveAt(i);
    }
  }
  if (!DefaultFileName.IsEmpty())
  {
    PropFileNames.Insert(DefaultFileName, 0);
  }

  // Read all registry files and overwrite default registry values with user
  // registry files
  TMap<FString, int> PropIndexes;

  for (int32 i = 0; i < PropFileNames.Num(); ++i)
  {
    FString FileJsonContent;
    if (FFileHelper::LoadFileToString(FileJsonContent, *PropFileNames[i]))
    {
      TSharedPtr<FJsonObject> JsonParsed;
      TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(FileJsonContent);
      if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
      {
        auto PropJsonArray = JsonParsed->GetArrayField(CommonAttributes::DEFINITIONS);

        for (auto &PropJsonValue : PropJsonArray)
        {
          // Read Prop Json
          TSharedPtr<FJsonObject> PropJsonObject = PropJsonValue->AsObject();

          FString PropName = PropJsonObject->GetStringField(PropAttributes::NAME);
          FString PropMeshPath = PropJsonObject->GetStringField(PropAttributes::MESH_PATH);
          FString PropSize = PropJsonObject->GetStringField(PropAttributes::SIZE);

          // Build Prop Parameter
          UStaticMesh *PropMesh = LoadObject<UStaticMesh>(nullptr, *PropMeshPath);
          EPropSize PropSizeType = StringToPropSizeType(PropSize);
          FPropParameters Params {PropName, PropMesh, PropSizeType};

          // Add or Update
          if (PropIndexes.Contains(PropName))
          {
            PropParametersArray[PropIndexes[PropName]] = Params;
          }
          else
          {
            PropParametersArray.Add(Params);
            PropIndexes.Add(Params.Name, PropParametersArray.Num() - 1);
          }
        }
      }
    }
  }
}
