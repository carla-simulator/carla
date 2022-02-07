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

#define DEBUG_MSG(x, ...) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT(x), __VA_ARGS__));}
#define DEBUG_MSG_RED(x, ...) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT(x), __VA_ARGS__));}



FString UMapGeneratorWidget::GenerateMapFiles(const FMapGeneratorMetaInfo& metaInfo)
{
    DEBUG_MSG("Generating new map...");

    FString errorMsg = "";

    // 1. Creating Levels
    CreateMainLargeMap(metaInfo);
    CreateTilesMaps(metaInfo);
    // bool bLoaded = LoadWorld(worldAssetData);
    // GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, bLoaded ? "Loaded CORRECT" : "NOT loaded");

    // 2. Applying heightmap
    // UWorld* world = CastChecked<UWorld>(worldAssetData.GetAsset());
    // ALandscape* landscape = (ALandscape*) UGameplayStatics::GetActorOfClass(world, ALandscape::StaticClass());
    // GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, landscape!=nullptr ? "L TRUE" : "L FALSE");
    // AssignLandscapeHeightMap(landscape);

    // 3. Saving world
    // bool bSaved = SaveWorld(worldAssetData, metaInfo.destinationPath, metaInfo.mapName);
    // GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, bSaved ? "Saved CORRECT" : "NOT saved");

    return errorMsg;
}

bool UMapGeneratorWidget::LoadBaseTileWorld(FAssetData& WorldAssetData)
{
    const FString baseMapPath= TEXT("/CarlaTools/MapGenerator/BaseMap/Tiles");
    return LoadWorld(WorldAssetData, baseMapPath);
}

bool UMapGeneratorWidget::LoadBaseLargeMapWorld(FAssetData& WorldAssetData)
{
    const FString baseMapPath= TEXT("/CarlaTools/MapGenerator/BaseMap/MainLargeMap");
    return LoadWorld(WorldAssetData, baseMapPath);
}

bool UMapGeneratorWidget::LoadWorld(FAssetData& WorldAssetData, const FString& baseMapPath)
{
    TArray<FAssetData> AssetDatas;
    UObjectLibrary *MapObjectLibrary;

    // Loading Map from folder using object library
    MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
    MapObjectLibrary->AddToRoot();
    MapObjectLibrary->LoadAssetDataFromPath(*baseMapPath);
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

bool UMapGeneratorWidget::SaveWorld(FAssetData& WorldToBeSaved, const FString& DestinationPath, const FString& WorldName)
{
    UWorld* world;
    UObjectRedirector *BaseMapRedirector = Cast<UObjectRedirector>(WorldToBeSaved.GetAsset());
    if(BaseMapRedirector != nullptr)
        world = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
    else
        world = CastChecked<UWorld>(WorldToBeSaved.GetAsset());

    // Create package
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Preparing package");
    UPackage *package = WorldToBeSaved.GetPackage();
    package->SetFolderName("TestFolder001");
    package->FullyLoad();
    package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(world);
    
    // Rename new World
    world->Rename(*WorldName, world->GetOuter());
    const FString PackagePath = DestinationPath + "/" + WorldName;
    FAssetRegistryModule::AssetRenamed(world, *PackagePath);
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, world->GetMapName());
    world->MarkPackageDirty();
    world->GetOuter()->MarkPackageDirty();

    // Saving package
    const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetMapPackageExtension());
    if(FPaths::FileExists(*PackageFileName))
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Package already Exists");
        return false;
    }
    return UPackage::SavePackage(package, world, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
        *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
}

bool UMapGeneratorWidget::CreateMainLargeMap(const FMapGeneratorMetaInfo& metaInfo)
{
    FAssetData worldAssetData;
    bool bLoaded = LoadBaseLargeMapWorld(worldAssetData);
    bool bSaved = SaveWorld(worldAssetData, metaInfo.destinationPath, metaInfo.mapName);

    return true;
}

bool UMapGeneratorWidget::CreateTilesMaps(const FMapGeneratorMetaInfo& metaInfo)
{
    FAssetData worldAssetData;
    
    int numberOfTiles = metaInfo.sizeX * metaInfo.sizeY;

    for(int i = 0; i < numberOfTiles; i++)
    {
        bool bLoaded = LoadBaseTileWorld(worldAssetData);

        ApplyHeightMapToLandscape(worldAssetData);

        const FString mapName = metaInfo.mapName + "_" + FString::FromInt(i);
        bool bSaved = SaveWorld(worldAssetData, metaInfo.destinationPath, mapName);
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, bSaved ? "Saved CORRECT" : "NOT saved");
    }
    return true;
}

bool UMapGeneratorWidget::ApplyHeightMapToLandscape(FAssetData& worldAssetData)
{
    UWorld* world;
    UObjectRedirector* BaseMapRedirector = Cast<UObjectRedirector>(worldAssetData.GetAsset());
    if(BaseMapRedirector != nullptr)
    {
        world = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
    }
    else
    {
        world = CastChecked<UWorld>(worldAssetData.GetAsset());
    }
    ALandscape* landscape = (ALandscape*) UGameplayStatics::GetActorOfClass(world, ALandscape::StaticClass());
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, landscape!=nullptr ? "L TRUE" : "L FALSE");
    AssignLandscapeHeightMap(landscape);
    return true;
}


// #endif