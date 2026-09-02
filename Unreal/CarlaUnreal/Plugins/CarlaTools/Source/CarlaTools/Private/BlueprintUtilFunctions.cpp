// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#include "BlueprintUtilFunctions.h"
#include "GeneralProjectSettings.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "AssetToolsModule.h"
#include "Engine/AssetManager.h"
#include "IAssetTools.h"
#include "ObjectTools.h"
#include "UObject/UObjectGlobals.h"
#include "EditorAssetLibrary.h"
#include "FileHelpers.h"
#include "WorldPartition/WorldPartitionBlueprintLibrary.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Components/SplineComponent.h"

DEFINE_LOG_CATEGORY(LogDigitalTwinsToolBlueprintUtil);

FString UBlueprintUtilFunctions::GetProjectName()
{
  const UGeneralProjectSettings* ProjectSettings = GetDefault<UGeneralProjectSettings>();
  return ProjectSettings->ProjectName;
}

UObject* UBlueprintUtilFunctions::CopyAssetToPlugin(UObject* SourceObject, FString PluginName)
{
#if WITH_EDITOR
  if (!SourceObject) {
    return nullptr;
  }

  // ue58-dev adaptation: CARLA maps are not per-map plugins. When no plugin name
  // is given or the destination mount point does not exist, keep using the source
  // asset in place (identity mapping) - this matches the verified BP_BuildingGen
  // flow where atoms are consumed directly from /CarlaDigitalTwinsTool/.
  {
    FString MountCheck;
    if (PluginName.IsEmpty() ||
        !FPackageName::TryConvertLongPackageNameToFilename(
            FString::Printf(TEXT("/%s/"), *PluginName), MountCheck))
    {
      return SourceObject;
    }
  }

  FString SourcePath = SourceObject->GetPathName();
  FString SourceAssetName = SourceObject->GetName();

  FString RootPath = TEXT("/CarlaDigitalTwinsTool/");
  int32 RootIndex = SourcePath.Find(RootPath);

  FString SourceFolderPath = FPaths::GetPath(SourcePath);

  FString RelativePath = SourceFolderPath.Mid(RootIndex + RootPath.Len());
  FString DestinationPath = FString::Printf(TEXT("/%s/%s"), *PluginName, *RelativePath);
  FString TargetAssetPath = DestinationPath + "/" + SourceAssetName;

  if (UEditorAssetLibrary::DoesAssetExist(TargetAssetPath))
  {
    UObject* ExistingAsset = UEditorAssetLibrary::LoadAsset(TargetAssetPath);
    if (ExistingAsset)
    {
      return ExistingAsset;
    }
    UE_LOG(LogDigitalTwinsToolBlueprintUtil, Error, TEXT("Invalid Asset: %s"), *TargetAssetPath);
    return nullptr;
  }

  FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();
  UObject* DuplicatedAsset = AssetToolsModule.Get().DuplicateAsset(SourceAssetName, DestinationPath, SourceObject);

  if (!DuplicatedAsset)
  {
    UE_LOG(LogDigitalTwinsToolBlueprintUtil, Error, TEXT("Failed to duplicate asset."));
    return nullptr;
  }

  FString DuplicatedPath = DuplicatedAsset->GetPathName();
  if (!UEditorAssetLibrary::SaveAsset(DuplicatedPath, false))
  {
    UE_LOG(LogDigitalTwinsToolBlueprintUtil, Warning, TEXT("Duplicated asset created but not saved: %s"), *DuplicatedPath);
  }

  TSet<UObject*> SubObjects;
  TQueue<UObject*> Pending;
  Pending.Enqueue(DuplicatedAsset);

  while (!Pending.IsEmpty())
  {
    UObject* CurrentObject = nullptr;
    Pending.Dequeue(CurrentObject);

    if(!CurrentObject|| SubObjects.Contains(CurrentObject)) continue;

    SubObjects.Add(CurrentObject);

    TArray<UObject*> FoundRefs;
    FReferenceFinder ReferenceCollector(FoundRefs, nullptr, false, true, true);
    ReferenceCollector.FindReferences(CurrentObject);

    for (UObject* Ref : FoundRefs)
    {
      if (Ref && !SubObjects.Contains(Ref) && Ref->IsAsset())
      {
        Pending.Enqueue(Ref);
      }
    }
  }

  TMap<UObject*, UObject*> ReplacementMap;
  TSet<UObject*> ObjectsToReplaceWithin;
  ObjectsToReplaceWithin.Add(DuplicatedAsset);

  for (UObject* RefObj : SubObjects)
  {
    if (RefObj)
    {
      FString RefObjPath = RefObj->GetPathName();
      RootPath = TEXT("/CarlaDigitalTwinsTool/");
      RootIndex = RefObjPath.Find(RootPath);

      SourceFolderPath = FPaths::GetPath(RefObjPath);
      RelativePath = SourceFolderPath.Mid(RootIndex + RootPath.Len());

      DestinationPath = FString::Printf(TEXT("/%s/%s"), *PluginName, *RelativePath);
      FString TargetRefObjPath = DestinationPath + "/" + RefObj->GetName();

      UObject* LoadedDuplicatedRefObject = nullptr;

      if (!UEditorAssetLibrary::DoesAssetExist(TargetRefObjPath) && RefObjPath.Contains(TEXT("/CarlaDigitalTwinsTool")))
      {
        AssetToolsModule.Get().DuplicateAsset(*RefObj->GetName(), DestinationPath, RefObj);
      }

      LoadedDuplicatedRefObject = UEditorAssetLibrary::LoadAsset(TargetRefObjPath);

      if (LoadedDuplicatedRefObject)
      {
        ReplacementMap.Add(RefObj, LoadedDuplicatedRefObject);
        ObjectsToReplaceWithin.Add(LoadedDuplicatedRefObject);
      }
    }
  }

  for (const auto& Pair : ReplacementMap)
  {
    UObject* Original = Pair.Key;
    UObject* Replacement = Pair.Value;

    TArray<UObject*> ObjectsToReplace = {Original};
    ObjectTools::ForceReplaceReferences(Replacement, ObjectsToReplace, ObjectsToReplaceWithin);
  }

  UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

  return DuplicatedAsset;

#else
  return nullptr;
#endif
}

