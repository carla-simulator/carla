// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "DisableNaniteOnInvalidMeshesCommandlet.h"

#include <util/ue-header-guard-begin.h>
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMesh.h"
#include "UObject/SavePackage.h"
#include "UObject/Package.h"
#include <util/ue-header-guard-end.h>

UDisableNaniteOnInvalidMeshesCommandlet::UDisableNaniteOnInvalidMeshesCommandlet()
{
  IsClient = false;
  IsEditor = true;
  IsServer = false;
  LogToConsole = true;
}

#if WITH_EDITORONLY_DATA

int32 UDisableNaniteOnInvalidMeshesCommandlet::Main(const FString &Params)
{
  UE_LOG(LogTemp, Display, TEXT("DisableNaniteOnInvalidMeshes: Scanning static meshes under /Game/Carla/Static/Vegetation/ ..."));

  FAssetRegistryModule &AssetRegistryModule =
      FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
  IAssetRegistry &AssetRegistry = AssetRegistryModule.Get();
  AssetRegistry.SearchAllAssets(true);

  TArray<FAssetData> AssetList;
  AssetRegistry.GetAssetsByPath(
      FName(TEXT("/Game/Carla/Static/Vegetation")),
      AssetList,
      true);

  int32 DisabledCount = 0;
  int32 ScannedCount = 0;

  for (const FAssetData &AssetData : AssetList)
  {
    if (AssetData.AssetClassPath != UStaticMesh::StaticClass()->GetClassPathName())
    {
      continue;
    }

    UStaticMesh *Mesh = Cast<UStaticMesh>(AssetData.GetAsset());
    if (!Mesh)
    {
      continue;
    }

    ++ScannedCount;

    if (!Mesh->IsNaniteEnabled())
    {
      continue;
    }

    Mesh->NaniteSettings.bEnabled = false;

    UPackage *Package = Mesh->GetOutermost();
    Package->MarkPackageDirty();

    const FString PackageFileName = FPackageName::LongPackageNameToFilename(
        Package->GetName(),
        FPackageName::GetAssetPackageExtension());

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags =
        EObjectFlags::RF_Public |
        EObjectFlags::RF_Standalone;
    SaveArgs.Error = GError;
    SaveArgs.SaveFlags = SAVE_NoError;

    if (UPackage::SavePackage(Package, Mesh, *PackageFileName, SaveArgs))
    {
      UE_LOG(LogTemp, Display,
          TEXT("DisableNaniteOnInvalidMeshes: Disabled Nanite on %s"),
          *Mesh->GetPathName());
      ++DisabledCount;
    }
    else
    {
      UE_LOG(LogTemp, Warning,
          TEXT("DisableNaniteOnInvalidMeshes: Failed to save %s"),
          *Mesh->GetPathName());
    }
  }

  UE_LOG(LogTemp, Display,
      TEXT("DisableNaniteOnInvalidMeshes: Scanned %d meshes, disabled Nanite on %d."),
      ScannedCount, DisabledCount);

  return 0;
}

#endif // WITH_EDITORONLY_DATA
