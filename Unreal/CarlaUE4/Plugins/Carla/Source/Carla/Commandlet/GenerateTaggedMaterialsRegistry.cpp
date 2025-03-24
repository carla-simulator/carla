#include "GenerateTaggedMaterialsRegistry.h"
#include "PrepareAssetsForCookingCommandlet.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Materials/MaterialInstanceConstant.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#include "Settings/ProjectPackagingSettings.h"
#endif // WITH_EDITOR

UGenerateTaggedMaterialsRegistryCommandlet::UGenerateTaggedMaterialsRegistryCommandlet() {
  // Set necessary flags to run commandlet
  IsClient = false;
  IsEditor = true;
  IsServer = false;
  LogToConsole = true;
}

#if WITH_EDITOR
IAssetRegistry& UGenerateTaggedMaterialsRegistryCommandlet::GetAssetRegistry() const
{
  if (!CachedAssetRegistry)
  {
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    CachedAssetRegistry = &AssetRegistryModule.Get();
  }
  return *CachedAssetRegistry;
}

TSet<UMaterialInterface*> UGenerateTaggedMaterialsRegistryCommandlet::FindMaskedMaterialInterfaces(const FName& PackageName) {
  if (ScannedAssets.Contains(PackageName)) {
    return {};
  } else {
    ScannedAssets.Add(PackageName);
  }

  IAssetRegistry& AssetRegistry = GetAssetRegistry();
  TSet<UMaterialInterface*> FoundMaterialInterfaces;

  // Check the assets in this package, whether they are UMaterials or UMaterialInstanceConstants and add those
  TArray<FAssetData> AssetsData;
  AssetRegistry.GetAssetsByPackageName(PackageName, AssetsData);
  for (const FAssetData& AssetData : AssetsData) {
    if (AssetData.AssetClass == UMaterial::StaticClass()->GetFName() ||
        AssetData.AssetClass == UMaterialInstanceConstant::StaticClass()->GetFName()) {
      UMaterialInterface* MaterialInterface = CastChecked<UMaterialInterface>(AssetData.GetAsset());
      if (MaterialInterface->IsMasked()) {
        FoundMaterialInterfaces.Add(MaterialInterface);
      }
    }
  }

  // Check the dependencies of this package and recursively add all referenced UMaterials and UMaterialInstanceConstants
  TArray<FName> Dependencies;
  AssetRegistry.GetDependencies(PackageName, Dependencies);
  for (const FName& Dependency : Dependencies) {
    FoundMaterialInterfaces.Append(FindMaskedMaterialInterfaces(Dependency));
  }

  return FoundMaterialInterfaces;
}

void UGenerateTaggedMaterialsRegistryCommandlet::CreateMapPackage(const FString& PackageName, UTaggedMaterialsRegistry* TaggedMaterialsRegistry) {
  // Create new package for the map
  FString WorldPackageName = TEXT("TaggedMaterials_") + PackageName + TEXT("_Map");
  FString PackagePath = UTaggedMaterialsRegistry::TaggedMaterialsRootDir / WorldPackageName;
  UPackage* Package = CreatePackage(*PackagePath);

  // Create a duplicate of the BaseMap, that we will modify
  FSoftObjectPath BaseMapPath = FSoftObjectPath("/Game/Carla/Maps/BaseMap/BaseMap.BaseMap");
  UObject* BaseMapObject = BaseMapPath.TryLoad();
  UWorld* World = DuplicateObject<UWorld>(CastChecked<UWorld>(BaseMapObject), Package);
  World->Rename(*WorldPackageName);
  FAssetRegistryModule::AssetCreated(World);

  // Add the TaggedMaterialsRegistry to the map
  ATaggedMaterialsRegistryActor* RegistryActor = World->SpawnActor<ATaggedMaterialsRegistryActor>(ATaggedMaterialsRegistryActor::StaticClass(), FTransform());
  RegistryActor->TaggedMaterialsRegistry = TaggedMaterialsRegistry;
  World->MarkPackageDirty();

  // Save the new package
  const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetMapPackageExtension());
  UPackage::SavePackage(Package, World, RF_Public | RF_Standalone, *PackageFileName);
}

