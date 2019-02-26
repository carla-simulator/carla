// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.


#include "MapProcessCommandlet.h"
#include "GameFramework/WorldSettings.h"
#include "UObject/MetaData.h"
//#include "CommandletPluginPrivate.h"

UMapProcessCommandlet::UMapProcessCommandlet()
{
  IsClient = false;
  IsEditor = false;
  IsServer = false;
  LogToConsole = true;
  #if WITH_EDITORONLY_DATA
  static ConstructorHelpers::FObjectFinder<UMaterial> MarkingNode(TEXT(
      "Material'/Game/Carla/Static/GenericMaterials/LaneMarking/M_MarkingLane_W.M_MarkingLane_W'"));
  static ConstructorHelpers::FObjectFinder<UMaterial> RoadNode(TEXT(
      "Material'/Game/Carla/Static/GenericMaterials/Masters/LowComplexity/M_Road1.M_Road1'"));
  static ConstructorHelpers::FObjectFinder<UMaterial> RoadNodeAux(TEXT(
      "Material'/Game/Carla/Static/GenericMaterials/LaneMarking/M_MarkingLane_Y.M_MarkingLane_Y'"));
  static ConstructorHelpers::FObjectFinder<UMaterial> TerrainNodeMaterial(TEXT(
      "Material'/Game/Carla/Static/GenericMaterials/Grass/M_Grass01.M_Grass01'"));

  MarkingNodeMaterial = (UMaterial *) MarkingNode.Object;
  RoadNodeMaterial = (UMaterial *) RoadNode.Object;
  MarkingNodeMaterialAux = (UMaterial *) RoadNodeAux.Object;
  #endif
}
#if WITH_EDITORONLY_DATA

bool UMapProcessCommandlet::ParseParams(const FString& InParams)
{
	TArray<FString> Tokens;
	TArray<FString> Params;
	TMap<FString, FString> ParamVals;

	ParseCommandLine(*InParams, Tokens, Params, ParamVals);

	const bool bEnoughParams = ParamVals.Num() > 1;

	MapName = ParamVals.FindRef(TEXT("mapname"));

  bOverrideMaterials = Params.Contains(TEXT("use-carla-materials"));

	return bEnoughParams;
}

void UMapProcessCommandlet::MoveMeshes(const FString &SrcPath, const TArray<FString> &DestPath)
{

  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();
  AssetsObjectLibrary->LoadAssetDataFromPath(*SrcPath);
  AssetsObjectLibrary->LoadAssetsFromAssetData();

  MapContents.Empty();
  AssetsObjectLibrary->GetAssetDataList(MapContents);

  UStaticMesh *MeshAsset;
  FString PackagePath;
  FString ObjectName;

  if (MapContents.Num() > 0)
  {
    int ChosenIndex = 0;
    FString AssetName;
    FAssetToolsModule &AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    for (auto MapAsset : MapContents)
    {
      MeshAsset = CastChecked<UStaticMesh>(MapAsset.GetAsset());

      ObjectName = MeshAsset->GetName();

      MapAsset.AssetName.ToString(AssetName);


      if (SrcPath.Len())
      {
        const FString CurrentPackageName = MeshAsset->GetOutermost()->GetName();
        //UE_LOG(LogCommandletPlugin, Display, TEXT("DANIEL: %s"), *CurrentPackageName);

        // This is a relative operation
        if (!ensure(CurrentPackageName.StartsWith(SrcPath)))
        {
          continue;
        }

        if (AssetName.Contains("MarkingNode"))
        {
          ChosenIndex=MARKINGLINE_INDEX;
        }
        else if (AssetName.Contains("RoadNode"))
        {
          ChosenIndex=ROAD_INDEX;
        } else if (AssetName.Contains("Terrain")) {
          ChosenIndex=TERRAIN_INDEX;
        }

        const int32 ShortPackageNameLen = FPackageName::GetLongPackageAssetName(CurrentPackageName).Len();
        const int32 RelativePathLen = CurrentPackageName.Len() - ShortPackageNameLen - SrcPath.Len() - 1;   //
                                                                                                            // -1
                                                                                                            // to
                                                                                                            // exclude
                                                                                                            // the
                                                                                                            // trailing
                                                                                                            // "/"
        const FString RelativeDestPath = CurrentPackageName.Mid(SrcPath.Len(), RelativePathLen);

        PackagePath = DestPath[ChosenIndex] + RelativeDestPath;
      }
      else
      {
        // Only a DestPath was supplied, use it
        PackagePath = DestPath[ChosenIndex];
      }


      MeshAsset->AddToRoot();
      FString NewPackageName = PackagePath + "/" + ObjectName;

      UPackage *AssetPackage = MapAsset.GetPackage();
      AssetPackage->SetFolderName(*PackagePath);
      AssetPackage->FullyLoad();
      AssetPackage->MarkPackageDirty();
      FAssetRegistryModule::AssetCreated(MeshAsset);

      MeshAsset->MarkPackageDirty();
      MeshAsset->GetOuter()->MarkPackageDirty();

      FString CompleteFilename = FPackageName::LongPackageNameToFilename(NewPackageName,
          FPackageName::GetAssetPackageExtension());
      UPackage::SavePackage(AssetPackage, MeshAsset, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
          *CompleteFilename, GError, nullptr, true, true, SAVE_NoError);
    }

  }
}

