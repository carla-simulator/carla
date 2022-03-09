// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// #if WITH_EDITOR

#include "MapGeneratorWidget.h"

#include "AssetRegistryModule.h"
#include "EditorLevelLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"


FString UMapGeneratorWidget::GenerateMapFiles(const FMapGeneratorMetaInfo& MetaInfo)
{

    FString ErrorMsg = "";

    // 1. Creating Levels
    CreateMainLargeMap(MetaInfo);
    CreateTilesMaps(MetaInfo);
    // bool bLoaded = LoadWorld(WorldAssetData);
    // GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, 
    //      bLoaded ? "Loaded CORRECT" : "NOT loaded");

    // 2. Applying heightmap
    // UWorld* World = CastChecked<UWorld>(WorldAssetData.GetAsset());
    // ALandscape* landscape = 
    //      (ALandscape*) UGameplayStatics::GetActorOfClass(World, ALandscape::StaticClass());
    // GEngine->AddOnScreenDebugMessage(
    //      -1, 15.0f, FColor::Green, landscape!=nullptr ? "L TRUE" : "L FALSE");
    // AssignLandscapeHeightMap(landscape);

    // 3. Saving World
    // bool bSaved = SaveWorld(WorldAssetData, MetaInfo.DestinationPath, MetaInfo.MapName);

    return ErrorMsg;
}

bool UMapGeneratorWidget::LoadBaseTileWorld(FAssetData& WorldAssetData)
{
    const FString BaseMapPath= TEXT("/CarlaTools/MapGenerator/BaseMap/Tiles");
    return LoadWorld(WorldAssetData, BaseMapPath);
}

bool UMapGeneratorWidget::LoadBaseLargeMapWorld(FAssetData& WorldAssetData)
{
    const FString BaseMapPath= TEXT("/CarlaTools/MapGenerator/BaseMap/MainLargeMap");
    return LoadWorld(WorldAssetData, BaseMapPath);
}

bool UMapGeneratorWidget::LoadWorld(FAssetData& WorldAssetData, const FString& BaseMapPath)
{
    TArray<FAssetData> AssetDatas;
    UObjectLibrary *MapObjectLibrary;

    // Loading Map from folder using object library
    MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
    MapObjectLibrary->AddToRoot();
    MapObjectLibrary->LoadAssetDataFromPath(*BaseMapPath);
    MapObjectLibrary->LoadAssetsFromAssetData();
    MapObjectLibrary->GetAssetDataList(AssetDatas);
    if (AssetDatas.Num() > 0)
    {
        // Extract first asset found in folder path (i.e. the BaseMap)
        WorldAssetData = AssetDatas.Pop();
        return true;
    }
    else
        return false;
}

bool UMapGeneratorWidget::SaveWorld(
    FAssetData& WorldToBeSaved, 
    const FString& DestinationPath, 
    const FString& WorldName
    )
{
    UWorld* World;
    UObjectRedirector *BaseMapRedirector = 
        Cast<UObjectRedirector>(WorldToBeSaved.GetAsset());
    if(BaseMapRedirector != nullptr)
        World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
    else
        World = CastChecked<UWorld>(WorldToBeSaved.GetAsset());

    // Create Package
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Preparing Package");
    UPackage *Package = WorldToBeSaved.GetPackage();
    Package->SetFolderName("MapGeneratorPackage");
    Package->FullyLoad();
    Package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(World);
    
    // Rename new World
    World->Rename(*WorldName, World->GetOuter());
    const FString PackagePath = DestinationPath + "/" + WorldName;
    FAssetRegistryModule::AssetRenamed(World, *PackagePath);
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, World->GetMapName());
    World->MarkPackageDirty();
    World->GetOuter()->MarkPackageDirty();

    // Saving Package
    const FString PackageFileName = FPackageName::LongPackageNameToFilename(
        PackagePath, 
        FPackageName::GetMapPackageExtension()
    );
    if(FPaths::FileExists(*PackageFileName))
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Package already Exists");
        return false;
    }
    return UPackage::SavePackage(
        Package, World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
        *PackageFileName, GError, nullptr, true, true, SAVE_NoError
    );
}

bool UMapGeneratorWidget::CreateMainLargeMap(const FMapGeneratorMetaInfo& MetaInfo)
{
    FAssetData WorldAssetData;
    bool bLoaded = LoadBaseLargeMapWorld(WorldAssetData);
    bool bSaved = SaveWorld(WorldAssetData, MetaInfo.DestinationPath, MetaInfo.MapName);

    return true;
}

bool UMapGeneratorWidget::CreateTilesMaps(const FMapGeneratorMetaInfo& MetaInfo)
{
    FAssetData WorldAssetData;

    for(int i = 0; i < MetaInfo.SizeX; i++)
    {
        for(int j = 0; j < MetaInfo.SizeY; j++)
        {
            bool bLoaded = LoadBaseTileWorld(WorldAssetData);

            FMapGeneratorTileMetaInfo MetaTileInfo;
            MetaTileInfo.IndexX = i;
            MetaTileInfo.IndexY = j;
            ApplyHeightMapToLandscape(WorldAssetData,MetaTileInfo);

            const FString MapName = 
                MetaInfo.MapName + "_Tile_" + FString::FromInt(i) + "_" + FString::FromInt(j);
            bool bSaved = SaveWorld(WorldAssetData, MetaInfo.DestinationPath, MapName);
        }
    }
    return true;
}

bool UMapGeneratorWidget::ApplyHeightMapToLandscape(
    FAssetData& WorldAssetData, 
    FMapGeneratorTileMetaInfo tileMetaInfo
    )
{
    UWorld* World;
    UObjectRedirector* BaseMapRedirector = 
        Cast<UObjectRedirector>(WorldAssetData.GetAsset());
    if(BaseMapRedirector != nullptr)
    {
        World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
    }
    else
    {
        World = CastChecked<UWorld>(WorldAssetData.GetAsset());
    }
    ALandscape* landscape = (ALandscape*) UGameplayStatics::GetActorOfClass(
        World, 
        ALandscape::StaticClass()
    );
    AssignLandscapeHeightMap(landscape, tileMetaInfo);
    return true;
}


// #endif