TArray<FString> UGenerateTaggedMaterialsRegistryCommandlet::GetPackagePaths(const FString& PackageName) {
  TArray<FString> TopLevelPackages;

  if (PackageName == TEXT("Carla")) {
    // Find maps for default Carla package
    UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();
    PackagingSettings->LoadConfig();
    for (const FFilePath& MapFilePath : PackagingSettings->MapsToCook) {
      TopLevelPackages.Add(MapFilePath.FilePath);
    }

    // The default Carla package should also contain the spawnable actors
    TopLevelPackages.Add(TEXT("/Game/Carla/Blueprints/Vehicles/VehicleFactory"));
    TopLevelPackages.Add(TEXT("/Game/Carla/Blueprints/Walkers/WalkerFactory"));
    TopLevelPackages.Add(TEXT("/Game/Carla/Blueprints/Props/PropFactory"));
  } else {
    FAssetsPaths AssetsPaths = UPrepareAssetsForCookingCommandlet::GetAssetsPathFromPackage(PackageName);
    for (FMapData MapPath : AssetsPaths.MapsPaths) {
      TopLevelPackages.Add(MapPath.Path + TEXT("/") + MapPath.Name);
    }
  }

  return TopLevelPackages;
}

int32 UGenerateTaggedMaterialsRegistryCommandlet::Main(const FString& Params)
{
  UE_LOG(LogCarla, Log, TEXT("Running UGenerateTaggedMaterialsRegistryCommandlet..."));

  FString PackageName;
  FParse::Value(*Params, TEXT("PackageName="), PackageName);

  // 1. Collect all relevant top-level packages
  TArray<FString> RequestedTopLevelPackages = GetPackagePaths(PackageName);

  // 2. If non-default Carla package is requested, we search for those assets anyway.
  //    These assets are then already part of ScannedAssets, avoiding duplicates in the requested package.
  if (PackageName != TEXT("Carla")) {
    UE_LOG(LogCarla, Log, TEXT("Inspecting default Carla packages to avoid duplicates."));
    TArray<FString> CarlaTopLevelPackages = GetPackagePaths(TEXT("Carla"));
    for (const FString& TopLevelPackage : CarlaTopLevelPackages) {
      FindMaskedMaterialInterfaces(*TopLevelPackage);
    }
  }

  // 3. Recursively search for all masked material interfaces in the requested top-level packages and create tag-injected versions
  UTaggedMaterialsRegistry* TaggedMaterialsRegistry = UTaggedMaterialsRegistry::Create(PackageName);
  for (const FString& TopLevelPackage : RequestedTopLevelPackages) {
    UE_LOG(LogCarla, Log, TEXT("Creating tag-injected materials for package: %s"), *TopLevelPackage);

    TSet<UMaterialInterface*> MaterialInterfaces = FindMaskedMaterialInterfaces(*TopLevelPackage);
    for (UMaterialInterface* MaterialInterface : MaterialInterfaces) {
      // Calling GetTaggedMaterial creates and registers a new tag-injected version of the MaterialInterface
      TaggedMaterialsRegistry->GetTaggedMaterial(MaterialInterface);
    }
  }
  if (TaggedMaterialsRegistry->Num() > 0) {
    TaggedMaterialsRegistry->Save();
  }

  // 4. Create empty map containing the TaggedMaterialsRegistry for cooking
  UE_LOG(LogCarla, Log, TEXT("Creating empty map containing the TaggedMaterialsRegistry for cooking."));
  CreateMapPackage(PackageName, TaggedMaterialsRegistry);

  UE_LOG(LogCarla, Log, TEXT("UGenerateTaggedMaterialsRegistryCommandlet finished."));

  return 0;
}
#endif // WITH_EDITOR
