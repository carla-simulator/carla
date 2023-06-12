// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "StreetMapImportBlueprintLibrary.h"
#include "StreetMapFactory.h"
#include "StreetMap.h"

UStreetMap* UStreetMapImportBlueprintLibrary::ImportStreetMap(FString Path, FString DestinationAssetPath, FVector2D OriginLatLon)
{
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
    return nullptr;
  }
}