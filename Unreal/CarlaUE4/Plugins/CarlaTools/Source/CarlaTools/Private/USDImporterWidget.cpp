// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "USDImporterWidget.h"
#include "OmniverseUSDImporter.h"


void UUSDImporterWidget::ImportUSDProp(
    const FString& USDPath, const FString& DestinationAssetPath)
{
  FOmniverseImportSettings Settings;
  Settings.bImportUnusedReferences = false;
  Settings.bImportAsBlueprint = true;
  FOmniverseUSDImporter::LoadUSD(USDPath, DestinationAssetPath, Settings);
}

void UUSDImporterWidget::ImportUSDVehicle(
    const FString& USDPath, const FString& DestinationAssetPath)
{

}

