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

TSet<UMaterialInterface*> UGenerateTaggedMaterialsRegistryCommandlet::FindMaskedMaterialInterfaces(const FName& PackageName, TSet<FName>& ScannedAssets) {
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
    FoundMaterialInterfaces.Append(FindMaskedMaterialInterfaces(Dependency, ScannedAssets));
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

TArray<FString> UGenerateTaggedMaterialsRegistryCommandlet::ParseAndSplitParamList(const TArray<FString>& Switches, const FString& SwitchKey) {
  // Implementation adapted from CookCommandlet.cpp#718
  TArray<FString> ValueElements;
  for (const FString& Switch: Switches) {
    if (Switch.StartsWith(SwitchKey + TEXT("=")) == true) {
      FString ValuesList = Switch.Right(Switch.Len() - (SwitchKey + TEXT("=")).Len());
      // Allow support for -KEY=Value1+Value2+Value3 as well as -KEY=Value1 -KEY=Value2
      for (int32 PlusIdx = ValuesList.Find(TEXT("+"), ESearchCase::CaseSensitive); PlusIdx != INDEX_NONE; PlusIdx = ValuesList.Find(TEXT("+"), ESearchCase::CaseSensitive))
      {
        const FString ValueElement = ValuesList.Left(PlusIdx);
        ValueElements.Add(ValueElement);

        ValuesList.RightInline(ValuesList.Len() - (PlusIdx + 1), false);
      }
      ValueElements.Add(ValuesList);
    }
  }
  return ValueElements;
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
  UE_LOG(LogCarla, Display, TEXT("Running UGenerateTaggedMaterialsRegistryCommandlet..."));

  // Parse the provided package names
  TArray<FString> Tokens;
  TArray<FString> Switches;
  ParseCommandLine(*Params, Tokens, Switches);
  TArray<FString> PackageNames = ParseAndSplitParamList(Switches, TEXT("PackageNames"));

  // Parse, whether a monolithic target-archive is requested
  FString TargetArchive;
  bool bIsMonolithic = FParse::Value(*Params, TEXT("TargetArchive="), TargetArchive);

  // If Carla package is requested, we store it. If not, we search for those assets anyway.
  // These assets are then already part of ScannedAssetsCarla, avoiding duplicates in the requested packages.
  FString PackageNameCarla = TEXT("Carla");
  TSet<FName> ScannedAssetsCarla;
  {
    UE_LOG(LogCarla, Display, TEXT("Scanning tag-injected materials for package: %s"), *PackageNameCarla);
    UTaggedMaterialsRegistry* TaggedMaterialsRegistryCarla = UTaggedMaterialsRegistry::Create(PackageNameCarla);
    TArray<FString> CarlaTopLevelPackages = GetPackagePaths(PackageNameCarla);
    for (const FString& TopLevelPackage : CarlaTopLevelPackages) {
      UE_LOG(LogCarla, Display, TEXT("Scanning %s"), *TopLevelPackage);
      TSet<UMaterialInterface*> MaterialInterfaces = FindMaskedMaterialInterfaces(*TopLevelPackage, ScannedAssetsCarla);
      for (UMaterialInterface* MaterialInterface : MaterialInterfaces) {
        // Calling GetTaggedMaterial creates and registers a new tag-injected version of the MaterialInterface
        TaggedMaterialsRegistryCarla->GetTaggedMaterial(MaterialInterface);
      }
    }
    if (PackageNames.Contains(PackageNameCarla)) {
      TaggedMaterialsRegistryCarla->Save();
      CreateMapPackage(PackageNameCarla, TaggedMaterialsRegistryCarla);
    }
  }

  // Always start with already scanned Carla assets to avoid duplicates in the other packages
  TSet<FName> ScannedAssetsPackage = ScannedAssetsCarla; // shallow copy
  UTaggedMaterialsRegistry* TaggedMaterialsRegistry = UTaggedMaterialsRegistry::Create(TargetArchive);
  for (FString PackageName : PackageNames) {
    if (PackageName == PackageNameCarla) {
      // Skip the Carla package, since it was explicitly handled before
      continue;
    }
    if (!bIsMonolithic) {
      // If no monolithic target-archive is requested, start from Carla assets for every package.
      // Otherwise ScannedAssetsPackage grows from package to package, avoiding further duplicates.
      ScannedAssetsPackage = ScannedAssetsCarla;
      TaggedMaterialsRegistry = UTaggedMaterialsRegistry::Create(PackageName);
    }

    // Recursively search for all masked material interfaces in the requested top-level packages and create tag-injected versions
    TArray<FString> RequestedTopLevelPackages = GetPackagePaths(PackageName);
    for (const FString& TopLevelPackage : RequestedTopLevelPackages) {
      UE_LOG(LogCarla, Display, TEXT("Creating tag-injected materials for package: %s"), *TopLevelPackage);

      TSet<UMaterialInterface*> MaterialInterfaces = FindMaskedMaterialInterfaces(*TopLevelPackage, ScannedAssetsPackage);
      for (UMaterialInterface* MaterialInterface : MaterialInterfaces) {
        // Calling GetTaggedMaterial creates and registers a new tag-injected version of the MaterialInterface
        TaggedMaterialsRegistry->GetTaggedMaterial(MaterialInterface);
      }
    }
    if (!bIsMonolithic && TaggedMaterialsRegistry->Num() > 0) {
      // Save asset and create empty map containing the TaggedMaterialsRegistry for cooking
      UE_LOG(LogCarla, Display, TEXT("Creating asset and map for package '%s' containing the TaggedMaterialsRegistry for cooking."), *PackageName);
      TaggedMaterialsRegistry->Save();
      CreateMapPackage(PackageName, TaggedMaterialsRegistry);
    }
  }
  if (bIsMonolithic) {
    // Save monolithic TaggedMaterialsRegistry for entire target-archive
    UE_LOG(LogCarla, Display, TEXT("Creating monolithic asset and map for target-archive '%s' containing the TaggedMaterialsRegistry for cooking."), *TargetArchive);
    TaggedMaterialsRegistry->Save();
    CreateMapPackage(TargetArchive, TaggedMaterialsRegistry);
  }

  UE_LOG(LogCarla, Display, TEXT("UGenerateTaggedMaterialsRegistryCommandlet finished."));

  return 0;
}
#endif // WITH_EDITOR
