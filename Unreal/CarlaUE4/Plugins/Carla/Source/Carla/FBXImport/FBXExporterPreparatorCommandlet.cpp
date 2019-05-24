// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FBXExporterPreparatorCommandlet.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/PackageName.h"
#include "UObject/MetaData.h"
#include "UObject/Package.h"


UFBXExporterPreparatorCommandlet::UFBXExporterPreparatorCommandlet()
{
  IsClient = false;
  IsEditor = true;
  IsServer = false;
  LogToConsole = true;
}
#if WITH_EDITORONLY_DATA

bool UFBXExporterPreparatorCommandlet::ParseParams(const FString &InParams)
{
  Params = InParams;
  ParseCommandLine(*Params, Tokens, Switches);

  return true;
}

void UFBXExporterPreparatorCommandlet::LoadWorld(FAssetData &AssetData)
{
  FString SeedMap;
  FParse::Value(*Params, TEXT("SourceMap="), SeedMap);

  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  MapObjectLibrary->AddToRoot();
  MapObjectLibrary->LoadAssetDataFromPath(*SeedMap);
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetDatas);

  if (AssetDatas.Num() > 0)
  {
    AssetData = AssetDatas.Pop();
  }
}

void UFBXExporterPreparatorCommandlet::AddMeshesToWorld()
{
  TArray<FString> CmdLineDirEntries;

  const FString CookDirPrefix = TEXT("MESHESDIR=");
  for (int32 SwitchIdx = 0; SwitchIdx < Switches.Num(); SwitchIdx++)
  {
    const FString &Switch = Switches[SwitchIdx];
    if (Switch.StartsWith(CookDirPrefix))
    {
      FString DirToCook = Switch.Right(Switch.Len() - 10);

      // Allow support for -COOKDIR=Dir1+Dir2+Dir3 as well as -COOKDIR=Dir1
      // -COOKDIR=Dir2
      for (int32 PlusIdx = DirToCook.Find(TEXT("+"));
          PlusIdx != INDEX_NONE;
          PlusIdx = DirToCook.Find(TEXT("+")))
      {
        FString DirName = DirToCook.Left(PlusIdx);

        // The dir may be contained within quotes
        DirName = DirName.TrimQuotes();
        FPaths::NormalizeDirectoryName(DirName);
        CmdLineDirEntries.Add(DirName);

        DirToCook = DirToCook.Right(DirToCook.Len() - (PlusIdx + 1));
        DirToCook = DirToCook.TrimQuotes();
      }
    }
  }

  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();
  AssetsObjectLibrary->LoadAssetDataFromPaths(CmdLineDirEntries);
  AssetsObjectLibrary->LoadAssetsFromAssetData();

  const FTransform zeroTransform = FTransform();
  FVector initialVector = FVector(0, 0, 0);
  FRotator initialRotator = FRotator(0, 180, 0);
  FActorSpawnParameters SpawnInfo;

  MapContents.Empty();
  AssetsObjectLibrary->GetAssetDataList(MapContents);

  UStaticMesh *MeshAsset;
  AStaticMeshActor *MeshActor;
  for (auto MapAsset : MapContents)
  {
    MeshAsset = CastChecked<UStaticMesh>(MapAsset.GetAsset());
    MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
        initialVector,
        initialRotator);
    MeshActor->GetStaticMeshComponent()->SetStaticMesh(CastChecked<UStaticMesh>(MeshAsset));
  }
  World->MarkPackageDirty();

}

bool UFBXExporterPreparatorCommandlet::SaveWorld(FAssetData &AssetData)
{
  FString DestPath, WorldName, PackageName;
  FParse::Value(*Params, TEXT("DestMapPath="), DestPath);
  FParse::Value(*Params, TEXT("DestMapName="), WorldName);

  PackageName = DestPath + "/" + WorldName;
  UPackage *Package = AssetData.GetPackage();
  Package->SetFolderName(*DestPath);
  Package->FullyLoad();
  Package->MarkPackageDirty();
  FAssetRegistryModule::AssetCreated(World);

  // Notify the system about the map's name change
  World->Rename(*WorldName, World->GetOuter());
  FAssetRegistryModule::AssetRenamed(World, *PackageName);
  World->MarkPackageDirty();
  World->GetOuter()->MarkPackageDirty();

  // Saving the package
  FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName,
      FPackageName::GetMapPackageExtension());
  return UPackage::SavePackage(Package, World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
}

int32 UFBXExporterPreparatorCommandlet::Main(const FString &ParamsIn)
{
  ParseParams(ParamsIn);
  FAssetData AssetData;
  LoadWorld(AssetData);
  World = CastChecked<UWorld>(AssetData.GetAsset());
  AddMeshesToWorld();
  SaveWorld(AssetData);
  return 0;
}

#endif
