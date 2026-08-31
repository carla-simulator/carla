// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/ContentPacks/ContentPackManifest.h"
#include "Carla.h"

#include <util/ue-header-guard-begin.h>
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include <util/ue-header-guard-end.h>

bool FCarlaContentPackManifest::IsValidPackName(const FString &InName)
{
  if (InName.IsEmpty())
  {
    return false;
  }
  for (const TCHAR C : InName)
  {
    if (!FChar::IsAlnum(C) && C != TEXT('_') && C != TEXT('-'))
    {
      return false;
    }
  }
  return true;
}

bool FCarlaContentPackManifest::Parse(
    const FString &JsonText,
    FCarlaContentPackManifest &Out,
    FString &OutError)
{
  TSharedPtr<FJsonObject> Root;
  TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(JsonText);
  if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
  {
    OutError = FString::Printf(TEXT("manifest is not valid JSON: %s"), *Reader->GetErrorMessage());
    return false;
  }

  if (!Root->TryGetStringField(TEXT("name"), Out.Name) || Out.Name.IsEmpty())
  {
    OutError = TEXT("manifest has no \"name\"");
    return false;
  }
  if (!IsValidPackName(Out.Name))
  {
    OutError = FString::Printf(
        TEXT("manifest \"name\" '%s' is not a valid pack name (letters, digits, '_' and '-' only)"),
        *Out.Name);
    return false;
  }

  Root->TryGetStringField(TEXT("version"), Out.Version);
  Root->TryGetStringField(TEXT("carla_version"), Out.CarlaVersion);
  Root->TryGetStringField(TEXT("base_release"), Out.BaseRelease);
  Root->TryGetStringField(TEXT("platform"), Out.Platform);

  const TSharedPtr<FJsonObject> *Engine = nullptr;
  if (Root->TryGetObjectField(TEXT("engine"), Engine) && Engine != nullptr && Engine->IsValid())
  {
    (*Engine)->TryGetStringField(TEXT("version"), Out.EngineVersion);
    (*Engine)->TryGetStringField(TEXT("commit"), Out.EngineCommit);
  }

  Out.Maps.Reset();
  const TArray<TSharedPtr<FJsonValue>> *MapsArray = nullptr;
  if (Root->TryGetArrayField(TEXT("maps"), MapsArray) && MapsArray != nullptr)
  {
    for (const TSharedPtr<FJsonValue> &Value : *MapsArray)
    {
      const TSharedPtr<FJsonObject> MapObject = Value.IsValid() ? Value->AsObject() : nullptr;
      if (!MapObject.IsValid())
      {
        OutError = TEXT("manifest \"maps\" entries must be objects");
        return false;
      }
      FCarlaContentPackMap Map;
      MapObject->TryGetStringField(TEXT("name"), Map.Name);
      MapObject->TryGetStringField(TEXT("package"), Map.Package);
      MapObject->TryGetStringField(TEXT("xodr"), Map.Xodr);
      MapObject->TryGetBoolField(TEXT("world_partition"), Map.bWorldPartition);
      if (Map.Name.IsEmpty())
      {
        OutError = TEXT("manifest \"maps\" entry without \"name\"");
        return false;
      }
      if (Map.Package.IsEmpty())
      {
        // Default to the canonical layout.
        Map.Package = FString::Printf(TEXT("/%s/Maps/%s"), *Out.Name, *Map.Name);
      }
      const FString ExpectedRoot = FString::Printf(TEXT("/%s/"), *Out.Name);
      if (!Map.Package.StartsWith(ExpectedRoot))
      {
        OutError = FString::Printf(
            TEXT("map '%s' package '%s' is not under the pack mount point '%s'"),
            *Map.Name, *Map.Package, *ExpectedRoot);
        return false;
      }
      Out.Maps.Add(MoveTemp(Map));
    }
  }

  Out.Catalogs.Reset();
  const TArray<TSharedPtr<FJsonValue>> *CatalogsArray = nullptr;
  if (Root->TryGetArrayField(TEXT("catalogs"), CatalogsArray) && CatalogsArray != nullptr)
  {
    for (const TSharedPtr<FJsonValue> &Value : *CatalogsArray)
    {
      FString Catalog;
      if (Value.IsValid() && Value->TryGetString(Catalog) && !Catalog.IsEmpty())
      {
        Out.Catalogs.Add(Catalog);
      }
    }
  }

  Out.Files.Reset();
  const TSharedPtr<FJsonObject> *FilesObject = nullptr;
  if (Root->TryGetObjectField(TEXT("files"), FilesObject) && FilesObject != nullptr && FilesObject->IsValid())
  {
    for (const auto &Pair : (*FilesObject)->Values)
    {
      FString Digest;
      if (Pair.Value.IsValid() && Pair.Value->TryGetString(Digest))
      {
        Out.Files.Add(FString(Pair.Key.ToView()), Digest);
      }
    }
  }

  return true;
}

bool FCarlaContentPackManifest::LoadFromFile(
    const FString &InManifestPath,
    FCarlaContentPackManifest &Out,
    FString &OutError)
{
  const FString FullPath = FPaths::ConvertRelativePathToFull(InManifestPath);
  FString JsonText;
  if (!FFileHelper::LoadFileToString(JsonText, *FullPath))
  {
    OutError = FString::Printf(TEXT("cannot read manifest '%s'"), *FullPath);
    return false;
  }
  if (!Parse(JsonText, Out, OutError))
  {
    OutError = FString::Printf(TEXT("%s: %s"), *FullPath, *OutError);
    return false;
  }
  Out.ManifestPath = FullPath;
  Out.PackDir = FPaths::GetPath(FullPath);
  return true;
}

TArray<FString> FCarlaContentPackManifest::GetMapNames() const
{
  TArray<FString> Names;
  Names.Reserve(Maps.Num());
  for (const FCarlaContentPackMap &Map : Maps)
  {
    Names.Add(Map.Name);
  }
  return Names;
}
