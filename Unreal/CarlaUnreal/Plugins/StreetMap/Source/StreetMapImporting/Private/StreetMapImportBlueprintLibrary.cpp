// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "StreetMapImportBlueprintLibrary.h"
#include "StreetMapFactory.h"
#include "StreetMap.h"
#include "StreetMapImporting.h"
#include "AssetToolsModule.h"
#include "Engine/AssetManager.h"

#if ENGINE_MAJOR_VERSION > 4
#include "AssetRegistry/IAssetRegistry.h"
#else
#include "AssetData.h"
#endif

UStreetMap* UStreetMapImportBlueprintLibrary::ImportStreetMap(FString Path, FString DestinationAssetPath, FVector2D OriginLatLon)
{
  UAssetManager& Manager = UAssetManager::Get();
	IAssetRegistry& AssetRegistry = Manager.GetAssetRegistry();
  TArray<FAssetData> AssetData;
  if( AssetRegistry.GetAssetsByPath(*DestinationAssetPath, AssetData, true, true) ) {
    if(AssetData.Num() > 0){
      return Cast<UStreetMap>(AssetData[0].GetAsset() );
    }
  }
  
  FString FileName = FPaths::GetCleanFilename(Path);
  FileName.RemoveFromEnd(".osm");
  FTopLevelAssetPath AssetClassPath = FTopLevelAssetPath(FName("/Script/StreetMapRuntime"), FName("StreetMap")); 
  if (AssetRegistry.GetAssetsByClass(AssetClassPath, AssetData, false))
  {
    for( auto Asset : AssetData ){
      UE_LOG(LogStreetMapImporting, Log, TEXT("FileName %s, AssetName %s ."), *FileName,*(Asset.AssetName.ToString() ) );
      if( FileName.Equals( Asset.AssetName.ToString() ) ){
         return Cast<UStreetMap>(Asset.GetAsset() );
      }
    }
  }



  TArray<FString> AssetPath;
  AssetPath.Add(Path);

  IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
  // Create a new street map factory
  UStreetMapFactory* Factory = NewObject<UStreetMapFactory>();
  Factory->LatLonOrigin = OriginLatLon;
  // Create a new Blueprint asset with the given name
  TArray<UObject*> NewAssets = AssetTools.ImportAssets(AssetPath, DestinationAssetPath, Factory, true );

  if(NewAssets.Num() != 0){
    return Cast<UStreetMap>(NewAssets[0]);
  }else{
    UE_LOG(LogStreetMapImporting, Error, TEXT("Error importing %s ."), *Path );

    return nullptr;
  }
}
