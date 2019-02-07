// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "HelloWorldCommandlet.h"
#include "CommandletPluginPrivate.h"

UHelloWorldCommandlet::UHelloWorldCommandlet()
{
  IsClient = false;
  IsEditor = false;
  IsServer = false;
  LogToConsole = true;
}

void UHelloWorldCommandlet::CreateMapFromBaseMap()
{
  // Creating the Object Library that will manage the load
  auto ObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  ObjectLibrary->AddToRoot();
  ObjectLibrary->LoadAssetDataFromPath(TEXT("/Game/Carla/Maps/BaseMap"));
  ObjectLibrary->LoadAssetsFromAssetData();

  // Obtaining the asset data, aka the map
  TArray<FAssetData> AssetDatas;
  ObjectLibrary->GetAssetDataList(AssetDatas);
  FAssetData assetData;
  if (AssetDatas.Num() > 0)
  {
    assetData = AssetDatas.Pop();
  }
  UWorld *world = CastChecked<UWorld>(assetData.GetAsset());

  // Getting the package and doing some rename stuff for the package
  FString PackageName = TEXT("/Game/Carla/Maps/Test_ToolRoadRunner");
  UPackage *Package = assetData.GetPackage();
  Package->SetFolderName(TEXT("/Game/Carla/Maps"));
  Package->FullyLoad();
  Package->MarkPackageDirty();
  FAssetRegistryModule::AssetCreated(world);

  // Renaming stuff for the map
  world->Rename(TEXT("Test_ToolRoadRunner"), world->GetOuter());
  FAssetRegistryModule::AssetRenamed(world, TEXT("/Game/Carla/Maps/Test_ToolRoadRunner"));
  world->MarkPackageDirty();
  world->GetOuter()->MarkPackageDirty();

  auto ObjectLibraryForAssets = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  ObjectLibraryForAssets->AddToRoot();
  ObjectLibraryForAssets->LoadAssetDataFromPath(TEXT("/Game/Carla/Test_ToolRoadRunner"));
  ObjectLibraryForAssets->LoadAssetsFromAssetData();

  UClass * staticMeshClass = AStaticMeshActor::StaticClass();

  const FTransform zeroTransform = FTransform();
  FVector initialVector = FVector(0,0,0);
  FRotator initialRotator = FRotator(0,180,0);
  FActorSpawnParameters SpawnInfo;

  AStaticMeshActor * smActor;

  TArray<FAssetData> MapContents;
  ObjectLibraryForAssets->GetAssetDataList(MapContents);
  UStaticMesh* SMAsset_Cube;
  if (MapContents.Num() > 0)
  {
    FString assetName;
    for(auto MapAsset : MapContents) {
      assetName = MapAsset.GetFullName();
      FString meshPlace = assetName;
      SpawnInfo.Name = MapAsset.AssetName;
      SpawnInfo.ObjectFlags = RF_Standalone | RF_Public;
      UE_LOG(LogCommandletPlugin, Display, TEXT("DANIEL: %s"), *meshPlace);
      //static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshOb_torus(*meshPlace);
      SMAsset_Cube = CastChecked<UStaticMesh>(MapAsset.GetAsset());
      smActor = world->SpawnActor<AStaticMeshActor>(initialVector, initialRotator, SpawnInfo);
      smActor->GetStaticMeshComponent()->SetStaticMesh(SMAsset_Cube);
    }
  }

  // Filling the map stuff
  world->SpawnActor(AActor::StaticClass(), new FVector(), NULL);

  // Saving the package
  FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName,
      FPackageName::GetMapPackageExtension());
  bool bSaved = UPackage::SavePackage(Package, world, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
}

void UHelloWorldCommandlet::ImportFBX()
{
  /*const FName moduleName = "AssetTools";
  FModuleManager::LoadModuleChecked(moduleName);
  FAssetToolsModule &AssetToolsModule = FModuleManager::GetModuleChecked(moduleName);*/

}

int32 UHelloWorldCommandlet::Main(const FString &Params)
{
  CreateMapFromBaseMap();
  //ImportFBX();

  return 0;
}