void UMapProcessCommandlet::LoadWorld(const FString &SrcPath, FAssetData &AssetData)
{

  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  MapObjectLibrary->AddToRoot();
  MapObjectLibrary->LoadAssetDataFromPath(*SrcPath);
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetDatas);

  if (AssetDatas.Num() > 0)
  {
    AssetData = AssetDatas.Pop();
  }
}

void UMapProcessCommandlet::AddMeshesToWorld(
    const TArray<FString> &SrcPath,
    bool bMaterialWorkaround)
{

  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();
  AssetsObjectLibrary->LoadAssetDataFromPaths(SrcPath);
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

    if (bMaterialWorkaround)
    {
      FString AssetName;
      MapAsset.AssetName.ToString(AssetName);
      if (AssetName.Contains("MarkingNode"))
      {
        MeshActor->GetStaticMeshComponent()->SetMaterial(0, MarkingNodeMaterial);
        MeshActor->GetStaticMeshComponent()->SetMaterial(1, MarkingNodeMaterialAux);
      }
      else if (AssetName.Contains("RoadNode"))
      {
        MeshActor->GetStaticMeshComponent()->SetMaterial(0, RoadNodeMaterial);
      } else if (AssetName.Contains("Terrain"))
      {
        MeshActor->GetStaticMeshComponent()->SetMaterial(0, TerrainNodeMaterial);
      }

    }
  }
  World->MarkPackageDirty();

}

bool UMapProcessCommandlet::SaveWorld(FAssetData &AssetData, FString &DestPath, FString &WorldName)
{
  FString PackageName = DestPath + "/" + WorldName;
  UPackage *Package = AssetData.GetPackage();
  Package->SetFolderName(*DestPath);
  Package->FullyLoad();
  Package->MarkPackageDirty();
  FAssetRegistryModule::AssetCreated(World);

  // Renaming stuff for the map
  World->Rename(*WorldName, World->GetOuter());
  FAssetRegistryModule::AssetRenamed(World, *PackageName);
  World->MarkPackageDirty();
  World->GetOuter()->MarkPackageDirty();

  // Filling the map stuff
  AOpenDriveActor *OpenWorldActor =
      CastChecked<AOpenDriveActor>(World->SpawnActor(AOpenDriveActor::StaticClass(), new FVector(), NULL));
  OpenWorldActor->BuildRoutes(WorldName);
  OpenWorldActor->AddSpawners();

  // Saving the package
  FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName,
      FPackageName::GetMapPackageExtension());
  return UPackage::SavePackage(Package, World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
}

int32 UMapProcessCommandlet::Main(const FString &Params)
{
  ParseParams(Params);
  FString SrcPath = TEXT("/Game/Carla/Static/Imported/") + MapName;
  FString BaseMap = TEXT("/Game/Carla/Maps/BaseMap");
  FString WorldDestPath = TEXT("/Game/Carla/ExportedMaps");

  FString RoadsPath = TEXT("/Game/Carla/Static/Road/") + MapName;
  FString MarkingLinePath = TEXT("/Game/Carla/Static/RoadLines/") + MapName;
  FString TerrainPath = TEXT("/Game/Carla/Static/Terrain/") + MapName;



  TArray<FString> DataPath;
  DataPath.Add(RoadsPath);
  DataPath.Add(MarkingLinePath);
  DataPath.Add(TerrainPath);

  FAssetData AssetData;
  MoveMeshes(SrcPath, DataPath);
  LoadWorld(*BaseMap, AssetData);
  World = CastChecked<UWorld>(AssetData.GetAsset());
  AddMeshesToWorld(DataPath, bOverrideMaterials);
  SaveWorld(AssetData, WorldDestPath, MapName);

  return 0;
}

#endif