void UBlueprintUtilFunctions::UnloadRegionWorldPartition(
    UWorld* World,
    const FBox& RegionBounds,
    const FString& RegionName)
{
  // 1. Query descriptors in your region
  TArray<FActorDesc> Descs;
  //UWorldPartitionBlueprintLibrary::GetIntersectingActorDescs(RegionBounds, Descs);  // :contentReference[oaicite:0]{index=0}

  // 2. Prepare an array for GUIDs to unload
  TArray<FGuid> ToUnload;
  ToUnload.Reserve(Descs.Num());

  // 3. Filter out any actor whose CDO has a spline component
  for (const FActorDesc& Desc : Descs)
  {
      // a) Get the actor class from the descriptor
      UClass* ActorClass = Cast<UClass>(Desc.Class.TryLoad());
      if (!ActorClass)
      {
          // No valid class—safe to unload
          ToUnload.Add(Desc.Guid);
          continue;
      }

      // b) Grab the class default object and check for spline
      AActor* CDO = Cast<AActor>(ActorClass->GetDefaultObject());
      if (CDO && CDO->FindComponentByClass<USplineComponent>())  // :contentReference[oaicite:1]{index=1}
      {
          // Skip any actor types that include a spline component
          continue;
      }

      // c) Otherwise mark for unload
      ToUnload.Add(Desc.Guid);
  }

  // 4. Issue the unload call for all the remaining GUIDs
  //UWorldPartitionBlueprintLibrary::UnloadActors(ToUnload);
  
}
