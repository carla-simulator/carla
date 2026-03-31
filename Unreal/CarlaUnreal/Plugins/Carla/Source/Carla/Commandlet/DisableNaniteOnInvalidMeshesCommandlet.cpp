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
  FAssetRegistryModule &AssetRegistryModule =
      FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
  IAssetRegistry &AssetRegistry = AssetRegistryModule.Get();
  AssetRegistry.SearchAllAssets(true);

  TArray<FAssetData> AssetList;
  AssetRegistry.GetAssetsByPath(
      FName(TEXT("/Game/Carla/Static/Vegetation")),
      AssetList,
      true);

  UE_LOG(LogTemp, Display,
      TEXT("DisableNaniteOnInvalidMeshes: Scanning /Game/Carla/Static/Vegetation/ ..."));

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

    // Disable Nanite and configure full fallback to traditional LOD
    Mesh->NaniteSettings.bEnabled = false;
    Mesh->NaniteSettings.FallbackPercentTriangles = 1.0f;
    Mesh->NaniteSettings.FallbackRelativeError = 0.0f;

    // Trigger Derived Data regeneration to strip Nanite bulk data.
    // Without this, the NaniteBuilder still processes stale Nanite
    // resources during cooking even though bEnabled is false.
    Mesh->PostEditChange();

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
