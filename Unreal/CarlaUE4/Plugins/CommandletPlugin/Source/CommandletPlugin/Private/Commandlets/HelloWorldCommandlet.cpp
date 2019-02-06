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


int32 UHelloWorldCommandlet::Main(const FString& Params)
{
  //Creating the Object Library that will manage the load
  auto ObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  ObjectLibrary->AddToRoot();
  ObjectLibrary->LoadAssetDataFromPath(TEXT("/Game/Carla/Maps/BaseMap"));
  ObjectLibrary->LoadAssetsFromAssetData();

  //Obtaining the asset data, aka the map
  TArray<FAssetData> AssetDatas;
  ObjectLibrary->GetAssetDataList(AssetDatas);
  FAssetData assetData;
  if(AssetDatas.Num()>0) assetData = AssetDatas.Pop();
  UWorld * world = CastChecked<UWorld>(assetData.GetAsset());

  //Getting the package and doing some rename stuff for the package
  FString PackageName = TEXT("/Game/Carla/ExportedMaps/TestMap01");
  UPackage* Package = assetData.GetPackage();
  Package->SetFolderName(TEXT("/Game/Carla/ExportedMaps"));
  Package->FullyLoad();
  Package->MarkPackageDirty();
  FAssetRegistryModule::AssetCreated(world);

  //Renaming stuff for the map
  world->Rename(TEXT("TestMap01"), world->GetOuter());
  FAssetRegistryModule::AssetRenamed(world, TEXT("/Game/Carla/ExportedMaps/TestMap01"));
  world->MarkPackageDirty();
  world->GetOuter()->MarkPackageDirty();

  //Filling the map stuff
  world->SpawnActor(AActor::StaticClass(), new FVector(), NULL);

  //Saving the package
  FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetMapPackageExtension());
  bool bSaved = UPackage::SavePackage(Package, world, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
  *PackageFileName, GError, nullptr, true, true, SAVE_NoError);

  return 0;
}
